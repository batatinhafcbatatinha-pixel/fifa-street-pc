/*
 * Native bodies for the D3D8 library calls that still ran the original code.
 *
 * With the primary renderer the backend (hle_d3d11.c, hle_draw.c) draws every
 * call itself; what the original bodies did beyond that was (a) keep the
 * library's CPU-side state -- which the game, the backend and the remaining
 * library code read -- and (b) encode NV2A commands, which hle_d3d8.c rewound.
 * The functions here do (a) in C and never touch the pushbuffer:
 *
 *   SetVertexShader   (0x2550E0)  device shader slots, dirty bits, FVF program
 *                                 object, pass-through program constants
 *   SetPixelShader    (0x25DEC0)  device shader slots, the PS render-state copy
 *   SetRenderTarget   (0x252FB0)  surface references and busy times, depth
 *                                 format scale/pitch, surface format word,
 *                                 back-buffer scale state, Dxt1 noise flag
 *   Swap              (0x25DB90)  fences (completed at once: the backend has
 *                                 drawn everything before them), buffer
 *                                 rotation, the miniport's flip queue and the
 *                                 title's swap callback
 *   vblank service    (0x259390)  the miniport's vblank count, flip completion,
 *                                 vblank event and the title's vblank callback
 *
 * None of them writes an NV2A register: CRTC start address, palette DAC and
 * interrupt acknowledges were what made a flip visible on the Xbox; here the
 * backend presents the render target itself.
 *
 * RECOMP_HLE_LIBAPI=1   run the original bodies again (A/B).
 * RECOMP_HLE_APICHECK=1 run both for SetVertexShader / SetPixelShader /
 *                       SetRenderTarget: the native body, then -- from the
 *                       same starting state -- the original, and report any
 *                       difference in the device, the library globals and the
 *                       surfaces involved. The original's result is kept.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <math.h>
#include "recomp/gen/recomp_types.h"
#include "recomp/gen/recomp_funcs.h"
#include "hle_d3d11.h"

#define D3D_DEVICE_PTR   0x261238u
#define MINIPORT         0x262E68u
#define DIRTY            0x260D98u
#define PRESENT_INTERVAL 0x26119Cu
#define FVF_SHADER       0x2638D0u
#define VSH_CONSTANTS    0x260180u
#define PS_RENDER_STATE  0x260FA0u
#define FORMAT_INFO      0x25ED88u   /* per-format byte: bits 2..5 = bits per pixel / 4 */

/* Device fields (byte offsets). */
#define DEV_PUT          0x000u
#define DEV_FLAGS        0x008u
#define DEV_FENCE_TIME   0x02Cu
#define DEV_FENCE_DONE   0x030u      /* -> completed fence time */
#define DEV_FENCE_RESET  0x03Cu
#define DEV_FENCE_SLOTS  0x064u      /* 64 x {time, pushbuffer address} */
#define DEV_PIXEL_SHADER 0x784u
#define DEV_VERTEX_SHADER 0x794u
#define DEV_ZSCALE       0x948u
#define DEV_ZPITCH       0x94Cu
#define DEV_RENDER_TARGET 0x1A04u
#define DEV_DEPTH_STENCIL 0x1A08u
#define DEV_SURFACE_FMT  0x1A0Cu
#define DEV_BUFFER_COUNT 0x1A10u
#define DEV_BACK_BUFFER  0x1A14u
#define DEV_FRONT_BUFFER 0x1A18u
#define DEV_FRAME_FENCE  0x1974u
#define DEV_SWAP_COUNT   0x2478u

/* Miniport fields (dword indices, as the library's code has them). */
#define MP(i)            MEM32(MINIPORT + 4u * (i))

volatile uint64_t hle_api_native_calls, hle_api_fallbacks;
static uint64_t s_fallback_why[8];
static uint64_t s_begin_calls, s_begin_push_calls, s_end_push_calls, s_push_reserve_fallbacks;
int hle_resources_call(int id, const uint32_t *a);
int hle_states_call(int id, const uint32_t *a);
int hle_misc_call(int id, const uint32_t *a);
int hle_create_call(int id, const uint32_t *a);
void hle_resource_free(uint32_t r);
uint32_t hle_native_reserve(uint32_t dwords);
/* Dirty bits the scaled Swap's blit leaves after its flush: its texture,
 * texture-stage and render-state restores (seen constant under APICHECK). */
static uint32_t s_blit_dirty = 0x4B01u;

static int api_mode(void)
{
    static int mode = -1;
    if (mode < 0) {
        const char *lib = getenv("RECOMP_HLE_LIBAPI"), *chk = getenv("RECOMP_HLE_APICHECK");
        mode = lib && *lib == '1' ? 0 : chk && *chk == '1' ? 2 : 1;
        fprintf(stderr, "[HLE-API] SetVertexShader/SetPixelShader/SetRenderTarget/Swap: %s\n",
                mode == 0 ? "original library bodies" : mode == 2 ? "native, checked against the original" : "native");
    }
    return mode;
}

static uint32_t dev_va(void)
{
    return MEM32(D3D_DEVICE_PTR);
}

/* 0x25A930: only a TFACTOR input changes CPU state; the returned combiner
 * encoding is consumed exclusively by the Xbox pushbuffer. */
static void flush_combiner_arg(uint32_t arg)
{
    if ((arg & 15u) == 4)
        MEM32(0x261248u) |= 0x40u;
}

/* 0x25A9D0's jump table is poorly recovered by the decompiler. These cases
 * follow the XBE instruction blocks, including implicit blend inputs and
 * DOTPRODUCT3's suppression of the alpha operation. */
static uint32_t flush_combiners(uint32_t dev, uint32_t dirty)
{
    uint32_t old = MEM32(dev + 8), first = MEM32(0x261178u) ? 3u : 0u;
    MEM32(dev + 8) = old & 0xFFFEFFBFu;
    for (uint32_t stage = first; stage < 4; stage++) {
        uint32_t state = 0x260DA0u + stage * 0x80u;
        for (uint32_t alpha = 0; alpha < 2; alpha++) {
            uint32_t op = MEM32(state + 0x30u + alpha * 0x10u);
            uint32_t a1 = MEM32(state + 0x34u + alpha * 0x10u);
            uint32_t a2 = MEM32(state + 0x38u + alpha * 0x10u);
            uint32_t a0 = MEM32(state + 0x3Cu + alpha * 0x10u);
            if (!alpha && op == 1) goto done;
            switch (MEM32(0x25AF28u + (op - 1u) * 4u)) {
            case 0x25AA8E: break;                         /* DISABLE */
            case 0x25AAD8: flush_combiner_arg(a2); break;
            case 0x25AAF0: flush_combiner_arg(a0); break;
            case 0x25AB05: case 0x25AB0E: case 0x25AB19:
            case 0x25AB33: case 0x25AB3E: case 0x25AB46:
            case 0x25AB61:
                flush_combiner_arg(a2); flush_combiner_arg(a0); break;
            case 0x25AB7C: case 0x25AC3A: case 0x25AC6D:
                flush_combiner_arg(a2); flush_combiner_arg(a0); break;
            case 0x25ABA8:
                flush_combiner_arg(op - 12u);
                flush_combiner_arg(a2); flush_combiner_arg(a0); break;
            case 0x25ABD9:
                flush_combiner_arg(a2); flush_combiner_arg(a0); break;
            case 0x25ABFE: flush_combiner_arg(a2); break;
            case 0x25AC66: case 0x25AC99:
                flush_combiner_arg(a2); flush_combiner_arg(a0); break;
            case 0x25ACD7:                               /* bump environment */
                MEM32(dev + 8) |= 0x10000u; break;
            case 0x25AD05:
                flush_combiner_arg(a2); flush_combiner_arg(a0);
                alpha = 1; break;
            case 0x25AD4A:
                flush_combiner_arg(a1); flush_combiner_arg(a2);
                flush_combiner_arg(a0); break;
            case 0x25AD73:
                flush_combiner_arg(a1); flush_combiner_arg(a2);
                flush_combiner_arg(a1); flush_combiner_arg(a0); break;
            }
        }
    }
done:
    if ((old | MEM32(dev + 8)) & 0x10000u) dirty |= 0x400Fu;
    return dirty;
}

/* CPU effects of 0x2578D0 and its deferred-state helpers. Point size, texture
 * modes, fog, texgen, light vectors and matrices only emitted GPU commands;
 * the native backend reads their source state directly at each draw. */
static void native_flush_state(uint32_t dev)
{
    uint32_t dirty = MEM32(DIRTY);
    if ((dirty & 0x800u) && !MEM32(dev + DEV_PIXEL_SHADER))
        dirty = flush_combiners(dev, dirty);
    for (uint32_t stage = 0; stage < 4; stage++) {
        if (dirty & (1u << stage)) {
            uint32_t s = 0x260DA4u + stage * 0x80u;
            uint32_t ctl = (MEM32(s + 24) << 26) | MEM32(s + 40) | MEM32(s + 32) | 0x3FFC0u;
            uint32_t mag = MEM32(s + 12), min = MEM32(s + 8);
            if (MEM32(dev + 0xF88u + stage * 4u)) ctl |= 0x40000000u;
            if ((mag > 2 || min > 2) && mag < 4 && min < 4 && MEM32(s + 28))
                ctl |= (MEM32(s + 28) - 1u) * 16u;
            MEM32(dev + 0x774u + stage * 4u) = ctl | 0x40000000u;
        }
    }
    if ((dirty & 0x1000u) && !(MEM8(MEM32(dev + DEV_VERTEX_SHADER) + 4) & 0x12u) &&
        MEM32(0x261138u) && (MEM32(dev + 0x1928u) & ~0x10u) != 1u) {
        uint32_t mask = 0;
        if (MEM32(0x261144u)) {
            for (uint32_t i = 0; i < 8; i++) mask = (mask << 2) | MEM32(0x261148u + i * 4u);
            uint32_t flags = MEM32(MEM32(dev + DEV_VERTEX_SHADER) + 4);
            if (!(flags & 0x400u)) mask &= 0xFFFFFFAAu;
            if (!(flags & 0x800u)) mask &= 0xFFFFFF55u;
            if (!(flags & 0x1000u)) mask &= 0xFFFFAAFFu;
            if (!(flags & 0x2000u)) mask &= 0xFFFF55FFu;
        }
        MEM32(dev + 0x7D0u) = mask;
    }
    MEM32(DIRTY) = dirty & 0xC0000070u;
}

static void native_begin(uint32_t dev)
{
    native_flush_state(dev);
    MEM32(dev + 8) |= 0x800u;
}

static void native_begin_push(uint32_t dev)
{
    uint32_t dirty = MEM32(DIRTY);
    MEM32(DIRTY) = dirty & 0xFFFFFFAFu;
    if (dirty & 0x3FFFFF8Fu) native_flush_state(dev);
    if (!(dirty & 0x40000000u) && ((dirty & 0x40u) || MEM32(dev + 0x20u)))
        MEM32(dev + 0x20u) = 0;
}

static float f32(uint32_t v) { float f; memcpy(&f, &v, 4); return f; }
static uint32_t u32f(float f) { uint32_t v; memcpy(&v, &f, 4); return v; }

/* Calls guest code (library helper, kernel export or title callback) with
 * stack arguments, keeping the callee-saved registers and the stack as they
 * were whatever its convention. */
static uint32_t guest_call(uint32_t va, int n, const uint32_t *args);
uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args)
{
    return guest_call(va, n, args);
}

static uint32_t guest_call(uint32_t va, int n, const uint32_t *args)
{
    uint32_t esp0 = g_esp, ebx = g_ebx, esi = g_esi, edi = g_edi, ebp = g_ebp, ecx = g_ecx;
    recomp_func_t fn;
    int i;
    /* Kernel exports by ordinal (0x80000000 | n, as the library's import
     * table holds them in the XBE): call through this title's thunk slot. */
    if ((va & 0xFFFFFE00u) == 0x80000000u) {
        static const uint32_t slots[][2] = {
            { 0x01, 0x277284u }, { 0x02, 0x277288u }, { 0x2F, 0x2771E4u }, { 0x64, 0x277298u },
            { 0xBB, 0x27711Cu }, { 0xBE, 0x2771D0u }, { 0xDB, 0x2771D8u }, { 0xE2, 0x277120u },
            { 0xEC, 0x2771D4u },
        };
        uint32_t ordinal = va & 0x1FFu, slot = 0;
        for (i = 0; i < (int)(sizeof slots / sizeof slots[0]); i++)
            if (slots[i][0] == ordinal) slot = slots[i][1];
        if (!slot) {
            fprintf(stderr, "[HLE-API] kernel ordinal %u has no thunk slot in this title\n", ordinal);
            return 0;
        }
        va = MEM32(slot);
    }
    fn = recomp_lookup_manual(va);
    if (!fn) fn = recomp_lookup(va);
    if (!fn) fn = recomp_lookup_kernel(va);
    if (!fn) {
        fprintf(stderr, "[HLE-API] no code at %08X\n", va);
        return 0;
    }
    for (i = n - 1; i >= 0; i--) {
        g_esp -= 4;
        MEM32(g_esp) = args[i];
    }
    g_esp -= 4;
    MEM32(g_esp) = 0;
    fn();
    g_esp = esp0;
    g_ebx = ebx; g_esi = esi; g_edi = edi; g_ebp = ebp; g_ecx = ecx;
    return g_eax;
}

/* Surface helpers: +0 common (refcount << 19, type), +4 data, +8 busy time,
 * +0xC format word (log2 width/height at bits 20/24, format at byte 1),
 * +0x10 explicit size (0 = power of two), +0x14 parent. */
static uint32_t surf_width(uint32_t s)
{
    uint32_t sz = MEM32(s + 0x10);
    return sz ? (sz & 0xFFFu) + 1 : 1u << ((MEM32(s + 0xC) >> 20) & 0xF);
}

static uint32_t surf_height(uint32_t s)
{
    uint32_t sz = MEM32(s + 0x10);
    return sz ? ((sz >> 12) & 0xFFFu) + 1 : 1u << ((MEM32(s + 0xC) >> 24) & 0xF);
}

/* 0x252E10 */
static uint32_t surf_pitch(uint32_t s)
{
    uint32_t n, fmt;
    if (MEM32(s + 0x10))
        return (MEM8(s + 0x13) + 1u) << 6;
    n = 1u << ((MEM32(s + 0xC) >> 20) & 0xF);
    fmt = MEM8(s + 0xD);
    if (fmt == 0xC)
        return n * 2;
    if (fmt > 0xD && fmt < 0x10)
        return n * 4;
    return ((MEM8(FORMAT_INFO + fmt) & 0x3Cu) * n) >> 3;
}

/* 0x257C20: AddRef */
static void res_addref(uint32_t r)
{
    uint32_t w = MEM32(r), parent = MEM32(r + 0x14);
    if ((w & 0x780000u) == 0 && (w & 0x70000u) == 0x50000u && parent)
        MEM32(parent) += 0x80000u;
    MEM32(r) += 0x80000u;
}

static int s_nofree;                  /* APICHECK: the original frees, not us */

static void res_free(uint32_t r)
{
    if (!s_nofree)
        hle_resource_free(r);
}

/* The inline release several setters use: busy until the next fence
 * (when busy_time), one reference less, freed at zero. */
static void res_unbind(uint32_t r, int busy_time)
{
    uint32_t w;
    if (!r)
        return;
    if (busy_time)
        MEM32(r + 8) = MEM32(dev_va() + DEV_FENCE_TIME);
    w = MEM32(r) - 0x80000u;
    MEM32(r) = w;
    if ((w & 0x78FFFFu) == 0)
        res_free(r);
}

/* 0x257E50: Release; the last reference frees through the library (0x257B40). */
static void res_release(uint32_t r)
{
    uint32_t w = MEM32(r);
    if ((w & 0x780000u) == 0x80000u) {
        uint32_t parent = MEM32(r + 0x14);
        if ((w & 0x70000u) == 0x50000u && parent) {
            uint32_t p = MEM32(parent) - 0x80000u;
            MEM32(parent) = p;
            if ((p & 0x78FFFFu) == 0)
                res_free(parent);
        }
        w = MEM32(r);
        if ((w & 0xFFFFu) == 0) {
            res_free(r);
            return;
        }
    }
    MEM32(r) = w - 0x80000u;
}

/* ------------------------------------------------------- SetVertexShader */

/* 0x254470: the program object for an FVF handle. */
static void build_fvf(uint32_t o, uint32_t fvf)
{
    uint32_t pos = fvf & 0xE, off = 0, i;
    MEM32(o + 4) = 0;
    MEM32(o + 0x10) = 0;
    for (i = 0; i < 16; i++)
        MEM32(o + 0x1C + 16 * i) = 2;
    if (pos == 4) {
        MEM32(o + 4) |= 2;
        MEM32(o + 0x1C) = 0x42;
        off = 0x10;
    } else if (pos) {
        MEM32(o + 0x1C) = 0x32;
        off = 0xC;
        if (pos > 5) {
            uint32_t w = ((pos - 6) >> 1) + 1;
            MEM32(o + 0x28) = 0xC;
            MEM32(o + 0x2C) = (w * 0x10) | 2;
            off = w * 4 + 0xC;
        }
    }
    if (fvf & 0x10) {
        MEM32(o + 0x38) = off;
        MEM32(o + 0x3C) = 0x32;
        off += 0xC;
    }
    if (fvf & 0x40) {
        MEM32(o + 0x48) = off;
        MEM32(o + 0x4C) = 0x40;
        MEM32(o + 4) |= 0x400;
        off += 4;
    }
    if (fvf & 0x80) {
        MEM32(o + 0x58) = off;
        MEM32(o + 0x5C) = 0x40;
        MEM32(o + 4) |= 0x800;
        off += 4;
    }
    {
        uint32_t n = (fvf >> 8) & 0xF, sizes = fvf >> 16, shift = 0, p = o + 0xAC;
        for (; n; n--) {
            uint32_t c = MEM32(0x25F2F8u + (sizes & 3) * 4);
            MEM32(p) = (c << 4) | 2;
            MEM32(p - 4) = off;
            off += c * 4;
            MEM32(o + 0x10) |= c << (shift & 0x1F);
            sizes >>= 2;
            p += 16;
            shift += 8;
        }
    }
}

/* 0x2545B0: a pass-through program's constant loads, into the library's
 * constant copy (and, for method 0xB00, the device's copy at +0x10A8). */
static void passthrough_constants(uint32_t prog)
{
    uint32_t dev = dev_va(), tok = prog + 0x114, dst = prog, aux = dev + 0x10A8;
    for (;;) {
        uint32_t w = MEM32(tok), n = w >> 18, src = tok + 4, i;
        if (n == 1) {
            dst = VSH_CONSTANTS + MEM32(src) * 16;
        } else if ((w & 0xFFFF) == 0xB00) {
            for (i = 0; i < n; i++)
                MEM32(aux + 4 * i) = MEM32(src + 4 * i);
            aux += 4 * n;
        } else {
            for (i = 0; i < n; i++)
                MEM32(dst + 4 * i) = MEM32(src + 4 * i);
            dst += 4 * n;
        }
        tok = src + 4 * n;
        if (!MEM32(tok))
            break;
    }
}

static void native_set_vertex_shader(uint32_t h)
{
    uint32_t dev = dev_va(), cur = MEM32(dev + DEV_VERTEX_SHADER);
    uint32_t cur_flags = MEM32(cur + 4), cur_strides = MEM32(cur + 0x10), o;

    if (!(h & 1)) {
        o = FVF_SHADER;
        build_fvf(o, h);
    } else {
        o = h - 1;
        if (MEM8(o + 4) & 0x10) {             /* pass-through program */
            if (!(MEM32(dev + DEV_FLAGS) & 0x10))
                passthrough_constants(o);
            MEM32(DIRTY) |= ((MEM32(o + 4) ^ cur_flags) & 0x13) ? 0x1070u : 0x70u;
            MEM32(dev + DEV_VERTEX_SHADER) = o;
            MEM32(dev + DEV_VERTEX_SHADER + 4) = h;
            MEM32(dev + DEV_VERTEX_SHADER + 8) = 0;
            return;
        }
    }
    MEM32(DIRTY) |= cur_strides != MEM32(o + 0x10) ? 0x470u : 0x70u;
    if (cur_flags != MEM32(o + 4))
        MEM32(DIRTY) |= 0x1600u;
    MEM32(dev + DEV_VERTEX_SHADER) = o;
    MEM32(dev + DEV_VERTEX_SHADER + 4) = h;
    MEM32(dev + DEV_VERTEX_SHADER + 8) = 0;
}

/* ------------------------------------------- texture, palette, streams */

/* 0x253800 SetTexture(stage, texture): device slot 0x3E2+stage, the stage's
 * format word (device 3+stage) and dirty bits. */
static void native_set_texture(uint32_t st, uint32_t tex)
{
    uint32_t dev = dev_va(), old = MEM32(dev + 4 * (0x3E2 + st)), fmt, key, cur;
    res_unbind(old, 1);
    MEM32(dev + 4 * (0x3E2 + st)) = tex;
    if (!tex) {
        MEM32(dev + 4 * (3 + st)) = 0x80000000u;
        MEM32(DIRTY) |= 0x4800u;
        return;
    }
    MEM32(tex) += 0x80000u;
    fmt = MEM32(tex + 12);
    key = fmt & 0x20F4u;
    cur = MEM32(dev + 4 * (3 + st));
    if (cur != key) {
        if ((key >> 8) & 0xFF) {
            key = fmt & 0xF4u;
            if ((fmt & 0xFF00u) > 0x29FFu && (fmt & 0xFF00u) < 0x3101u)
                key |= 0x40000000u;
        }
        MEM32(dev + 4 * (3 + st)) = key;
        MEM32(DIRTY) |= 0x4000u;
        if (!old)
            MEM32(DIRTY) |= 0x800u;
    }
}

/* 0x2539B0 SetPalette(stage, palette): device slot 0x3E6+stage. */
static void native_set_palette(uint32_t st, uint32_t pal)
{
    uint32_t dev = dev_va();
    res_unbind(MEM32(dev + 4 * (0x3E6 + st)), 1);
    MEM32(dev + 4 * (0x3E6 + st)) = pal;
    if (pal)
        MEM32(pal) += 0x80000u;
}

/* 0x254CE0 SetStreamSource(stream, vb, stride): the library's stream table
 * at 0x2636F8 (12 bytes each: stride, -, buffer). */
static void native_set_stream_source(uint32_t s, uint32_t vb, uint32_t stride)
{
    uint32_t slot = 0x2636F8u + 12 * s, old_stride;
    if (vb)
        MEM32(vb) += 0x80000u;
    res_unbind(MEM32(slot + 8), 1);
    old_stride = MEM32(slot);
    MEM32(slot) = stride;
    MEM32(slot + 8) = vb;
    MEM32(DIRTY) |= stride == old_stride ? 0x40u : 0x70u;
}

/* 0x253A40 SetIndices(ib, base vertex): device +0x7A0 / +0x1C, the index
 * data address at 0x260D94. */
static void native_set_indices(uint32_t ib, uint32_t base)
{
    uint32_t dev = dev_va();
    if (ib) {
        MEM32(ib) += 0x80000u;
        MEM32(0x260D94u) = MEM32(ib + 4);
    } else {
        MEM32(0x260D94u) = 0;
    }
    res_unbind(MEM32(dev + 0x7A0), 0);
    MEM32(dev + 0x7A0) = ib;
    MEM32(dev + 0x1C) = base;
}

/* ------------------------------------------------- viewport, PS constants */

static uint32_t trunc_f(float f);

/* 0x2521E0: out = a x b (row-major 4x4, float, the library's order). */
static void mat_mul(uint32_t out, uint32_t a, const float *b)
{
    float r[16];
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[4 * i + j] = MEMF(a + 16 * i) * b[j] + MEMF(a + 16 * i + 4) * b[4 + j] +
                           MEMF(a + 16 * i + 8) * b[8 + j] + MEMF(a + 16 * i + 12) * b[12 + j];
    for (i = 0; i < 16; i++)
        MEMF(out + 4 * i) = r[i];
}

/* 0x250210: the viewport transform (device +0xEE0 viewport, +0x95C/+0x960
 * target scale, +0x948 depth scale) composed into device +0x980 from the
 * projection at +0xCA0. */
static void viewport_transform(uint32_t dev)
{
    float sx = (float)MEM32(dev + 0xEE8) * MEMF(dev + 0x95C) * 0.5f;
    float sy = (float)MEM32(dev + 0xEEC) * MEMF(dev + 0x960) * -0.5f;
    float ty = -sy;
    float sz = (MEMF(dev + 0xEF4) - MEMF(dev + 0xEF0)) * MEMF(dev + 0x948);
    float tz = MEMF(dev + 0x948) * MEMF(dev + 0xEF0);
    float w = 1.0f, v[16];
    if (MEM32(0x2611DCu) == 2) {
        w = MEMF(dev + 0x944) * MEMF(dev + 0x948);
        sy = sy * w;
        sz = w * sz;
        sx = w * sx;
        ty = w * ty;
        tz = w * tz;
    }
    memset(v, 0, sizeof v);
    v[0] = sx; v[5] = sy; v[10] = sz;
    v[12] = sx; v[13] = ty; v[14] = tz; v[15] = w;
    mat_mul(dev + 0x980, dev + 0xCA0, v);
    MEM32(DIRTY) |= 0x200u;
}

/* 0x2535D0 SetViewport(D3DVIEWPORT8 *): clipped to the render target (the
 * front buffer's size while drawing to the scaled back buffer) and to the
 * scaled size, stored at device +0xEE0; scissors reset (0x253D20(0, 0, 0):
 * no rects); the transform. */
static void native_set_viewport(uint32_t vp)
{
    uint32_t dev = dev_va();
    if (vp) {
        uint32_t s = MEM32(dev + DEV_RENDER_TARGET), x, y, r, b, lim;
        if (s == MEM32(dev + DEV_BACK_BUFFER))
            s = MEM32(dev + DEV_FRONT_BUFFER);
        x = MEM32(vp);
        y = MEM32(vp + 4);
        r = x + MEM32(vp + 8);
        if (surf_width(s) <= r)
            r = surf_width(s);
        b = y + MEM32(vp + 12);
        if (surf_height(s) <= b)
            b = surf_height(s);
        lim = MEM32(dev + 0x954);
        if (lim < trunc_f((float)(int32_t)r * MEMF(dev + 0x95C) + 0.5f))
            r = trunc_f((float)(int32_t)lim / MEMF(dev + 0x95C));
        lim = MEM32(dev + 0x958);
        if (lim < trunc_f((float)(int32_t)b * MEMF(dev + 0x960) + 0.5f))
            b = trunc_f((float)(int32_t)lim / MEMF(dev + 0x960));
        MEM32(dev + 0xEE0) = x;
        MEM32(dev + 0xEEC) = b - y;
        MEM32(dev + 0xEE4) = y;
        MEM32(dev + 0xEE8) = r - x;
        MEM32(dev + 0xEF0) = MEM32(vp + 16);
        MEM32(dev + 0xEF4) = MEM32(vp + 20);
        MEM32(DIRTY) |= 0x100u;
    }
    MEM32(dev + 0x1C00) = 0;                 /* scissor rect count, exclusive */
    MEM32(dev + 0x1C04) = 0;
    viewport_transform(dev);
}

/* 0x25E0C0 SetPixelShaderConstant(reg, data, count): each constant clamped
 * to the library's [lo, hi] (0x307090 / 0x2DBD50), scaled (0x2DBD60) and
 * packed A8R8G8B8 into device +0x8E4+4*reg; the combiner's colour-constant
 * render states that the current shader maps to that register (table
 * 0x2DBD70 against its definition's +0xE4/+0xE8/+0xEC) get it too. */
/* Combiner constant slots written by SetPixelShaderConstant since the last
 * SetPixelShader: bits 0-7 C0 of stages 0-7, 8-15 C1, 16-17 final C0/C1.
 * With device flag 0x10 the library never copies a shader's definition to
 * 0x260FA0, so the renderer takes the rest from the definition itself. */
uint32_t hle_ps_constants_written;

static int native_set_ps_constant(uint32_t reg, uint32_t src, uint32_t count)
{
    uint32_t dev = dev_va(), ps = MEM32(dev + DEV_PIXEL_SHADER), def;
    if (!ps)
        return 0;
    def = MEM32(ps + 8);
    for (; count; count--, reg++, src += 16) {
        uint32_t q[4], u, m, k;
        int c;
        for (c = 0; c < 4; c++) {
            float v = MEMF(src + 4 * c), hi = MEMF(0x2DBD50u + 4 * c), lo = MEMF(0x307090u + 4 * c);
            v = v < hi ? v : hi;                 /* minps */
            v = v > lo ? v : lo;                 /* maxps */
            /* cvtss2si: round to nearest, as the Xbox CPU does. (The lifted
             * library truncates there -- a cast in the generated C -- so
             * APICHECK can show these bytes 1 lower in the original.) */
            q[c] = (uint32_t)lrintf(v * MEMF(0x2DBD60u + 4 * c));
        }
        u = q[0] << 16 | q[1] << 8 | q[2] | q[3] << 24;
        MEM32(dev + 4 * (0x239 + reg)) = u;
        m = MEM32(0x2DBD70u + 4 * reg);
        for (k = 0; k < 8; k++)
            if ((((m ^ MEM32(def + 0xE4)) >> (4 * k)) & 0xF) == 0)
                MEM32(0x260FC8u + 4 * k) = u, hle_ps_constants_written |= 1u << k;
        for (k = 0; k < 8; k++)
            if ((((m ^ MEM32(def + 0xE8)) >> (4 * k)) & 0xF) == 0)
                MEM32(0x260FE8u + 4 * k) = u, hle_ps_constants_written |= 0x100u << k;
        for (k = 0; k < 2; k++)
            if ((((m ^ MEM32(def + 0xEC)) >> (4 * k)) & 0xF) == 0)
                MEM32(0x26104Cu + 4 * k) = u, hle_ps_constants_written |= 0x10000u << k;
    }
    return 1;
}

/* --------------------------------------------- render-state setters */

/* The SetRenderState_* bodies that keep one global (0x2611C8..0x261234) and
 * otherwise only push. Dirty bits where the original sets them. */
static const struct { int id; uint32_t global, dirty; } k_rs_store[] = {
    { HLE_D3DDevice_SetRenderState_EdgeAntiAlias,        0x2611FCu, 0 },
    { HLE_D3DDevice_SetRenderState_ShadowFunc,           0x261210u, 0 },
    { HLE_D3DDevice_SetRenderState_FogColor,             0x2611C8u, 0 },
    { HLE_D3DDevice_SetRenderState_CullMode,             0x2611ECu, 0 },
    { HLE_D3DDevice_SetRenderState_FrontFace,            0x2611E8u, 0 },  /* + CullMode(same) */
    { HLE_D3DDevice_SetRenderState_LogicOp,              0x2611F8u, 0 },
    { HLE_D3DDevice_SetRenderState_FillMode,             0x2611CCu, 0 },
    { HLE_D3DDevice_SetRenderState_StencilEnable,        0x2611E0u, 0 },
    { HLE_D3DDevice_SetRenderState_StencilFail,          0x2611E4u, 0 },
    { HLE_D3DDevice_SetRenderState_OcclusionCullEnable,  0x261224u, 0 },
    { HLE_D3DDevice_SetRenderState_StencilCullEnable,    0x261228u, 0 },
    { HLE_D3DDevice_SetRenderState_MultiSampleMask,      0x261204u, 0 },
    { HLE_D3DDevice_SetRenderState_SampleAlpha,          0x261218u, 0 },
    { HLE_D3DDevice_SetRenderState_YuvEnable,            0x261220u, 0 },
    { HLE_D3DDevice_SetRenderState_TextureFactor,        0x2611F0u, 0 },
    { HLE_D3DDevice_SetRenderState_NormalizeNormals,     0x2611D8u, 0x200u },
    { HLE_D3DDevice_SetRenderState_RopZCmpAlwaysRead,    0x26122Cu, 0 },
    { HLE_D3DDevice_SetRenderState_RopZRead,             0x261230u, 0 },
    { HLE_D3DDevice_SetRenderState_DoNotCullUncompressed, 0x261234u, 0 },
    { HLE_D3DDevice_SetRenderState_Dxt1NoiseEnable,      0x26121Cu, 0 },
};

static int rs_index(int id)
{
    unsigned i;
    for (i = 0; i < sizeof k_rs_store / sizeof k_rs_store[0]; i++)
        if (k_rs_store[i].id == id)
            return (int)i;
    return -1;
}

static void native_set_rs(int idx, uint32_t v)
{
    uint32_t dev = dev_va();
    int id = k_rs_store[idx].id;
    if (id == HLE_D3DDevice_SetRenderState_Dxt1NoiseEnable) {
        /* 0x2505E0: device flag bit 0 follows it on DXT1 render targets. */
        uint32_t want = (MEM8(FORMAT_INFO + MEM8(MEM32(dev + DEV_RENDER_TARGET) + 0xD)) & 0x3C) == 0x20 ? v : 0;
        if (want != (MEM32(dev + DEV_FLAGS) & 1))
            MEM32(dev + DEV_FLAGS) ^= 1;
    }
    MEM32(k_rs_store[idx].global) = v;
    MEM32(DIRTY) |= k_rs_store[idx].dirty;
    if (id == HLE_D3DDevice_SetRenderState_RopZCmpAlwaysRead || id == HLE_D3DDevice_SetRenderState_RopZRead ||
        id == HLE_D3DDevice_SetRenderState_DoNotCullUncompressed) {
        /* 0x250E70: the ROP/Z control words at device +0x2428/+0x242C. */
        uint32_t w = MEM32(dev + 0x2428) & ~8u;
        if (MEM32(0x261234u)) w |= 8u;
        MEM32(dev + 0x2428) = w;
        w = MEM32(dev + 0x242C) & 0xE7EFFFFFu;
        if (MEM32(0x26122Cu)) w |= 0x100000u;
        if (MEM32(0x261230u)) w |= 0x8000000u;
        MEM32(dev + 0x242C) = w;
    }
}

static int s_rs_check_idx;

/* -------------------------------------------------------- SetPixelShader */

static void native_set_pixel_shader(uint32_t ps)
{
    uint32_t dev = dev_va(), def, v, i;

    MEM32(dev + DEV_PIXEL_SHADER) = ps;
    hle_ps_constants_written = 0;
    if (!ps) {
        MEM32(DIRTY) |= MEM32(dev + DEV_PIXEL_SHADER + 4) ? 0x6800u : 0x4800u;
        return;
    }
    def = MEM32(ps + 8);
    v = MEM32(def + 0x24) | MEM32(def + 0x20);
    if (!v && MEM32(dev + DEV_PIXEL_SHADER + 4))
        MEM32(DIRTY) |= 0x2000u;
    MEM32(dev + DEV_PIXEL_SHADER + 4) = v;
    MEM32(dev + DEV_PIXEL_SHADER + 8) = MEM32(def + 0xEC) & 0x100;
    MEM32(dev + DEV_PIXEL_SHADER + 12) = MEM32(def + 0xD8);
    MEM32(DIRTY) |= 0x4000u;
    if (!(MEM32(dev + DEV_FLAGS) & 0x10)) {
        for (i = 0; i < 0x39; i++)
            MEM32(PS_RENDER_STATE + 4 * i) = MEM32(def + 4 * i);
        MEM32(0x2611C0u) = MEM32(def + 0xD8);
    }
}

/* ------------------------------------------------------- SetRenderTarget */

/* 0x258200: the surface-format word for a colour/depth pair. The switch
 * tables are the library's own data. */
static uint32_t surface_format(uint32_t rt, uint32_t z)
{
    uint32_t fmt = MEM8(rt + 0xD), tgt = MEM32(0x2582BCu + 4u * MEM8(0x2582E5u + fmt));
    uint32_t v, lin = (((MEM32(rt + 0xC) & 0xF00000u) | 0x2000u) >> 4) | (MEM32(rt + 0xC) & 0xF000000u);
    switch (tgt) {
    case 0x258216u: v = 8 | lin; break;
    case 0x25823Du: v = 8 | 0x100; break;
    case 0x258269u: v = 4 | lin; break;
    case 0x258270u: v = 4 | 0x100; break;
    case 0x258277u: v = 3 | lin; break;
    case 0x25827Eu: v = 3 | 0x100; break;
    case 0x258285u: v = 1 | lin; break;
    case 0x25828Cu: v = 1 | 0x100; break;
    case 0x258293u: v = 9 | 0x100; break;
    case 0x25829Au: v = 0xA | 0x100; break;
    default:
        fprintf(stderr, "[HLE-API] surface format %02X: unknown case %08X\n", fmt, tgt);
        v = 0;
        break;
    }
    if (z) {
        uint32_t zi = MEM8(z + 0xD) - 0x2Au;
        if (zi > 7)
            return v;
        return v | (MEM32(0x258304u + 4u * MEM8(0x25830Cu + zi)) == 0x2582A1u ? 0x20u : 0x10u);
    }
    return v | ((MEM8(FORMAT_INFO + fmt) & 0x3C) == 0x20 ? 0x20u : 0x10u);
}

/* 0x258320: the depth format's largest Z value. */
static void depth_scale(uint32_t z, uint32_t out)
{
    uint32_t zi = MEM8(z + 0xD) - 0x2Au, tgt;
    if (zi > 7)
        return;
    tgt = MEM32(0x258368u + 4u * zi);
    switch (tgt) {
    case 0x258332u: MEM32(out) = 0x477FFF00u; break;
    case 0x25833Fu: MEM32(out) = 0x4B7FFFFFu; break;
    case 0x25834Cu: MEM32(out) = 0x43FFF800u; break;
    case 0x258359u: MEM32(out) = 0x7149F2CAu; break;
    default: break;
    }
}

static uint32_t trunc_f(float f)
{
    return (uint32_t)(int32_t)f;
}

/* 0x250F40 (ecx = device, edx = viewport): the back buffer scale and
 * multisample state, then SetViewport through its routed entry. */
static void apply_target_scale(uint32_t dev, uint32_t viewport)
{
    uint32_t ms = MEM32(0x261208u), rt = MEM32(dev + DEV_RENDER_TARGET);
    uint32_t w, h, fmt, flags;
    float sx, sy, m;

    if (rt == MEM32(dev + DEV_BACK_BUFFER)) {
        uint32_t fr = MEM32(dev + DEV_FRONT_BUFFER), bb = rt;
        sy = f32(MEM32(dev + 0x970));
        sx = f32(MEM32(dev + 0x96C));
        w = trunc_f((float)surf_width(fr) * sx + 0.5f);
        h = trunc_f((float)surf_height(fr) * sy + 0.5f);
        if (surf_width(bb) <= w)
            w = surf_width(bb);
        if (h >= surf_height(bb))
            h = surf_height(bb);
    } else {
        sx = sy = 1.0f;
        ms = MEM32(0x26120Cu);
        w = surf_width(rt);
        h = surf_height(rt);
    }
    fmt = MEM32(dev + DEV_SURFACE_FMT);
    flags = MEM32(dev + DEV_FLAGS) & ~0x8000u;
    MEM32(dev + DEV_FLAGS) = flags;
    if (ms && !(fmt & 0x200)) {
        sx *= 0.5f;
        w = (w + 1) >> 1;
        MEM32(dev + DEV_FLAGS) = flags | 0x8000u;
        if (ms == 2) {
            sy *= 0.5f;
            h = (h + 1) >> 1;
        }
    }
    MEM32(dev + 0x960) = u32f(sy);
    MEM32(dev + 0x95C) = u32f(sx);
    MEM32(dev + 0x954) = w;
    MEM32(dev + 0x958) = h;
    m = sx < sy ? sx : sy;
    if (f32(MEM32(dev + 0x964)) != m) {
        MEM32(dev + 0x964) = u32f(m);
        MEM32(dev + 0x968) = MEM32(0x25EDCCu + 4u * trunc_f(m + m + 0.5f));
        MEM32(DIRTY) |= 0x10Fu;
    }
    native_set_viewport(viewport);
}

static void native_set_render_target(uint32_t rt, uint32_t z)
{
    uint32_t dev = dev_va(), old, p, dxt;

    if (!rt)
        rt = MEM32(dev + DEV_RENDER_TARGET);
    res_addref(rt);
    old = MEM32(dev + DEV_RENDER_TARGET);
    if (old) {
        p = MEM32(old + 0x14) ? MEM32(old + 0x14) : old;
        MEM32(p + 8) = MEM32(dev + DEV_FENCE_TIME);
        res_release(old);
    }
    old = MEM32(dev + DEV_DEPTH_STENCIL);
    MEM32(dev + DEV_RENDER_TARGET) = rt;
    if (old) {
        p = MEM32(old + 0x14) ? MEM32(old + 0x14) : old;
        MEM32(p + 8) = MEM32(dev + DEV_FENCE_TIME);
        res_release(old);
    }
    MEM32(dev + DEV_DEPTH_STENCIL) = z;
    if (z) {
        res_addref(z);
        depth_scale(z, dev + DEV_ZSCALE);
        MEM32(dev + DEV_ZPITCH) = surf_pitch(z);
    }
    if ((MEM32(dev + DEV_FLAGS) & 0x100) && rt == MEM32(dev + DEV_BACK_BUFFER))
        MEM32(dev + DEV_FLAGS) &= ~0x100u;
    MEM32(dev + DEV_SURFACE_FMT) = surface_format(rt, z);
    apply_target_scale(dev, 0x25F040u);
    /* 0x2505E0, SetRenderState_Dxt1NoiseEnable(current value) */
    dxt = MEM32(0x26121Cu);
    p = (MEM8(FORMAT_INFO + MEM8(MEM32(dev + DEV_RENDER_TARGET) + 0xD)) & 0x3C) == 0x20 ? dxt : 0;
    if (p != (MEM32(dev + DEV_FLAGS) & 1))
        MEM32(dev + DEV_FLAGS) ^= 1;
    MEM32(0x26121Cu) = dxt;
}

/* ------------------------------------------------------------ miniport */

/* 0x2592B0: complete the queued flips that are due this vblank. The CRTC
 * start address, palette upload and PGRAPH flip bit it also wrote only made
 * the flip visible on the Xbox. */
static int complete_flips(void)
{
    int n = 0;
    uint32_t k = MP(0x6F) & 1;
    while (MP(0x5D + 3 * k)) {
        if (MP(0x70) != MP(0x5E + 3 * k))
            return n;
        MP(0x5D + 3 * k) = 0;
        MEM32(0x2639F4u) = MP(0x5F + 3 * k);
        if (MP(0x1F7 + (MP(0x6F) & 1)) == 1)
            MP(0x1F7 + (MP(0x6F) & 1)) = 0;
        MP(0x6F) = MP(0x6F) + 1;
        k = MP(0x6F) & 1;
        n++;
    }
    return n;
}

/* 0x259690 case 1: queue a flip. v = software-method word: bits 5.. hold
 * the front buffer address | immediate (8) | interval (0..7). */
static void queue_flip(uint32_t v)
{
    uint32_t p = v >> 5, interval = p & 7, k;
    int32_t target = (int32_t)(MP(0x72) + interval), now = (int32_t)MP(0x70), ahead = target - now;
    uint32_t missed = 0, until = 0;

    if (ahead < 1) {
        if (interval)
            missed = (uint32_t)(1 - ahead);
        target = (p & 8) ? now : now + 1;
        ahead = target - now;
    }
    MP(0x71) = (uint32_t)target + interval;
    MP(0x72) = (uint32_t)target;
    if (MP(0x75)) {
        int32_t t = (int32_t)(MP(0x75) * (uint32_t)ahead + MP(0x76) - (uint32_t)__rdtsc());
        until = t < 0 ? 0 : (uint32_t)t;
    }
    k = MP(0x73) & 1;
    MP(0x5D + 3 * k) = 1;
    MP(0x5E + 3 * k) = (uint32_t)target;
    MP(0x5F + 3 * k) = p & ~0xFu;
    MP(0x73) = MP(0x73) + 1;
    complete_flips();
    if (MP(99)) {
        uint32_t data, cb = MP(99);
        g_esp -= 20;
        data = g_esp;
        MEM32(data + 0) = MP(0x73);
        MEM32(data + 4) = (uint32_t)target;
        MEM32(data + 8) = missed;
        MEM32(data + 12) = until;
        MEM32(data + 16) = MP(0x75) * interval;
        guest_call(cb, 1, &data);
        g_esp += 20;
    }
}

/* 0x259390: the vblank service, called by the runtime's vblank timer. */
void hle_api_vblank(void)
{
    uint32_t t = (uint32_t)__rdtsc(), flags = 0;
    int n;

    if (MP(0x76))
        MP(0x75) = t - MP(0x76);
    MP(0x76) = t;
    MP(0x70) = MP(0x70) + 1;
    n = complete_flips();
    if (n)
        flags = 1;
    else if (MP(0x70) == MP(0x71)) {
        MP(0x71) = MP(0x71) + 1;
        flags = 2;
    }
    if (!(MP(0x6D) & 0x1000000u))
        MP(0x74) = (MP(2) & 0xC0000000u) == 0x40000000u ? 0 : 1;
    {
        uint32_t args[3] = { MINIPORT + 0x194, 1, 0 };
        guest_call(MEM32(0x277148u), 3, args);     /* KeSetEvent(vblank event) */
    }
    if (MP(100)) {
        uint32_t data, cb = MP(100);
        g_esp -= 12;
        data = g_esp;
        MEM32(data + 0) = MP(0x70);
        MEM32(data + 4) = MP(0x6F);
        MEM32(data + 8) = flags;
        guest_call(cb, 1, &data);
        g_esp += 12;
    }
}

int hle_api_native_vblank(void)
{
    return api_mode() != 0;
}

/* ------------------------------------------------------------------ Swap */

/* 0x251CD0(0) without the commands: the fence is complete as soon as it
 * exists, since the backend executed everything before it synchronously. */
static uint32_t native_fence(uint32_t dev)
{
    uint32_t t = MEM32(dev + DEV_FENCE_TIME), k = (t >> 1) & 0x3F;
    MEM32(dev + DEV_FENCE_SLOTS + 8 * k) = t;
    MEM32(dev + DEV_FENCE_SLOTS + 8 * k + 4) = MEM32(dev + DEV_PUT);
    MEM32(dev + DEV_FENCE_RESET) = 0;
    MEM32(dev + DEV_FENCE_TIME) = t + 2;
    MEM32(MEM32(dev + DEV_FENCE_DONE)) = t;
    return t;
}

/* 0x251CD0 with flag 1: a fence that is also recorded in the device's
 * ring of reservation points (+0x34 index, +0x38 mask, +0x48 table). */
static void native_ring_fence(uint32_t dev)
{
    uint32_t put = MEM32(dev + DEV_PUT);
    uint32_t t = native_fence(dev), k = (MEM32(dev + 0x34) + 1u) & MEM32(dev + 0x38);
    MEM32(dev + 0x34) = k;
    MEM32(MEM32(dev + 0x48) + k * 8u) = t;
    MEM32(MEM32(dev + 0x48) + k * 8u + 4u) = put;
}

uint32_t hle_sync_scan_pos(void);

static int s_checking;                /* RECOMP_HLE_APICHECK: the original flips */

/* 0x259690, the graphics ISR's software-method handler, for the kinds that
 * are CPU work: 1 queues a flip, 5 signals the BlockOnTime event, 0xB is
 * nothing. 8/9 only write PGRAPH and are dropped by the caller. The rest
 * (callbacks, deferred copies) return 0 and still go to the library body. */
int hle_api_software_method(uint32_t v)
{
    if (!api_mode() || s_checking)
        return 0;
    switch (v & 0x1Fu) {
    case 1:
        queue_flip(v);
        return 1;
    case 5: {
        uint32_t args[3] = { MINIPORT + 0x1A4u, 1, 0 };
        guest_call(MEM32(0x277148u), 3, args);     /* KeSetEvent */
        return 1;
    }
    case 0xB:
        return 1;
    default:
        return 0;
    }
}


static uint32_t flip_word(uint32_t dev)
{
    uint32_t iv = MEM32(PRESENT_INTERVAL), v = MEM32(MEM32(dev + DEV_FRONT_BUFFER) + 4);
    uint32_t m = iv ? iv : 1;
    if (m & 1) v |= 1;
    if (m & 2) v |= 2;
    if (m & 4) v |= 3;
    if ((int32_t)m < 0) v |= 8;
    return (v << 5) | 1;
}

static void flip(uint32_t dev)
{
    if (!s_checking)
        queue_flip(flip_word(dev));
}

static void set_busy(uint32_t r, uint32_t t, int via_parent)
{
    if (!r)
        return;
    if (via_parent && MEM32(r + 0x14))
        r = MEM32(r + 0x14);
    MEM32(r + 8) = t;
}

/* 0x25DB50: the frame's fence; the front buffer is busy until then. */
static void frame_fence(uint32_t dev)
{
    set_busy(MEM32(dev + DEV_FRONT_BUFFER), MEM32(dev + DEV_FENCE_TIME), 1);
    MEM32(dev + DEV_FRAME_FENCE + 4 * (MEM32(dev + DEV_SWAP_COUNT) & 1)) = native_fence(dev);
}

/* Commands other library code left since the last kick (normally none:
 * every non-synchronising call is rewound) still go through KickOff. */
static void drain_pending(uint32_t dev)
{
    if (hle_sync_scan_pos() != MEM32(dev + DEV_PUT)) {
        uint32_t ecx = g_ecx;
        g_ecx = dev;
        g_esp -= 4; MEM32(g_esp) = 0;
        sub_00251BB0();
        g_ecx = ecx;
    }
}

static int swap_supported(uint32_t dev, uint32_t flags)
{
    uint32_t n = MEM32(dev + DEV_BUFFER_COUNT);
    if (MEM32(dev + 0x8D4) || MEM32(dev + 0x8D8)) { s_fallback_why[0]++; return 0; }
    if (!flags)
        flags = 5;
    if ((MEM32(dev + DEV_FLAGS) & 0x4000) ? flags != 5 : !(flags & 4)) { s_fallback_why[2]++; return 0; }
    if (n < 2 || n > 3)                            { s_fallback_why[4]++; return 0; }
    return 1;
}

/* Swap(0 or 5) with a scaled or multisampled back buffer (device flag 0x4000,
 * 0x25D850): the library filters the back buffer into the front buffer with
 * its own draw, then flips the front buffer. The backend presents the render
 * target itself, so of the blit only what it leaves behind in CPU state is
 * kept: its SetRenderTarget/SetTexture pairs mark the surfaces and textures
 * they replace busy until the next fence, and its state flush (0x2578D0)
 * leaves the dirty bits at (bits & 0xC0000070) | 0xFF1000. Every state value
 * it changes for the blit it restores itself. */
static void swap_scaled(uint32_t dev)
{
    uint32_t n = MEM32(dev + DEV_BUFFER_COUNT), t, i;

    native_fence(dev);
    MEM32(dev + DEV_SWAP_COUNT) += 1;
    if (n == 3) {
        uint32_t a = MEM32(dev + DEV_FRONT_BUFFER), b = MEM32(dev + DEV_FRONT_BUFFER + 4);
        uint32_t d = MEM32(b + 4), f = MEM32(b + 8);
        MEM32(b + 4) = MEM32(a + 4);
        MEM32(b + 8) = MEM32(a + 8);
        MEM32(a + 4) = d;
        MEM32(a + 8) = f;
    }
    /* 0x25D5D0: the state the blit replaces, saved in the device (it stays
     * there after the Swap). */
    MEM32(dev + 0x1A90) = MEM32(dev + DEV_RENDER_TARGET);
    MEM32(dev + 0x1A94) = MEM32(dev + DEV_DEPTH_STENCIL);
    for (i = 0; i < 4; i++)
        MEM32(dev + 0x1A98 + 4 * i) = MEM32(dev + 4 * (0x3E2 + i));
    for (i = 0; i < 6; i++)
        MEM32(dev + 0x1AA8 + 4 * i) = MEM32(dev + 0xEE0 + 4 * i);
    /* 0x25D330 sets the blit's render states from the library's table at
     * 0x25FFE8 ({state, value} x 20); 0x25D450 restores them, except -- with
     * device flag 0x10 -- states below 0x5C, which stay as the blit left them. */
    if (MEM32(dev + DEV_FLAGS) & 0x10)
        for (i = 0; i < 20; i++) {
            uint32_t st = MEM32(0x25FFE8u + 8 * i);
            if (st < 0x5C)
                MEM32(PS_RENDER_STATE + 4 * st) = MEM32(0x25FFECu + 8 * i);
        }
    t = MEM32(dev + DEV_FENCE_TIME);
    set_busy(MEM32(dev + DEV_RENDER_TARGET), t, 1);
    set_busy(MEM32(dev + DEV_DEPTH_STENCIL), t, 1);
    for (i = 0; i < 4; i++)
        set_busy(MEM32(dev + 4 * (0x3E2 + i)), t, 0);
    set_busy(MEM32(dev + DEV_FRONT_BUFFER), t, 1);
    set_busy(MEM32(dev + DEV_BACK_BUFFER), t, 0);
    if (n < 3)
        flip(dev);
    MEM32(DIRTY) = (MEM32(DIRTY) & 0xC0000070u) | s_blit_dirty;
    if (n == 3)
        flip(dev);
    frame_fence(dev);
    MEM32(DIRTY) |= 0xFF1000u;
}

/* Swap with the back buffer shown directly (0x25DA40): rotate the buffers'
 * memory (data, +4; busy time, +8) one step, the newest back buffer becoming
 * the front buffer, and flip it. */
static void swap_direct(uint32_t dev)
{
    uint32_t n = MEM32(dev + DEV_BUFFER_COUNT), i, first, d, f, o, iv;

    native_fence(dev);
    MEM32(dev + DEV_SWAP_COUNT) += 1;
    first = MEM32(dev + DEV_BUFFER_COUNT + 4 * n);
    d = MEM32(first + 4);
    f = MEM32(first + 8);
    o = first;
    for (i = n - 1; i > 0; i--) {
        uint32_t prev = MEM32(dev + DEV_BUFFER_COUNT + 4 * i);
        MEM32(o + 4) = MEM32(prev + 4);
        MEM32(o + 8) = MEM32(prev + 8);
        o = prev;
    }
    MEM32(o + 4) = d;
    MEM32(o + 8) = f;
    iv = MEM32(PRESENT_INTERVAL);
    if (!(MEM32(dev + DEV_RENDER_TARGET) == MEM32(dev + DEV_BACK_BUFFER) || iv == 0x80000000u))
        MEM32(dev + DEV_FLAGS) |= 0x100;
    flip(dev);
    frame_fence(dev);
    MEM32(DIRTY) |= 0xFF1000u;
}

static void native_swap(uint32_t dev)
{
    if (MEM32(dev + DEV_FLAGS) & 0x4000)
        swap_scaled(dev);
    else
        swap_direct(dev);
}

/* ------------------------------------------------------ synchronisation */

static int fence_done(uint32_t dev, uint32_t t)
{
    uint32_t cur = MEM32(dev + DEV_FENCE_TIME);
    return cur - MEM32(MEM32(dev + DEV_FENCE_DONE)) <= cur - t;
}

/* 0x251D90 BlockOnTime(t): every fence completes when it is inserted (the
 * backend has executed what came before it), so the only wait left is for
 * the current time -- which inserting a fence satisfies at once. */
static void native_block(uint32_t dev, uint32_t t, uint32_t flags)
{
    drain_pending(dev);
    if (fence_done(dev, t))
        return;
    if (t == MEM32(dev + DEV_FENCE_TIME)) {
        native_fence(dev);
        if (fence_done(dev, t))
            return;
    }
    /* A time issued by another CPU thread must still wait for that thread;
     * never report a future fence as complete merely to avoid a wait. */
    while (!fence_done(dev, t)) {
        extern void guest_cpu_yield(void);
        guest_cpu_yield();
        drain_pending(dev);
    }
    (void)flags;
}

/* 0x257BE0: the device holds a reference (bits 19..22) to it or its parent. */
static int res_device_held(uint32_t r)
{
    uint32_t w = MEM32(r), p;
    if (w & 0x780000u)
        return 1;
    if ((w & 0x70000u) != 0x50000u || !(p = MEM32(r + 0x14)))
        return 0;
    return (MEM32(p) & 0x780000u) != 0;
}

/* 0x2520C0 BlockUntilNotBusy: held by the device -> wait for everything so
 * far; otherwise for the resource's own busy time. */
static void native_block_until_not_busy(uint32_t r)
{
    uint32_t dev = dev_va(), t;
    if (!dev)
        return;
    if ((MEM32(r) & 0x70000u) == 0x50000u && MEM32(r + 0x14) && res_device_held(r)) {
        native_block(dev, MEM32(dev + DEV_FENCE_TIME), 2);
        return;
    }
    if ((MEM32(r) & 0x70000u) == 0x50000u && MEM32(r + 0x14))
        r = MEM32(r + 0x14);
    t = MEM32(r + 8);
    if (res_device_held(r)) {
        native_block(dev, MEM32(dev + DEV_FENCE_TIME), 2);
        return;
    }
    if (t)
        native_block(dev, t, 0x10);
}

/* 0x258020 VertexBuffer Lock2(vb, flags): the NV2A vertex-cache flush is
 * dropped; unless NOOVERWRITE/READONLY (0xA0), wait as 0x252140 does. */
static uint32_t native_lock2(uint32_t vb, uint32_t flags)
{
    uint32_t dev = dev_va();
    if (!(flags & 0xA0) && dev) {
        if (MEM32(vb) & 0x780000u)
            native_block(dev, MEM32(dev + DEV_FENCE_TIME), 2);
        else if (MEM32(vb + 8))
            native_block(dev, MEM32(vb + 8), 0x10);
    }
    return MEM32(vb + 4) | 0x80000000u;
}

/* 0x257CA0 Release: the internal (low 16-bit) reference; the last one frees
 * the resource unless the device still holds it. */
static uint32_t native_release(uint32_t r)
{
    uint32_t w = MEM32(r);
    if ((w & 0xFFFFu) == 1) {
        if ((w & 0x70000u) == 0x50000u && MEM32(r + 0x14))
            native_release(MEM32(r + 0x14));
        w = MEM32(r);
        if ((w & 0x780000u) == 0) {
            res_free(r);
            return 0;
        }
    }
    MEM32(r) = w - 1;
    return (w - 1) & 0xFFFFu;
}

void hle_api_report(void)
{
    if (!hle_api_native_calls && !hle_api_fallbacks)
        return;
    fprintf(stderr, "[HLE-API] native %llu, original %llu (Swap: callbacks %llu, scaled %llu, "
                    "no-flip %llu, persist %llu, buffers %llu)\n",
            (unsigned long long)hle_api_native_calls, (unsigned long long)hle_api_fallbacks,
            (unsigned long long)s_fallback_why[0], (unsigned long long)s_fallback_why[1],
            (unsigned long long)s_fallback_why[2], (unsigned long long)s_fallback_why[3],
            (unsigned long long)s_fallback_why[4]);
    fprintf(stderr, "[HLE-FLUSH] Begin %llu, BeginPush %llu, EndPush %llu, original reservation %llu\n",
            (unsigned long long)s_begin_calls, (unsigned long long)s_begin_push_calls,
            (unsigned long long)s_end_push_calls, (unsigned long long)s_push_reserve_fallbacks);
}

void hle_api_apply_target_scale(uint32_t dev, uint32_t viewport) { apply_target_scale(dev, viewport); }
void hle_api_viewport_transform(uint32_t dev) { viewport_transform(dev); }
void hle_api_set_vertex_shader(uint32_t shader) { native_set_vertex_shader(shader); }
void hle_api_set_render_target(uint32_t rt, uint32_t z) { native_set_render_target(rt, z); }
void hle_api_set_texture(uint32_t stage, uint32_t texture) { native_set_texture(stage, texture); }
void hle_api_flush(void) { native_flush_state(dev_va()); }
void hle_api_block_resource(uint32_t r) { native_block_until_not_busy(r); }
uint32_t hle_api_release(uint32_t r) { return native_release(r); }
void hle_native_drain(void) { drain_pending(dev_va()); }
uint32_t hle_native_fence(void) { return native_fence(dev_va()); }
void hle_native_wait_idle(void) { native_block(dev_va(), MEM32(dev_va() + DEV_FENCE_TIME), 2); }
int hle_api_render_store(int id, uint32_t value)
{
    int idx = rs_index(id);
    if (idx < 0) return 0;
    native_set_rs(idx, value);
    return 1;
}

/* CPU-only reservation: already consumed commands need no NV2A ring jump.
 * Extend the window or grow its storage, retaining the library's fence queue. */
/* 0x25D210, at the end of the first Swap after a boot that persisted the
 * dashboard's picture (device +0x1DE0): wait for the GPU, program the
 * display mode for the new front buffer one vblank step at a time, encoder
 * options, field parity, then free the persisted picture. Its 0x25A1AD only
 * reprograms PRAMDAC clock registers (no CPU state) and is left out. */
static void native_persist_release(uint32_t dev)
{
    uint32_t regs = MEM32(dev + 0x1C28), pitch = MEM32(dev + 0x1C2C);
    uint32_t a3 = MEM32(dev + 0x1C30), a4 = MEM32(dev + 0x1C34), front, step = 0, out, field, saved;
    extern void guest_cpu_yield(void);

    native_block(dev, MEM32(dev + DEV_FENCE_TIME), 2);              /* 0x2520A0 */
    if (MEM32(dev + 0x1DDC) & 0x1000000u)
        pitch >>= 1;
    front = MEM32(MEM32(dev + DEV_FRONT_BUFFER) + 4);
    do {
        uint32_t wait[5] = { dev + 0x1DBC, 6, 1, 0, 0 };          /* 0x252CE0 */
        uint32_t mode[6] = { regs, step, a3, a4, pitch, front };
        MEM32(dev + 0x1DC0) = 0;
        guest_call(MEM32(0x27714Cu), 5, wait);                    /* KeWaitForSingleObject */
        step = guest_call(MEM32(0x27728Cu), 6, mode);             /* AvSetDisplayMode */
        if (step)
            guest_cpu_yield();
    } while (step);
    if (MEM32(dev + 0x1DDC) & 0x1000000u) {
        uint32_t opt[4] = { regs, 0xB, 0, 0 };
        guest_call(MEM32(0x277288u), 4, opt);                     /* AvSendTVEncoderOption */
    }
    g_esp -= 4;
    out = g_esp;
    MEM32(out) = 0;
    {
        uint32_t opt[4] = { MEM32(dev + 0x1C28), 0xF, 0, out };
        guest_call(MEM32(0x277288u), 4, opt);
    }
    field = MEM32(out);
    g_esp += 4;
    if (!((MEM32(dev + 0x1DE8) ^ field) & 1u))
        MEM32(dev + 0x1DE8) += 1;
    MEM32(0x2636F0u) = 0;
    MEM32(0x2636ECu) = 0;
    saved = guest_call(MEM32(0x277284u), 0, NULL);                /* AvGetSavedDataAddress */
    if (saved) {
        uint32_t size = guest_call(MEM32(0x277170u), 1, &saved); /* MmQueryAllocationSize */
        uint32_t persist[3] = { saved, size, 0 }, zero = 0;
        guest_call(MEM32(0x277128u), 3, persist);                 /* MmPersistContiguousMemory */
        guest_call(MEM32(0x277134u), 1, &saved);                  /* MmFreeContiguousMemory */
        guest_call(MEM32(0x277290u), 1, &zero);                   /* AvSetSavedDataAddress */
    }
    {
        uint32_t opt[4] = { regs, 9, 0, 0 };
        guest_call(MEM32(0x277288u), 4, opt);
    }
    MEM32(dev + 0x1DE0) = 0;
}

uint32_t hle_native_reserve(uint32_t dwords)
{
    uint32_t dev = dev_va(), put = MEM32(dev), base = MEM32(dev + 0x24), end = MEM32(dev + 0x28);
    uint64_t bytes = (uint64_t)dwords * 4u;
    extern void hle_sync_rebase(uint32_t pos);
    if ((uint64_t)put + bytes < (uint64_t)MEM32(dev + 4) + 0x200u) return put;
    drain_pending(dev);
    if (MEM32(dev + 8) & 4u) {
        uint32_t record = MEM32(dev + 0x764);
        MEM32(dev + 0x768) += put - MEM32(record + 4);
        MEM32(dev) = MEM32(record + 4);
        hle_sync_rebase(MEM32(dev));
        return MEM32(dev);
    }
    if (bytes + 0x204u > (uint64_t)end - base) {
        uint64_t capacity = (bytes + 0x4203u) & ~0x3FFFULL;
        if (capacity > UINT32_MAX) return 0;
        uint32_t args[2] = { (uint32_t)capacity, 0xBC800000u };
        uint32_t block = guest_call(0x1609A7u, 2, args);
        if (!block) return 0;
        uint32_t old_base = base;
        MEM32(dev + 0x44) = put - base;
        MEM32(dev + 0x40)++;
        MEM32(dev + 0x24) = block;
        MEM32(dev + 0x28) = block + (uint32_t)capacity;
        MEM32(dev) = put = base = block;
        end = block + (uint32_t)capacity;
        hle_sync_rebase(put);
        args[0] = old_base;
        args[1] = 0xA1800000u;
        guest_call(0x160A47u, 2, args);
    } else if ((uint64_t)put + bytes + 0x204u > end) {
        MEM32(dev + 0x40)++;
        MEM32(dev + 0x44) = put - base;
        MEM32(dev) = put = base;
        hle_sync_rebase(put);
    }
    uint64_t window = bytes + 0x204u;
    uint32_t chunk = MEM32(0x2639E8u);
    if (window < chunk) window = chunk;
    uint64_t limit = (uint64_t)put + window;
    if (limit + 0x4000u >= end) limit = end;
    MEM32(dev + 4) = (uint32_t)limit - 0x204u;
    if (!(MEM32(dev + 8) & 0x800u)) native_ring_fence(dev);
    else MEM32(dev + 8) |= 0x1000u;
    hle_sync_rebase(put);
    return put;
}

/* ------------------------------------------------------------ the check */

#define CHECK_GLOBALS_LO 0x260000u
#define CHECK_GLOBALS_HI 0x264000u
#define CHECK_DEV_SIZE   0x2500u

typedef struct { uint32_t va, size; uint8_t *a, *b; } Region;

static uint64_t s_check_calls, s_check_bad;

static void snap(Region *r, int n, int which)
{
    for (int i = 0; i < n; i++)
        memcpy(which ? r[i].b : r[i].a, (const void *)XBOX_PTR(r[i].va), r[i].size);
}

static void restore(Region *r, int n)
{
    for (int i = 0; i < n; i++)
        memcpy((void *)XBOX_PTR(r[i].va), r[i].a, r[i].size);
}

static int skip_offset(uint32_t va, uint32_t dev)
{
    if (va >= dev && va < dev + 8)                       /* put / end */
        return 1;
    if (va >= dev + DEV_FENCE_SLOTS && va < dev + DEV_FENCE_SLOTS + 0x200 && ((va - dev - DEV_FENCE_SLOTS) & 4))
        return 1;                                        /* fence slot pushbuffer addresses */
    if ((va >= MINIPORT && va < MINIPORT + 0x800) || va == 0x2639F4u)   /* vblank thread */
        return 1;
    if (va >= dev + 0x1C08 && va < dev + 0x1C18)         /* 0x250C20's cache, push side only */
        return 1;
    return 0;
}

static void compare(const char *name, Region *r, int n, uint32_t dev)
{
    int bad = 0;
    s_check_calls++;
    for (int i = 0; i < n; i++)
        for (uint32_t off = 0; off < r[i].size; off += 4) {
            uint32_t x, y;
            memcpy(&x, r[i].a + off, 4);
            memcpy(&y, r[i].b + off, 4);
            if (x != y && !skip_offset(r[i].va + off, dev)) {
                if (s_check_bad < 64)
                    fprintf(stderr, "[HLE-APICHECK] %s: %08X (dev+%X) native %08X original %08X\n",
                            name, r[i].va + off, r[i].va + off - dev, x, y);
                bad = 1;
            }
        }
    if (bad)
        s_check_bad++;
    if (s_check_calls % 20000 == 0 || (bad && s_check_bad <= 4))
        fprintf(stderr, "[HLE-APICHECK] %llu calls checked, %llu differ\n",
                (unsigned long long)s_check_calls, (unsigned long long)s_check_bad);
}

static int guest_va_ok(uint32_t va)
{
    return (va >= 0x10000u && va < 0x04000000u) || (va >= 0x80000000u && va < 0x84000000u);
}

static void checked(const char *name, int id_kind, const uint32_t *a, void (*gen)(void))
{
    uint32_t dev = dev_va(), esp0 = g_esp;
    uint32_t objs[6] = { a[0], a[1], MEM32(dev + DEV_RENDER_TARGET), MEM32(dev + DEV_DEPTH_STENCIL), 0, 0 };
    Region r[16];
    int n = 0, i;
    static uint8_t *pool;
    size_t used = 0;

    if (!pool)
        pool = (uint8_t *)malloc(2 * (CHECK_GLOBALS_HI - CHECK_GLOBALS_LO + CHECK_DEV_SIZE + 16 * 0x20));
    r[n].va = dev; r[n].size = CHECK_DEV_SIZE; n++;
    r[n].va = CHECK_GLOBALS_LO; r[n].size = CHECK_GLOBALS_HI - CHECK_GLOBALS_LO; n++;
    r[n].va = MEM32(dev + DEV_FENCE_DONE); r[n].size = 4; n++;  /* completed fence time */
    if (id_kind == 3) {                                   /* Swap: buffers and textures */
        objs[0] = MEM32(dev + DEV_BACK_BUFFER);
        objs[1] = MEM32(dev + DEV_FRONT_BUFFER);
        objs[4] = MEM32(dev + 4 * 0x3E2);
        objs[5] = MEM32(dev + DEV_FRONT_BUFFER + 4);
    }
    if (id_kind >= 4 && id_kind <= 7) {                   /* setters: new and old objects */
        uint32_t o = id_kind == 4 ? MEM32(dev + 4 * (0x3E2 + a[0]))
                   : id_kind == 5 ? MEM32(dev + 4 * (0x3E6 + a[0]))
                   : id_kind == 6 ? MEM32(0x2636F8u + 12 * a[0] + 8) : MEM32(dev + 0x7A0);
        if (o && o != (id_kind == 7 ? a[0] : a[1]) && ((MEM32(o) - 0x80000u) & 0x78FFFFu) == 0) {
            static uint64_t skipped;
            if (skipped++ < 8)
                fprintf(stderr, "[HLE-APICHECK] %s: old object %08X would be freed; not checked\n", name, o);
            g_esp = esp0;
            gen();
            return;
        }
        objs[0] = id_kind == 7 ? a[0] : a[1];
        objs[1] = id_kind == 4 ? MEM32(dev + 4 * (0x3E2 + a[0]))
                : id_kind == 5 ? MEM32(dev + 4 * (0x3E6 + a[0]))
                : id_kind == 6 ? MEM32(0x2636F8u + 12 * a[0] + 8)
                : MEM32(dev + 0x7A0);
        objs[2] = objs[3] = 0;
    }
    if (id_kind >= 13) { objs[1] = objs[2] = objs[3] = 0; }
    if ((id_kind >= 2 && id_kind <= 7) || (id_kind >= 13 && id_kind <= 15))  /* the surfaces */
        for (i = 0; i < 6; i++)
            if (guest_va_ok(objs[i]) && (objs[i] < dev || objs[i] >= dev + CHECK_DEV_SIZE)) {
                r[n].va = objs[i]; r[n].size = 0x20; n++;
                /* +0x14 is the parent only on surfaces (type 5); on other
                 * resources it is the next allocation's bytes. */
                if ((MEM32(objs[i]) & 0x70000u) == 0x50000u && guest_va_ok(MEM32(objs[i] + 0x14))) {
                    r[n].va = MEM32(objs[i] + 0x14); r[n].size = 0x20; n++;
                }
            }
    for (i = 0; i < n; i++) {
        r[i].a = pool + used; used += r[i].size;
        r[i].b = pool + used; used += r[i].size;
    }
    snap(r, n, 0);
    /* native */
    if (id_kind == 0) native_set_vertex_shader(a[0]);
    else if (id_kind == 1) native_set_pixel_shader(a[0]);
    else if (id_kind == 2) native_set_render_target(a[0], a[1]);
    else if (id_kind == 3) { s_checking = 1; native_swap(dev); s_checking = 0; }
    else if (id_kind == 16) native_begin(dev);
    else if (id_kind == 17) native_begin_push(dev);
    else {
        s_nofree = 1;
        if (id_kind == 4) native_set_texture(a[0], a[1]);
        else if (id_kind == 5) native_set_palette(a[0], a[1]);
        else if (id_kind == 6) native_set_stream_source(a[0], a[1], a[2]);
        else if (id_kind == 7) native_set_indices(a[0], a[1]);
        else if (id_kind == 8) native_set_viewport(a[0]);
        else if (id_kind == 9) native_set_ps_constant(a[0], a[1], a[2]);
        else if (id_kind == 10) native_set_rs(s_rs_check_idx, a[0]);
        else if (id_kind == 11) { drain_pending(dev); native_fence(dev); }
        else if (id_kind == 12) native_block(dev, a[0], 4);
        else if (id_kind == 13) native_block_until_not_busy(a[0]);
        else if (id_kind == 14) native_lock2(a[0], a[1]);
        else native_release(a[0]);
        s_nofree = 0;
    }
    snap(r, n, 1);
    /* swap the roles: a = native result, b = start state */
    for (i = 0; i < n; i++) { uint8_t *t = r[i].a; r[i].a = r[i].b; r[i].b = t; }
    for (i = 0; i < n; i++)
        memcpy((void *)XBOX_PTR(r[i].va), r[i].b, r[i].size);
    g_esp = esp0;
    {
        extern volatile uint32_t hle_sync_kick_count;
        uint32_t kicks = hle_sync_kick_count;
        gen();
        /* The original Swap kicks off twice (its two fences); a third kick
         * is its pushbuffer ring wrapping, whose fence bookkeeping the
         * native Swap -- writing nothing to the ring -- never needs. */
        if (hle_sync_kick_count - kicks > (id_kind == 3 ? 2u : id_kind >= 11 && id_kind <= 14 ? 1u : 0u)) {
            static uint64_t wraps;
            if (wraps++ < 4)
                fprintf(stderr, "[HLE-APICHECK] %s: original wrapped the pushbuffer ring; not compared\n", name);
            return;
        }
    }
    snap(r, n, 1);                                        /* b = original result */
    compare(name, r, n, dev);
}

/* RECOMP_HLE_APICHECK_KINDS=<digits>: check only those kinds (default all). */
static int check_kind(int k)
{
    static const char *kinds = (const char *)1;
    if (kinds == (const char *)1)
        kinds = getenv("RECOMP_HLE_APICHECK_KINDS");
    return !kinds || strchr(kinds, k < 10 ? '0' + k : 'A' + k - 10) != NULL;
}

/* Entry from hle_d3d8.c for the routed calls it owns natively. Returns 1 when
 * handled (stack popped, eax set), 0 to run the original body. */
int hle_api_call(int id, const uint32_t *a, void (*gen)(void))
{
    int mode = api_mode();
    if (!mode || !hle_native_owns_state())
        return 0;
    if (hle_resources_call(id, a) || hle_states_call(id, a) || hle_misc_call(id, a) || hle_create_call(id, a)) {
        hle_api_native_calls++;
        return 1;
    }
    switch (id) {
    case HLE_D3DDevice_Begin:
        s_begin_calls++;
        if (mode == 2 && check_kind(16)) { checked("Begin", 16, a, gen); return 1; }
        native_begin(dev_va());
        g_esp += 8;
        break;
    case HLE_D3DDevice_BeginPush_4: {
        uint32_t dev = dev_va();
        s_begin_push_calls++;
        if (mode == 2 && check_kind(17)) { checked("BeginPush", 17, a, gen); return 1; }
        native_begin_push(dev);
        g_eax = hle_native_reserve(a[0] + 1u);
        g_esp += 8;
        break;
    }
    /* 0x25CEA0: the END method goes to the backend through observe; a ring
     * fence deferred by a reservation inside Begin/End is inserted here. */
    case HLE_D3DDevice_End: {
        uint32_t dev = dev_va(), flags = MEM32(dev + 8);
        MEM32(dev + 8) = flags & 0xFFFFE7FFu;
        if (flags & 0x1000u) {
            drain_pending(dev);
            native_ring_fence(dev);
        }
        g_esp += 4;
        break;
    }
    case HLE_D3DDevice_EndPush:
        s_end_push_calls++;
        MEM32(dev_va()) = a[0];
        g_eax = a[0];
        g_esp += 8;
        break;
    case HLE_D3DDevice_SetVertexShader:
        if (mode == 2 && check_kind(0)) { checked("SetVertexShader", 0, a, gen); return 1; }
        native_set_vertex_shader(a[0]);
        g_esp += 8;
        break;
    /* Only the NV2A command: the backend took Method/Value in observe. */
    case HLE_D3DDevice_SetRenderState_Simple:
        g_esp += 4;
        break;
    /* Immediate-mode vertex data (0x25CD70/0x25CDB0/0x25CE10): inline NV2A
     * attributes only; the backend assembled the vertex in observe. */
    case HLE_D3DDevice_SetVertexData2f:
        g_esp += 16;
        break;
    case HLE_D3DDevice_Clear:
        g_esp += 28;
        break;
    case HLE_D3DDevice_SetVertexData4f:
        g_esp += 24;
        break;
    case HLE_D3DDevice_SetVertexDataColor:
        g_esp += 12;
        break;
    case HLE_D3DDevice_SetTexture:
        if (mode == 2 && check_kind(4)) { checked("SetTexture", 4, a, gen); return 1; }
        native_set_texture(a[0], a[1]);
        g_esp += 12;
        break;
    case HLE_D3DDevice_SetPalette:
        if (mode == 2 && check_kind(5)) { checked("SetPalette", 5, a, gen); return 1; }
        native_set_palette(a[0], a[1]);
        g_esp += 12;
        break;
    case HLE_D3DDevice_SetStreamSource:
        if (mode == 2 && check_kind(6)) { checked("SetStreamSource", 6, a, gen); return 1; }
        native_set_stream_source(a[0], a[1], a[2]);
        g_esp += 16;
        break;
    case HLE_D3DDevice_SetViewport:
        if (mode == 2 && check_kind(8)) { checked("SetViewport", 8, a, gen); return 1; }
        native_set_viewport(a[0]);
        g_esp += 8;
        break;
    case HLE_D3DDevice_SetPixelShaderConstant:
        if (!MEM32(dev_va() + DEV_PIXEL_SHADER))
            return 0;
        if (mode == 2 && check_kind(9)) { checked("SetPixelShaderConstant", 9, a, gen); return 1; }
        native_set_ps_constant(a[0], a[1], a[2]);
        g_esp += 16;
        break;
    case HLE_D3DDevice_InsertFence: {
        uint32_t dev = dev_va();
        if (mode == 2 && check_kind(0xB)) { drain_pending(dev_va()); checked("InsertFence", 11, a, gen); return 1; }
        drain_pending(dev);
        g_eax = native_fence(dev);
        g_esp += 4;
        break;
    }
    case HLE_D3DDevice_BlockOnFence:
        if (mode == 2 && check_kind(0xC)) { drain_pending(dev_va()); checked("BlockOnFence", 12, a, gen); return 1; }
        native_block(dev_va(), a[0], 4);
        g_esp += 8;
        break;
    case HLE_D3DResource_BlockUntilNotBusy:
        if (mode == 2 && check_kind(0xD)) { drain_pending(dev_va()); checked("BlockUntilNotBusy", 13, a, gen); return 1; }
        native_block_until_not_busy(a[0]);
        g_esp += 8;
        break;
    case HLE_D3DVertexBuffer_Lock2:
        if (mode == 2 && check_kind(0xE)) { drain_pending(dev_va()); checked("Lock2", 14, a, gen); return 1; }
        g_eax = native_lock2(a[0], a[1]);
        g_esp += 12;
        break;
    case HLE_D3DResource_Release:
        if (mode == 2 && check_kind(0xF)) {
            uint32_t w = MEM32(a[0]);
            if ((w & 0xFFFFu) == 1)             /* may free: the original alone */
                return 0;
            checked("Release", 15, a, gen);
            return 1;
        }
        g_eax = native_release(a[0]);
        g_esp += 8;
        break;
    case HLE_D3DDevice_SetIndices:
        if (mode == 2 && check_kind(7)) { checked("SetIndices", 7, a, gen); return 1; }
        native_set_indices(a[0], a[1]);
        g_esp += 12;
        break;
    case HLE_D3DDevice_SetPixelShader:
        if (mode == 2 && check_kind(1)) { checked("SetPixelShader", 1, a, gen); return 1; }
        native_set_pixel_shader(a[0]);
        g_esp += 8;
        break;
    case HLE_D3DDevice_SetRenderTarget:
        if (mode == 2 && check_kind(2)) { checked("SetRenderTarget", 2, a, gen); return 1; }
        native_set_render_target(a[0], a[1]);
        g_esp += 12;
        break;
    case HLE_D3DDevice_Swap: {
        uint32_t dev = dev_va();
        drain_pending(dev);
        if (!swap_supported(dev, a[0])) {
            hle_api_fallbacks++;
            return 0;
        }
        if (mode == 2 && check_kind(3)) { checked("Swap", 3, a, gen); return 1; }
        native_swap(dev);
        if (MEM32(dev + 0x1DE0))
            native_persist_release(dev);
        g_eax = MEM32(dev + DEV_SWAP_COUNT);
        g_esp += 8;
        break;
    }
    default: {
        int idx = rs_index(id);
        if (idx < 0)
            return 0;
        if (mode == 2 && check_kind(0xA)) {
            s_rs_check_idx = idx;
            checked("SetRenderState_*", 10, a, gen);
            return 1;
        }
        native_set_rs(idx, a[0]);
        g_esp += 8;
        break;
    }
    }
    hle_api_native_calls++;
    return 1;
}
