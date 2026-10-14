/* Native shader objects, display bookkeeping and command/query APIs. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "hle_d3d11_internal.h"

extern uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args);
extern void hle_native_drain(void);
extern void hle_native_wait_idle(void);
extern void hle_draw_decode_push(uint32_t begin, uint32_t end);
extern void hle_native_gamma_changed(uint32_t guest_table);
extern uint32_t hle_native_surface_readback(uint32_t source, uint32_t out, uint32_t width,
    uint32_t height, uint32_t pitch, uint32_t format);
extern uint32_t hle_resource_layout(uint32_t width, uint32_t height, uint32_t depth,
    uint32_t levels, uint32_t fmt, uint32_t pitch, int cube, int border,
    uint32_t *fmtword, uint32_t *sizeword);
extern void hle_resource_surface_header(uint32_t out, uint32_t fmtword, uint32_t sizeword, uint32_t data);

#define DEVICE MEM32(0x261238u)
#define INVALID_CALL 0x8876086Cu
#define OUT_OF_MEMORY 0x8007000Eu
#define INCOMPLETE 0x88760828u
#define GUEST_PTR(va) ((void *)XBOX_PTR(va))

static uint32_t allocate(uint32_t bytes, uint32_t flags)
{
    uint32_t args[2] = {bytes, flags};
    return hle_guest_call(0x1609A7u, 2, args);
}

static void release(uint32_t va)
{
    uint32_t args[2] = {va, 0x24800000u};
    hle_guest_call(0x160A47u, 2, args);
}

/* Declarations carry stream/register metadata and optional constant packets. */
static uint32_t declaration_constants(uint32_t decl, uint32_t out)
{
    uint32_t count = 0, previous = 0xDEADBEEFu;
    if (!decl) return 0;
    for (uint32_t p = decl; ; p += 4) {
        uint32_t token = MEM32(p);
        if (token == 0xFFFFFFFFu) break;
        if ((token & 0xE0000000u) == 0x80000000u) {
            uint32_t n = (token >> 25) & 15u, reg = token & 255u;
            if (reg != previous + 1u) {
                if (out) { MEM32(out + count * 4) = 0x41EA4u; MEM32(out + count * 4 + 4) = reg; }
                count += 2;
            }
            previous = reg;
            p += 4;
            do {
                uint32_t batch = n > 8 ? 8 : n;
                if (out) {
                    MEM32(out + count * 4) = batch * 0x100000u + 0xB80u;
                    memcpy(GUEST_PTR(out + (count + 1) * 4), GUEST_PTR(p), batch * 16);
                }
                count += 1 + batch * 4;
                p += batch * 16;
                n -= batch;
            } while (n);
            p -= 4;
        }
    }
    return count;
}

static void declaration_layout(uint32_t shader, uint32_t decl, int fixed)
{
    uint32_t stream = 0, offset = 0;
    for (uint32_t p = decl; ; p += 4) {
        uint32_t token = MEM32(p), kind = token & 0xE0000000u;
        if (kind > 0x60000000u) {
            if (kind == 0x80000000u) { p += ((token >> 25) & 15u) * 16; continue; }
            return;
        }
        if (!kind) continue;
        if (kind == 0x20000000u) { stream = token & 15u; offset = 0; continue; }
        uint32_t reg = token & 31u, at = shader + reg * 16;
        if (kind == 0x60000000u) {
            MEM32(at + 0x14) = 0; MEM32(at + 0x18) = 0; MEM32(at + 0x1C) = 2;
            MEM8(at + 0x21) = (token >> 20) & 15u;
            MEM8(at + 0x20) = token & 0x10000000u ? 2 : 1;
            continue;
        }
        if (token & 0x10000000u) {
            uint32_t skip = (token >> 16) & 15u;
            offset += token & 0x08000000u ? skip : skip * 4;
            continue;
        }
        uint32_t type = (token >> 16) & 255u;
        MEM32(at + 0x14) = stream; MEM32(at + 0x18) = offset; MEM32(at + 0x1C) = type;
        MEM8(at + 0x20) = 0;
        if (fixed) {
            if (reg > 8 && reg < 13) {
                uint32_t size = type >> 4;
                if (size == 7) size = 4;
                else if (type == 0x16) size = 3;
                MEM32(shader + 0x10) |= size << ((reg * 8 + 0xB8) & 31u);
            }
            if (reg == 3) MEM32(shader + 4) |= 0x400;
            if (reg == 4) MEM32(shader + 4) |= 0x800;
            if (reg == 7) MEM32(shader + 4) |= 0x1000;
            if (reg == 8) MEM32(shader + 4) |= 0x2000;
        }
        offset += MEM8(0x25EE84u + (type >> 4)) * MEM8(0x25EE7Cu + (type & 15u));
    }
}

static uint32_t create_vertex_shader(const uint32_t *a)
{
    uint32_t words = a[1] ? (uint32_t)MEM16(a[1] + 2) * 4 : 0;
    uint32_t packets = a[1] ? (words + 31) / 32 : 0;
    uint32_t constants = declaration_constants(a[0], 0);
    uint32_t total = words + packets + constants;
    uint32_t shader = allocate(0x118u + total * 4, 0x24800000u);
    if (!shader) return OUT_OF_MEMORY;
    memset(GUEST_PTR(shader), 0, 0x118);
    MEM32(shader) = 1; MEM32(shader + 4) = a[3];
    for (uint32_t i = 0; i < 16; i++) MEM32(shader + 0x1C + i * 16) = 2;
    if (a[0]) declaration_layout(shader, a[0], !a[1]);
    uint32_t out = shader + 0x114;
    if (a[1]) {
        uint32_t signature = MEM16(a[1]), flags = MEM32(shader + 4);
        MEM32(shader + 4) = flags | (signature == 0x2078 ? 0x10 : 0x11);
        if (signature == 0x7378) MEM32(shader + 4) |= 8;
        MEM32(shader + 8) = MEM16(a[1] + 2);
        uint32_t src = a[1] + 4;
        /* A zero-length function still carries the empty upload packet. */
        do {
            uint32_t n = words > 32 ? 32 : words;
            MEM32(out) = n * 0x40000u + 0xB00u;
            memcpy(GUEST_PTR(out + 4), GUEST_PTR(src), n * 4);
            out += (n + 1) * 4; src += n * 4; words -= n;
        } while (words);
    }
    declaration_constants(a[0], out);
    MEM32(shader + 0xC) = total;
    MEM32(shader + 0x114 + total * 4) = 0;
    MEM32(a[2]) = shader | 1u;
    return 0;
}

typedef struct VisibilityQuery {
    ID3D11Query *query;
    uint32_t index;
    uint64_t timestamp;
    struct VisibilityQuery *next;
} VisibilityQuery;
static VisibilityQuery *s_visibility;
static ID3D11Query *s_active_query;
static HRESULT s_query_error;

static uint32_t visibility_begin(void)
{
    D3D11_QUERY_DESC desc = { D3D11_QUERY_OCCLUSION, 0 };
    hle_native_drain();
    if (s_active_query) return INVALID_CALL;
    s_query_error = g_hle.dev ? ID3D11Device_CreateQuery(g_hle.dev, &desc, &s_active_query) : E_FAIL;
    if (FAILED(s_query_error)) return (uint32_t)s_query_error;
    EnterCriticalSection(&g_hle.lock);
    ID3D11DeviceContext_Begin(g_hle.ctx, (ID3D11Asynchronous *)s_active_query);
    LeaveCriticalSection(&g_hle.lock);
    return 0;
}

static uint32_t visibility_end(uint32_t index)
{
    uint32_t dev = DEVICE, page = index >> 8;
    if (page >= 4 || !s_active_query) return INVALID_CALL;
    uint32_t table = MEM32(dev + 0x7D4 + page * 4);
    if (!table) {
        table = allocate(4096, 0xA1800000u);
        if (!table) return OUT_OF_MEMORY;
        memset(GUEST_PTR(table), 0, 4096);
        MEM32(dev + 0x7D4 + page * 4) = table;
    }
    VisibilityQuery *q = s_visibility;
    while (q && q->index != index) q = q->next;
    if (!q) {
        q = (VisibilityQuery *)calloc(1, sizeof(*q));
        if (!q) return OUT_OF_MEMORY;
        q->index = index; q->next = s_visibility; s_visibility = q;
    }
    hle_native_drain();
    if (q->query) ID3D11Query_Release(q->query);
    q->query = s_active_query; s_active_query = NULL;
    EnterCriticalSection(&g_hle.lock);
    ID3D11DeviceContext_End(g_hle.ctx, (ID3D11Asynchronous *)q->query);
    LeaveCriticalSection(&g_hle.lock);
    /* Xbox timestamps use the GPU timer; retain monotonic host time in ns. */
    LARGE_INTEGER ticks, frequency;
    QueryPerformanceCounter(&ticks); QueryPerformanceFrequency(&frequency);
    q->timestamp = (uint64_t)((double)ticks.QuadPart * 1.0e9 / (double)frequency.QuadPart);
    uint32_t entry = table + (index & 255u) * 16;
    MEM32(entry) = MEM32(dev + 0x2C); MEM32(entry + 0xC) = 0xFFFFFFFFu;
    return 0;
}

static uint32_t visibility_result(const uint32_t *a)
{
    VisibilityQuery *q = s_visibility;
    while (q && q->index != a[0]) q = q->next;
    if (!q || !q->query) return INCOMPLETE;
    uint64_t samples = 0;
    EnterCriticalSection(&g_hle.lock);
    HRESULT hr = ID3D11DeviceContext_GetData(g_hle.ctx, (ID3D11Asynchronous *)q->query,
                                            &samples, sizeof(samples), 0);
    LeaveCriticalSection(&g_hle.lock);
    if (hr == S_FALSE) return INCOMPLETE;
    if (FAILED(hr)) return (uint32_t)hr;
    /* Queries count native samples; return logical game pixels at higher scales. */
    double scale = g_hle.scale > 0.0f ? g_hle.scale : 1.0;
    samples = (uint64_t)((double)samples / (scale * scale) + 0.5);
    if (samples > 0xFFFFFFFFu) samples = 0xFFFFFFFFu;
    MEM32(a[1]) = (uint32_t)samples;
    if (a[2]) { MEM32(a[2]) = (uint32_t)q->timestamp; MEM32(a[2] + 4) = (uint32_t)(q->timestamp >> 32); }
    uint32_t table = MEM32(DEVICE + 0x7D4 + (a[0] >> 8) * 4);
    uint32_t entry = table + (a[0] & 255u) * 16;
    MEM32(entry) = (uint32_t)q->timestamp; MEM32(entry + 4) = (uint32_t)(q->timestamp >> 32);
    MEM32(entry + 8) = (uint32_t)samples; MEM32(entry + 0xC) = 0;
    return 0;
}

static void run_push_buffer(uint32_t push, uint32_t fixup)
{
    hle_native_drain();
    uint32_t dev = DEVICE, begin = MEM32(push + 4), bytes = MEM32(push + 0xC);
    if (fixup) {
        uint32_t p = MEM32(fixup + 4) + MEM32(fixup + 0xC);
        while (MEM32(p) != 0xFFFFFFFFu) {
            uint32_t size = MEM32(p), offset = MEM32(p + 4);
            memcpy(GUEST_PTR(begin + offset), GUEST_PTR(p + 8), size);
            p += size + 8;
        }
        MEM32(fixup + 8) = MEM32(dev + 0x2C);
    }
    if (bytes >= 4) hle_draw_decode_push(begin, begin + bytes - 4);
    MEM32(push + 8) = MEM32(dev + 0x2C);
    if (!(MEM32(push) & 0x80000000u)) {
        MEM32(dev + 0x50)++;
        MEM32(dev + 0x54)++;
        MEM32(push + 0x14) = MEM32(dev + 0x54);
    }
}

uint32_t hle_misc_persist_display(void)
{
    uint32_t dev = DEVICE;
    uint32_t old = hle_guest_call(MEM32(0x277284u), 0, NULL);
    if (old) {
        hle_guest_call(MEM32(0x277134u), 1, &old);
        uint32_t zero = 0;
        hle_guest_call(MEM32(0x277290u), 1, &zero);
    }
    if (!MEM32(dev + 0x938)) return 0x80004005u;
    hle_native_wait_idle();
    uint32_t front = MEM32(dev + 0x1A18), format = MEM32(front + 0xC), size = MEM32(front + 0x10);
    uint32_t w = size ? (size & 4095u) + 1 : 1u << ((format >> 20) & 15u);
    uint32_t h = size ? ((size >> 12) & 4095u) + 1 : 1u << ((format >> 24) & 15u);
    uint32_t fmtword = 0, sizeword = 0;
    uint32_t bytes = hle_resource_layout(w, h, 1, 1, (format >> 8) & 255u, 0, 0, 0, &fmtword, &sizeword);
    uint32_t allocation_args[6] = { bytes + 64, 0, 0x07FFFFFFu, 64, 0x404u, 0 };
    uint32_t surface = hle_guest_call(MEM32(0x27716Cu), 5, allocation_args);
    if (!surface) return 0x80004005u;
    hle_resource_surface_header(surface, fmtword, sizeword, (surface + 64) & 0x0FFFFFFFu);
    MEM32(surface) = 0x81050001u;
    uint32_t scratch = allocate(32, 0x24800000u);
    if (!scratch) {
        hle_guest_call(MEM32(0x277134u), 1, &surface);
        return OUT_OF_MEMORY;
    }
    memcpy(GUEST_PTR(scratch + 8), GUEST_PTR(front), 24);
    if (g_hle.last_present) MEM32(scratch + 12) = g_hle.last_present;
    hle_native_copy_rects(scratch + 8, 0, 0, surface, 0);
    MEM32(scratch) = sizeword ? ((sizeword >> 24) + 1u) * 64u :
        (w * ((MEM8(0x25ED88u + ((format >> 8) & 255u)) & 0x3Cu) >> 2) / 2u);
    MEM32(scratch + 4) = (surface + 64) | 0x80000000u;
    uint32_t readback_status = hle_native_surface_readback(scratch + 8, MEM32(scratch + 4),
        w, h, MEM32(scratch), (format >> 8) & 255u);
    if ((int32_t)readback_status < 0) {
        release(scratch); hle_guest_call(MEM32(0x277134u), 1, &surface);
        return readback_status;
    }
    uint32_t display_format = (format >> 8) & 255u;
    switch (display_format) {
    case 2: display_format = 0x10; break;
    case 3: display_format = 0x1C; break;
    case 5: display_format = 0x11; break;
    case 6: display_format = 0x12; break;
    case 7: display_format = 0x1E; break;
    case 0x2A: display_format = 0x2E; break;
    case 0x2B: display_format = 0x2F; break;
    case 0x2C: display_format = 0x30; break;
    case 0x2D: display_format = 0x31; break;
    }
    uint32_t display_args[6] = { MEM32(dev + 0x1C28), 0, MEM32(dev + 0x1C30),
        display_format, MEM32(scratch), MEM32(surface + 4) };
    do {
        display_args[1] = hle_guest_call(MEM32(0x27728Cu), 6, display_args);
        if (display_args[1]) guest_cpu_yield();
    } while (display_args[1]);
    release(scratch);
    MEM32(0x2636F0u) = 0; MEM32(0x2636ECu) = 0;
    uint32_t allocation_size = hle_guest_call(MEM32(0x277170u), 1, &surface);
    uint32_t protect_args[3] = { surface, allocation_size, 1 };
    hle_guest_call(MEM32(0x277128u), 3, protect_args);
    hle_guest_call(MEM32(0x277290u), 1, &surface);
    return 0;
}

int hle_misc_call(int id, const uint32_t *a)
{
    uint32_t dev = DEVICE, args = 0;
    switch (id) {
    case HLE_D3DDevice_CreatePixelShader: {
        uint32_t shader = allocate(0xFC, 0x24800000u);
        g_eax = OUT_OF_MEMORY;
        if (shader) {
            MEM32(shader) = 1; MEM32(shader + 4) = 1; MEM32(shader + 8) = shader + 12;
            memcpy(GUEST_PTR(shader + 12), GUEST_PTR(a[0]), 0xF0);
            MEM32(a[1]) = shader; g_eax = 0;
        }
        args = 2; break;
    }
    case HLE_D3DDevice_DeletePixelShader:
        if (--MEM32(a[0]) == 0 && MEM32(a[0] + 4)) release(a[0]);
        args = 1; break;
    case HLE_D3DDevice_CreateVertexShader:
        g_eax = create_vertex_shader(a); args = 4; break;
    case HLE_D3DDevice_DeleteVertexShader:
        if (--MEM32(a[0] - 1) == 0) release(a[0] - 1);
        args = 1; break;
    case HLE_D3DDevice_GetGammaRamp:
        memcpy(GUEST_PTR(a[0]), GUEST_PTR(dev + 0x1E04 + MEM32(dev + 0x240C) * 0x300), 0x300);
        args = 1; break;
    case HLE_D3DDevice_SetGammaRamp: {
        uint32_t current = dev + 0x1E04 + MEM32(dev + 0x240C) * 0x300;
        if (memcmp(GUEST_PTR(current), GUEST_PTR(a[1]), 0x300)) {
            uint32_t index = MEM32(dev + 0x2478) & 1;
            memcpy(GUEST_PTR(dev + 0x1E04 + index * 0x300), GUEST_PTR(a[1]), 0x300);
            MEM32(dev + 0x240C) = index;
            if (!(a[0] & 2)) MEM32(dev + 0x2404 + index * 4) = 1;
            hle_native_gamma_changed(dev + 0x1E04 + index * 0x300);
        }
        args = 2; break;
    }
    case HLE_D3DDevice_GetTile:
        memcpy(GUEST_PTR(a[1]), GUEST_PTR(dev + 0x1AC0 + a[0] * 24), 24);
        args = 2; break;
    case HLE_D3DDevice_SetTile: {
        hle_native_wait_idle();
        uint32_t dst = dev + 0x1AC0 + a[0] * 24;
        if (a[1] && MEM32(a[1] + 4)) {
            memcpy(GUEST_PTR(dst), GUEST_PTR(a[1]), 24);
            if (!(MEM32(dst) & 0x80000000u)) { MEM32(dst + 16) = 0; MEM32(dst + 20) = 0; }
        } else memset(GUEST_PTR(dst), 0, 24);
        args = 2; break;
    }
    case HLE_D3DDevice_SetSwapCallback:
        MEM32(dev + 0x1DB4) = a[0]; args = 1; break;
    case HLE_D3DDevice_SetVerticalBlankCallback:
        MEM32(dev + 0x1DB8) = a[0]; args = 1; break;
    case HLE_D3DDevice_SetFlickerFilter:
        if (!MEM32(0x2636F0u) || MEM32(0x2636E8u) != a[0]) {
            uint32_t encoder_args[4] = { MEM32(dev + 0x1C28), 11, a[0], 0 };
            hle_guest_call(0x80000002u, 4, encoder_args);
        }
        MEM32(0x2636E8u) = a[0]; MEM32(0x2636F0u) = 1; args = 1; break;
    case HLE_D3DDevice_SetSoftDisplayFilter:
        if (!MEM32(0x2636ECu) || MEM32(0x2636E4u) != (a[0] != 0)) {
            uint32_t encoder_args[4] = { MEM32(dev + 0x1C28), 14, a[0], 0 };
            hle_guest_call(0x80000002u, 4, encoder_args);
        }
        MEM32(0x2636E4u) = a[0] != 0; MEM32(0x2636ECu) = 1; args = 1; break;
    case HLE_D3DDevice_IsFencePending: {
        uint32_t now = MEM32(dev + 0x2C), done = MEM32(MEM32(dev + 0x30));
        g_eax = (uint32_t)(now - a[0]) < (uint32_t)(now - done); args = 1; break;
    }
    case HLE_D3DDevice_InsertCallback:
        if (a[0]) hle_native_wait_idle(); else hle_native_drain();
        if (a[1]) hle_guest_call(a[1], 1, a + 2);
        args = 3; break;
    case HLE_D3DDevice_BeginVisibilityTest:
        g_eax = visibility_begin(); break;
    case HLE_D3DDevice_EndVisibilityTest:
        g_eax = visibility_end(a[0]); args = 1; break;
    case HLE_D3DDevice_GetVisibilityTestResult:
        g_eax = visibility_result(a); args = 3; break;
    case HLE_D3DDevice_FlushVertexCache:
        hle_native_drain(); break;
    case HLE_D3D_SetPushBufferSize:
        MEM32(0x2639ECu) = a[0]; MEM32(0x2639E8u) = a[1]; args = 2; break;
    case HLE_D3DDevice_RunPushBuffer:
        run_push_buffer(a[0], a[1]); args = 2; break;
    case HLE_D3DDevice_PersistDisplay:
        g_eax = hle_misc_persist_display(); break;
    default: return 0;
    }
    g_esp += 4 + args * 4;
    return 1;
}
