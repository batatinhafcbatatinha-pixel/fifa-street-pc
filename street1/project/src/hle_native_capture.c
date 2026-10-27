/* XDK debug screenshot/movie-cache protocol around Swap. */
#include <stdint.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "hle_d3d11_internal.h"

extern uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args);
extern void hle_native_wait_idle(void);
extern uint32_t hle_resource_layout(uint32_t, uint32_t, uint32_t, uint32_t,
    uint32_t, uint32_t, int, int, uint32_t *, uint32_t *);

#define CPTR(va) ((void *)XBOX_PTR(va))
#define CDEV MEM32(0x261238u)
#define CG(va) MEM32(va)
static uint32_t s_width, s_height, s_pitch, s_format;

static uint32_t capture_alloc(uint32_t bytes, uint32_t flags)
{
    uint32_t args[2] = { bytes, flags };
    return hle_guest_call(0x1609A7u, 2, args);
}

static void capture_free(uint32_t ptr, uint32_t flags)
{
    uint32_t args[2] = { ptr, flags };
    hle_guest_call(0x160A47u, 2, args);
}

static uint64_t counter(void)
{
    LARGE_INTEGER t; QueryPerformanceCounter(&t); return (uint64_t)t.QuadPart;
}

static void write64(uint32_t p, uint64_t v)
{
    MEM32(p) = (uint32_t)v; MEM32(p + 4) = (uint32_t)(v >> 32);
}

static uint64_t read64(uint32_t p)
{
    return MEM32(p) | (uint64_t)MEM32(p + 4) << 32;
}

void hle_capture_before_swap(void)
{
    if ((MEM32(CDEV + 0x8D8) & 0x10) && (MEM32(0x263A2Cu) & 0x70) && MEM32(0x263A28u) == 0xFFFFFFFFu) {
        static const uint32_t interval[8] = {0, 1, 2, 4, 0x80000000u, 0x80000001u, 0x80000002u, 0x80000004u};
        MEM32(0x263A28u) = MEM32(0x26119Cu);
        MEM32(0x26119Cu) = interval[(MEM32(0x263A2Cu) >> 4) & 7u];
    }
}

static uint32_t movie_open(int replay)
{
    uint32_t scratch = capture_alloc(40, 0x24800000u);
    if (!scratch) return 0x8007000Eu;
    memset(CPTR(scratch), 0, 40);
    MEM32(scratch + 4) = 0x2DBDE0u;
    MEM32(scratch + 8) = 0x40;
    MEM32(0x263A24u) = 0xFFFFFFFFu;
    uint32_t args[9] = { 0x263A24u, replay ? 0x80110000u : 0x40100100u,
        scratch, 0x263A10u, replay ? 0 : scratch + 24, 0, 0, replay ? 1 : 0, replay ? 0xC : 0x48 };
    if (!replay) {
        uint32_t disk_args[4] = { 0x260174u, scratch + 24, 0, 0 };
        if (!hle_guest_call(0x15D7F0u, 4, disk_args)) {
            capture_free(scratch, 0x24800000u);
            return 0xC000007Fu;
        }
        uint64_t available = read64(scratch + 24);
        if (available > 0x40000000u) write64(scratch + 24, 0x40000000u);
    }
    uint32_t status = hle_guest_call(0x800000BEu, 9, args);
    capture_free(scratch, 0x24800000u);
    MEM32(0x263A18u) = 0; MEM32(0x263A1Cu) = 0;
    return status;
}

static void capture_acknowledge(uint32_t dev)
{
    if (MEM32(dev + 0x8D4)) return;
    MEM32(dev + 0x8D4) = 0x26000000u;
    for (uint32_t n = 0; MEM32(dev + 0x8D4) == 0x26000000u; n++) {
        Sleep(1); guest_cpu_yield();
        if (n > 9999) {
            MEM32(0x263A20u)--;
            MEM32(dev + 0x8D4) = 0;
            MEM32(dev + 0x8D8) &= ~0x10u;
        }
    }
}

uint32_t hle_native_surface_readback(uint32_t source, uint32_t out, uint32_t width,
    uint32_t height, uint32_t pitch, uint32_t format)
{
    uint32_t descriptor = capture_alloc(24, 0x24800000u);
    if (!descriptor) return E_OUTOFMEMORY;
    memcpy(CPTR(descriptor), CPTR(source), 24);
    if (source == MEM32(CDEV + 0x1A18) && g_hle.last_present) MEM32(descriptor + 4) = g_hle.last_present;
    hle_native_wait_idle();
    EnterCriticalSection(&g_hle.lock);
    HleSurface *s = hle_surface_for(descriptor);
    HRESULT hr = E_FAIL;
    ID3D11Texture2D *staging = NULL;
    ID3D11Texture2D *tex = hle_surface_resolved(s);
    if (tex) {
        D3D11_TEXTURE2D_DESC td;
        ID3D11Texture2D_GetDesc(tex, &td);
        td.Usage = D3D11_USAGE_STAGING; td.BindFlags = 0; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ; td.MiscFlags = 0;
        hr = ID3D11Device_CreateTexture2D(g_hle.dev, &td, NULL, &staging);
        if (SUCCEEDED(hr)) {
            D3D11_MAPPED_SUBRESOURCE map;
            ID3D11DeviceContext_CopyResource(g_hle.ctx, (ID3D11Resource *)staging, (ID3D11Resource *)tex);
            hr = ID3D11DeviceContext_Map(g_hle.ctx, (ID3D11Resource *)staging, 0, D3D11_MAP_READ, 0, &map);
            if (SUCCEEDED(hr)) {
                for (uint32_t y = 0; y < height; y++) {
                    uint32_t sy = (uint32_t)((uint64_t)(2 * y + 1) * s->th / (2 * height));
                    const uint8_t *row = (const uint8_t *)map.pData + sy * map.RowPitch;
                    for (uint32_t x = 0; x < width; x++) {
                        uint32_t sx = (uint32_t)hle_map_x(s, ((float)x + 0.5f) * s->w / width);
                        if (sx >= s->tw) sx = s->tw - 1;
                        const uint8_t *pixel = row + sx * 4;
                        uint32_t dst = out + y * pitch;
                        if (format == 0x1C || format == 3 || format == 0x10 || format == 2)
                            MEM16(dst + x * 2) = (uint16_t)((pixel[2] >> 3) << 10 | (pixel[1] >> 3) << 5 | pixel[0] >> 3 |
                                ((format == 0x10 || format == 2) ? (uint32_t)(pixel[3] >> 7) << 15 : 0x8000u));
                        else if (format == 0x11 || format == 5)
                            MEM16(dst + x * 2) = (uint16_t)((pixel[2] >> 3) << 11 | (pixel[1] >> 2) << 5 | pixel[0] >> 3);
                        else MEM32(dst + x * 4) = (uint32_t)pixel[0] | (uint32_t)pixel[1] << 8 | (uint32_t)pixel[2] << 16 | (uint32_t)pixel[3] << 24;
                    }
                }
                ID3D11DeviceContext_Unmap(g_hle.ctx, (ID3D11Resource *)staging, 0);
            }
            ID3D11Texture2D_Release(staging);
        }
    }
    LeaveCriticalSection(&g_hle.lock);
    capture_free(descriptor, 0x24800000u);
    return hr;
}

static void capture_frame(uint32_t dev)
{
    uint32_t flags = MEM32(dev + 0x8D8), buffer = MEM32(dev + 0x8DC);
    if (!buffer) return;
    uint32_t bytes = (MEM32(0x263A30u) + 511u) & ~511u;
    uint32_t args[8] = { MEM32(0x263A24u), 0, 0, 0, 0x263A10u, buffer, bytes, 0x263A18u };
    if (flags & 0x20) {
        uint32_t status = hle_guest_call(0x800000DBu, 8, args);
        if ((int32_t)status < 0) MEM32(buffer + 0x18) = 0;
        else {
            uint32_t wait_args[2] = { args[0], 1000 };
            hle_guest_call(0x15B2CAu, 2, wait_args);
            write64(0x263A18u, read64(0x263A18u) + bytes);
            MEM32(0x263A20u)--; MEM32(buffer + 0x18) = MEM32(0x263A20u);
        }
        capture_acknowledge(dev);
        return;
    }
    if (!(flags & 0x10)) return;
    uint64_t now = counter(), previous = read64(0x263A08u);
    if (previous && now - previous < read64(0x263A00u)) return;
    write64(0x263A08u, now);
    if (MEM8(0x263A2Cu) & 0x80) {
        uint32_t wait_args[2] = { MEM32(0x263A24u), 1000 };
        hle_guest_call(0x15B2CAu, 2, wait_args);
    }
    HRESULT hr = (HRESULT)hle_native_surface_readback(MEM32(dev + 0x1A18), buffer + 64,
        s_width, s_height, s_pitch, s_format);
    if (FAILED(hr)) { MEM32(dev + 0x8D4) = 0x25000003u; return; }
    LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency);
    write64(buffer, counter()); write64(buffer + 8, (uint64_t)frequency.QuadPart);
    MEM32(buffer + 0x18) = MEM32(0x263A20u);
    if (MEM8(0x263A2Cu) & 0x80) {
        uint32_t status = hle_guest_call(0x800000ECu, 8, args);
        if ((int32_t)status >= 0) { MEM32(0x263A20u)++; write64(0x263A18u, read64(0x263A18u) + bytes); }
    } else { capture_acknowledge(dev); MEM32(0x263A20u)++; }
}

void hle_capture_after_swap(void)
{
    uint32_t dev = CDEV, state = MEM32(dev + 0x8D4) & 0xFF000000u;
    if (MEM32(0x263A28u) != 0xFFFFFFFFu && (MEM32(dev + 0x8D8) & 0x10)) {
        MEM32(0x26119Cu) = MEM32(0x263A28u); MEM32(0x263A28u) = 0xFFFFFFFFu;
    }
    if (state == 0x41000000u) {
        uint32_t request = MEM32(dev + 0x8DC);
        MEM32(0x263A34u) = MEM32(request);
        if (MEM32(request) != 1) MEM32(dev + 0x8D4) = 0x44000004u;
        else { strcpy((char *)CPTR(MEM32(request + 4)), "1.00.5849.1 Release"); MEM32(dev + 0x8D4) = 0x43000000u; }
    } else if (state == 0x21000000u) {
        uint32_t request = MEM32(dev + 0x8DC);
        MEM32(0x263A2Cu) = MEM32(request); MEM32(0x263A3Cu) = MEM32(request + 8); MEM32(0x263A38u) = MEM32(request + 12);
        MEM32(0x263A28u) = 0xFFFFFFFFu;
        LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency);
        write64(0x263A00u, (uint64_t)MEM32(request + 4) * (uint64_t)frequency.QuadPart / 1000);
        write64(0x263A08u, 0);
        uint32_t front = MEM32(dev + 0x1A18), size = MEM32(front + 16), format = MEM32(front + 12), shrink = MEM32(request) & 3;
        s_width = (size ? (size & 4095u) + 1 : 1u << ((format >> 20) & 15u)) >> shrink;
        s_height = (size ? ((size >> 12) & 4095u) + 1 : 1u << ((format >> 24) & 15u)) >> shrink;
        s_format = (MEM32(request) & 4) ? 0x1E : 0x1C;
        uint32_t fw = 0, sz = 0;
        hle_resource_layout(s_width, s_height, 1, 1, s_format, 0, 0, 0, &fw, &sz);
        s_pitch = ((sz >> 24) + 1u) * 64u;
        uint32_t bytes = s_pitch * s_height + 64;
        MEM32(0x263A30u) = bytes;
        uint32_t buffer = capture_alloc((bytes + 511u) & ~511u, 0xAE800000u);
        MEM32(dev + 0x8DC) = buffer;
        if (!buffer) { MEM32(dev + 0x8D4) = 0x25000003u; return; }
        memset(CPTR(buffer), 0, (bytes + 511u) & ~511u);
        MEM32(buffer) = 40; MEM32(buffer + 4) = s_width; MEM32(buffer + 8) = s_height;
        MEM16(buffer + 12) = 1; MEM16(buffer + 14) = s_format == 0x1E ? 24 : 16;
        MEM32(buffer + 20) = bytes; MEM32(buffer + 36) = s_pitch;
        if ((MEM8(0x263A2Cu) & 0x80) && (int32_t)movie_open(0) < 0) { MEM32(dev + 0x8D4) = 0x25000003u; return; }
        MEM32(0x263A20u) = 0; MEM32(dev + 0x8D8) |= 0x10; MEM32(dev + 0x8D4) = 0x22000000u;
    } else if (state == 0x23000000u) {
        if ((MEM8(0x263A2Cu) & 0x80) && MEM32(0x263A24u)) {
            uint32_t scratch = capture_alloc(1, 0x24800000u); MEM8(scratch) = 1;
            uint32_t args[5] = { MEM32(0x263A24u), 0x263A10u, scratch, 1, 13 };
            hle_guest_call(0x800000E2u, 5, args); capture_free(scratch, 0x24800000u);
            hle_guest_call(0x800000BBu, 1, args); MEM32(0x263A24u) = 0;
        }
        MEM32(dev + 0x8D8) &= ~0x30u; MEM32(dev + 0x8D4) = 0x24000000u;
        if (MEM32(dev + 0x8DC)) capture_free(MEM32(dev + 0x8DC), 0xA1800000u);
        MEM32(dev + 0x8DC) = 0;
    } else if (state == 0x29000000u) {
        MEM32(dev + 0x8D8) = (MEM32(dev + 0x8D8) & ~0x10u) | 0x20u;
        uint32_t args[2] = { MEM32(0x263A24u), 1000 };
        hle_guest_call(0x15B2CAu, 2, args); MEM32(0x263A3Cu) &= ~1u;
        hle_guest_call(0x800000BBu, 1, args);
        if ((int32_t)movie_open(1) < 0) MEM32(dev + 0x8D4) = 0x25000003u;
        else MEM32(dev + 0x8D4) = 0x2A000000u;
    } else if (state == 0x28000000u) {
        while (MEM32(dev + 0x8D4)) { Sleep(1); guest_cpu_yield(); }
    } else if (!state) capture_frame(dev);
}
