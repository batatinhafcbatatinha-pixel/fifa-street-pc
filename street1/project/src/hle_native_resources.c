/* Native Xbox resource headers and layouts; NV2A commands are owned by the backend. */
#include <stdint.h>
#include "recomp/gen/recomp_types.h"
#include "hle_d3d11.h"

uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args);
void hle_api_block_resource(uint32_t resource);

static uint32_t resource_alloc(uint32_t bytes, uint32_t flags)
{
    uint32_t a[2] = { bytes, flags };
    return hle_guest_call(0x1609A7u, 2, a);
}

static void resource_free(uint32_t ptr)
{
    uint32_t a[2] = { ptr, 0x24800000u };
    hle_guest_call(0x160A47u, 2, a);
}

static uint32_t trailing_bit(uint32_t v)
{
    uint32_t n = 0;
    if (v) while (!(v & 1u)) { v >>= 1; n++; }
    return n;
}

static uint32_t maximum(uint32_t a, uint32_t b) { return a > b ? a : b; }
static int compressed(uint32_t f) { return f == 12 || f == 14 || f == 15; }
static uint32_t bits(uint32_t f) { return MEM8(0x25ED88u + f) & 0x3Cu; }
static uint32_t level_bytes(uint32_t x, uint32_t y, uint32_t z, uint32_t f)
{
    uint32_t minimum = compressed(f) ? 2u : 0u;
    return (1u << ((maximum(x, minimum) + maximum(y, minimum) + z) & 31)) * bits(f) >> 3;
}

uint32_t hle_resource_format(uint32_t f)
{
    switch (f) {
    case 2: return 0x10; case 3: return 0x1C; case 5: return 0x11;
    case 6: return 0x12; case 7: return 0x1E; case 0x2A: return 0x2E;
    case 0x2B: return 0x2F; case 0x2C: return 0x30; case 0x2D: return 0x31;
    default: return f;
    }
}

uint32_t hle_resource_pitch(uint32_t width, uint32_t fmt)
{
    uint32_t pitch = ((bits(fmt) * width >> 3) + 63u) & ~63u;
    for (uint32_t i = 0; i < 26; i++) {
        uint32_t allowed = MEM32(0x25EE90u + i * 4);
        if (pitch <= allowed) return allowed;
    }
    return pitch;
}

uint32_t hle_resource_layout(uint32_t width, uint32_t height, uint32_t depth,
    uint32_t levels, uint32_t fmt, uint32_t pitch, int cube, int border,
    uint32_t *formatword, uint32_t *sizeword)
{
    uint32_t x = 0, y = 0, z = 0, total = 0;
    if (!(MEM8(0x25ED88u + fmt) & 1) && !compressed(fmt)) {
        if (!levels) levels = 1;
        if (!pitch) pitch = ((bits(fmt) * width >> 3) + 63u) & ~63u;
        total = height * pitch;
        *sizeword = (((pitch >> 6) - 1u) * 0x1000u | (height - 1u)) << 12 | (width - 1u);
    } else {
        x = trailing_bit(width); y = trailing_bit(height); z = trailing_bit(depth);
        if (!levels) levels = maximum(maximum(x, y), z) + 1;
        uint32_t lx = x, ly = y, lz = z;
        for (uint32_t i = 0; i < levels; i++) {
            total += level_bytes(lx, ly, lz, fmt);
            if (lx) lx--; if (ly) ly--; if (lz) lz--;
        }
        if (cube) total = ((total + 127u) & ~127u) * 6u;
        *sizeword = 0;
    }
    *formatword = (((((z << 4 | y) << 4 | x) << 4 | levels) << 8 | fmt) << 4 |
        (border ? 3u : 2u)) << 4 | (cube ? 4u : 0u) | 9u;
    return total;
}

void hle_resource_surface_header(uint32_t out, uint32_t formatword, uint32_t sizeword, uint32_t data)
{
    MEM32(out) = 0x1050001u;
    MEM32(out + 4) = data & 0x0FFFFFFFu;
    MEM32(out + 8) = 0;
    MEM32(out + 12) = formatword;
    MEM32(out + 16) = sizeword;
    MEM32(out + 20) = 0;
}

void hle_resource_addref(uint32_t r)
{
    uint32_t common = MEM32(r);
    if (!(common & 0xFFFFu) && (common & 0x70000u) == 0x50000u && MEM32(r + 20))
        hle_resource_addref(MEM32(r + 20));
    MEM32(r) = common + 1u;
}

void hle_resource_free(uint32_t r)
{
    uint32_t common = MEM32(r), type = common & 0x70000u;
    if (type != 0x50000u || (int32_t)common < 0) hle_api_block_resource(r);
    /* Resource private-data nodes own an optional title COM reference. */
    uint32_t link = 0x2639F0u, node = MEM32(link);
    while (node) {
        if (MEM32(node + 4) != r) { link = node; node = MEM32(node); continue; }
        MEM32(link) = MEM32(node);
        if (MEM8(node + 28) & 1) {
            uint32_t object = MEM32(node + 32);
            hle_guest_call(MEM32(MEM32(object) + 8), 1, &object);
        }
        resource_free(node);
        node = MEM32(link);
    }
    common = MEM32(r);
    if ((type == 0x50000u && (int32_t)common < 0) ||
        (type == 0x20000u && (int32_t)common >= 0) ||
        (type != 0x50000u && type != 0x20000u && type != 0x10000u && type != 0x60000u)) {
        uint32_t a[2] = { MEM32(r + 4), 0xA1800000u };
        if (type != 0x20000u) a[0] |= 0x80000000u;
        hle_guest_call(0x160A47u, 2, a);
    }
    resource_free(r);
}

void hle_resource_private_addref(uint32_t r)
{
    uint32_t common = MEM32(r);
    if (!(common & 0x780000u) && (common & 0x70000u) == 0x50000u && MEM32(r + 20))
        MEM32(MEM32(r + 20)) += 0x80000u;
    MEM32(r) += 0x80000u;
}

void hle_resource_private_release(uint32_t r)
{
    uint32_t common = MEM32(r);
    if ((common & 0x780000u) == 0x80000u) {
        if ((common & 0x70000u) == 0x50000u && MEM32(r + 20)) {
            uint32_t parent = MEM32(r + 20);
            MEM32(parent) -= 0x80000u;
            if (!(MEM32(parent) & 0x78FFFFu)) hle_resource_free(parent);
        }
        if (!(MEM32(r) & 0xFFFFu)) { hle_resource_free(r); return; }
    }
    MEM32(r) -= 0x80000u;
}

static void surface_level(uint32_t r, uint32_t level, uint32_t *data,
    uint32_t *pitch, uint32_t *bytes, uint32_t *formatword, uint32_t *sizeword)
{
    uint32_t f = MEM32(r + 12), sz = MEM32(r + 16), fmt = (f >> 8) & 255u;
    *data = MEM32(r + 4) | 0x80000000u;
    *sizeword = sz;
    if (sz) {
        *formatword = f;
        *pitch = ((sz >> 24) + 1u) * 64u;
        *bytes = (((sz >> 12) & 4095u) + 1u) * *pitch;
        return;
    }
    uint32_t x = (f >> 20) & 15u, y = (f >> 24) & 15u, z = f >> 28;
    for (uint32_t i = 0; i < level; i++) {
        *data += level_bytes(x, y, z, fmt);
        if (x) x--; if (y) y--; if (z) z--;
    }
    uint32_t px = maximum(x, compressed(fmt) ? 2u : 0u);
    *pitch = fmt == 12 ? (1u << px) * 2u : fmt == 14 || fmt == 15 ?
        (1u << px) * 4u : (1u << px) * bits(fmt) >> 3;
    *bytes = level_bytes(x, y, 0, fmt);
    *formatword = ((z << 4 | y) << 4 | x) << 20 | (f & 0xFFFFFu);
}

static uint32_t resource_type(uint32_t r)
{
    uint32_t t = MEM32(r) & 0x70000u, f = MEM32(r + 12);
    switch (t) {
    case 0: return 6; case 0x10000: return 7; case 0x20000: return 8;
    case 0x30000: return 9;
    case 0x40000: return (f & 4u) ? 5u : 3u + ((f & 0xF0u) > 0x20u);
    case 0x50000: return 1u + ((f & 0xF0u) > 0x20u);
    default: return 10;
    }
}

/* 0x2584E0 (GetDesc of a surface, GetLevelDesc of a texture level). */
static void level_desc(uint32_t r, uint32_t level, uint32_t out)
{
    uint32_t data, pitch, bytes, fw, sz, dev = MEM32(0x261238u);
    uint32_t f = MEM8(r + 13), info = MEM8(0x25ED88u + f);
    surface_level(r, level, &data, &pitch, &bytes, &fw, &sz);
    MEM32(out) = f; MEM32(out + 4) = resource_type(r);
    MEM32(out + 8) = level ? 0u : (info & 128u) ? 1u : (info & 64u) ? 2u : 0;
    MEM32(out + 12) = bytes;
    MEM32(out + 16) = MEM32(r + 4) == MEM32(MEM32(dev + 0x1A14u) + 4) ? MEM32(dev + 0x196Cu) : 0x11u;
    MEM32(out + 20) = sz ? (sz & 4095u) + 1 : 1u << ((fw >> 20) & 15u);
    MEM32(out + 24) = sz ? ((sz >> 12) & 4095u) + 1 : 1u << ((fw >> 24) & 15u);
}

int hle_resources_call(int id, const uint32_t *a)
{
    uint32_t r, data, pitch, bytes, fw, sz, dev = MEM32(0x261238u);
    switch (id) {
    case HLE_D3DResource_Register:
        data = MEM32(a[0] + 4) + a[1];
        if ((MEM32(a[0]) & 0x70000u) != 0x20000u) data &= 0x0FFFFFFFu;
        MEM32(a[0] + 4) = data; g_eax = data; g_esp += 12; return 1;
    case HLE_D3DResource_IsBusy:
        r = a[0];
        if ((MEM32(r) & 0x70000u) == 0x50000u && MEM32(r + 20)) {
            if ((MEM32(r) | MEM32(MEM32(r + 20))) & 0x780000u) {
                g_eax = 1; g_esp += 8; return 1;
            }
            r = MEM32(r + 20);
        }
        g_eax = (MEM32(r) & 0x780000u) != 0;
        if (!g_eax && MEM32(r + 8)) {
            uint32_t current = MEM32(dev + 44), done = MEM32(MEM32(dev + 48));
            g_eax = (current - MEM32(r + 8)) < (current - done);
        }
        if (!g_eax) MEM32(r + 8) = 0;
        g_esp += 8; return 1;
    case HLE_D3DDevice_CreateIndexBuffer2:
        r = resource_alloc(a[0] + 12u, 0x24800000u);
        if (r) { MEM32(r) = 0x1010001u; MEM32(r + 4) = r + 12; MEM32(r + 8) = 0; }
        g_eax = r; g_esp += 8; return 1;
    case HLE_D3DDevice_CreateVertexBuffer2:
        r = resource_alloc(12, 0x64800000u);
        if (r) {
            data = resource_alloc(a[0], 0xB2800000u);
            if (!data) { resource_free(r); r = 0; }
            else { MEM32(r) = 0x1000001u; MEM32(r + 4) = data & 0x0FFFFFFFu; }
        }
        g_eax = r; g_esp += 8; return 1;
    case HLE_D3D_AllocContiguousMemory:
        g_eax = resource_alloc(a[0], 0xB0800000u | ((a[1] <= 4096 ? 12u : trailing_bit(a[1])) << 24));
        g_esp += 12; return 1;
    case HLE_D3DDevice_CreateTexture2:
        bytes = hle_resource_layout(a[0], a[1], a[2], a[3], a[5], 0, a[6] == 5, a[6] == 4, &fw, &sz);
        if (a[4] & 0x10000u) fw &= ~8u;
        r = resource_alloc(20, 0x64800000u);
        if (r) {
            data = resource_alloc(bytes, 0xB7800000u);
            if (!data) { resource_free(r); r = 0; }
            else {
                MEM32(r) = 0x1040001u; MEM32(r + 4) = data & 0x0FFFFFFFu;
                MEM32(r + 8) = 0; MEM32(r + 12) = fw; MEM32(r + 16) = sz;
            }
        }
        g_eax = r; g_esp += 32; return 1;
    case HLE_D3DTexture_GetSurfaceLevel2:
        surface_level(a[0], a[1], &data, &pitch, &bytes, &fw, &sz);
        r = resource_alloc(24, 0x64800000u);
        if (r) { hle_resource_surface_header(r, fw, sz, data); MEM32(r + 20) = a[0]; hle_resource_addref(a[0]); }
        g_eax = r; g_esp += 12; return 1;
    case HLE_D3DSurface_GetDesc:
        level_desc(a[0], 0, a[1]);
        g_eax = MEM32(a[1] + 20); g_esp += 12; return 1;
    case HLE_D3DTexture_GetLevelDesc:
        level_desc(a[0], a[1], a[2]);
        g_eax = MEM32(a[2] + 20); g_esp += 16; return 1;
    /* 0x25A860: a one-level image surface that owns its memory (bit 31). */
    case HLE_D3DDevice_CreateImageSurface:
        bytes = hle_resource_layout(a[0], a[1], 1, 1, a[3], 0, 0, 0, &fw, &sz);
        r = resource_alloc(24, 0x64800000u);
        if (r) {
            data = resource_alloc(bytes, 0xB6800000u);
            if (!data) { resource_free(r); r = 0; }
            else {
                hle_resource_surface_header(r, fw, sz, data);
                MEM32(r) = 0x81050001u;
            }
        }
        g_eax = r; g_esp += 20; return 1;
    case HLE_D3D_FreeContiguousMemory: {
        uint32_t f[2] = { a[0], 0xA1800000u };
        hle_guest_call(0x160A47u, 2, f);
        g_esp += 8; return 1;
    }
    case HLE_D3D_ReturnTrue_25E390:
        g_eax = 1; g_esp += 8; return 1;
    case HLE_D3DSurface_LockRect:
        if (!(a[3] & 32u)) hle_api_block_resource(a[0]);
        surface_level(a[0], 0, &data, &pitch, &bytes, &fw, &sz);
        if (a[3] & 64u) data |= 0xF0000000u;
        if (a[2]) data += MEM32(a[2] + 4) * pitch + (bits(MEM8(a[0] + 13)) * MEM32(a[2]) >> 3);
        MEM32(a[1]) = pitch; MEM32(a[1] + 4) = data;
        g_eax = a[2] ? pitch : a[1]; g_esp += 20; return 1;
    case HLE_D3DDevice_GetBackBuffer2:
        r = MEM32(dev + (a[0] == 0xFFFFFFFFu ? 0x1A18u : a[0] ? 0x1A1Cu : 0x1A14u));
        hle_resource_addref(r); g_eax = r; g_esp += 8; return 1;
    case HLE_D3DDevice_GetDepthStencilSurface2:
        r = MEM32(dev + 0x1A08u); if (r) hle_resource_addref(r);
        g_eax = r; g_esp += 4; return 1;
    case HLE_D3DDevice_CopyRects:
        r = MEM32(a[0] + 20); MEM32((r ? r : a[0]) + 8) = MEM32(dev + 44);
        r = MEM32(a[3] + 20); MEM32((r ? r : a[3]) + 8) = MEM32(dev + 44);
        g_esp += 24; return 1;
    default: return 0;
    }
}
