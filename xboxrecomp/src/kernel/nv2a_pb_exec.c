/**
 * Execute the parts of the title's pushbuffer that produce visible pixels.
 *
 * The title builds NV2A commands in guest RAM and advances DMA_PUT; without
 * something consuming them the framebuffer stays whatever it was, which is how
 * a fully booted title renders a black screen. This walks the same command
 * stream nv2a_pb_scan.c surveys and carries out the subset that decides what is
 * on screen: which surface is being drawn into, and clearing it.
 *
 * It also rasterises geometry, but only the part that can be drawn honestly:
 * batches whose attribute 0 is already in screen space, flat-shaded, straight
 * into the same guest framebuffer the clear writes. Titles draw their UI, HUD
 * and 2D overlays that way, so it is the first geometry to appear. Batches that
 * need a vertex program executed are counted and skipped rather than drawn
 * somewhere wrong -- see raster_batch(). Texturing, depth and vertex programs
 * are still a renderer, not a command decoder; the upgrade path is the D3D11
 * translator in src/nv2a/nv2a_pgraph_d3d11.c.
 *
 * Everything this does not handle is counted and ranked by
 * nv2a_pb_exec_report(), so what remains is a list rather than a guess.
 *
 * Enabled with RECOMP_PB_EXEC. RECOMP_RASTER_TEST draws one known triangle
 * after every clear, which separates "the pixel path is broken" from "the title
 * has not given us any vertices". RECOMP_FB_DUMP=<prefix> writes the surface to
 * <prefix>NNN.bmp, so the result can be looked at without a display.
 */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "kernel.h"   /* XBOX_CONTIG_BASE / XBOX_CONTIG_SIZE */
#include "xbox_memory_layout.h"   /* xbox_Nv2aFrameCounterFlip */
/* The swizzle decoder the D3D8 layer already uses -- one implementation of
 * Morton order, not a second one that can disagree with it. */
#include "../d3d/d3d8_swizzle.h"
#include "nv2a_vsh_interp.h"
#include "nv2a_combiner.h"
#include "nv2a_gpu_compute.h"
#if defined(_WIN32)
#include <windows.h>
#define NV_TLS __declspec(thread)
#else
#define NV_TLS __thread
#endif

extern ptrdiff_t xbox_GetMemoryOffset(void);
extern void xbox_FramebufferWindowSet(uint32_t fb_va, uint32_t pitch);
extern void xbox_FramebufferWindowStart(void);
extern uint32_t g_xbox_image_lo, g_xbox_image_hi;

/* Would writing this surface land on the title's own image?
 *
 * NV097_SET_SURFACE_COLOR_OFFSET is an offset within the colour DMA object,
 * not a guest virtual address, and this executor has always used it as one.
 * That is harmless while the two happen to agree and catastrophic when they do
 * not: the Xbox Dashboard names surface 0x00088000 at 1280x960x4, so clearing
 * it wrote 4.9 MB of opaque black from 0x00088000 to 0x00538000 -- straight
 * over its own code, its D3D context at 0x000BBFC0 and the register-block
 * pointer at 0x000BE2C4. The symptom was a title that submitted one perfect
 * frame and then spun forever in a pushbuffer-full loop, three layers away,
 * with every D3D global reading 0xFF000000: the clear colour.
 *
 * So refuse, and say so. Getting the address right needs the DMA object base
 * this ignores (NV097_SET_CONTEXT_DMA_COLOR); until that exists, writing
 * nothing is strictly better than writing over the guest, and a title that
 * cannot draw is easier to debug than one that has been overwritten.
 */
static int surface_hits_image(uint32_t base, uint32_t bytes)
{
    if (!g_xbox_image_hi || !bytes)
        return 0;
    return base < g_xbox_image_hi && base + bytes > g_xbox_image_lo;
}

/* Where a DMA-object offset actually lives.
 *
 * NV097_SET_SURFACE_COLOR_OFFSET is an offset inside the colour DMA object,
 * and for a framebuffer that object covers physical memory -- so the offset
 * is a physical address, not a guest VA. Those are the same number in this
 * runtime, which is why treating it as a VA works until it does not: on
 * hardware the image is mapped at VA 0x00010000 from arbitrary physical
 * pages, so a framebuffer at physical 0x84000 does not overlap it. Here it
 * would.
 *
 * The title tells us which it is by where it allocated. Half-Life 2's
 * framebuffer comes from MmAllocateContiguousMemory, which this runtime
 * serves from the window at XBOX_CONTIG_BASE, so physical P is visible at
 * XBOX_CONTIG_BASE + P -- clear of the image, and the same bytes the title's
 * own writes and the framebuffer window reach.
 *
 * So: use the offset as a VA when that is credible, and fall back to the
 * physical mirror exactly when it is not. Titles whose surfaces already sit
 * in ordinary RAM (Wreckless renders to the tiled alias of physical
 * 0x01954000) keep the first path and are unaffected.
 */
static uint32_t dma_resolve(uint32_t offset)
{
    extern uint32_t xbox_ContiguousAllocatedBytes(void);

    /* Did this runtime hand the offset out as contiguous memory? Then the
     * bytes live in the window, and that is not a guess: the arena is a bump
     * allocator from XBOX_CONTIG_BASE, so everything below its high-water
     * mark is memory some MmAllocateContiguousMemory call returned. The
     * title's own writes go through the window, so the executor's must too.
     *
     * Checking this BEFORE the image test is the whole point. The image test
     * only catches an offset that would land on the title's code, and whether
     * it does is an accident of where the image happens to end: Half-Life 2's
     * colour surface is physical 0x00A6C000, which clears the image by 700 KB.
     * So it looked like an ordinary VA, and the executor cleared 1.2 MB of
     * black straight through the guest heap -- which faulted the title three
     * frames later on a pointer that had been overwritten, while the real
     * framebuffer at 0x80A6C000 stayed untouched and the screen stayed black. */
    if (offset < xbox_ContiguousAllocatedBytes())
        return XBOX_CONTIG_BASE + offset;
    if (!surface_hits_image(offset, 1))
        return offset;
    if ((uint64_t)offset < XBOX_CONTIG_SIZE)
        return XBOX_CONTIG_BASE + offset;
    return offset;                         /* nothing better to offer */
}

/* Resolve the DMA handle through the RAMHT written by the guest miniport.
 * NV2A descriptor layout: flags/limit/frame; address combines frame and adjust.
 * The semaphore write happens here, after preceding commands have executed,
 * rather than mirroring a submitted counter before its work has completed. */
/* The command scanner consumes USER channel zero. SET_OBJECT carries a
 * RAMHT handle, not a graphics class; keep the binding for each subchannel. */
static uint32_t s_graphics_class[8];
static uint32_t gpu_object_class(uint32_t handle)
{
    uintptr_t mem = (uintptr_t)xbox_GetMemoryOffset();
    uint32_t reg = *(volatile uint32_t *)(mem + 0xFD002210u);
    uint32_t base = (reg & 0x1F0u) << 8;
    uint32_t bytes = 4096u << ((reg >> 16) & 3u);
    if ((uint64_t)base + bytes > 0x100000u) return 0;
    const volatile uint32_t *table =
        (const volatile uint32_t *)(mem + 0xFD700000u + base);
    for (uint32_t i = 0; i < bytes / 4; i += 2) {
        if (table[i] != handle) continue;
        uint32_t context = table[i + 1];
        /* NV_RAMHT_STATUS, ENGINE_GRAPHICS and CHID zero. */
        if (!(context & 0x80000000u) ||
            (context & 0x00030000u) != 0x00010000u ||
            (context & 0x1F000000u)) continue;
        uint32_t instance = (context & 0xFFFFu) << 4;
        return *(volatile uint32_t *)(mem + 0xFD700000u + instance) & 0xFFu;
    }
    return 0;
}

/* Resident render target (see gpu_surface_bind): every point where the
 * guest or a CPU path may observe the surface first returns it to guest
 * memory. Only the executor thread owns the D3D context. */
static void gpu_surface_sync(void);
static void trace_surface_write(const char *what, uint32_t base, uint32_t bytes);
static _Thread_local int t_exec_thread;
static unsigned s_tex_epoch;
static unsigned s_span_epoch;   /* validated guest spans; moves at flip/sync */

static int gpu_aux_method(uint32_t subch, uint32_t method, uint32_t param)
{
    if (subch >= 8) return 0;
    if (method == 0) { /* SET_OBJECT */
        s_graphics_class[subch] = gpu_object_class(param);
        return s_graphics_class[subch] != 0;
    }
    if (s_graphics_class[subch] == 0x97 && method == 0x100) {
        /* A nonzero NV097 NOP is a software notification, not a pixel draw. */
        if (param) {
            gpu_surface_sync();
            xbox_Nv2aNotify(subch, param);
        }
        return 1;
    }
    if (s_graphics_class[subch] == 0x44 && method == 0x310) {
        /* NV044_SET_MONOCHROME_COLOR0 -> NV_PGRAPH_PATT_COLOR0.
         * Xbox D3D uses the pattern color as an ordered progress marker.
         * Execute this command's actual value; never mirror a submitted fence. */
        uintptr_t mem = (uintptr_t)xbox_GetMemoryOffset();
        *(volatile uint32_t *)(mem + 0xFD400B10u) = param;
        static unsigned shown;
        if (shown++ < 8)
            fprintf(stderr, "[PGRAPH] pattern marker=%08X subch=%u class=44\n",
                    param, subch);
        return 1;
    }
    return 0;
}

static uint32_t s_semaphore_handle, s_semaphore_offset;
static int semaphore_release(uint32_t value)
{
    gpu_surface_sync();
    uintptr_t mem = (uintptr_t)xbox_GetMemoryOffset();
    const volatile uint32_t *ramht_reg = (const volatile uint32_t *)(mem + 0xFD002210u);
    uint32_t reg = *ramht_reg;
    uint32_t base = (reg & 0x1F0u) << 8;
    uint32_t bytes = 4096u << ((reg >> 16) & 3u);
    if ((uint64_t)base + bytes > 0x100000u) return 0;
    const volatile uint32_t *table = (const volatile uint32_t *)(mem + 0xFD700000u + base);
    for (uint32_t i = 0; i < bytes / 4; i += 2) {
        if (table[i] != s_semaphore_handle) continue;
        uint32_t context = table[i + 1];
        if (!(context & 0x80000000u)) continue;
        uint32_t instance = (context & 0xFFFFu) << 4;
        if (instance + 12 > 0x100000u) continue;
        const volatile uint32_t *desc = (const volatile uint32_t *)(mem + 0xFD700000u + instance);
        uint32_t flags = desc[0], limit = desc[1];
        uint32_t cls = flags & 0xFFFu;
        if (cls != 2 && cls != 3 && cls != 0x3D) continue;
        uint32_t physical = ((desc[2] & 0xFFFFF000u) | (flags >> 20)) & 0x07FFFFFFu;
        if ((s_semaphore_offset & 3u) || (uint64_t)s_semaphore_offset + 3 > limit ||
            (uint64_t)physical + s_semaphore_offset + 4 > XBOX_CONTIG_SIZE) return 0;
        uint32_t address = XBOX_CONTIG_BASE + physical + s_semaphore_offset;
        *(volatile uint32_t *)(mem + address) = value;
        static unsigned shown;
        if (shown++ < 8)
            fprintf(stderr, "[SEMAPHORE] release %08X -> %08X (DMA handle %X)\n", value, address, s_semaphore_handle);
        return 1;
    }
    return 0;
}

static int surface_write_refused(uint32_t base, uint32_t bytes, const char *what)
{
    static int said;

    if (!surface_hits_image(base, bytes))
        return 0;
    if (!said) {
        said = 1;
        fprintf(stderr,
                "  [GPU] REFUSING to %s surface 0x%08X..0x%08X: that overlaps "
                "the loaded image (0x%08X..0x%08X).\n"
                "  [GPU]   SET_SURFACE_COLOR_OFFSET is a DMA-object offset, not "
                "a guest VA, and this executor treats it as one. Writing here "
                "would destroy the title's own code and globals.\n",
                what, base, base + bytes, g_xbox_image_lo, g_xbox_image_hi);
        fflush(stderr);
    }
    return 1;
}

/* NV097 methods this executor acts on. */
/* Blending. The pair this title programs, read from its own pushbuffer
 * rather than guessed: BLEND_ENABLE written 1168 times and left on,
 * SFACTOR 0x0302 (SRC_ALPHA) and DFACTOR 0x0303 (ONE_MINUS_SRC_ALPHA).
 * ALPHA_TEST_ENABLE is written 390 times and left at zero, so this is
 * blending and not an alpha test. */
#define NV097_SET_BLEND_ENABLE            0x0304
#define NV097_SET_BLEND_FUNC_SFACTOR      0x0344
#define NV097_SET_BLEND_FUNC_DFACTOR      0x0348
#define NV_BLEND_SRC_ALPHA                0x0302
#define NV_BLEND_ONE_MINUS_SRC_ALPHA      0x0303
#define NV097_SET_BLEND_COLOR             0x034C
#define NV097_SET_BLEND_EQUATION          0x0350

#define NV097_SET_SURFACE_CLIP_HORIZONTAL 0x0200
#define NV097_SET_SURFACE_CLIP_VERTICAL   0x0204
#define NV097_SET_SURFACE_FORMAT          0x0208
#define NV097_SET_SURFACE_PITCH           0x020C
#define NV097_SET_SURFACE_COLOR_OFFSET    0x0210
#define NV097_SET_COLOR_CLEAR_VALUE       0x1D90
#define NV097_CLEAR_SURFACE               0x1D94
#define NV097_SET_VERTEX_DATA_ARRAY_OFFSET 0x1720   /* +i*4, 16 attributes */
#define NV097_SET_VERTEX_DATA_ARRAY_FORMAT 0x1760   /* +i*4 */
#define NV097_SET_BEGIN_END               0x17FC
#define NV097_SET_TEXTURE_OFFSET          0x1B00   /* +i*0x40 */
#define NV097_SET_TEXTURE_FORMAT          0x1B04
#define NV097_SET_TEXTURE_PALETTE         0x1B20   /* offset | size<<2 | dma */
#define NV097_SET_TEXTURE_ADDRESS         0x1B08
#define NV097_SET_TEXTURE_CONTROL1        0x1B10
#define NV097_SET_TEXTURE_IMAGE_RECT      0x1B1C
/* The buffer flip. A title double-buffers by telling the GPU which buffer
 * the CRTC reads and which it draws into, advancing the write index and
 * then stalling until the flip has happened. Ignoring these means the
 * stall never clears: Half-Life 2's loader submits its initialisation,
 * asks for a flip, and waits for it in a loop that makes no kernel calls
 * and burns no dispatch, which reads as a hang with no cause.
 *
 * ponytail: the flip completes the moment it is asked for, because there is
 * no scanout to be in the middle of. That makes every frame land instantly
 * and a title that paces itself on the flip runs as fast as it can draw.
 * Pacing wants the vblank clock in the kernel, not a sleep in here. */
#define NV097_SET_FLIP_READ               0x0120
#define NV097_SET_FLIP_WRITE              0x0124
#define NV097_SET_FLIP_MODULO             0x0128
#define NV097_FLIP_INCREMENT_WRITE        0x012C
#define NV097_FLIP_STALL                  0x0130
#define NV097_ARRAY_ELEMENT16             0x1800
#define NV097_ARRAY_ELEMENT32             0x1808
/* Draw a run of vertices straight out of the arrays, with no index list:
 * bits 0..23 are the first vertex, bits 24..31 the count minus one. It may
 * appear several times inside one BEGIN_END to draw a longer run. */
#define NV097_DRAW_ARRAYS                 0x1810
#define NV097_INLINE_ARRAY                0x1818
/* Immediate-mode vertices. SET_VERTEX3F/4F carry the position, and writing
 * its last component completes a vertex using whatever the SET_VERTEX_DATA*
 * registers currently hold for the other attributes. This is how Half-Life
 * 2's Xbox loader and the game's own 2D drawing submit every quad -- neither
 * uses INLINE_ARRAY -- so without these the executor saw SET_BEGIN_END pairs
 * with nothing attached and reported `draws 0` while a million and a half
 * textured quads a minute went past it. */
#define NV097_SET_VERTEX3F                0x1500   /* +0..0x08, 3 floats */
#define NV097_SET_VERTEX4F                0x1518   /* +0..0x0C, 4 floats */
#define NV097_SET_VERTEX_DATA2F_M         0x1880   /* + attr*8,  2 floats */
#define NV097_SET_VERTEX_DATA4F_M         0x1A00   /* + attr*16, 4 floats */
#define NV097_SET_VERTEX_DATA4UB          0x1940   /* + attr*4,  4 x u8 */
#define NV097_SET_VERTEX_DATA2S           0x1900   /* + attr*4,  2 x s16 */
#define NV097_SET_VERTEX_DATA4S_M         0x1980   /* + attr*8,  4 x s16 */

/* One immediate vertex, as this file packs it for the shared draw path:
 * position float4, diffuse D3DCOLOR, texcoord0 float2. */
/* An immediate-mode vertex: all 16 attributes as float4. */
#define IMM_VERTEX_DWORDS (NV_VERTEX_ATTRS * 4)

#define NV097_CLEAR_COLOR_MASK            0xF0   /* R,G,B,A bits */

/* One vertex attribute stream, as the title describes it. Attribute 0 is
 * position; the rest are colours, texture coordinates and so on. */
typedef struct {
    uint32_t offset;      /* guest address of element 0 */
    uint32_t type;        /* NV097 data type nibble */
    uint32_t size;        /* components per element */
    uint32_t stride;      /* bytes between elements */
} VertexAttr;

#define NV_VERTEX_ATTRS 16
#define NV_MAX_INDICES  4096
#define NV_MAX_INLINE   65536           /* dwords of INLINE_ARRAY per batch */

/* Texture stage 0, decoded from what the title programmed.
 *
 * Only stage 0: it is the only one the dashboard configures, and a stage
 * nothing writes to is a stage nothing can be sampled from. The rest arrive
 * as unhandled methods and are counted as such, which is how the next title
 * that needs them will say so. */
typedef struct {
    uint32_t offset;                    /* guest address of texel (0,0)  */
    uint32_t width, height;             /* from IMAGE_RECT               */
    uint32_t pitch;                     /* bytes per row, from CONTROL1  */
    uint32_t color;                     /* NV097 colour-format code      */
    uint32_t addr_u, addr_v;            /* wrap mode per axis            */
    uint32_t palette;                   /* guest address of the CLUT, P8 */
    uint32_t levels;                    /* mip levels, from FORMAT       */
    uint32_t filter;                    /* SET_TEXTURE_FILTER            */
    int      cube;                      /* FORMAT: six faces, not one    */
    int      valid;
} Texture;

static struct {
    VertexAttr attr[NV_VERTEX_ATTRS];
    uint32_t   prim;                    /* SET_BEGIN_END parameter, 0 = ended */
    uint32_t   idx[NV_MAX_INDICES];
    uint32_t   idx_count;
    /* INLINE_ARRAY payload: vertices written straight into the pushbuffer
     * instead of into a buffer the title points at. Same vertex format, a
     * different place to read them from. */
    uint32_t   inline_buf[NV_MAX_INLINE];
    uint32_t   inline_count;
    /* Current values of the immediate-mode attributes, and how many complete
     * vertices they have produced in this batch. */
    /* Immediate mode: every attribute's current value, as the SET_VERTEX
     * methods leave it. Writing attribute 0 (position) emits a vertex. */
    float      imm_attr[NV_VERTEX_ATTRS][4];
    uint16_t   imm_used;                /* attributes written since BEGIN */
    uint32_t   imm_count;
    int        inline_active;
    uint32_t   draws, verts, nonzero_draws;
    float      min_x, max_x, min_y, max_y;
    uint32_t color_offset, color_base, pitch, format;
    /* The surface the last batch actually drew into. A double-buffered title
     * has already pointed color_offset at the next buffer and cleared it by
     * the time the flip arrives, so dumping the current one dumps the frame
     * that has not been drawn yet -- which is how a correctly rendered
     * sequence came out as 12 black BMPs. */
    uint32_t drawn_offset;
    /* ...and the shape of that surface. A title also draws into small
     * render targets (Burnout 3: 128x128 shadow/reflection maps, pitch 512),
     * often after the main scene, so "last surface drawn" alone picked one of
     * those and read it with the framebuffer's pitch: horizontal noise. The
     * biggest surface drawn since the last flip is the one being presented. */
    uint32_t drawn_pitch, drawn_x, drawn_y, drawn_w, drawn_h, drawn_bpp;
    uint32_t drawn_samples_x, drawn_samples_y;
    int drawn_stale;      /* set at a flip: the next draw starts a new frame */
    uint64_t pixels;
    uint32_t pixel_max;   /* brightest value any pixel write carried */
    uint32_t clip_x, clip_w, clip_y, clip_h;
    uint32_t clear_color;
    uint32_t clears, unhandled_total;
    uint32_t flip_read, flip_write, flip_modulo, flips;
    uint32_t tris_drawn, tris_skipped_offscreen, batches_untransformed;
    /* Why a batch came out flat. "Untextured" has two causes that look
     * identical on screen and want opposite fixes: the batch carried no
     * texture coordinates, or it did and the stage was not usable. */
    uint32_t batches_textured, batches_no_uv, batches_no_tex;
    uint32_t blend_enable, blend_sfactor, blend_dfactor;
    uint32_t blend_equation, blend_color;
    uint32_t color_mask;        /* SET_COLOR_MASK: A<<24 R<<16 G<<8 B */
    /* Visibility tests (D3D's Begin/EndVisibilityTest): pixels that pass the
     * depth test while counting is on, reported by GET_REPORT. */
    uint32_t zpass_enable, zpass_count, reports;
    uint32_t blend_pairs[16];   /* sfactor<<16 | dfactor seen, for the report */
    int blend_npairs;
    Texture  texs[4];                   /* one per texture stage */
    /* Register combiners and the per-pixel state around them. rc_seen: the
     * title has programmed the combiners, so they decide every pixel's
     * colour; until then the old "stage 0 texel * diffuse" stands in. */
    Nv2aCombiner rc;
    int      rc_seen;
    uint32_t clip_plane_mode;           /* SET_SHADER_CLIP_PLANE_MODE */
    uint32_t alpha_test, alpha_func, alpha_ref;
    uint32_t fog_enable, fog_mode, fog_color;
    float    fog_param[2];
    /* Vertex programs (nv2a_vsh_interp.c) and the depth buffer, which
     * together are what a 3D scene needs and a 2D one never used. */
    uint32_t xf_mode;                   /* TRANSFORM_EXECUTION_MODE: 2 = program */
    uint32_t batches_program, verts_program, tris_behind;
    /* Where program-path triangles go, so "nothing drew" has a reason. */
    uint32_t xf_degenerate, xf_offscreen, xf_drawn;
    uint64_t xf_depth_fail, xf_pixels;
    float xf_min[3], xf_max[3];
    uint32_t xf_fmt[16];  /* attribute formats seen: type | size<<4 | slot<<8 */
    int xf_nfmt, xf_seeded;
    uint32_t depth_test, depth_func, depth_mask;
    uint32_t zeta_offset, zstencil_clear;
} s_gpu;

/* Unhandled methods, ranked. The interesting output is not that something was
 * skipped but which things dominate, because that is the order to implement
 * them in. */
#define PB_EXEC_MAX_UNHANDLED 2048
typedef struct { uint32_t method, count, last_param; } PbUnhandled;
static PbUnhandled s_unhandled[PB_EXEC_MAX_UNHANDLED];
static int s_unhandled_count;

/* Every texture-stage register, as the title last set it.
 * Texturing is not implemented yet; knowing which formats and sizes a title
 * actually programs is what decides which ones are worth implementing. */
#define NV_TEX_FIRST 0x1B00
#define NV_TEX_LAST  0x1BFC
static uint32_t s_tex_reg[(NV_TEX_LAST - NV_TEX_FIRST) / 4 + 1];
static uint8_t  s_tex_set[(NV_TEX_LAST - NV_TEX_FIRST) / 4 + 1];

/* Formats whose dimensions come from the format word and whose coordinates
 * arrive normalised, rather than from a pitch and SET_TEXTURE_IMAGE_RECT with
 * coordinates in texels. Swizzled and block-compressed are both in this group,
 * and every place that used to test only for swizzled needs the pair. */
static int tex_size_from_format(uint32_t fmt)
{
    return d3d8_format_is_swizzled(fmt) || d3d8_format_dxt_block_bytes(fmt);
}

static void tex_update_valid(Texture *t)
{
    t->valid = t->offset && t->width && t->height
            && (tex_size_from_format(t->color) || t->pitch);
}

static void record_tex_reg(uint32_t method, uint32_t param)
{
    s_tex_epoch++;
    s_tex_reg[(method - NV_TEX_FIRST) / 4] = param;
    s_tex_set[(method - NV_TEX_FIRST) / 4] = 1;
    /* A pitch is a linear texture's property. A swizzled one has no rows and
     * so no pitch, and requiring one here refused every swizzled texture --
     * which is nearly all of them, since swizzled is the Xbox default. That
     * left the title's own textures unsampled and every textured quad drawn in
     * flat vertex colour. */
    tex_update_valid(&s_gpu.texs[0]);
}

/* Every distinct texture a batch was drawn with, and how many batches used it.
 *
 * The per-draw verbose print shows the first few draws of the first frame,
 * which is enough to see that texturing works at all and not enough to answer
 * "is a font page ever bound". This is the same shape as the unhandled-method
 * table below it: a small set, ranked, printed with the rest of the report. */
#define PB_EXEC_MAX_TEXTURES 64
typedef struct {
    uint32_t offset, color, width, height, batches;
} PbTexUse;
static PbTexUse s_tex_use[PB_EXEC_MAX_TEXTURES];
static int s_tex_use_count;

/* Defined below, next to the sampler it goes through. */
static void dump_texture_bmp(uint32_t seq);

/* Repeat dumps are numbered from well past the first-use sequence, so a
 * listing sorts them after the textures they came from and no first-use file
 * is ever overwritten by one. */
#define TEX_DUMP_SEQ_BASE 1000u
#define TEX_DUMP_SEQ_MAX  40u

static void note_texture_use(void)
{
    int i;

    if (!s_gpu.texs[0].valid)
        return;
    for (i = 0; i < s_tex_use_count; i++) {
        if (s_tex_use[i].offset == s_gpu.texs[0].offset
         && s_tex_use[i].color  == s_gpu.texs[0].color) {
            s_tex_use[i].batches++;
            /* Dump a surface that is redrawn, every Nth time it is bound.
             *
             * First use alone cannot tell a decode error that is wrong in
             * every frame from one that accumulates across them. A block
             * transform that is wrong is wrong on its own, in the keyframe
             * as much as anywhere; motion compensation that is wrong starts
             * from a clean keyframe and smears further with each predicted
             * frame after it. In a single frame the two look identical, and
             * in a sequence they look nothing alike -- so the sequence is
             * what has to be captured.
             *
             * It belongs on this side of the return: a video surface keeps
             * one address for the whole film, so after the first frame it is
             * only ever found here, and the first-use dump below never fires
             * for it again. RECOMP_TEX_DUMP_EVERY=<n> sets the interval, and
             * RECOMP_TEX_DUMP still names the files. */
            {
                static int every = -1;
                static unsigned binds, seq;
                if (every < 0) {
                    const char *e = getenv("RECOMP_TEX_DUMP_EVERY");
                    every = e ? atoi(e) : 0;
                }
                if (every > 0 && ++binds % (unsigned)every == 0
                    && seq < TEX_DUMP_SEQ_MAX)
                    dump_texture_bmp(TEX_DUMP_SEQ_BASE + seq++);
            }
            return;
        }
    }
    if (s_tex_use_count < PB_EXEC_MAX_TEXTURES) {
        s_tex_use[s_tex_use_count].offset  = s_gpu.texs[0].offset;
        s_tex_use[s_tex_use_count].color   = s_gpu.texs[0].color;
        s_tex_use[s_tex_use_count].width   = s_gpu.texs[0].width;
        s_tex_use[s_tex_use_count].height  = s_gpu.texs[0].height;
        s_tex_use[s_tex_use_count].batches = 1;
        s_tex_use_count++;
        dump_texture_bmp((uint32_t)s_tex_use_count - 1);
    }
}

static void note_unhandled(uint32_t method, uint32_t param)
{
    int i;

    s_gpu.unhandled_total++;
    for (i = 0; i < s_unhandled_count; i++) {
        if (s_unhandled[i].method == method) {
            s_unhandled[i].count++;
            s_unhandled[i].last_param = param;
            return;
        }
    }
    if (s_unhandled_count < PB_EXEC_MAX_UNHANDLED) {
        s_unhandled[s_unhandled_count].method = method;
        s_unhandled[s_unhandled_count].count = 1;
        s_unhandled[s_unhandled_count].last_param = param;
        s_unhandled_count++;
    }
}

/* Read attribute `a` of vertex `index` as floats. Only the float and the
 * normalised-byte types appear in practice; anything else returns 0 so a
 * caller sees a degenerate vertex rather than reading past the array. */
static int gpu_guest_span(uint32_t address, uint64_t bytes, int write);
static int fetch_attr(const VertexAttr *a, uint32_t index, float out[4])
{
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    const uint8_t *p;
    uint32_t i;

    out[0] = out[1] = out[2] = 0.0f;
    out[3] = 1.0f;
    if (!a->size || !a->stride)
        return 0;
    uint32_t components = a->size < 4 ? a->size : 4;
    uint32_t bytes;
    switch (a->type) {
    case 0: case 6: bytes = 4; break;
    case 1: case 5: bytes = components * 2; break;
    case 2: bytes = components * 4; break;
    case 4: bytes = components; break;
    default: return 0;
    }
    if (s_gpu.inline_active) {
        /* The batch arrived as INLINE_ARRAY, so `offset` is a byte offset into
         * the buffered payload rather than a guest address -- and 0 is a legal
         * one there, which is why the offset test is on the other side of this
         * branch. */
        size_t at = (size_t)a->offset + (size_t)index * a->stride;
        if (at + bytes > (size_t)s_gpu.inline_count * 4)
            return 0;
        p = (const uint8_t *)s_gpu.inline_buf + at;
    } else {
        if (!a->offset)
            return 0;
        uint64_t at = (uint64_t)a->offset + (uint64_t)index * a->stride;
        if (at > UINT32_MAX || !gpu_guest_span((uint32_t)at, bytes, 0))
            return 0;
        p = mem + (size_t)at;
    }

    switch (a->type) {
    case 0:                                  /* D3DCOLOR */
        /* A DWORD 0xAARRGGBB, so little-endian bytes are B,G,R,A -- not the
         * component order of every other format here. Returned as R,G,B,A so
         * callers need not know which format the title chose. */
        out[0] = (float)p[2] / 255.0f;
        out[1] = (float)p[1] / 255.0f;
        out[2] = (float)p[0] / 255.0f;
        out[3] = (float)p[3] / 255.0f;
        return 1;
    case 2:                                  /* float */
        for (i = 0; i < a->size && i < 4; i++)
            out[i] = ((const float *)p)[i];
        return 1;
    case 4:                                  /* unsigned byte, normalised */
        for (i = 0; i < a->size && i < 4; i++)
            out[i] = (float)p[i] / 255.0f;
        return 1;
    /* The formats 3D geometry uses and screen-space quads never did. */
    case 1:                                  /* signed short, normalised */
        for (i = 0; i < a->size && i < 4; i++)
            out[i] = (float)((const int16_t *)p)[i] / 32767.0f;
        return 1;
    case 5:                                  /* signed short, as is */
        for (i = 0; i < a->size && i < 4; i++)
            out[i] = (float)((const int16_t *)p)[i];
        return 1;
    case 6: {                                /* packed 11:11:10 normal */
        uint32_t v = *(const uint32_t *)p;
        out[0] = (float)((int32_t)(v << 21) >> 21) / 1023.0f;
        out[1] = (float)((int32_t)(v << 10) >> 21) / 1023.0f;
        out[2] = (float)((int32_t)v >> 22) / 511.0f;
        return 1;
    }
    default:
        return 0;
    }
}

static uint32_t surface_bpp(void);

/* NV2A stores two samples horizontally, or four in a 2 x 2 block.
 * Clip coordinates remain in pixels. See xemu pgraph.h's AA factor. */
static unsigned surface_samples_x(void)
{ unsigned aa = (s_gpu.format >> 12) & 15; return aa == 1 || aa == 2 ? 2 : 1; }
static unsigned surface_samples_y(void)
{ return ((s_gpu.format >> 12) & 15) == 2 ? 2 : 1; }

static void note_drawn(void)
{
    if (!s_gpu.drawn_stale && s_gpu.drawn_offset
        && s_gpu.clip_w * s_gpu.clip_h < s_gpu.drawn_w * s_gpu.drawn_h)
        return;
    s_gpu.drawn_stale = 0;
    s_gpu.drawn_offset = s_gpu.color_offset;
    s_gpu.drawn_pitch = s_gpu.pitch;
    s_gpu.drawn_bpp = surface_bpp();
    s_gpu.drawn_samples_x = surface_samples_x();
    s_gpu.drawn_samples_y = surface_samples_y();
    s_gpu.drawn_x = s_gpu.clip_x; s_gpu.drawn_y = s_gpu.clip_y;
    s_gpu.drawn_w = s_gpu.clip_w; s_gpu.drawn_h = s_gpu.clip_h;
}

static uint32_t surface_bpp(void)
{
    /* The surface format says it outright (NV097_SET_SURFACE_FORMAT_COLOR).
     * Deriving it from pitch / clip width -- the only way before -- is right
     * only while the clip spans the whole surface: Burnout 3 narrows the clip
     * to a 250-pixel window to draw its option values, 2560 / 250 came out as
     * 10 bytes a pixel, and every such text quad was refused. */
    switch (s_gpu.format & 0xF) {
    case 0x1: case 0x2: case 0x3: case 0xA:
        return 2;
    case 0x4: case 0x5: case 0x6: case 0x7: case 0x8:
        return 4;
    case 0x9:
        return 1;
    default:
        break;
    }
    /* The pitch and the clip width together give the pixel size, which is more
     * reliable than decoding the format field: the format's colour code is
     * only meaningful alongside a type the title also sets, while the pitch is
     * always exactly how many bytes a row occupies. */
    if (!s_gpu.clip_w)
        return 0;
    return s_gpu.pitch / s_gpu.clip_w;
}


/* Write the current surface out as a 24-bit BMP.
 *
 * A framebuffer window needs someone watching it. A file does not, which makes
 * this the only way to check what a title actually rendered on a machine you
 * are not sitting at -- and the only way to put a picture in a bug report.
 *
 * ponytail: bottom-up 24bpp BMP, no palette, no compression. That is the one
 * format every viewer reads and it is 30 lines; PNG would need a dependency.
 */
/* Set while RECOMP_FB_DUMP_FLIPS is dumping every flip: the report and
 * after-draw dumps stand down then, or they would put duplicate frames into
 * the sequence. */
static int s_flip_dumping;

/* Set by a native renderer that draws the frames itself from the D3D API
 * (street1's D3D8 HLE backend, RECOMP_HLE_PRIMARY). The executor then keeps
 * what the pushbuffer means for synchronisation -- semaphores, notifies,
 * reports, flips -- and skips the rendering: no clears, no draws, nothing
 * handed to the framebuffer window. */
volatile int xbox_nv2a_draw_disabled;

static void dump_surface_bmp(void)
{
    if (t_exec_thread)
        gpu_surface_sync();
    const char *prefix = getenv("RECOMP_FB_DUMP");
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    int drawn = s_gpu.drawn_offset != 0;
    uint32_t offset = drawn ? s_gpu.drawn_offset : s_gpu.color_offset;
    uint32_t pitch = drawn ? s_gpu.drawn_pitch : s_gpu.pitch;
    uint32_t cx = drawn ? s_gpu.drawn_x : s_gpu.clip_x;
    uint32_t cy = drawn ? s_gpu.drawn_y : s_gpu.clip_y;
    static int seq;
    char path[512];
    uint32_t w = drawn ? s_gpu.drawn_w : s_gpu.clip_w;
    uint32_t h = drawn ? s_gpu.drawn_h : s_gpu.clip_h, y, x;
    uint32_t bpp = drawn ? s_gpu.drawn_bpp : surface_bpp();
    uint32_t sx = drawn ? s_gpu.drawn_samples_x : surface_samples_x();
    uint32_t sy = drawn ? s_gpu.drawn_samples_y : surface_samples_y();
    uint32_t row_bytes, pad, filesz;
    uint8_t hdr[54];
    FILE *f;

    if (!prefix || !w || !h || (bpp != 2 && bpp != 4) || !offset)
        return;

    row_bytes = w * 3;
    pad = (4 - (row_bytes & 3)) & 3;
    filesz = 54 + (row_bytes + pad) * h;

    snprintf(path, sizeof path, "%s%05d.bmp", prefix, seq++);
    f = fopen(path, "wb");
    if (!f)
        return;

    memset(hdr, 0, sizeof hdr);
    hdr[0] = 'B'; hdr[1] = 'M';
    memcpy(hdr + 2, &filesz, 4);
    hdr[10] = 54;
    hdr[14] = 40;
    memcpy(hdr + 18, &w, 4);
    memcpy(hdr + 22, &h, 4);
    hdr[26] = 1;
    hdr[28] = 24;
    fwrite(hdr, 1, sizeof hdr, f);

    /* BMP rows run bottom-up. */
    for (y = h; y-- > 0; ) {
        for (x = 0; x < w; x++) {
            uint8_t bgr[3];
            uint32_t sum[3] = {0, 0, 0};
            for (unsigned iy = 0; iy < sy; ++iy) {
                const uint8_t *row = mem + dma_resolve(offset)
                                   + (size_t)((cy + y) * sy + iy) * pitch;
                for (unsigned ix = 0; ix < sx; ++ix) {
                    unsigned col = (cx + x) * sx + ix;
                    if (bpp == 4) {
                        uint32_t v = ((const uint32_t *)row)[col];
                        sum[0] += v & 255; sum[1] += (v >> 8) & 255; sum[2] += (v >> 16) & 255;
                    } else {
                        uint16_t v = ((const uint16_t *)row)[col];
                        sum[0] += (v & 31) * 255 / 31;
                        sum[1] += ((v >> 5) & 63) * 255 / 63;
                        sum[2] += ((v >> 11) & 31) * 255 / 31;
                    }
                }
            }
            for (unsigned channel = 0; channel < 3; ++channel)
                bgr[channel] = (uint8_t)(sum[channel] / (sx * sy));
            fwrite(bgr, 1, 3, f);
        }
        if (pad) {
            static const uint8_t zero[3] = {0, 0, 0};
            fwrite(zero, 1, pad, f);
        }
    }
    fclose(f);
    if (seq == 1)
        fprintf(stderr, "  [GPU] framebuffer dump: %s (%ux%u from 0x%08X %ubpp)\n",
                path, w, h, s_gpu.color_offset, bpp);
}

/* Defined below, next to the rest of the rasteriser; the clear path uses it
 * for RECOMP_RASTER_TEST. */
static void raster_triangle(const float a[2], const float b[2],
                            const float c[2], uint32_t argb,
                            const float uv[3][2]);

static void clear_surface(uint32_t param)
{
    uint8_t *mem = (uint8_t *)xbox_GetMemoryOffset();
    uint32_t bpp = surface_bpp();
    uint32_t y, x;

    if (!(param & NV097_CLEAR_COLOR_MASK))
        return;                            /* depth/stencil only */
    if (!s_gpu.color_offset || !s_gpu.pitch || !s_gpu.clip_h || bpp == 0)
        return;
    {
        uint32_t base = dma_resolve(s_gpu.color_offset);
        if (surface_write_refused(base,
                                  (s_gpu.clip_y + s_gpu.clip_h) * surface_samples_y() * s_gpu.pitch,
                                  "clear"))
            return;
        s_gpu.color_base = base;
        trace_surface_write("clear", base,
                            (s_gpu.clip_y + s_gpu.clip_h) * surface_samples_y() * s_gpu.pitch);
    }

    for (y = 0; y < s_gpu.clip_h * surface_samples_y(); y++) {
        uint8_t *row = mem + s_gpu.color_base
                     + (size_t)(s_gpu.clip_y * surface_samples_y() + y) * s_gpu.pitch;
        if (bpp == 4) {
            uint32_t *p = (uint32_t *)row + s_gpu.clip_x * surface_samples_x();
            for (x = 0; x < s_gpu.clip_w * surface_samples_x(); x++)
                p[x] = s_gpu.clear_color;
        } else if (bpp == 2) {
            /* The clear value is always given as A8R8G8B8; a 16-bit surface
             * takes the same colour reduced to 5:6:5. */
            uint16_t v = (uint16_t)(((s_gpu.clear_color >> 8) & 0xF800)
                                  | ((s_gpu.clear_color >> 5) & 0x07E0)
                                  | ((s_gpu.clear_color >> 3) & 0x001F));
            uint16_t *p = (uint16_t *)row + s_gpu.clip_x * surface_samples_x();
            for (x = 0; x < s_gpu.clip_w * surface_samples_x(); x++)
                p[x] = v;
        }
    }
    s_gpu.clears++;
    /* Progress markers, interleaved with everything else in the log. The
     * summary says drawing stopped; only a marker next to the surrounding
     * activity says what the title was doing when it stopped. */
    if ((s_gpu.clears % 100) == 0)
        fprintf(stderr, "  [GPU] clear #%u\n", s_gpu.clears);
    /* Distinct clear colours actually used. "Cleared to black" and "the clear
     * never ran" look identical in the framebuffer, and only one of them is a
     * bug -- so record what was asked for, not just how often. */
    {
        static uint32_t seen[8];
        static int n;
        int i;
        for (i = 0; i < n; i++)
            if (seen[i] == s_gpu.clear_color) break;
        if (i == n && n < 8) {
            seen[n++] = s_gpu.clear_color;
            fprintf(stderr, "  [GPU] clear colour 0x%08X -> surface 0x%08X"
                            " (%ubpp)\n",
                    s_gpu.clear_color, s_gpu.color_offset, surface_bpp());
        }
    }

    /* Prove the pixel path end to end, independent of whether the title has
     * given us any geometry yet.
     *
     * "Nothing on screen" has three very different causes -- the surface
     * address or pitch is wrong, the rasteriser is broken, or the title's
     * vertex buffers are empty -- and they are indistinguishable from a black
     * window. RECOMP_RASTER_TEST draws one known triangle into the surface
     * just cleared, so a visible triangle rules out the first two and leaves
     * only the third. On Wreckless it is the third: attribute 0 decodes
     * correctly (float, size 2, stride 16) and the buffer it points at stays
     * zero.
     *
     * ponytail: bring-up aid, not a feature. It costs one branch per clear. */
    static int raster_test = -1;
    if (raster_test < 0)
        raster_test = getenv("RECOMP_RASTER_TEST") != NULL;
    if (raster_test) {
        static int announced;
        /* Every clear, not once: the title clears each frame and double-buffers,
         * so a triangle drawn a single time is erased before anyone sees it. */
        if (s_gpu.clip_w && s_gpu.clip_h) {
            float a[2], b[2], c[2];
            a[0] = s_gpu.clip_w * 0.5f; a[1] = s_gpu.clip_h * 0.15f;
            b[0] = s_gpu.clip_w * 0.85f; b[1] = s_gpu.clip_h * 0.85f;
            c[0] = s_gpu.clip_w * 0.15f; c[1] = s_gpu.clip_h * 0.85f;
            raster_triangle(a, b, c, 0xFFFF00FFu, NULL);  /* magenta: never a clear colour */
            if (announced++ == 0)
            fprintf(stderr, "  [GPU] raster self-test: triangle (%.0f,%.0f)"
                            " (%.0f,%.0f) (%.0f,%.0f) into 0x%08X %ubpp\n",
                    a[0], a[1], b[0], b[1], c[0], c[1],
                    s_gpu.color_offset, surface_bpp());
        }
    }

    /* Show the surface actually being drawn into. A title that double-buffers
     * renders into the back buffer, so following AvSetDisplayMode's address
     * would show the one nothing is writing. */
    /* The window has to read where the pixels actually are, which is the
     * resolved address rather than the DMA-object offset. */
    /* Only until the title flips. Following the draw surface on every scan
     * shows the buffer being written right now, half a frame at a time; past
     * the first flip the window is repointed at the finished one instead. */
    if (s_gpu.flips == 0)
        xbox_FramebufferWindowSet(dma_resolve(s_gpu.color_offset), s_gpu.pitch);

    /* And open the window, rather than waiting for AvSetDisplayMode to do it.
     *
     * That was the only caller, so a title which draws before setting a display
     * mode -- or never sets one at all -- got no window however much it
     * rendered. The Xbox Dashboard clears a 1280x960 surface at 0x00088000 on
     * its first frame and had not called AvSetDisplayMode by then, so
     * RECOMP_FB_WINDOW=1 was set, the executor knew the address and the pitch,
     * and nothing appeared.
     *
     * Here is the better trigger anyway: this runs when a surface address is
     * known to be real, because a clear just used it. Idempotent and gated on
     * RECOMP_FB_WINDOW, so the cost is one interlocked compare per clear. */
    xbox_FramebufferWindowStart();
}


/* ── Rasteriser ──────────────────────────────────────────────────────────
 *
 * Fills triangles straight into the guest framebuffer, the same memory
 * clear_surface() writes and the framebuffer window already shows. That is the
 * whole reason it is done on the CPU rather than through D3D: nothing new has
 * to be plumbed for the result to be visible.
 *
 * ponytail: flat-shaded, no depth buffer, no texturing, no perspective
 * correction, and only batches whose attribute 0 is already in screen space.
 * A title running a vertex program hands over object-space positions that mean
 * nothing without executing the program, so those batches are counted and
 * skipped rather than drawn somewhere wrong. Upgrade path is the D3D11
 * translator in src/nv2a/nv2a_pgraph_d3d11.c once vertex programs are
 * translated; this exists to get the first geometry on screen for every title,
 * which in practice is UI, HUD and 2D overlays -- all pre-transformed.
 */

/* One texel, in the title's own format.
 *
 * The codes are the NV097 colour field, which is the Xbox D3DFMT_ enum --
 * src/d3d/d3d8_xbox.h is the table, and it is the table to check against
 * rather than recollection: 0x1E is LIN_X8R8G8B8 and not, as this first read
 * it, a byte-reversed BGRA. Getting that one wrong turned an opaque black
 * render target into a screen of pure blue, which is the kind of wrong that
 * looks like content.
 *
 * Only the linear (LIN_) formats are read. A swizzled texture stores its
 * texels in Morton order rather than in rows, so reading one as if it had a
 * pitch does not give a slightly wrong colour, it gives a different image --
 * and inventing that image is exactly what this is not for. An unsupported
 * format samples nothing and the caller keeps the vertex colour, which is
 * visibly wrong rather than quietly wrong.
 *
 * ponytail: nearest texel, no filtering, whatever SET_TEXTURE_FILTER asked
 * for. Bilinear when a title's output actually depends on it.
 */
/* Off the edge of the texture, the way the title asked for.
 *
 * Refusing to sample instead is not neutral: it hands the caller back the
 * vertex colour, so a pass whose coordinates reach the last texel by half a
 * texel gets a bright line down the edge of the screen. The dashboard's
 * resolve does exactly that -- its last column and last row, 1119 pixels of
 * white on a black frame, from a rounding step at the boundary.
 */
static uint32_t wrap_coord(uint32_t c, uint32_t size, uint32_t mode)
{
    /* Coordinates are signed: a bilinear tap one texel left of or above
     * texel 0 is -1, which must wrap to the far edge or clamp to 0 -- as an
     * unsigned value it clamped to the far edge instead. */
    int32_t sc = (int32_t)c, n = (int32_t)size;
    if (!size)
        return 0;
    if (mode == 1)                         /* wrap */
        return (uint32_t)(((sc % n) + n) % n);
    return sc < 0 ? 0u : (sc >= n ? size - 1 : c);   /* clamp, and the rest */
}

static uint32_t expand(uint32_t v, uint32_t bits)
{
    return d3d8_expand_channel(v, bits);
}

/* The linear format that decodes the same texels as a swizzled one.
 *
 * Swizzling changes where a texel lives, not what it says: A8R8G8B8 (0x06) and
 * LIN_A8R8G8B8 (0x12) are the same four bytes in the same order. So the whole
 * difference is the address calculation, and one of those lets every format
 * below serve both. Pairs read off the table in d3d8_xbox.h rather than
 * recalled -- the comment above this one is about getting exactly that wrong. */
static uint32_t linear_twin(uint32_t fmt)
{
    switch (fmt) {
    case 0x00: return 0x13;                /* L8        -> LIN_L8        */
    case 0x02: return 0x10;                /* A1R5G5B5  -> LIN_A1R5G5B5  */
    case 0x03: return 0x1C;                /* X1R5G5B5  -> LIN_X1R5G5B5  */
    case 0x04: return 0x1D;                /* A4R4G4B4  -> LIN_A4R4G4B4  */
    case 0x05: return 0x11;                /* R5G6B5    -> LIN_R5G6B5    */
    case 0x06: return 0x12;                /* A8R8G8B8  -> LIN_A8R8G8B8  */
    case 0x07: return 0x1E;                /* X8R8G8B8  -> LIN_X8R8G8B8  */
    case 0x19: return 0x1F;                /* A8        -> LIN_A8        */
    default:   return fmt;                 /* already linear, or unhandled */
    }
}

/* Sample texel (u, v) of stage texture `t`; `face` offsets a cube map. */
static int sample_tex(const Texture *t, uint32_t face_offset,
                      uint32_t u, uint32_t v, uint32_t *argb)
{
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    const uint8_t *p;
    uint32_t fmt, base = t->offset + face_offset;

    if (!t->valid)
        return 0;
    u = wrap_coord(u, t->width,  t->addr_u);
    v = wrap_coord(v, t->height, t->addr_v);

    fmt = t->color;
    if (d3d8_format_dxt_block_bytes(fmt)) {
        /* Decoded blocks, direct-mapped by address. Every texel of a DXT
         * texture decodes its whole 4x4 block, and bilinear reads four
         * neighbours that nearly always share one -- this was a tenth of
         * the executor's time. Keyed on the block's bytes as well as its
         * address, so a texture rewritten in place (a render target reused,
         * a streamed mip) never returns stale texels.
         * ponytail: single-threaded executor, so a plain static table. */
        /* Allocated on the first DXT read of each thread: as a 384 KB
         * thread-local array it went into every thread's static TLS, which
         * glibc carves out of the thread's stack -- a library thread with a
         * small stack (SDL's 256 KB audio hotplug thread) then failed to
         * start, and SDL waited for it for ever. */
        typedef struct { uintptr_t key; uint32_t fmt; uint64_t raw[2];
                         uint32_t px[16]; } DxtBlock;
        static NV_TLS DxtBlock *cache;
        uint32_t bb = d3d8_format_dxt_block_bytes(fmt);
        if (!cache && !(cache = (DxtBlock *)calloc(4096, sizeof *cache)))
            return 0;
        uint32_t bx = u >> 2, by = v >> 2, bw = (t->width + 3) >> 2;
        const uint8_t *blk = mem + base + ((size_t)by * bw + bx) * bb;
        uintptr_t key = (uintptr_t)blk;
        size_t slot = ((key / bb) ^ (key >> 16)) & 4095;
        uint64_t raw[2] = {0, 0};
        memcpy(raw, blk, bb);
        if (cache[slot].key != key || cache[slot].fmt != fmt
            || cache[slot].raw[0] != raw[0] || cache[slot].raw[1] != raw[1]) {
            uint32_t i;
            for (i = 0; i < 16; i++)
                d3d8_dxt_decode_texel(mem + base, fmt, bx * 4 + (i & 3),
                                      by * 4 + (i >> 2), t->width,
                                      &cache[slot].px[i]);
            cache[slot].key = key;
            cache[slot].fmt = fmt;
            cache[slot].raw[0] = raw[0];
            cache[slot].raw[1] = raw[1];
        }
        *argb = cache[slot].px[(v & 3) * 4 + (u & 3)];
        return 1;
    }
    if (d3d8_format_is_swizzled(fmt)) {
        /* Morton order: a texel's index is interleaved from x and y instead of
         * v*pitch + u, so index from the base of the image. The switch below
         * casts to each format's own width, which makes that index a texel
         * index for every one of them. */
        fmt = linear_twin(fmt);
        p = mem + base;
        u = swizzle_offset(u, v, t->width, t->height);
    } else {
        p = mem + base + (size_t)v * t->pitch;
    }

    switch (fmt) {

    /* 32-bit, alpha-red-green-blue in the dword. */
    case 0x12:                                      /* LIN_A8R8G8B8 */
        *argb = ((const uint32_t *)p)[u];
        return 1;
    case 0x1E:                                      /* LIN_X8R8G8B8 */
        *argb = ((const uint32_t *)p)[u] | 0xFF000000u;
        return 1;

    /* 8-bit palette index, swizzled. Burnout 3 draws its logo and frontend
     * header art this way; without a case every such quad came out as the
     * unsupported-format fill, a white box where the logo should be. */
    case 0x0B:                                      /* SZ_I8_A8R8G8B8 */
        if (!t->palette)
            return 0;
        *argb = ((const uint32_t *)(mem + t->palette))[p[u]];
        return 1;

    /* 32-bit, other channel orders. The name gives the byte order from the
     * top of the dword down, so each is a permutation of the same four. */
    case 0x3F: {                                    /* LIN_A8B8G8R8 */
        uint32_t t = ((const uint32_t *)p)[u];
        *argb = (t & 0xFF00FF00u) | ((t & 0xFF) << 16) | ((t >> 16) & 0xFF);
        return 1;
    }
    case 0x40: {                                    /* LIN_B8G8R8A8 */
        uint32_t t = ((const uint32_t *)p)[u];
        *argb = ((t & 0xFFu) << 24)                 /* A, from the bottom */
              | (((t >>  8) & 0xFFu) << 16)         /* R */
              | (((t >> 16) & 0xFFu) <<  8)         /* G */
              |  ((t >> 24) & 0xFFu);               /* B, from the top */
        return 1;
    }
    case 0x41: {                                    /* LIN_R8G8B8A8 */
        uint32_t t = ((const uint32_t *)p)[u];
        *argb = ((t & 0xFFu) << 24) | (t >> 8);
        return 1;
    }

    /* 16-bit. */
    case 0x10: {                                    /* LIN_A1R5G5B5 */
        uint32_t t = ((const uint16_t *)p)[u];
        *argb = ((t & 0x8000u) ? 0xFF000000u : 0u)
              | (expand((t >> 10) & 0x1F, 5) << 16)
              | (expand((t >>  5) & 0x1F, 5) <<  8)
              |  expand( t        & 0x1F, 5);
        return 1;
    }
    case 0x1C: {                                    /* LIN_X1R5G5B5 */
        uint32_t t = ((const uint16_t *)p)[u];
        *argb = 0xFF000000u
              | (expand((t >> 10) & 0x1F, 5) << 16)
              | (expand((t >>  5) & 0x1F, 5) <<  8)
              |  expand( t        & 0x1F, 5);
        return 1;
    }
    case 0x11: {                                    /* LIN_R5G6B5 */
        uint32_t t = ((const uint16_t *)p)[u];
        *argb = 0xFF000000u
              | (expand((t >> 11) & 0x1F, 5) << 16)
              | (expand((t >>  5) & 0x3F, 6) <<  8)
              |  expand( t        & 0x1F, 5);
        return 1;
    }
    case 0x1D: {                                    /* LIN_A4R4G4B4 */
        uint32_t t = ((const uint16_t *)p)[u];
        *argb = (expand((t >> 12) & 0x0F, 4) << 24)
              | (expand((t >>  8) & 0x0F, 4) << 16)
              | (expand((t >>  4) & 0x0F, 4) <<  8)
              |  expand( t        & 0x0F, 4);
        return 1;
    }

    /* 8-bit. */
    case 0x13: {                                    /* LIN_L8 */
        uint32_t t = p[u];
        *argb = 0xFF000000u | (t << 16) | (t << 8) | t;
        return 1;
    }
    case 0x1F:                                      /* LIN_A8 */
        *argb = ((uint32_t)p[u] << 24) | 0x00FFFFFFu;
        return 1;

    /* 4:2:2 packed YUV, two texels per four bytes.
     *
     * This is how a title hands over a decoded video frame, and without it
     * the frame falls through to `default` -- which returns 0, so the caller
     * paints the quad's vertex colour and the movie is a flat rectangle.
     *
     * The chroma pair is shared between an even texel and the one after it,
     * so the group is found by masking the bottom bit of the index. BT.601,
     * the same coefficients the D3D8 upload path converts with, so the two
     * paths agree rather than each having its own idea of the colour. */
    case 0x24:                                      /* LC_CR8YB8CB8YA8, YUY2 */
    case 0x25: {                                    /* LC_YB8CR8YA8CB8, UYVY */
        uint32_t yoff = (fmt == 0x24) ? 0u : 1u;
        const uint8_t *g = p + (size_t)(u & ~1u) * 2;
        int c  = (int)g[(u & 1u) ? 2 + yoff : yoff] - 16;
        int cu = (int)g[1 - yoff] - 128;
        int cv = (int)g[3 - yoff] - 128;
        int r = (298 * c + 409 * cv + 128) >> 8;
        int gg = (298 * c - 100 * cu - 208 * cv + 128) >> 8;
        int b = (298 * c + 516 * cu + 128) >> 8;
        if (r < 0) r = 0;
        if (r > 255) r = 255;
        if (gg < 0) gg = 0;
        if (gg > 255) gg = 255;
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        *argb = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)gg << 8)
              | (uint32_t)b;
        return 1;
    }

    default:
        return 0;
    }
}

static int sample_texture(uint32_t u, uint32_t v, uint32_t *argb)
{
    return sample_tex(&s_gpu.texs[0], 0, u, v, argb);
}

/* Write a bound texture out as a BMP, through the sampler rather than around it.
 *
 * "Which texture is this" is not answerable from an address and a format, and
 * it is the question behind most of the ones that matter -- is that a font
 * page or an icon atlas, did the swizzle decode, is the alpha inverted. Going
 * through sample_texture means the file shows exactly what the rasteriser
 * sees, so a decode bug appears here rather than only as a wrong-looking
 * triangle.
 *
 * ponytail: RGB only, alpha dropped. A glyph page is alpha and would come out
 * black, so alpha is composited onto mid-grey to stay legible; that is a
 * viewing choice, not a decode. One file per distinct texture, first use only.
 */
static void dump_texture_bmp(uint32_t seq)
{
    const char *prefix = getenv("RECOMP_TEX_DUMP");
    uint32_t w = s_gpu.texs[0].width, h = s_gpu.texs[0].height, x, y;
    uint32_t row_bytes, pad, filesz;
    uint8_t hdr[54];
    char path[512];
    FILE *f;

    if (!prefix || !w || !h || w > 4096 || h > 4096)
        return;
    row_bytes = w * 3;
    pad = (4 - (row_bytes & 3)) & 3;
    filesz = 54 + (row_bytes + pad) * h;

    snprintf(path, sizeof path, "%s%02u_%08X_fmt%02X.bmp",
             prefix, seq, s_gpu.texs[0].offset, s_gpu.texs[0].color);
    f = fopen(path, "wb");
    if (!f)
        return;
    memset(hdr, 0, sizeof hdr);
    hdr[0] = 'B'; hdr[1] = 'M';
    memcpy(hdr + 2, &filesz, 4);
    hdr[10] = 54; hdr[14] = 40;
    memcpy(hdr + 18, &w, 4);
    memcpy(hdr + 22, &h, 4);
    hdr[26] = 1; hdr[28] = 24;
    fwrite(hdr, 1, sizeof hdr, f);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            uint32_t argb = 0, a;
            uint8_t px[3];
            if (!sample_texture(x, h - 1 - y, &argb))
                argb = 0;
            a = (argb >> 24) & 0xFFu;
            /* over mid-grey, so an alpha-only page is visible either way */
            px[0] = (uint8_t)(((argb & 0xFFu) * a + 128u * (255u - a)) / 255u);
            px[1] = (uint8_t)((((argb >> 8) & 0xFFu) * a + 128u * (255u - a)) / 255u);
            px[2] = (uint8_t)((((argb >> 16) & 0xFFu) * a + 128u * (255u - a)) / 255u);
            fwrite(px, 1, 3, f);
        }
        if (pad) {
            static const uint8_t zero[3] = {0, 0, 0};
            fwrite(zero, 1, pad, f);
        }
    }
    fclose(f);
    fprintf(stderr, "  [TEXDUMP] %s (%ux%u fmt 0x%02X)\n",
            path, w, h, s_gpu.texs[0].color);
    fflush(stderr);
}

/* The surface, resolved once per batch.
 *
 * dma_resolve consults the contiguous arena's high-water mark and
 * surface_hits_image walks the image range; both were being done per pixel
 * -- dma_resolve twice -- which cost more than the rasterisation they
 * guarded. Neither answer can change inside a batch, because the colour
 * offset arrives as a method and a method cannot arrive mid-triangle.
 *
 * This is not a micro-optimisation for its own sake: the loader's video
 * paces on frames actually presented, so the rasteriser's throughput is the
 * playback rate. */
static uint8_t *s_surface;          /* host address of surface row 0 */

static int surface_begin_batch(const uint8_t *mem)
{
    uint32_t base = dma_resolve(s_gpu.color_offset);

    if (surface_hits_image(base, (s_gpu.clip_y + s_gpu.clip_h) * surface_samples_y() * s_gpu.pitch))
        return 0;
    s_surface = (uint8_t *)mem + base;
    return 1;
}

static uint32_t pack_color(const float c[4]);

/* One GL blend factor, per channel (r g b a). */
static void blend_factor(uint32_t f, const float s[4], const float d[4],
                         float out[4])
{
    int k;
    for (k = 0; k < 4; k++) {
        float c = (float)((s_gpu.blend_color >> (k == 3 ? 24 : 16 - 8 * k))
                          & 0xFF) / 255.0f;
        switch (f) {
        case 0x0000: out[k] = 0.0f;            break;   /* ZERO                */
        case 0x0001: out[k] = 1.0f;            break;   /* ONE                 */
        case 0x0300: out[k] = s[k];            break;   /* SRC_COLOR           */
        case 0x0301: out[k] = 1.0f - s[k];     break;
        case 0x0302: out[k] = s[3];            break;   /* SRC_ALPHA           */
        case 0x0303: out[k] = 1.0f - s[3];     break;
        case 0x0304: out[k] = d[3];            break;   /* DST_ALPHA           */
        case 0x0305: out[k] = 1.0f - d[3];     break;
        case 0x0306: out[k] = d[k];            break;   /* DST_COLOR           */
        case 0x0307: out[k] = 1.0f - d[k];     break;
        case 0x0308: out[k] = k == 3 ? 1.0f : fminf(s[3], 1.0f - d[3]); break;
        case 0x8001: out[k] = c;               break;   /* CONSTANT_COLOR      */
        case 0x8002: out[k] = 1.0f - c;        break;
        case 0x8003: out[k] = (float)(s_gpu.blend_color >> 24) / 255.0f; break;
        case 0x8004: out[k] = 1.0f - (float)(s_gpu.blend_color >> 24) / 255.0f; break;
        default:     out[k] = f ? 1.0f : 0.0f; break;
        }
    }
}

static void put_sample(uint8_t *mem, uint32_t bpp, int x, int y, unsigned sample, uint32_t argb)
{
    uint8_t *row;

    (void)mem;
    if (x < (int)s_gpu.clip_x || x >= (int)(s_gpu.clip_x + s_gpu.clip_w))
        return;
    if (y < (int)s_gpu.clip_y || y >= (int)(s_gpu.clip_y + s_gpu.clip_h))
        return;
    y = y * (int)surface_samples_y() + (int)(sample / surface_samples_x());
    x = x * (int)surface_samples_x() + (int)(sample % surface_samples_x());
    s_gpu.pixels++;
    if ((argb & 0x00FFFFFFu) > (s_gpu.pixel_max & 0x00FFFFFFu))
        s_gpu.pixel_max = argb;
    row = s_surface + (size_t)y * s_gpu.pitch;

    /* Blending, with the GL factor set and equations the NV2A takes.
     *
     * Only SRC_ALPHA / ONE_MINUS_SRC_ALPHA used to be honoured and every other
     * pair was written opaque. Menus never noticed; a race does: Burnout 3
     * finishes its 3D frame with screen-space passes that modulate or add
     * over the scene, and written opaque they paint the whole view over. */
    if (s_gpu.blend_enable
        && !(s_gpu.blend_sfactor == 1 && s_gpu.blend_dfactor == 0)) {
        uint32_t dst = 0xFF000000u;
        float sc[4], dc[4], sf[4], df[4], out[4];
        int k;
        if (bpp == 4) {
            dst = ((const uint32_t *)row)[x];
        } else if (bpp == 2) {
            uint32_t t = ((const uint16_t *)row)[x];
            dst = 0xFF000000u | ((t & 0xF800u) << 8) | ((t & 0x07E0u) << 5)
                | ((t & 0x001Fu) << 3);
        }
        for (k = 0; k < 4; k++) {           /* r g b a, 0..1 */
            int sh = k == 3 ? 24 : 16 - 8 * k;
            sc[k] = (float)((argb >> sh) & 0xFF) / 255.0f;
            dc[k] = (float)((dst  >> sh) & 0xFF) / 255.0f;
        }
        blend_factor(s_gpu.blend_sfactor, sc, dc, sf);
        blend_factor(s_gpu.blend_dfactor, sc, dc, df);
        for (k = 0; k < 4; k++) {
            float a = sc[k] * sf[k], b = dc[k] * df[k];
            switch (s_gpu.blend_equation) {
            case 0x800A: out[k] = a - b; break;               /* SUBTRACT     */
            case 0x800B: out[k] = b - a; break;               /* REV_SUBTRACT */
            case 0x8007: out[k] = fminf(sc[k], dc[k]); break; /* MIN          */
            case 0x8008: out[k] = fmaxf(sc[k], dc[k]); break; /* MAX          */
            default:     out[k] = a + b; break;               /* ADD          */
            }
        }
        argb = pack_color(out);
    }

    if (s_gpu.color_mask != 0x01010101u) {
        uint32_t keep = 0, old;
        if (!(s_gpu.color_mask & 0x01000000u)) keep |= 0xFF000000u;
        if (!(s_gpu.color_mask & 0x00010000u)) keep |= 0x00FF0000u;
        if (!(s_gpu.color_mask & 0x00000100u)) keep |= 0x0000FF00u;
        if (!(s_gpu.color_mask & 0x00000001u)) keep |= 0x000000FFu;
        if (keep == 0xFFFFFFFFu)
            return;
        if (bpp == 4) {
            old = ((const uint32_t *)row)[x];
        } else {
            uint32_t t = ((const uint16_t *)row)[x];
            old = ((t & 0xF800u) << 8) | ((t & 0x07E0u) << 5) | ((t & 0x001Fu) << 3);
        }
        argb = (argb & ~keep) | (old & keep);
    }
    if (bpp == 4) {
        ((uint32_t *)row)[x] = argb;
    } else if (bpp == 2) {
        ((uint16_t *)row)[x] = (uint16_t)(((argb >> 8) & 0xF800)
                                        | ((argb >> 5) & 0x07E0)
                                        | ((argb >> 3) & 0x001F));
    }
}

static void put_pixel(uint8_t *mem, uint32_t bpp, int x, int y, uint32_t argb)
{
    unsigned count = surface_samples_x() * surface_samples_y();
    for (unsigned sample = 0; sample < count; ++sample)
        put_sample(mem, bpp, x, y, sample, argb);
}

/* Half-space fill. Barycentric edge functions rather than scanline slopes:
 * the same test decides both windings, so a title that emits clockwise
 * triangles does not silently render nothing. */
static int gpu_guest_span(uint32_t address, uint64_t bytes, int write)
{
#ifdef _WIN32
    uintptr_t start = (uintptr_t)xbox_GetMemoryOffset() + address;
    if (!bytes || bytes > UINT32_MAX || (uint64_t)address + bytes > (UINT64_C(1) << 32)) return 0;
    uintptr_t end = start + (uintptr_t)bytes;
    if (end < start) return 0;
    /* Regions already validated in this batch. fetch_attr asks per vertex
     * attribute, and a VirtualQuery each time was most of a frame. The cache
     * dies with s_span_epoch, which moves at every flip and sync -- the
     * points where the title may have released memory -- so it is checked
     * again from there. */
    static struct { uintptr_t lo, hi; int writable; } cache[4];
    static unsigned cache_epoch = ~0u, cache_next;
    if (cache_epoch != s_span_epoch) {
        memset(cache, 0, sizeof cache);
        cache_epoch = s_span_epoch;
    }
    for (unsigned c = 0; c < 4; c++)
        if (start >= cache[c].lo && end <= cache[c].hi && (!write || cache[c].writable))
            return 1;
    uintptr_t lo = 0, hi = 0;
    int all_writable = 1;
    for (uintptr_t at = start; at < end;) {
        MEMORY_BASIC_INFORMATION info;
        if (!VirtualQuery((const void *)at, &info, sizeof(info)) || info.State != MEM_COMMIT
            || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS))) return 0;
        DWORD prot = info.Protect & 0xFF;
        int readable = prot == PAGE_READONLY || prot == PAGE_READWRITE || prot == PAGE_WRITECOPY
            || prot == PAGE_EXECUTE_READ || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_EXECUTE_WRITECOPY;
        int writable = prot == PAGE_READWRITE || prot == PAGE_WRITECOPY
            || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_EXECUTE_WRITECOPY;
        if (!readable || (write && !writable)) return 0;
        uintptr_t next = (uintptr_t)info.BaseAddress + info.RegionSize;
        if (next <= at) return 0;
        if (!lo) lo = (uintptr_t)info.BaseAddress;
        hi = next;
        all_writable &= writable;
        at = next;
    }
    cache[cache_next].lo = lo;
    cache[cache_next].hi = hi;
    cache[cache_next].writable = all_writable;
    cache_next = (cache_next + 1) & 3;
    return 1;
#else
    (void)address; (void)bytes; (void)write;
    return 0; /* Hardware backend is Windows-only. */
#endif
}

static int gpu_texture_layout(const Texture *t, size_t *bytes)
{
    uint64_t span;
    uint32_t block = d3d8_format_dxt_block_bytes(t->color);
    if (!t->width || !t->height || t->width > 4096 || t->height > 4096) return 0;
    if (block) span = ((uint64_t)t->width + 3) / 4 * (((uint64_t)t->height + 3) / 4) * block;
    else {
        uint32_t fmt = linear_twin(t->color), bytes;
        switch (fmt) {
        case 0x0B: case 0x13: case 0x1F: bytes = 1; break;
        case 0x10: case 0x11: case 0x1C: case 0x1D: case 0x24: case 0x25: bytes = 2; break;
        default: bytes = 4; break;
        }
        if (d3d8_format_is_swizzled(t->color))
            span = ((uint64_t)swizzle_offset(t->width - 1, t->height - 1, t->width, t->height) + 1) * bytes;
        else {
            uint64_t row = (uint64_t)t->width * bytes;
            if (fmt == 0x24 || fmt == 0x25) row = ((uint64_t)t->width + 1) / 2 * 4;
            if (t->pitch < row) return 0;
            span = (uint64_t)(t->height - 1) * t->pitch + row;
        }
        if (fmt == 0x0B && t->palette && !gpu_guest_span(t->palette, 256u * 4u, 0)) return 0;
    }
    /* Only level zero is read by this backend. Cube faces remain on CPU. */
    if (bytes) *bytes = (size_t)span;
    return gpu_guest_span(t->offset, span, 0);
}

static int gpu_texture_span(const Texture *t)
{
    return gpu_texture_layout(t, NULL);
}

/* Decoded level-zero texels, shared by all stages, found by descriptor and
 * replaced least recently used. Source bytes, including the complete P8
 * palette, are compared before reuse: address identity alone cannot
 * establish that streamed textures are unchanged. A match frame uses far
 * more than the four direct-mapped slots per stage this used to have. */
#define NV_DECODE_SLOTS 1024            /* 256 sets of 4 */
#define NV_DECODE_BUDGET (512u * 1024u * 1024u)
static struct {
    Texture key;
    uint8_t *raw;
    uint32_t *decoded;
    size_t raw_capacity, decoded_capacity, bytes;
    uint8_t palette[1024];
    int valid;
    unsigned used;
} s_decode_cache[NV_DECODE_SLOTS];
static unsigned s_decode_tick;
static uint64_t s_decode_hits, s_decode_misses, s_decode_texels;

/* Which textures keep missing: the report lists them, then starts over. */
static struct { uint32_t offset, color, width, height, misses, cacheable; } s_miss[32];
static void note_decode_miss(const Texture *t, int cacheable)
{
    unsigned i, free_slot = 32;
    for (i = 0; i < 32; i++) {
        if (s_miss[i].misses && s_miss[i].offset == t->offset && s_miss[i].color == t->color
            && s_miss[i].width == t->width && s_miss[i].height == t->height) {
            s_miss[i].misses++;
            return;
        }
        if (!s_miss[i].misses && free_slot == 32) free_slot = i;
    }
    if (free_slot < 32) {
        s_miss[free_slot].offset = t->offset; s_miss[free_slot].color = t->color;
        s_miss[free_slot].width = t->width; s_miss[free_slot].height = t->height;
        s_miss[free_slot].cacheable = (uint32_t)cacheable; s_miss[free_slot].misses = 1;
    }
}

/* Only what decoding depends on: wrap modes and filtering do not change
 * the texels, and keying on them made one texture several entries. */
static int decode_key_same(const Texture *a, const Texture *b)
{
    return a->offset == b->offset && a->width == b->width && a->height == b->height
        && a->pitch == b->pitch && a->color == b->color && a->palette == b->palette
        && a->cube == b->cube;
}

static size_t s_decode_bytes;           /* raw + decoded allocations held */

/* A match frame samples a few hundred textures in a cycle; with 96 fully
 * associative LRU entries every one of them missed every time. */
static unsigned gpu_decode_slot(unsigned stage, const Texture *t)
{
    unsigned set = ((t->offset >> 6) ^ (t->offset >> 17) ^ (t->color * 0x9E37u)
                    ^ (t->width * 31u) ^ t->height) & 255u;
    unsigned first = set * 4, victim = first, oldest = ~0u;
    (void)stage;
    s_decode_tick++;
    for (unsigned i = first; i < first + 4; i++)
        if (s_decode_cache[i].valid && decode_key_same(&s_decode_cache[i].key, t)) {
            s_decode_cache[i].used = s_decode_tick;
            return i;
        }
    for (unsigned i = first; i < first + 4; i++) {
        unsigned age = s_decode_cache[i].valid ? s_decode_cache[i].used : 0;
        if (age < oldest) { oldest = age; victim = i; }
    }
    s_decode_cache[victim].used = s_decode_tick;
    return victim;
}

static int gpu_decode_texture(unsigned stage, uint32_t width, uint32_t height,
                              uint32_t *output, int allow_missing)
{
    if (stage >= 4) return 0;
    const Texture *t = &s_gpu.texs[stage];
    unsigned slot = gpu_decode_slot(stage, t);
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    size_t bytes, count = (size_t)width * height;
    int palette = linear_twin(t->color) == 0x0B && t->palette;
    int cacheable, complete = 1;
    if (width != t->width || height != t->height || !output ||
        !gpu_texture_layout(t, &bytes)) return 0;
    /* At most 4 MiB per slot, including both source and decoded allocations. */
    cacheable = count <= 1024u * 1024u && bytes + count * 4 <= 4u * 1024u * 1024u;
    if (cacheable && s_decode_cache[slot].valid &&
        s_decode_cache[slot].bytes == bytes &&
        decode_key_same(&s_decode_cache[slot].key, t) &&
        !memcmp(s_decode_cache[slot].raw, mem + t->offset, bytes) &&
        (!palette || !memcmp(s_decode_cache[slot].palette, mem + t->palette, 1024))) {
        memcpy(output, s_decode_cache[slot].decoded, count * 4);
        s_decode_hits++;
        return 1;
    }
    s_decode_misses++;
    note_decode_miss(t, cacheable);
    s_decode_cache[slot].valid = 0;
    if (cacheable && s_decode_bytes + bytes + count * 4 > NV_DECODE_BUDGET
        && bytes > s_decode_cache[slot].raw_capacity)
        cacheable = 0;                 /* over budget: decode, do not keep */
    if (cacheable) {
        size_t raw_capacity = s_decode_cache[slot].raw_capacity > bytes
            ? s_decode_cache[slot].raw_capacity : bytes;
        size_t decoded_capacity = s_decode_cache[slot].decoded_capacity > count
            ? s_decode_cache[slot].decoded_capacity : count;
        if (raw_capacity + decoded_capacity * 4 > 4u * 1024u * 1024u) {
            s_decode_bytes -= s_decode_cache[slot].raw_capacity
                + s_decode_cache[slot].decoded_capacity * 4;
            free(s_decode_cache[slot].raw); free(s_decode_cache[slot].decoded);
            s_decode_cache[slot].raw = NULL; s_decode_cache[slot].decoded = NULL;
            s_decode_cache[slot].raw_capacity = s_decode_cache[slot].decoded_capacity = 0;
        }
        if (bytes > s_decode_cache[slot].raw_capacity) {
            void *grown = realloc(s_decode_cache[slot].raw, bytes);
            if (grown) {
                s_decode_bytes += bytes - s_decode_cache[slot].raw_capacity;
                s_decode_cache[slot].raw = grown; s_decode_cache[slot].raw_capacity = bytes;
            }
            else cacheable = 0;
        }
        if (count > s_decode_cache[slot].decoded_capacity) {
            void *grown = realloc(s_decode_cache[slot].decoded, count * 4);
            if (grown) {
                s_decode_bytes += (count - s_decode_cache[slot].decoded_capacity) * 4;
                s_decode_cache[slot].decoded = grown; s_decode_cache[slot].decoded_capacity = count;
            }
            else cacheable = 0;
        }
        if (cacheable) {
            memcpy(s_decode_cache[slot].raw, mem + t->offset, bytes);
            if (palette) memcpy(s_decode_cache[slot].palette, mem + t->palette, 1024);
        }
    }
    /* Linear 32-bit ARGB is already the atlas format: copy rows. The title
     * samples its own 1280x480 back buffer this way every frame, and one
     * sample_tex call per texel was a large part of a match frame. The span
     * was validated by gpu_texture_layout above. */
    static int no_lin = -1;
    if (no_lin < 0) no_lin = getenv("RECOMP_NO_LIN_FAST") != NULL;
    if (!no_lin && (t->color == 0x12 || t->color == 0x1E) && t->pitch >= width * 4u) {
        for (uint32_t y = 0; y < height; y++) {
            uint32_t *dst = &output[(size_t)y * width];
            memcpy(dst, mem + t->offset + (size_t)y * t->pitch, (size_t)width * 4);
            if (t->color == 0x1E)
                for (uint32_t x = 0; x < width; x++)
                    dst[x] |= 0xFF000000u;
        }
        s_decode_texels += count;
        height = 0;                    /* skip the per-texel loop below */
    }
    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++) {
            s_decode_texels++;
            if (!sample_tex(t, 0, x, y, &output[(size_t)y * width + x])) {
                complete = 0;
                output[(size_t)y * width + x] = 0xFFFFFFFFu;
            }
        }
    if (cacheable && complete &&
        !memcmp(s_decode_cache[slot].raw, mem + t->offset, bytes) &&
        (!palette || !memcmp(s_decode_cache[slot].palette, mem + t->palette, 1024))) {
        memcpy(s_decode_cache[slot].decoded, output, count * 4);
        s_decode_cache[slot].key = *t;
        s_decode_cache[slot].bytes = bytes;
        s_decode_cache[slot].valid = 1;
    }
    return complete || allow_missing;
}

/* Resident render target.
 *
 * Each triangle used to copy its destination tile in, dispatch, and wait
 * for the result to come back before the next one: in a match frame that
 * is thousands of GPU round trips. Now the colour surface and its depth
 * buffer stay on the GPU while consecutive triangles draw into them, in
 * submission order, and return to guest memory (and the host depth buffer)
 * at the points where something else can look: fences, notifications,
 * reports, flips, clears, CPU raster fallbacks, a texture that overlaps the
 * surface, or a different surface. That is where real hardware's writes
 * become meaningful to the CPU as well. The next draw after a sync uploads
 * the guest bytes again, so CPU writes between sync points are picked up. */
#define NV_ZBUF_W 2048
#define NV_ZBUF_H 1024
static float *zbuf_current(int create);
static struct {
    int active, dirty;
    int valid;                         /* GPU copy == snap, guest == guest copy */
    uint32_t base, pitch, bpp, stride, rows, zeta;
    uint32_t x0, y0, x1, y1;           /* samples drawn since the upload */
    float *zb;
} s_res;
/* Guest bytes of the region as last uploaded or written back. A later bind
 * of the same surface whose guest bytes still match reuses the GPU copy
 * rather than uploading it again: titles fence several times a frame, and
 * every fence returned the surface to guest memory and dropped it. */
static uint8_t *s_res_guest;
static size_t s_res_guest_cap;
static unsigned long long s_res_reuses;
/* host: transfer buffer; snap: what was uploaded. Only samples the GPU
 * changed go back, so CPU writes elsewhere in the region survive -- titles
 * reuse render-target memory as scratch while loading. */
static Nv2aGpuPixel *s_res_host, *s_res_snap;
static size_t s_res_host_cap;
static unsigned long long s_res_binds, s_res_syncs, s_atlas_reuse;

static int res_host(size_t n)
{
    if (n <= s_res_host_cap)
        return 1;
    Nv2aGpuPixel *grown = (Nv2aGpuPixel *)realloc(s_res_host, n * sizeof(*grown));
    if (!grown)
        return 0;
    s_res_host = grown;
    grown = (Nv2aGpuPixel *)realloc(s_res_snap, n * sizeof(*grown));
    if (!grown)
        return 0;
    s_res_snap = grown;
    s_res_host_cap = n;
    return 1;
}

static int res_guest_save(void)
{
    size_t bytes = (size_t)s_res.rows * s_res.pitch;
    if (bytes > s_res_guest_cap) {
        uint8_t *g = (uint8_t *)realloc(s_res_guest, bytes);
        if (!g)
            return 0;
        s_res_guest = g;
        s_res_guest_cap = bytes;
    }
    memcpy(s_res_guest, (uint8_t *)xbox_GetMemoryOffset() + s_res.base, bytes);
    return 1;
}

static void gpu_surface_sync(void)
{
    if (!s_res.active)
        return;
    s_res.active = 0;
    s_tex_epoch++;
    if (!s_res.dirty)
        return;
    s_res.dirty = 0;
    size_t n = (size_t)s_res.stride * s_res.y1;
    uint64_t st[3];
    if (!n || !nv2a_gpu_surface_download(s_res_host, n, st)) {
        fprintf(stderr, "[GPU-RES] download failed; surface %08X keeps older contents\n",
                s_res.base);
        s_res.valid = 0;
        return;
    }
    uint8_t *mem = (uint8_t *)xbox_GetMemoryOffset() + s_res.base;
    for (uint32_t r = s_res.y0; r < s_res.y1; r++) {
        uint8_t *row = mem + (size_t)r * s_res.pitch;
        const Nv2aGpuPixel *src = s_res_host + (size_t)r * s_res.stride;
        const Nv2aGpuPixel *old = s_res_snap + (size_t)r * s_res.stride;
        for (uint32_t c = s_res.x0; c < s_res.x1; c++) {
            if (src[c].color != old[c].color) {
                if (s_res.bpp == 4)
                    ((uint32_t *)row)[c] = src[c].color;
                else
                    ((uint16_t *)row)[c] = (uint16_t)(((src[c].color >> 8) & 0xF800)
                                                      | ((src[c].color >> 5) & 0x07E0)
                                                      | ((src[c].color >> 3) & 0x001F));
            }
            if (s_res.zb && memcmp(&src[c].depth, &old[c].depth, sizeof(float)))
                s_res.zb[(size_t)r * NV_ZBUF_W + c] = src[c].depth;
        }
    }
    /* The GPU now matches snap everywhere: it changed only the dirty box. */
    if (s_res.y1 > s_res.y0)
        memcpy(s_res_snap + (size_t)s_res.y0 * s_res.stride,
               s_res_host + (size_t)s_res.y0 * s_res.stride,
               (size_t)(s_res.y1 - s_res.y0) * s_res.stride * sizeof(*s_res_snap));
    s_res.valid = res_guest_save();
    s_gpu.pixels += st[0];
    s_gpu.xf_pixels += st[0];
    s_gpu.xf_depth_fail += st[1];
    s_gpu.zpass_count += (uint32_t)st[2];
    s_res_syncs++;
}

/* RECOMP_WRITE_TRACE=1: every distinct colour surface the GPU writes, once,
 * with time -- what it covers in guest memory, to match against corruption. */
static void trace_surface_write(const char *what, uint32_t base, uint32_t bytes)
{
    static int on = -1;
    static uint32_t seen[256][3];
    static unsigned n;
    if (on < 0) on = getenv("RECOMP_WRITE_TRACE") != NULL;
    if (!on) return;
    for (unsigned i = 0; i < n; i++)
        if (seen[i][0] == base && seen[i][1] == bytes && seen[i][2] == s_gpu.format) return;
    if (n < 256) { seen[n][0] = base; seen[n][1] = bytes; seen[n][2] = s_gpu.format; n++; }
    fprintf(stderr, "[WTRACE] %s t=%llu surface=%08X..%08X offset=%08X pitch=%u"
            " clip=%ux%u+%u+%u format=%08X zeta=%08X\n", what,
            (unsigned long long)clock(), base, base + bytes,
            s_gpu.color_offset, s_gpu.pitch, s_gpu.clip_w, s_gpu.clip_h,
            s_gpu.clip_x, s_gpu.clip_y, s_gpu.format, s_gpu.zeta_offset);
}

/* Something on the CPU is about to write the surface or its depth buffer:
 * bring the GPU's writes back, and do not trust the GPU copy afterwards. */
static void gpu_surface_invalidate(void)
{
    gpu_surface_sync();
    s_res.valid = 0;
}

static int gpu_surface_bind(uint32_t bpp, unsigned sy)
{
    uint32_t base = dma_resolve(s_gpu.color_offset);
    uint32_t stride = bpp ? s_gpu.pitch / bpp : 0;
    uint32_t rows = (s_gpu.clip_y + s_gpu.clip_h) * sy;
    float *zb = zbuf_current(1);       /* may invalidate: a slot changing owner */
    int same = s_res.base == base && s_res.pitch == s_gpu.pitch
        && s_res.bpp == bpp && s_res.rows >= rows && s_res.zb == zb
        && s_res.zeta == s_gpu.zeta_offset;
    if (s_res.active && same)
        return 1;
    gpu_surface_sync();
    static int no_reuse = -1;
    if (no_reuse < 0) no_reuse = getenv("RECOMP_NO_RES_REUSE") != NULL;
    if (same && s_res.valid && !no_reuse
        && !memcmp(s_res_guest, (const uint8_t *)xbox_GetMemoryOffset() + base,
                   (size_t)s_res.rows * s_res.pitch)) {
        s_res.x0 = s_res.y0 = UINT32_MAX;
        s_res.x1 = s_res.y1 = 0;
        s_res.active = 1;
        s_res.dirty = 0;
        s_res_reuses++;
        s_tex_epoch++;
        return 1;
    }
    if (!stride || !rows || s_gpu.pitch % bpp || stride > NV_ZBUF_W
        || rows > NV_ZBUF_H || !zb
        || !gpu_guest_span(base, (uint64_t)rows * s_gpu.pitch, 1))
        return 0;
    size_t n = (size_t)stride * rows;
    if (!res_host(n))
        return 0;
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset() + base;
    for (uint32_t r = 0; r < rows; r++) {
        const uint8_t *row = mem + (size_t)r * s_gpu.pitch;
        Nv2aGpuPixel *dst = s_res_host + (size_t)r * stride;
        for (uint32_t c = 0; c < stride; c++) {
            uint32_t v;
            if (bpp == 4) v = ((const uint32_t *)row)[c];
            else {
                uint32_t t = ((const uint16_t *)row)[c];
                v = 0xFF000000u | ((t & 0xF800u) << 8)
                    | ((t & 0x07E0u) << 5) | ((t & 0x001Fu) << 3);
            }
            dst[c].color = v;
            dst[c].depth = zb[(size_t)r * NV_ZBUF_W + c];
            dst[c].covered = dst[c].depth_fail = 0;
        }
    }
    if (!nv2a_gpu_surface_upload(s_res_host, n))
        return 0;
    memcpy(s_res_snap, s_res_host, n * sizeof(*s_res_snap));
    s_res.x0 = s_res.y0 = UINT32_MAX;
    s_res.x1 = s_res.y1 = 0;
    s_res.active = 1;
    s_res.dirty = 0;
    s_res.base = base;
    s_res.pitch = s_gpu.pitch;
    s_res.bpp = bpp;
    s_res.stride = stride;
    s_res.rows = rows;
    s_res.zb = zb;
    s_res.zeta = s_gpu.zeta_offset;
    s_res.valid = res_guest_save();
    s_res_binds++;
    trace_surface_write("draw", base, rows * s_gpu.pitch);
    s_tex_epoch++;
    return 1;
}

/* The atlas of the previous triangle stays valid for the rest of its batch
 * while the texture registers are unchanged: guest memory under a texture
 * cannot change mid-batch except by drawing into it, which the feedback and
 * overlap checks below exclude. Each new batch decodes and compares again. */
static struct {
    unsigned epoch;
    int valid;
    Texture texs[4];
    uint32_t tex[4], stage_info[4][4], rcmeta[2], formats;
} s_atlas;

static int gpu_triangle(Nv2aGpuParams *p, uint32_t bpp, float *zb,
                        uint32_t zpitch, uint64_t *written,
                        uint64_t *depth_failed, uint32_t *zpass)
{
    unsigned sx = surface_samples_x(), sy = surface_samples_y();
    size_t count = (size_t)p->box[2] * p->box[3] * sx * sy;
    size_t texcount = p->rcmeta[0] ? p->rcmeta[1] : (size_t)p->tex[0] * p->tex[1];
    Nv2aGpuPixel *input = NULL, *output = NULL;
    uint32_t *texture = NULL;
    int ok = 0;
    (void)zb; (void)zpitch; (void)written; (void)depth_failed; (void)zpass;
    if (!nv2a_gpu_enabled()) return 0;
    if ((sx != 1 && sx != 2) || (sy != 1 && sy != 2)) {
        nv2a_gpu_fallback("multisample"); return 0;
    }
    p->depth[3] = sx; p->alpha[3] = sy;
    if (!count || count > 4096u * 4096u || texcount > 4096u * 4096u) {
        nv2a_gpu_fallback("oversize"); return 0;
    }
    uint64_t surface_bytes = (uint64_t)(p->box[1] + p->box[3] - 1) * sy * s_gpu.pitch
        + (sy - 1) * (uint64_t)s_gpu.pitch + (uint64_t)(p->box[0] + p->box[2]) * sx * bpp;
    if ((uint64_t)(p->box[0] + p->box[2]) * sx * bpp > s_gpu.pitch
        || !gpu_guest_span(dma_resolve(s_gpu.color_offset), surface_bytes, 1)) {
        nv2a_gpu_fallback("surface-span"); return 0;
    }
    for (unsigned st = 0; st < (p->rcmeta[0] ? 4u : 1u); st++) {
        /* Feedback from a render target into its own texture must preserve
         * the existing ordered guest-memory reads; snapshotting changes it. */
        size_t nt = p->rcmeta[0] ? (size_t)p->stage_info[st][0] * p->stage_info[st][1] : texcount;
        if (!nt) continue;
        if (!gpu_texture_span(&s_gpu.texs[st])) { nv2a_gpu_fallback("texture-span"); return 0; }
        uint64_t tb = s_gpu.texs[st].offset;
        uint64_t te = tb + fmax((double)s_gpu.texs[st].pitch * s_gpu.texs[st].height,
                               (double)nt * 4);
        uint64_t sb = dma_resolve(s_gpu.color_offset);
        uint64_t se = sb + (uint64_t)(p->box[1] + p->box[3]) * s_gpu.pitch;
        if (tb < se && sb < te) { nv2a_gpu_fallback("texture-feedback"); return 0; }
        /* Reading a texture that lies under the resident surface (beyond
         * this box): its pixels must be in guest memory first. */
        uint64_t rb = s_res.base, re = rb + (uint64_t)s_res.rows * s_res.pitch;
        if (s_res.active && s_res.dirty && tb < re && rb < te)
            gpu_surface_sync();
    }
    if (!gpu_surface_bind(bpp, sy)) { nv2a_gpu_fallback("surface-bind"); return 0; }
    for (unsigned st = 0; st < 4; st++)
        p->rc_control[2] |= (s_gpu.texs[st].color & 255u) << (st * 8);
    int reuse = texcount && s_atlas.valid && s_atlas.epoch == s_tex_epoch
        && !memcmp(s_atlas.texs, s_gpu.texs, sizeof(s_atlas.texs))
        && !memcmp(s_atlas.tex, p->tex, sizeof(s_atlas.tex))
        && !memcmp(s_atlas.stage_info, p->stage_info, sizeof(s_atlas.stage_info))
        && s_atlas.rcmeta[0] == p->rcmeta[0] && s_atlas.rcmeta[1] == p->rcmeta[1]
        && s_atlas.formats == p->rc_control[2];
    if (texcount && !reuse) {
        s_atlas.valid = 0;
        if (!nv2a_gpu_host_buffers(1, texcount, &input, &output, &texture)) goto done;
        if (p->rcmeta[0]) {
            for (unsigned st = 0; st < 4; st++) {
                uint32_t w = p->stage_info[st][0], h = p->stage_info[st][1], off = p->stage_info[st][2];
                if (w && h && !gpu_decode_texture(st, w, h, texture + off, 1)) goto done;
            }
        } else if (!gpu_decode_texture(0, p->tex[0], p->tex[1], texture, 0)) {
            nv2a_gpu_fallback("texture-format"); goto done;
        }
    }
    if (!nv2a_gpu_surface_draw(p, reuse ? NULL : texture, s_res.stride,
                               s_gpu.zpass_enable != 0))
        goto done;
    s_res.dirty = 1;
    {
        uint32_t x0 = p->box[0] * sx, y0 = p->box[1] * sy;
        uint32_t x1 = x0 + p->box[2] * sx, y1 = y0 + p->box[3] * sy;
        if (x0 < s_res.x0) s_res.x0 = x0;
        if (y0 < s_res.y0) s_res.y0 = y0;
        if (x1 > s_res.x1) s_res.x1 = x1 > s_res.stride ? s_res.stride : x1;
        if (y1 > s_res.y1) s_res.y1 = y1 > s_res.rows ? s_res.rows : y1;
    }
    if (reuse)
        s_atlas_reuse++;
    else if (texcount) {
        s_atlas.valid = 1;
        s_atlas.epoch = s_tex_epoch;
        memcpy(s_atlas.texs, s_gpu.texs, sizeof(s_atlas.texs));
        memcpy(s_atlas.tex, p->tex, sizeof(s_atlas.tex));
        memcpy(s_atlas.stage_info, p->stage_info, sizeof(s_atlas.stage_info));
        s_atlas.rcmeta[0] = p->rcmeta[0];
        s_atlas.rcmeta[1] = p->rcmeta[1];
        s_atlas.formats = p->rc_control[2];
    }
    ok = 1;
done:
    return ok;
}

/* NV097_CLEAR_SURFACE on the resident surface: two screen-space quads over
 * the clip rectangle with the clear colour and depth, colour mask from the
 * clear flags, no blending, no depth test. Queued in order with the draws
 * around them, so no round trip. Stencil is not modelled here or anywhere
 * else in this backend. Returns 0 when the GPU path does not apply; the
 * caller then clears guest memory and the host depth buffer as before. */
static unsigned long long s_gpu_clears;
static float zclear_value(void);
static int gpu_clear(uint32_t param)
{
    uint32_t bpp = surface_bpp();
    static int off = -1;
    if (off < 0) off = getenv("RECOMP_NO_GPU_CLEAR") != NULL;
    if (off) return 0;
    if (!nv2a_gpu_enabled() || (bpp != 4 && bpp != 2) || !s_gpu.color_offset
        || !s_gpu.pitch || !s_gpu.clip_w || !s_gpu.clip_h)
        return 0;
    if (!(param & 0xF1))
        return 1;                                  /* nothing we model */
    float x0 = (float)s_gpu.clip_x, y0 = (float)s_gpu.clip_y;
    float x1 = x0 + (float)s_gpu.clip_w, y1 = y0 + (float)s_gpu.clip_h;
    float z = zclear_value();
    uint32_t mask = ((param & 0x80) ? 0x01000000u : 0) | ((param & 0x10) ? 0x00010000u : 0)
                  | ((param & 0x20) ? 0x00000100u : 0) | ((param & 0x40) ? 0x00000001u : 0);
    const float q[2][3][2] = { { {x0, y0}, {x1, y0}, {x0, y1} },
                               { {x1, y0}, {x1, y1}, {x0, y1} } };
    uint32_t zpass = s_gpu.zpass_enable;
    int ok = 1;
    s_gpu.zpass_enable = 0;                        /* clears are not visibility */
    for (int t = 0; t < 2 && ok; t++) {
        Nv2aGpuParams p;
        uint64_t written = 0, failed = 0;
        uint32_t passed = 0;
        memset(&p, 0, sizeof p);
        for (int k = 0; k < 3; k++) {
            p.pos[k][0] = q[t][k][0];
            p.pos[k][1] = q[t][k][1];
            p.pos[k][2] = z;
            p.pos[k][3] = 1.0f;
        }
        p.box[0] = s_gpu.clip_x; p.box[1] = s_gpu.clip_y;
        p.box[2] = s_gpu.clip_w; p.box[3] = s_gpu.clip_h;
        p.state[0] = mask;
        p.state[2] = s_gpu.clear_color;
        p.depth[2] = (param & 0x01) ? 1u : 0u;
        ok = gpu_triangle(&p, bpp, NULL, 0, &written, &failed, &passed);
    }
    s_gpu.zpass_enable = zpass;
    if (!ok)
        return 0;
    /* The caller clears guest memory too. Record the cleared rectangle as
     * the baseline, so the clear itself never counts as a GPU change at
     * write-back: whatever the CPU writes into the surface after the clear
     * (EA's logo player writes the back buffer directly) must survive, and
     * only what the GPU draws afterwards goes back. The host depth buffer
     * gets the same rectangle, keeping it equal to the baseline. */
    {
        unsigned sx = surface_samples_x(), sy = surface_samples_y();
        uint32_t c0 = s_gpu.clip_x * sx, c1 = (s_gpu.clip_x + s_gpu.clip_w) * sx;
        uint32_t r0 = s_gpu.clip_y * sy, r1 = (s_gpu.clip_y + s_gpu.clip_h) * sy;
        /* Params uses one boolean bit per byte to enable a channel. The
         * snapshot stores packed pixels, so each enabled channel needs all
         * eight bits cleared. An LSB-only baseline made unchanged palette
         * colours disappear after the guest clear on subsequent frames. */
        uint32_t pixel_mask = mask * 255u;
        if (c1 > s_res.stride) c1 = s_res.stride;
        if (r1 > s_res.rows) r1 = s_res.rows;
        for (uint32_t r = r0; r < r1; r++) {
            Nv2aGpuPixel *row = s_res_snap + (size_t)r * s_res.stride;
            for (uint32_t c = c0; c < c1; c++) {
                row[c].color = (s_gpu.clear_color & pixel_mask) | (row[c].color & ~pixel_mask);
                if (param & 0x01) {
                    row[c].depth = z;
                    if (s_res.zb) s_res.zb[(size_t)r * NV_ZBUF_W + c] = z;
                }
            }
        }
    }
    s_gpu_clears++;
    return 1;
}

#include "nv2a_texture_snapshot.inc"

static void raster_triangle(const float a[2], const float b[2],
                            const float c[2], uint32_t argb,
                            const float uv[3][2])
{
    uint8_t *mem = (uint8_t *)xbox_GetMemoryOffset();
    uint32_t bpp = surface_bpp();
    float area;
    int minx, maxx, miny, maxy, x, y;
    int textured = uv && s_gpu.texs[0].valid;

    if (bpp != 4 && bpp != 2)
        return;

    area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
    if (area == 0.0f)
        return;                            /* degenerate */

    /* Where this batch writes. The same check the per-pixel path made, made
     * once: a surface address landing on the title's own image is no safer
     * one pixel at a time than 4.9 MB at once. */
    if (!surface_begin_batch(mem))
        return;

    minx = (int)floorf(fminf(a[0], fminf(b[0], c[0])));
    maxx = (int)ceilf (fmaxf(a[0], fmaxf(b[0], c[0])));
    miny = (int)floorf(fminf(a[1], fminf(b[1], c[1])));
    maxy = (int)ceilf (fmaxf(a[1], fmaxf(b[1], c[1])));

    if (minx < (int)s_gpu.clip_x) minx = (int)s_gpu.clip_x;
    if (miny < (int)s_gpu.clip_y) miny = (int)s_gpu.clip_y;
    if (maxx > (int)(s_gpu.clip_x + s_gpu.clip_w)) maxx = (int)(s_gpu.clip_x + s_gpu.clip_w);
    if (maxy > (int)(s_gpu.clip_y + s_gpu.clip_h)) maxy = (int)(s_gpu.clip_y + s_gpu.clip_h);
    if (minx >= maxx || miny >= maxy) {
        s_gpu.tris_skipped_offscreen++;
        return;
    }

    if (nv2a_gpu_enabled()) {
        Nv2aGpuParams p = {0};
        uint64_t written = 0, failed = 0;
        uint32_t zpass = 0;
        const float *v[3] = { a, b, c };
        for (int k = 0; k < 3; k++) {
            p.pos[k][0] = v[k][0]; p.pos[k][1] = v[k][1];
            if (textured) { p.uvw[k][0] = uv[k][0]; p.uvw[k][1] = uv[k][1]; }
        }
        p.box[0] = minx; p.box[1] = miny; p.box[2] = maxx-minx; p.box[3] = maxy-miny;
        if (textured) { p.tex[0] = s_gpu.texs[0].width; p.tex[1] = s_gpu.texs[0].height; }
        p.tex[2] = s_gpu.texs[0].addr_u; p.tex[3] = s_gpu.texs[0].addr_v;
        p.blend[0] = s_gpu.blend_enable; p.blend[1] = s_gpu.blend_sfactor;
        p.blend[2] = s_gpu.blend_dfactor; p.blend[3] = s_gpu.blend_equation;
        p.state[0] = s_gpu.color_mask; p.state[1] = s_gpu.blend_color; p.state[2] = argb;
        if (gpu_triangle(&p, bpp, NULL, 0, &written, &failed, &zpass)) {
            s_gpu.tris_drawn++; note_drawn(); return;
        }
    }
    gpu_surface_invalidate();
    for (y = miny; y < maxy; y++) {
        for (x = minx; x < maxx; x++) {
            float px = (float)x + 0.5f, py = (float)y + 0.5f;
            float w0 = (b[0] - a[0]) * (py - a[1]) - (b[1] - a[1]) * (px - a[0]);
            float w1 = (c[0] - b[0]) * (py - b[1]) - (c[1] - b[1]) * (px - b[0]);
            float w2 = (a[0] - c[0]) * (py - c[1]) - (a[1] - c[1]) * (px - c[0]);
            if (!((w0 >= 0 && w1 >= 0 && w2 >= 0)
               || (w0 <= 0 && w1 <= 0 && w2 <= 0)))
                continue;
            if (textured) {
                /* Barycentric, straight from the edge functions already
                 * computed: w1 is the area opposite a, w2 opposite b, w0
                 * opposite c, and the three sum to the whole triangle.
                 *
                 * No perspective divide. These are screen-space vertices with
                 * no w to divide by -- which is exactly the case a full-screen
                 * pass is, and the only case that reaches here. */
                uint32_t texel;
                float su = (w1 * uv[0][0] + w2 * uv[1][0] + w0 * uv[2][0]) / area;
                float sv = (w1 * uv[0][1] + w2 * uv[1][1] + w0 * uv[2][1]) / area;
                if (su < 0.0f) su = 0.0f;
                if (sv < 0.0f) sv = 0.0f;
                if (sample_texture((uint32_t)su, (uint32_t)sv, &texel)) {
                    put_pixel(mem, bpp, x, y, texel);
                    continue;
                }
            }
            put_pixel(mem, bpp, x, y, argb);
        }
    }
    s_gpu.tris_drawn++;
    note_drawn();
}

/* Attribute 3 is diffuse colour in every NV2A layout that sets one. Absent it,
 * white -- a visible wrong colour beats an invisible correct one during
 * bring-up. */
/* Which attribute carries the colour.
 *
 * Slot 3 is diffuse by convention and titles that follow it are read straight
 * from there. Half-Life 2 does not: its vertex is position, colour, texcoord
 * at stride 24, and the colour arrives in slot 5. So fall back to the format
 * rather than the slot number -- D3DCOLOR is the one attribute type that is
 * only ever a colour, which makes it a stronger signal than the convention. */
static const VertexAttr *color_attr(void)
{
    uint32_t a;

    if (s_gpu.attr[3].offset && s_gpu.attr[3].stride)
        return &s_gpu.attr[3];
    for (a = 0; a < NV_VERTEX_ATTRS; a++)
        if (s_gpu.attr[a].type == 0 && s_gpu.attr[a].size == 4
                && s_gpu.attr[a].offset && s_gpu.attr[a].stride)
            return &s_gpu.attr[a];
    return &s_gpu.attr[3];
}

/* Attribute 9 is texture coordinate 0 in the NV2A vertex layout, the same way
 * 0 is position and 3 is diffuse -- for a title that follows the convention.
 *
 * Half-Life 2 does not, in either place. Its menu and HUD vertex is position,
 * colour, texcoord at stride 24, with the colour in slot 5 and the texcoords
 * in slot 7, so reading slot 9 found nothing and every batch drew untextured.
 * That is invisible rather than wrong-looking: the menu paints a full-screen
 * quad and then draws its text over it, and with no sampling both come out
 * white, so the screen is blank white and nothing suggests the text was ever
 * drawn.
 *
 * Falling back to the format works because the three attributes of such a
 * vertex are distinguishable: position is float3, colour is D3DCOLOR, and a
 * float2 is a texture coordinate and nothing else.
 *
 * ponytail: takes the first float2 it finds, so a title with two texcoord sets
 * gets stage 0's -- which is what this single-texture rasteriser samples
 * anyway. Multi-texture wants the D3D11 translator, not another heuristic. */
static const VertexAttr *texcoord_attr(void)
{
    uint32_t a;

    if (s_gpu.attr[9].offset && s_gpu.attr[9].stride)
        return &s_gpu.attr[9];
    for (a = 0; a < NV_VERTEX_ATTRS; a++)
        if (s_gpu.attr[a].type == 2 && s_gpu.attr[a].size == 2
                && s_gpu.attr[a].offset && s_gpu.attr[a].stride)
            return &s_gpu.attr[a];
    return &s_gpu.attr[9];
}

/* Texel coordinates, whichever convention the title used.
 *
 * The two are not interchangeable and the format decides which is in force: a
 * swizzled texture is addressed in [0,1], a linear one in texels. Both are
 * scaled to texels here so that everything downstream -- the barycentric
 * interpolation and the sampler -- works in one unit.
 *
 * This mattered the moment swizzled formats became samplable. Normalised
 * coordinates truncated to a texel index land on texel 0 for any coordinate
 * below 1.0, so a whole quad sampled a single texel and came out flat: the
 * background painted one near-black colour, which looks like a texture that
 * decoded wrong rather than one that was never indexed. */
static int fetch_texcoord(uint32_t index, float out[2])
{
    float t[4];

    if (!fetch_attr(texcoord_attr(), index, t))
        return 0;
    out[0] = t[0];
    out[1] = t[1];
    if (tex_size_from_format(s_gpu.texs[0].color)) {
        out[0] *= (float)s_gpu.texs[0].width;
        out[1] *= (float)s_gpu.texs[0].height;
    }
    return 1;
}

static uint32_t vertex_color(uint32_t index)
{
    float c[4];

    if (!fetch_attr(color_attr(), index, c))
        return 0xFFFFFFFFu;
    return ((uint32_t)(c[3] * 255.0f) << 24)
         | ((uint32_t)(c[0] * 255.0f) << 16)
         | ((uint32_t)(c[1] * 255.0f) <<  8)
         |  (uint32_t)(c[2] * 255.0f);
}

/* An untransformed batch drawn as if it were screen space smears a few pixels
 * into the corner, so the batch has to be classified before it is rasterised.
 *
 * This used to demand that every vertex land inside the surface, which is a
 * different question and the wrong one: geometry that extends past the
 * viewport is ordinary, and clipping it is raster_triangle's job (it clamps
 * its span to the clip rect). The dashboard is exactly the case that exposed
 * it -- a full-screen pass drawn as one oversized triangle, vertices at
 * (-0.5,-0.5), (2*w,-0.5), (-0.5,2*h), all correct and all rejected.
 *
 * What actually separates the two is scale. Object-space positions are model
 * units, a handful either side of the origin; screen-space ones are measured
 * in pixels of a surface hundreds of pixels wide. So: the batch has to be able
 * to touch the surface at all, and it has to be bigger than object space.
 *
 * ponytail: a genuinely tiny screen-space sprite reads as object space and is
 * skipped. It is counted as skipped rather than silently dropped, and the
 * unambiguous answer needs the vertex-program state, which is not tracked yet.
 */
#define OBJECT_SPACE_SPAN 8.0f

static int batch_is_screen_space(void)
{
    float p[4], lo_x, hi_x, lo_y, hi_y;
    uint32_t i;

    if (!s_gpu.clip_w || !s_gpu.clip_h || !s_gpu.idx_count)
        return 0;
    if (!fetch_attr(&s_gpu.attr[0], s_gpu.idx[0], p))
        return 0;
    lo_x = hi_x = p[0];
    lo_y = hi_y = p[1];
    for (i = 1; i < s_gpu.idx_count; i++) {
        if (!fetch_attr(&s_gpu.attr[0], s_gpu.idx[i], p))
            return 0;
        if (p[0] < lo_x) lo_x = p[0];
        if (p[0] > hi_x) hi_x = p[0];
        if (p[1] < lo_y) lo_y = p[1];
        if (p[1] > hi_y) hi_y = p[1];
    }

    /* Entirely off the surface: nothing to draw under either reading. */
    if (hi_x < (float)s_gpu.clip_x
     || lo_x > (float)(s_gpu.clip_x + s_gpu.clip_w)
     || hi_y < (float)s_gpu.clip_y
     || lo_y > (float)(s_gpu.clip_y + s_gpu.clip_h))
        return 0;

    /* Small enough to be model units rather than pixels. */
    if (hi_x - lo_x < OBJECT_SPACE_SPAN && hi_y - lo_y < OBJECT_SPACE_SPAN)
        return 0;

    return 1;
}

/* NV097 primitive types.
 *
 * These are the operand of SET_BEGIN_END, where 0 is END and the list starts
 * at 1. They were each one too low, so every title's geometry was decomposed
 * as the primitive below the one it asked for -- a strip as a fan, a fan as
 * quads, and TRIANGLES, the one case whose vertex count must be a multiple
 * of three, as a strip.
 *
 * The vertex order says which numbering is right without taking a table on
 * trust: a strip arrives in Z order and a fan in cyclic order, and they only
 * line up with the primitive under this one. */
#define NV_PRIM_POINTS         1
#define NV_PRIM_LINES          2
#define NV_PRIM_LINE_LOOP      3
#define NV_PRIM_LINE_STRIP     4
#define NV_PRIM_TRIANGLES      5
#define NV_PRIM_TRIANGLE_STRIP 6
#define NV_PRIM_TRIANGLE_FAN   7
#define NV_PRIM_QUADS          8
#define NV_PRIM_QUAD_STRIP     9
#define NV_PRIM_POLYGON        10

/* How many post-draw captures to keep: enough to see whether the geometry
 * is stable from frame to frame, few enough not to fill a directory. */
#define FB_DUMP_AFTER_DRAW 8
static int s_drawn_dumps;

static void dump_surface_bmp(void);

/* One triangle by vertex index: gather position and, if the batch has one,
 * texture coordinate 0. A vertex whose position cannot be read is not drawn;
 * a batch whose texcoords cannot be read is drawn untextured rather than not
 * at all, so a missing coordinate stream costs the colour and not the shape.
 */
static void raster_indexed(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t argb)
{
    float p[3][4], uv[3][2];
    int textured;

    if (!fetch_attr(&s_gpu.attr[0], i0, p[0])
     || !fetch_attr(&s_gpu.attr[0], i1, p[1])
     || !fetch_attr(&s_gpu.attr[0], i2, p[2]))
        return;

    textured = fetch_texcoord(i0, uv[0])
            && fetch_texcoord(i1, uv[1])
            && fetch_texcoord(i2, uv[2]);

    raster_triangle(p[0], p[1], p[2], argb,
                    textured ? (const float (*)[2])uv : NULL);
}

/* ---- 3D: vertex programs and depth -------------------------------------
 *
 * A batch drawn while TRANSFORM_EXECUTION_MODE says "program" carries
 * model-space positions. Each vertex is run through the title's own vertex
 * program (nv2a_vsh_interp.c), whose oPos output on Xbox D3D is already
 * screen space -- the runtime appends the viewport transform and the divide
 * by w -- with the clip-space w left in oPos.w. So the rasteriser gets pixel
 * coordinates and a depth, and w for perspective-correct texturing.
 *
 * ponytail: no clipping. A triangle with any vertex behind the eye (w <= 0)
 * is dropped rather than clipped, which loses the slivers that cross the near
 * plane; road right under the camera is where that shows. Clip in
 * homogeneous space if it matters. */

/* Depth buffers, host-side, one per zeta surface the title uses. The real one
 * lives in guest memory in a tiled format nothing here decodes, and nothing
 * the title does reads it back, so a float buffer per zeta offset is enough. */
#define NV_ZBUF_W 2048
#define NV_ZBUF_H 1024
static struct { uint32_t offset; float *z; } s_zbufs[4];
static int s_ftrace;            /* frame trace: 0 idle, 2 tracing, 3 done */
static int s_zbuf_next;

static float zclear_value(void)
{
    /* Zeta format in SURFACE_FORMAT bits 4-7: 1 is Z16, 2 is Z24S8. */
    uint32_t zf = (s_gpu.format >> 4) & 0xF;
    return zf == 1 ? (float)(s_gpu.zstencil_clear & 0xFFFF)
                   : (float)(s_gpu.zstencil_clear >> 8);
}

static float *zbuf_current(int create)
{
    int i;
    size_t k;

    for (i = 0; i < 4; i++)
        if (s_zbufs[i].z && s_zbufs[i].offset == s_gpu.zeta_offset)
            return s_zbufs[i].z;
    if (!create)
        return NULL;
    i = s_zbuf_next++ & 3;
    if (!s_zbufs[i].z)
        s_zbufs[i].z = (float *)malloc(sizeof(float) * NV_ZBUF_W * NV_ZBUF_H);
    if (!s_zbufs[i].z)
        return NULL;
    gpu_surface_invalidate();           /* the slot may be the resident one */
    s_zbufs[i].offset = s_gpu.zeta_offset;
    for (k = 0; k < (size_t)NV_ZBUF_W * NV_ZBUF_H; k++)
        s_zbufs[i].z[k] = 3.4e38f;
    return s_zbufs[i].z;
}

static void zbuf_clear(void)
{
    float *z = zbuf_current(1), v = zclear_value();
    size_t k;
    if (z)
        for (k = 0; k < (size_t)NV_ZBUF_W * NV_ZBUF_H; k++)
            z[k] = v;
}

static int depth_pass(float z, float stored)
{
    switch (s_gpu.depth_func) {                   /* GL enums, as the NV2A */
    case 0x200: return 0;
    case 0x201: return z <  stored;
    case 0x202: return z == stored;
    case 0x203: return z <= stored;
    case 0x204: return z >  stored;
    case 0x205: return z != stored;
    case 0x206: return z >= stored;
    default:    return 1;
    }
}

static uint32_t pack_color(const float c[4])
{
    int i;
    uint32_t b[4];
    for (i = 0; i < 4; i++) {
        float f = c[i] < 0.0f ? 0.0f : (c[i] > 1.0f ? 1.0f : c[i]);
        b[i] = (uint32_t)(f * 255.0f + 0.5f);
    }
    return (b[3] << 24) | (b[0] << 16) | (b[1] << 8) | b[2];
}

static uint32_t modulate(uint32_t t, const float c[4])
{
    float f[4];
    f[0] = (float)((t >> 16) & 0xFF) / 255.0f * c[0];
    f[1] = (float)((t >> 8) & 0xFF) / 255.0f * c[1];
    f[2] = (float)(t & 0xFF) / 255.0f * c[2];
    f[3] = (float)(t >> 24) / 255.0f * c[3];
    return pack_color(f);
}

/* Bytes one texel of a (non-DXT) format takes, for cube-face strides. */
static uint32_t tex_texel_bytes(uint32_t fmt)
{
    switch (fmt) {
    case 0x00: case 0x01: case 0x0B: case 0x13: case 0x19: case 0x1F:
        return 1;
    case 0x02: case 0x03: case 0x04: case 0x05: case 0x10: case 0x11:
    case 0x1C: case 0x1D:
        return 2;
    default:
        return 4;
    }
}

/* Bytes between cube faces: one face with all its mip levels, rounded up to
 * the NV2A's 128-byte face alignment (xemu texture.c). */
static uint32_t tex_face_stride(const Texture *t)
{
    uint32_t w = t->width, h = t->height, lv, total = 0;
    uint32_t block = d3d8_format_dxt_block_bytes(t->color);
    uint32_t levels = t->levels ? t->levels : 1;

    if (!tex_size_from_format(t->color))
        return (t->pitch * t->height + 127u) & ~127u;
    for (lv = 0; lv < levels; lv++) {
        uint32_t lw = (w >> lv) ? (w >> lv) : 1, lh = (h >> lv) ? (h >> lv) : 1;
        total += block ? ((lw + 3) / 4) * ((lh + 3) / 4) * block
                       : lw * lh * tex_texel_bytes(t->color);
    }
    return (total + 127u) & ~127u;
}

/* One texel at normalised (swizzled/DXT) or texel (linear) coordinates, as
 * floats. An unusable stage reads white, so a missing texture multiplies
 * through instead of blacking the pixel out. */
static void rc_texel(const Texture *t, uint32_t face, float u, float v,
                     float out[4])
{
    uint32_t texel, mag = (t->filter >> 24) & 0xF, min = (t->filter >> 16) & 0xFF;
    if (tex_size_from_format(t->color)) {
        u *= (float)t->width;
        v *= (float)t->height;
    }
    /* TENT (bilinear) when the title asks for it, magnifying or minifying:
     * a 64x32 sky gradient stretched over the screen is blocks with nearest
     * texels and a gradient with four. MAG 2 is tent; MIN 2 is tent at LOD 0
     * and 4/6 the tent mip modes.
     * ponytail: level 0 only, no mip selection -- minified textures shimmer.
     * Add LOD from the screen-space derivative when that shows. */
    if (mag == 2 || min == 2 || min == 4 || min == 6) {
        float fu = u - 0.5f, fv = v - 0.5f, wu, wv, c[4][4];
        int32_t iu = (int32_t)floorf(fu), iv = (int32_t)floorf(fv), k, j;
        wu = fu - (float)iu;
        wv = fv - (float)iv;
        for (k = 0; k < 4; k++) {
            if (sample_tex(t, face, (uint32_t)(iu + (k & 1)),
                           (uint32_t)(iv + (k >> 1)), &texel))
                nv2a_rc_unpack(texel, c[k]);
            else
                c[k][0] = c[k][1] = c[k][2] = c[k][3] = 1.0f;
        }
        for (j = 0; j < 4; j++)
            out[j] = (c[0][j] * (1.0f - wu) + c[1][j] * wu) * (1.0f - wv)
                   + (c[2][j] * (1.0f - wu) + c[3][j] * wu) * wv;
        return;
    }
    if (sample_tex(t, face, (uint32_t)(int32_t)floorf(u),
                   (uint32_t)(int32_t)floorf(v), &texel))
        nv2a_rc_unpack(texel, out);
    else
        out[0] = out[1] = out[2] = out[3] = 1.0f;
}

/* What texture stage `st` contributes, by its SHADER_STAGE_PROGRAM mode
 * (xemu psh.c). Returns 0 when a clip-plane stage kills the pixel. */
static int rc_stage_fetch(int st, const float c[4], float out[4])
{
    const Texture *t = &s_gpu.texs[st];
    uint32_t mode = (s_gpu.rc.stage_program >> (st * 5)) & 0x1F, j;
    float q = c[3] != 0.0f ? c[3] : 1.0f;

    switch (mode) {
    case 0:                                       /* NONE */
        out[0] = out[1] = out[2] = 0.0f; out[3] = 1.0f;
        return 1;
    case 1: case 2:                               /* PROJECT2D / 3D */
        rc_texel(t, 0, c[0] / q, c[1] / q, out);
        return 1;
    case 3: {                                     /* CUBEMAP */
        float x = c[0], y = c[1], z = c[2];
        float ax = fabsf(x), ay = fabsf(y), az = fabsf(z), ma, sc, tc;
        uint32_t face;
        if (!t->cube) {
            rc_texel(t, 0, c[0], c[1], out);
            return 1;
        }
        if (ax >= ay && ax >= az) {
            face = x > 0 ? 0 : 1; ma = ax; sc = x > 0 ? -z : z; tc = -y;
        } else if (ay >= az) {
            face = y > 0 ? 2 : 3; ma = ay; sc = x; tc = y > 0 ? z : -z;
        } else {
            face = z > 0 ? 4 : 5; ma = az; sc = z > 0 ? x : -x; tc = -y;
        }
        if (ma == 0.0f)
            ma = 1.0f;
        rc_texel(t, face * tex_face_stride(t), (sc / ma + 1.0f) * 0.5f,
                 (tc / ma + 1.0f) * 0.5f, out);
        return 1;
    }
    case 4:                                       /* PASSTHRU */
        for (j = 0; j < 4; j++)
            out[j] = c[j] < 0.0f ? 0.0f : (c[j] > 1.0f ? 1.0f : c[j]);
        return 1;
    case 5:                                       /* CLIPPLANE */
        out[0] = out[1] = out[2] = out[3] = 0.0f;
        for (j = 0; j < 4; j++) {
            int ge = (s_gpu.clip_plane_mode >> (st * 4 + j)) & 1;
            if (ge ? c[j] >= 0.0f : c[j] < 0.0f)
                return 0;
        }
        return 1;
    default:
        /* ponytail: bump-env and dot-product modes read zero; they are water
         * and bump effects -- add them when a title's look depends on one. */
        out[0] = out[1] = out[2] = out[3] = 0.0f;
        return 1;
    }
}

/* The fixed-function fog unit, per vertex, from the program's oFog.x
 * (xemu vsh.c). Fog disabled reads as factor 1: no fog. */
static float fog_factor(float d)
{
    float f, px = s_gpu.fog_param[0], py = s_gpu.fog_param[1];
    if (!s_gpu.fog_enable)
        return 1.0f;
    switch (s_gpu.fog_mode) {
    case 0x800: case 0x802:                       /* EXP, EXP_ABS */
        f = px + exp2f(d * py * 16.0f) - 1.5f;
        break;
    case 0x801: case 0x803:                       /* EXP2, EXP2_ABS */
        f = px + exp2f(-d * d * py * py * 32.0f) - 1.5f;
        break;
    default:                                      /* LINEAR, LINEAR_ABS */
        f = px + d * py - 1.0f;
        break;
    }
    if (s_gpu.fog_mode == 0x802 || s_gpu.fog_mode == 0x803
        || s_gpu.fog_mode == 0x804)
        f = fabsf(f);
    if (f != f)
        f = 1.0f;
    return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
}

static int alpha_test_pass(float a)
{
    int v = (int)(a * 255.0f + 0.5f), r = (int)(s_gpu.alpha_ref & 0xFF);
    switch (s_gpu.alpha_func) {
    case 0x200: return 0;                         /* NEVER    */
    case 0x201: return v <  r;                    /* LESS     */
    case 0x202: return v == r;                    /* EQUAL    */
    case 0x203: return v <= r;                    /* LEQUAL   */
    case 0x204: return v >  r;                    /* GREATER  */
    case 0x205: return v != r;                    /* NOTEQUAL */
    case 0x206: return v >= r;                    /* GEQUAL   */
    default:    return 1;                         /* ALWAYS   */
    }
}

/* Everything a triangle's pixels need, computed once per triangle, so the
 * pixel loop can run over any subset of rows on any thread. */
typedef struct {
    uint8_t *mem;
    uint32_t bpp;
    const Nv2aVshOutput *va, *vb, *vc;
    const float *a, *b, *c;
    float iw[3], uv[3][2], stc[3][4][4], vfog[3], fogc[4], inv_area;
    float *zb;
    int minx, maxx, miny, maxy, use_rc, textured;
} XfTri;

typedef struct { uint64_t depth_fail, pixels; uint32_t zpass; } XfCount;

/* Rows y0, y0+step, ... < maxy of triangle T. */
static void xf_rows(const XfTri *T, int y0, int step, XfCount *cnt)
{
    const float *a = T->a, *b = T->b, *c = T->c;
    const Nv2aVshOutput *va = T->va, *vb = T->vb, *vc = T->vc;
    int x, y, k;

    for (y = y0; y < T->maxy; y += step) {
        for (x = T->minx; x < T->maxx; x++) {
          unsigned sx = surface_samples_x(), sy = surface_samples_y();
          for (unsigned sample = 0; sample < sx * sy; ++sample) {
            float ox = 0.5f, oy = 0.5f;
            if (sx == 2 && sy == 1 && sample == 1) ox = oy = 0.0f;
            else if (sy == 2) {
                ox = (sample % 2) ? 0.75f : 0.25f;
                oy = (sample / 2) ? 0.75f : 0.25f;
            }
            float px = (float)x + ox, py = (float)y + oy;
            float w0 = (b[0] - a[0]) * (py - a[1]) - (b[1] - a[1]) * (px - a[0]);
            float w1 = (c[0] - b[0]) * (py - b[1]) - (c[1] - b[1]) * (px - b[0]);
            float w2 = (a[0] - c[0]) * (py - c[1]) - (a[1] - c[1]) * (px - c[0]);
            float l0, l1, l2, z, col[4], pw;
            uint32_t argb;
            if (!((w0 >= 0 && w1 >= 0 && w2 >= 0)
               || (w0 <= 0 && w1 <= 0 && w2 <= 0)))
                continue;
            /* w1 is opposite a, w2 opposite b, w0 opposite c. */
            l0 = w1 * T->inv_area; l1 = w2 * T->inv_area; l2 = w0 * T->inv_area;
            /* A constant depth plane stays constant. Summing rounded
             * barycentric weights moved coplanar UI layers by one or more
             * float ULPs and made LEQUAL randomly reject their pixels. */
            z = a[2] == b[2] && a[2] == c[2] ? a[2]
                : l0 * a[2] + l1 * b[2] + l2 * c[2];
            {
                float *zp = T->zb ? &T->zb[((size_t)y * sy + sample / sx) * NV_ZBUF_W
                                           + (size_t)x * sx + sample % sx] : NULL;
                int shade = (s_gpu.color_mask & 0x01010101u) || s_gpu.alpha_test;
                /* Depth test, then shade, then alpha test, and only then the
                 * depth write: an alpha-tested texel that is cut away must
                 * not leave its depth behind (foliage, fences). */
                if (zp && s_gpu.depth_test && !depth_pass(z, *zp)) {
                    cnt->depth_fail++;
                    continue;
                }
                argb = 0;
                if (shade) {
                    for (k = 0; k < 4; k++)
                        col[k] = l0 * va->d0[k] + l1 * vb->d0[k] + l2 * vc->d0[k];
                    pw = l0 * T->iw[0] + l1 * T->iw[1] + l2 * T->iw[2];
                    if (T->use_rc) {
                        float d1[4], fog[4], t[4][4], out[4], tc[4];
                        int st, j, keep = 1;
                        for (k = 0; k < 4; k++)
                            d1[k] = l0 * va->d1[k] + l1 * vb->d1[k] + l2 * vc->d1[k];
                        fog[0] = T->fogc[0]; fog[1] = T->fogc[1]; fog[2] = T->fogc[2];
                        fog[3] = l0 * T->vfog[0] + l1 * T->vfog[1] + l2 * T->vfog[2];
                        for (st = 0; st < 4 && keep; st++) {
                            if (!((s_gpu.rc.stage_program >> (st * 5)) & 0x1F)) {
                                t[st][0] = t[st][1] = t[st][2] = 0.0f;
                                t[st][3] = 1.0f;
                                continue;
                            }
                            for (j = 0; j < 4; j++)
                                tc[j] = (l0 * T->stc[0][st][j] + l1 * T->stc[1][st][j]
                                       + l2 * T->stc[2][st][j]) / pw;
                            keep = rc_stage_fetch(st, tc, t[st]);
                        }
                        if (!keep)
                            continue;
                        nv2a_rc_eval(&s_gpu.rc, col, d1, fog,
                                     (const float (*)[4])t, out);
                        if (s_gpu.alpha_test && !alpha_test_pass(out[3]))
                            continue;
                        argb = pack_color(out);
                    } else {
                        argb = pack_color(col);
                        if (T->textured) {
                            uint32_t texel;
                            float tu, tv;
                            tu = (l0 * T->uv[0][0] + l1 * T->uv[1][0] + l2 * T->uv[2][0]) / pw;
                            tv = (l0 * T->uv[0][1] + l1 * T->uv[1][1] + l2 * T->uv[2][1]) / pw;
                            if (sample_texture((uint32_t)(int32_t)floorf(tu),
                                               (uint32_t)(int32_t)floorf(tv), &texel))
                                argb = modulate(texel, col);
                        }
                        if (s_gpu.alpha_test
                            && !alpha_test_pass((float)(argb >> 24) / 255.0f))
                            continue;
                    }
                }
                if (zp && s_gpu.depth_mask)
                    *zp = z;
                if (s_gpu.zpass_enable)
                    cnt->zpass++;
                if (!(s_gpu.color_mask & 0x01010101u))
                    continue;              /* colour writes off: depth only */
                cnt->pixels++;
            }
            put_sample(T->mem, T->bpp, x, y, sample, argb);
          }
        }
    }
}

/* Worker threads for big triangles.
 *
 * The rasteriser is a software GPU, and a Burnout 3 frame is dominated by a
 * handful of full-screen passes -- composites, blurs, the menu backdrops --
 * each two triangles of 300,000 pixels through the register combiners. Those
 * split cleanly by row: every pixel reads only its own depth and colour, so
 * interleaved rows on N threads need no locking. Small triangles stay on the
 * executor thread, where waking workers would cost more than it saves.
 * RECOMP_RASTER_THREADS=<n> sets the count (1 = off); the default leaves a
 * few cores for the title and the host.
 * ponytail: s_gpu.pixels/pixel_max in put_pixel are unsynchronised stats and
 * may undercount; nothing depends on them. */
#define NV_RASTER_MAX_THREADS 16
#define NV_RASTER_MT_MIN_PIXELS 8192

#if defined(_WIN32)
static struct {
    int n;                                  /* threads incl. the caller */
    HANDLE start[NV_RASTER_MAX_THREADS], done;
    volatile LONG pending;
    const XfTri *tri;
    XfCount cnt[NV_RASTER_MAX_THREADS];
} s_pool;

static DWORD WINAPI raster_worker(LPVOID arg)
{
    int k = (int)(intptr_t)arg;
    for (;;) {
        WaitForSingleObject(s_pool.start[k], INFINITE);
        memset(&s_pool.cnt[k], 0, sizeof s_pool.cnt[k]);
        xf_rows(s_pool.tri, s_pool.tri->miny + k, s_pool.n, &s_pool.cnt[k]);
        if (InterlockedDecrement(&s_pool.pending) == 0)
            SetEvent(s_pool.done);
    }
    return 0;
}

static int raster_pool_size(void)
{
    static int init;
    if (!init) {
        const char *e = getenv("RECOMP_RASTER_THREADS");
        SYSTEM_INFO si;
        int n, k;
        init = 1;
        GetSystemInfo(&si);
        n = e ? atoi(e) : (int)si.dwNumberOfProcessors - 4;
        if (n < 1) n = 1;
        if (n > NV_RASTER_MAX_THREADS) n = NV_RASTER_MAX_THREADS;
        s_pool.n = n;
        if (n > 1) {
            s_pool.done = CreateEventA(NULL, FALSE, FALSE, NULL);
            for (k = 0; k < n - 1; k++) {
                s_pool.start[k] = CreateEventA(NULL, FALSE, FALSE, NULL);
                CloseHandle(CreateThread(NULL, 0, raster_worker,
                                         (LPVOID)(intptr_t)k, 0, NULL));
            }
        }
    }
    return s_pool.n;
}

static void xf_rows_parallel(const XfTri *T, XfCount *total)
{
    int n = raster_pool_size(), k;
    long px = (long)(T->maxx - T->minx) * (T->maxy - T->miny);

    if (n <= 1 || px < NV_RASTER_MT_MIN_PIXELS || T->maxy - T->miny < n) {
        xf_rows(T, T->miny, 1, total);
        return;
    }
    s_pool.tri = T;
    s_pool.pending = n - 1;
    for (k = 0; k < n - 1; k++)
        SetEvent(s_pool.start[k]);
    {
        XfCount mine = {0, 0, 0};
        xf_rows(T, T->miny + (n - 1), n, &mine);
        WaitForSingleObject(s_pool.done, INFINITE);
        total->depth_fail += mine.depth_fail;
        total->pixels += mine.pixels;
        total->zpass += mine.zpass;
    }
    for (k = 0; k < n - 1; k++) {
        total->depth_fail += s_pool.cnt[k].depth_fail;
        total->pixels += s_pool.cnt[k].pixels;
        total->zpass += s_pool.cnt[k].zpass;
    }
}
#else
static void xf_rows_parallel(const XfTri *T, XfCount *total)
{
    xf_rows(T, T->miny, 1, total);
}
#endif

static void raster_xf_triangle(const Nv2aVshOutput *va, const Nv2aVshOutput *vb,
                               const Nv2aVshOutput *vc)
{
    XfTri T;
    XfCount cnt = {0, 0, 0};
    const Nv2aVshOutput *v[3];
    const float *a, *b, *c;
    float area, su = 1.0f, sv = 1.0f;
    int k;
    static int no_rc = -1;

    if (no_rc < 0)
        no_rc = getenv("RECOMP_NO_COMBINERS") != NULL;
    memset(&T, 0, sizeof T);
    T.mem = (uint8_t *)xbox_GetMemoryOffset();
    T.bpp = surface_bpp();
    T.textured = s_gpu.texs[0].valid;
    T.use_rc = s_gpu.rc_seen && !no_rc;
    T.va = va; T.vb = vb; T.vc = vc;

    v[0] = va; v[1] = vb; v[2] = vc;
    a = va->pos; b = vb->pos; c = vc->pos;
    T.a = a; T.b = b; T.c = c;
    if (T.bpp != 4 && T.bpp != 2)
        return;
    if (a[3] <= 0.0f || b[3] <= 0.0f || c[3] <= 0.0f) {
        s_gpu.tris_behind++;
        return;
    }
    area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
    if (area == 0.0f || area != area) {
        s_gpu.xf_degenerate++;
        return;
    }
    for (k = 0; k < 3; k++) {
        if (!s_gpu.xf_seeded)
            s_gpu.xf_min[k] = s_gpu.xf_max[k] = a[k];
        if (a[k] < s_gpu.xf_min[k]) s_gpu.xf_min[k] = a[k];
        if (a[k] > s_gpu.xf_max[k]) s_gpu.xf_max[k] = a[k];
    }
    s_gpu.xf_seeded = 1;
    if (!surface_begin_batch(T.mem))
        return;
    if (s_gpu.depth_test || s_gpu.depth_mask)
        T.zb = zbuf_current(1);
    if (tex_size_from_format(s_gpu.texs[0].color)) {
        su = (float)s_gpu.texs[0].width;
        sv = (float)s_gpu.texs[0].height;
    }
    for (k = 0; k < 3; k++) {
        T.iw[k] = 1.0f / v[k]->pos[3];
        T.uv[k][0] = v[k]->tex[0][0] * su * T.iw[k];
        T.uv[k][1] = v[k]->tex[0][1] * sv * T.iw[k];
    }
    if (T.use_rc) {
        int st, j;
        for (k = 0; k < 3; k++) {
            for (st = 0; st < 4; st++)
                for (j = 0; j < 4; j++)
                    T.stc[k][st][j] = v[k]->tex[st][j] * T.iw[k];
            T.vfog[k] = fog_factor(v[k]->fog[0]);
        }
        /* FOG_COLOR is R in bits 0-7, the reverse of a D3DCOLOR. */
        nv2a_rc_unpack(s_gpu.fog_color, T.fogc);
        { float r = T.fogc[2]; T.fogc[2] = T.fogc[0]; T.fogc[0] = r; }
    }

    T.minx = (int)floorf(fminf(a[0], fminf(b[0], c[0])));
    T.maxx = (int)ceilf (fmaxf(a[0], fmaxf(b[0], c[0])));
    T.miny = (int)floorf(fminf(a[1], fminf(b[1], c[1])));
    T.maxy = (int)ceilf (fmaxf(a[1], fmaxf(b[1], c[1])));
    if (T.minx < (int)s_gpu.clip_x) T.minx = (int)s_gpu.clip_x;
    if (T.miny < (int)s_gpu.clip_y) T.miny = (int)s_gpu.clip_y;
    if (T.maxx > (int)(s_gpu.clip_x + s_gpu.clip_w)) T.maxx = (int)(s_gpu.clip_x + s_gpu.clip_w);
    if (T.maxy > (int)(s_gpu.clip_y + s_gpu.clip_h)) T.maxy = (int)(s_gpu.clip_y + s_gpu.clip_h);
    if (T.maxx > NV_ZBUF_W / (int)surface_samples_x()) T.maxx = NV_ZBUF_W / (int)surface_samples_x();
    if (T.maxy > NV_ZBUF_H / (int)surface_samples_y()) T.maxy = NV_ZBUF_H / (int)surface_samples_y();
    if (T.minx >= T.maxx || T.miny >= T.maxy) {
        s_gpu.tris_skipped_offscreen++;
        s_gpu.xf_offscreen++;
        return;
    }
    s_gpu.xf_drawn++;
    T.inv_area = 1.0f / area;

    int hardware = 0;
    if (nv2a_gpu_enabled()) {
        {
            Nv2aGpuParams p = {0};
            int supported = 1;
            for (k = 0; k < 3; k++) {
                memcpy(p.pos[k], v[k]->pos, sizeof(p.pos[k]));
                memcpy(p.color[k], v[k]->d0, sizeof(p.color[k]));
                p.uvw[k][0] = T.uv[k][0]; p.uvw[k][1] = T.uv[k][1]; p.uvw[k][2] = T.iw[k];
            }
            if (T.use_rc) {
                p.rcmeta[0] = 1; p.rcmeta[2] = s_gpu.clip_plane_mode;
                memcpy(p.rc_color_icw, s_gpu.rc.color_icw, sizeof(p.rc_color_icw));
                memcpy(p.rc_alpha_icw, s_gpu.rc.alpha_icw, sizeof(p.rc_alpha_icw));
                memcpy(p.rc_color_ocw, s_gpu.rc.color_ocw, sizeof(p.rc_color_ocw));
                memcpy(p.rc_alpha_ocw, s_gpu.rc.alpha_ocw, sizeof(p.rc_alpha_ocw));
                memcpy(p.rc_factor0, s_gpu.rc.factor0, sizeof(p.rc_factor0));
                memcpy(p.rc_factor1, s_gpu.rc.factor1, sizeof(p.rc_factor1));
                p.rc_final[0] = s_gpu.rc.final0; p.rc_final[1] = s_gpu.rc.final1;
                p.rc_final[2] = s_gpu.rc.final_c0; p.rc_final[3] = s_gpu.rc.final_c1;
                p.rc_control[0] = s_gpu.rc.control; p.rc_control[1] = s_gpu.rc.stage_program;
                for (k = 0; k < 3; k++) {
                    memcpy(p.color1[k], v[k]->d1, sizeof(p.color1[k]));
                    memcpy(p.fog[k], T.fogc, sizeof(p.fog[k])); p.fog[k][3] = T.vfog[k];
                    memcpy(p.stage_coords[k], T.stc[k], sizeof(p.stage_coords[k]));
                }
                for (unsigned st = 0; st < 4; st++) {
                    Texture *t = &s_gpu.texs[st];
                    uint32_t mode = (s_gpu.rc.stage_program >> (st * 5)) & 31;
                    if (mode == 3 && t->cube) { supported = 0; nv2a_gpu_fallback("cubemap"); break; }
                    if (mode < 1 || mode > 3 || !t->valid) continue;
                    if ((uint64_t)t->width * t->height + p.rcmeta[1] > 4096u * 4096u) {
                        supported = 0; nv2a_gpu_fallback("texture-atlas-size"); break;
                    }
                    uint32_t mag = (t->filter >> 24) & 15, min = (t->filter >> 16) & 255;
                    p.stage_info[st][0] = t->width; p.stage_info[st][1] = t->height;
                    p.stage_info[st][2] = p.rcmeta[1];
                    p.stage_info[st][3] = (tex_size_from_format(t->color) ? 4u : 0u)
                        | ((mag == 2 || min == 2 || min == 4 || min == 6) ? 8u : 0u)
                        | (t->addr_u == 1 ? 16u : 0u) | (t->addr_v == 1 ? 32u : 0u);
                    p.rcmeta[1] += t->width * t->height;
                }
            }
            p.box[0] = T.minx; p.box[1] = T.miny; p.box[2] = T.maxx-T.minx; p.box[3] = T.maxy-T.miny;
            if (T.textured) { p.tex[0] = s_gpu.texs[0].width; p.tex[1] = s_gpu.texs[0].height; }
            p.tex[2] = s_gpu.texs[0].addr_u; p.tex[3] = s_gpu.texs[0].addr_v;
            p.blend[0] = s_gpu.blend_enable; p.blend[1] = s_gpu.blend_sfactor;
            p.blend[2] = s_gpu.blend_dfactor; p.blend[3] = s_gpu.blend_equation;
            p.state[0] = s_gpu.color_mask; p.state[1] = s_gpu.blend_color; p.state[3] = 1;
            p.depth[0] = T.zb ? s_gpu.depth_test : 0; p.depth[1] = s_gpu.depth_func;
            p.depth[2] = T.zb ? s_gpu.depth_mask : 0;
            p.alpha[0] = s_gpu.alpha_test; p.alpha[1] = s_gpu.alpha_func; p.alpha[2] = s_gpu.alpha_ref;
            if (supported) hardware = gpu_triangle(&p, T.bpp, T.zb, NV_ZBUF_W, &cnt.pixels, &cnt.depth_fail, &cnt.zpass);
            if (hardware && !s_gpu.zpass_enable) cnt.zpass = 0;
        }
    }
    if (!hardware) {
        gpu_surface_invalidate();
        xf_rows_parallel(&T, &cnt);
    }
    s_gpu.xf_depth_fail += cnt.depth_fail;
    s_gpu.xf_pixels += cnt.pixels;
    s_gpu.zpass_count += cnt.zpass;
    s_gpu.tris_drawn++;
    note_drawn();
}


/* Near-plane clipping.
 *
 * A triangle with a vertex behind the eye (w <= 0) used to be dropped whole.
 * That is invisible for distant geometry and ruinous close up: in a chase
 * view the road under the camera is exactly the set of triangles that reach
 * behind it, so the bottom of the frame showed a hole -- a quarter of every
 * race frame's triangles went this way.
 *
 * Clipping has to happen in clip space, but the D3D epilogue has already
 * divided x, y and z by w and applied the viewport (c[58] scale, c[59]
 * offset) while leaving w itself alone. So each vertex is taken back to clip
 * space first: clip = (screen - offset) / scale * w. Everything else the
 * vertex carries -- colours, fog, texture coordinates -- is linear in clip
 * space and is interpolated directly. The polygon is clipped against
 * w = NV_CLIP_W (Sutherland-Hodgman, one plane), re-projected, and fanned.
 * ponytail: the near plane only; x/y/far fall to the rasteriser's bounds
 * check, which is correct, just slower for huge off-screen triangles.
 * Near and far follow when the viewport gives a depth scale. */
#define NV_CLIP_W 1e-3f

static void xf_to_clip(const Nv2aVshOutput *v, const float k[3],
                       const float o[3], float c[3])
{
    int i;
    for (i = 0; i < 3; i++)
        c[i] = k[i] != 0.0f ? (v->pos[i] - o[i]) / k[i] * v->pos[3] : 0.0f;
}

static void xf_lerp(const Nv2aVshOutput *a, const Nv2aVshOutput *b, float t,
                    Nv2aVshOutput *out)
{
    const float *pa = (const float *)a, *pb = (const float *)b;
    float *po = (float *)out;
    size_t i, n = sizeof(Nv2aVshOutput) / sizeof(float);
    for (i = 0; i < n; i++)
        po[i] = pa[i] + (pb[i] - pa[i]) * t;
}

/* Distance of a clip-space vertex inside one clip plane: 0 eye (w), 1 near
 * (z >= 0), 2 far (z <= w). D3D's clip volume, which the NV2A enforces: a
 * piece between the eye and the near plane has negative depth, and drawn it
 * passes every LESS test -- long streaks across the frame. */
static float clip_distance(const Nv2aVshOutput *v, int plane)
{
    switch (plane) {
    case 0:  return v->pos[3] - NV_CLIP_W;
    case 1:  return v->pos[2];
    default: return v->pos[3] - v->pos[2];
    }
}

static int clip_polygon(const Nv2aVshOutput *in, int n, Nv2aVshOutput *out,
                        int plane)
{
    int m = 0;
    for (int i = 0; i < n; i++) {
        const Nv2aVshOutput *p = &in[i], *q = &in[(i + 1) % n];
        float dp = clip_distance(p, plane), dq = clip_distance(q, plane);
        if (dp >= 0.0f)
            out[m++] = *p;
        if ((dp >= 0.0f) != (dq >= 0.0f))
            xf_lerp(p, q, dp / (dp - dq), &out[m++]);
    }
    return m;
}

static void raster_xf_clipped(const Nv2aVshOutput *a, const Nv2aVshOutput *b,
                              const Nv2aVshOutput *c)
{
    const Nv2aVshOutput *in[3] = { a, b, c };
    Nv2aVshOutput poly[2][12];
    float k[3], o[3], cl[3];
    int n = 3, i, j, inside = 0, below = 0, above = 0, behind = 0, planes;

    {
        const float *sc = nv2a_vsh_constant(58), *of = nv2a_vsh_constant(59);
        for (i = 0; i < 3; i++) { k[i] = sc[i]; o[i] = of[i]; }
    }
    /* Without a depth scale there is no depth range to clip against; keep
     * the eye plane only, as before. */
    planes = k[2] != 0.0f ? 3 : 1;
    for (i = 0; i < 3; i++) {
        if (in[i]->pos[3] <= NV_CLIP_W) { behind++; continue; }
        if (planes == 1) { inside++; continue; }
        float zn = (in[i]->pos[2] - o[2]) / k[2];      /* z / w, 0..1 */
        if (zn < -1e-6f) below++;
        else if (zn > 1.0f + 1e-6f) above++;
        else inside++;
    }
    if (inside == 3) {
        raster_xf_triangle(a, b, c);
        return;
    }
    if (behind == 3 || below == 3 || above == 3) {
        s_gpu.tris_behind++;                  /* wholly outside one plane */
        return;
    }
    for (i = 0; i < 3; i++) {
        poly[0][i] = *in[i];
        xf_to_clip(in[i], k, o, cl);
        for (j = 0; j < 3; j++) poly[0][i].pos[j] = cl[j];
    }
    int cur = 0;
    for (int pl = 0; pl < planes && n >= 3; pl++) {
        n = clip_polygon(poly[cur], n, poly[cur ^ 1], pl);
        cur ^= 1;
    }
    if (n < 3) {
        s_gpu.tris_behind++;
        return;
    }
    /* Back to screen space: divide by the (now positive) w, viewport. */
    for (i = 0; i < n; i++)
        for (j = 0; j < 3; j++)
            poly[cur][i].pos[j] = poly[cur][i].pos[j] / poly[cur][i].pos[3] * k[j] + o[j];
    for (i = 1; i + 1 < n; i++)
        raster_xf_triangle(&poly[cur][0], &poly[cur][i], &poly[cur][i + 1]);
}

static Nv2aVshOutput s_xf[NV_MAX_INDICES];
static Nv2aVshInput s_vsh_inputs[NV_MAX_INDICES];
static Nv2aVshState s_vsh_snapshot;

static int transform_vertex(uint32_t index, Nv2aVshOutput *out)
{
    float in[NV2A_VSH_INPUTS][4];
    uint32_t a;

    for (a = 0; a < NV2A_VSH_INPUTS; a++)
        fetch_attr(&s_gpu.attr[a], index, in[a]);   /* absent: 0,0,0,1 */
    return nv2a_vsh_run((const float (*)[4])in, out);
}

static void raster_batch_program(void)
{
    uint32_t i, n = s_gpu.idx_count;

    for (i = 0; i < n; i++)
        for (uint32_t a = 0; a < NV2A_VSH_INPUTS; a++)
            fetch_attr(&s_gpu.attr[a], s_gpu.idx[i], s_vsh_inputs[i].attr[a]);
    nv2a_vsh_snapshot(&s_vsh_snapshot);
    if (!nv2a_gpu_transform(&s_vsh_snapshot, s_vsh_inputs, s_xf, n)) {
        for (i = 0; i < n; i++)
            if (!nv2a_vsh_run((const float (*)[4])s_vsh_inputs[i].attr, &s_xf[i]))
                return;                                /* no program loaded */
    } else if (s_vsh_snapshot.cxt_write)
        for (i = 0; i < NV2A_VSH_CONSTANTS; i++)
            nv2a_vsh_set_constant(i, s_vsh_snapshot.constants[i]);
    s_gpu.batches_program++;
    s_gpu.verts_program += n;
    if (s_gpu.blend_enable) {
        uint32_t pair = s_gpu.blend_sfactor << 16 | (s_gpu.blend_dfactor & 0xFFFF);
        int j;
        for (j = 0; j < s_gpu.blend_npairs && s_gpu.blend_pairs[j] != pair; j++)
            ;
        if (j == s_gpu.blend_npairs && j < 16)
            s_gpu.blend_pairs[s_gpu.blend_npairs++] = pair;
    }
    {
        /* RECOMP_VSH_TRACE=<n>: print n program batches -- attribute setup,
         * raw inputs, key constants, and what came out. */
        static int left = -1;
        if (left < 0) {
            const char *t = getenv("RECOMP_VSH_TRACE");
            left = t ? atoi(t) : 0;
        }
        /* Spread out: one every 20000 program batches, so a run that spends
         * its first half in menus still traces the 3D scene. */
        if (left > 0 && n >= 3 && s_gpu.batches_program % 20000 == 0) {
            float in[4];
            left--;
            fprintf(stderr, "[VTRACE] prim %u n %u idx %u %u %u\n", s_gpu.prim,
                    n, s_gpu.idx[0], s_gpu.idx[1], s_gpu.idx[2]);
            for (i = 0; i < NV_VERTEX_ATTRS; i++) {
                const VertexAttr *at = &s_gpu.attr[i];
                if (!at->size)
                    continue;
                fetch_attr(at, s_gpu.idx[0], in);
                fprintf(stderr, "[VTRACE]   v%u off %08X type %u size %u"
                        " stride %u = %g %g %g %g\n", i, at->offset, at->type,
                        at->size, at->stride, in[0], in[1], in[2], in[3]);
            }
            {
                static const uint32_t cs[] = {58, 59, 96, 97, 112, 113, 114, 115};
                for (i = 0; i < 8; i++) {
                    const float *c = nv2a_vsh_constant(cs[i]);
                    fprintf(stderr, "[VTRACE]   c[%u] %g %g %g %g%c", cs[i],
                            c[0], c[1], c[2], c[3], 10);
                }
            }
            for (i = 0; i < 3; i++)
                fprintf(stderr, "[VTRACE]   out%u pos %g %g %g %g\n", i,
                        s_xf[i].pos[0], s_xf[i].pos[1], s_xf[i].pos[2],
                        s_xf[i].pos[3]);
        }
    }
    for (i = 0; i < NV2A_VSH_INPUTS; i++) {
        const VertexAttr *at = &s_gpu.attr[i];
        uint32_t f = at->type | (at->size << 4) | (i << 8), j;
        if (!at->size || !at->stride)
            continue;
        for (j = 0; j < (uint32_t)s_gpu.xf_nfmt && s_gpu.xf_fmt[j] != f; j++)
            ;
        if (j == (uint32_t)s_gpu.xf_nfmt && s_gpu.xf_nfmt < 16)
            s_gpu.xf_fmt[s_gpu.xf_nfmt++] = f;
    }
    if (s_gpu.texs[0].valid)
        note_texture_use();

    switch (s_gpu.prim) {
    case NV_PRIM_TRIANGLES:
        for (i = 0; i + 2 < n; i += 3)
            raster_xf_clipped(&s_xf[i], &s_xf[i+1], &s_xf[i+2]);
        break;
    case NV_PRIM_TRIANGLE_STRIP:
        for (i = 0; i + 2 < n; i++)
            raster_xf_clipped(&s_xf[i], &s_xf[i+1], &s_xf[i+2]);
        break;
    case NV_PRIM_TRIANGLE_FAN:
    case NV_PRIM_POLYGON:
        for (i = 1; i + 1 < n; i++)
            raster_xf_clipped(&s_xf[0], &s_xf[i], &s_xf[i+1]);
        break;
    case NV_PRIM_QUADS:
        for (i = 0; i + 3 < n; i += 4) {
            raster_xf_clipped(&s_xf[i], &s_xf[i+1], &s_xf[i+2]);
            raster_xf_clipped(&s_xf[i], &s_xf[i+2], &s_xf[i+3]);
        }
        break;
    case NV_PRIM_QUAD_STRIP:
        for (i = 0; i + 3 < n; i += 2) {
            raster_xf_clipped(&s_xf[i], &s_xf[i+1], &s_xf[i+3]);
            raster_xf_clipped(&s_xf[i], &s_xf[i+3], &s_xf[i+2]);
        }
        break;
    default:
        break;
    }
}

static void raster_batch(void)
{
    uint32_t i;
    uint32_t before = s_gpu.tris_drawn;

    if (s_gpu.idx_count < 3)
        return;
    static int no_vsh = -1;
    if (no_vsh < 0)
        no_vsh = getenv("RECOMP_NO_VSH") != NULL;
    if ((s_gpu.xf_mode & 3) == 2 && !no_vsh) {
        raster_batch_program();
        return;
    }
    if (!batch_is_screen_space()) {
        s_gpu.batches_untransformed++;
        return;
    }

    /* Count why, once per batch: the texture stage cannot change inside one. */
    {
        const VertexAttr *tc = texcoord_attr();

        if (!(tc->offset && tc->stride))
            s_gpu.batches_no_uv++;
        else if (!s_gpu.texs[0].valid)
            s_gpu.batches_no_tex++;
        else {
            s_gpu.batches_textured++;
            note_texture_use();
        }
    }

    switch (s_gpu.prim) {
    case NV_PRIM_TRIANGLES:
        for (i = 0; i + 2 < s_gpu.idx_count; i += 3)
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+1], s_gpu.idx[i+2],
                           vertex_color(s_gpu.idx[i]));
        break;
    case NV_PRIM_TRIANGLE_STRIP:
        for (i = 0; i + 2 < s_gpu.idx_count; i++)
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+1], s_gpu.idx[i+2],
                           vertex_color(s_gpu.idx[i]));
        break;
    case NV_PRIM_TRIANGLE_FAN:
    case NV_PRIM_POLYGON:
        for (i = 1; i + 1 < s_gpu.idx_count; i++)
            raster_indexed(s_gpu.idx[0], s_gpu.idx[i], s_gpu.idx[i+1],
                           vertex_color(s_gpu.idx[0]));
        break;
    case NV_PRIM_QUADS:
        /* Independent quads, four vertices each. A batch of eight is two
         * quads, not one six-triangle fan around the first vertex; with
         * exactly four the two agreed, which is why sharing the fan arm
         * looked right. */
        for (i = 0; i + 3 < s_gpu.idx_count; i += 4) {
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+1], s_gpu.idx[i+2],
                           vertex_color(s_gpu.idx[i]));
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+2], s_gpu.idx[i+3],
                           vertex_color(s_gpu.idx[i]));
        }
        break;
    case NV_PRIM_QUAD_STRIP:
        /* Each vertex pair past the first closes another quad against the
         * pair before it. */
        for (i = 0; i + 3 < s_gpu.idx_count; i += 2) {
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+1], s_gpu.idx[i+3],
                           vertex_color(s_gpu.idx[i]));
            raster_indexed(s_gpu.idx[i], s_gpu.idx[i+3], s_gpu.idx[i+2],
                           vertex_color(s_gpu.idx[i]));
        }
        break;
    default:
        break;                             /* points and lines: not yet */
    }

    if (s_gpu.tris_drawn && (s_gpu.tris_drawn % 500) == 0)
        fprintf(stderr, "  [GPU] %u triangles rasterised\n", s_gpu.tris_drawn);

    /* Capture the surface while the geometry is still on it.
     *
     * The periodic report dumps too, but a title clears every frame and draws
     * in only some of them, so a report almost always lands on a surface that
     * was wiped a moment ago -- which reads as "nothing was drawn" when the
     * triangles went down correctly just before it. A few frames that actually
     * contain geometry are worth more than any number of clears. */
    if (s_gpu.tris_drawn != before && s_drawn_dumps < FB_DUMP_AFTER_DRAW
        && !s_flip_dumping) {
        s_drawn_dumps++;
        dump_surface_bmp();
    }
}

/* What a batch actually contains. Before anything can be rasterised, the
 * question is what space attribute 0 arrives in: a title running a vertex
 * program hands over object-space positions that mean nothing without running
 * it, while pre-transformed screen-space coordinates can be drawn directly. */
/* RECOMP_FRAME_TRACE=<flag file>: once the file exists, log every batch of
 * the next whole frame (flip to flip) -- where it drew, with what texture,
 * blend and depth state, and how many pixels it actually wrote. "The frame
 * is black" has many causes and this is what tells them apart. */

static void frame_trace_flip(void)
{
    static const char *flag = (const char *)-1;
    if (flag == (const char *)-1)
        flag = getenv("RECOMP_FRAME_TRACE");
    if (!flag)
        return;
    if (s_ftrace == 3) {
        /* Deleting the flag file re-arms the trace for the next time it
         * appears, so one run can trace several moments. */
        FILE *f = fopen(flag, "rb");
        if (f)
            fclose(f);
        else
            s_ftrace = 0;
        return;
    }
    if (s_ftrace == 2) {
        s_ftrace = 3;
        fprintf(stderr, "[FTRACE] end of frame\n");
        return;
    }
    if (s_ftrace == 0) {
        FILE *f = fopen(flag, "rb");
        if (!f)
            return;
        fclose(f);
        s_ftrace = 2;
        fprintf(stderr, "[FTRACE] frame begins (flip %u)\n", s_gpu.flips);
    }
}

static void draw_primitive(void)
{
    s_tex_epoch++;                    /* a new batch rechecks textures */
    float v[4];
    uint32_t i;

    if (!s_gpu.prim || !s_gpu.idx_count)
        return;
    s_gpu.draws++;
    if ((s_gpu.draws % 200) == 0)
        fprintf(stderr, "  [GPU] draw #%u\n", s_gpu.draws);
    s_gpu.verts += s_gpu.idx_count;

    /* How many batches carry coordinates at all, and what range they span.
     * A pipeline that decodes perfectly and draws nothing is indistinguishable
     * from one that never ran, unless the vertices themselves are measured. */
    {
        float p[4];
        if (fetch_attr(&s_gpu.attr[0], s_gpu.idx[0], p)) {
            if (p[0] != 0.0f || p[1] != 0.0f || p[2] != 0.0f) {
                s_gpu.nonzero_draws++;
                if (p[0] < s_gpu.min_x) s_gpu.min_x = p[0];
                if (p[0] > s_gpu.max_x) s_gpu.max_x = p[0];
                if (p[1] < s_gpu.min_y) s_gpu.min_y = p[1];
                if (p[1] > s_gpu.max_y) s_gpu.max_y = p[1];
            }
        }
    }

    {
        uint32_t t0 = s_gpu.tris_drawn;
        uint64_t p0 = s_gpu.pixels;
        texture_snapshot_draw();
        raster_batch();
        if (s_ftrace == 2) {
            fprintf(stderr, "[FTRACE] %s prim %u n %u surf %08X %ux%u+%u+%u"
                    " pitch %u | tex %s %08X %ux%u fmt %02X | blend %u %X/%X"
                    " eq %X | z test %u func %X mask %u | tris %u px %llu\n",
                    (s_gpu.xf_mode & 3) == 2 ? "PRG" : "FIX", s_gpu.prim,
                    s_gpu.idx_count, s_gpu.color_offset, s_gpu.clip_w,
                    s_gpu.clip_h, s_gpu.clip_x, s_gpu.clip_y, s_gpu.pitch,
                    s_gpu.texs[0].valid ? "on" : "off", s_gpu.texs[0].offset,
                    s_gpu.texs[0].width, s_gpu.texs[0].height, s_gpu.texs[0].color,
                    s_gpu.blend_enable, s_gpu.blend_sfactor,
                    s_gpu.blend_dfactor, s_gpu.blend_equation,
                    s_gpu.depth_test, s_gpu.depth_func, s_gpu.depth_mask,
                    s_gpu.tris_drawn - t0,
                    (unsigned long long)(s_gpu.pixels - p0));
            if ((s_gpu.xf_mode & 3) == 2)
                fprintf(stderr, "[FTRACE]     v0 %g %g %g %g  v1 %g %g  v2 %g %g%c",
                        s_xf[0].pos[0], s_xf[0].pos[1], s_xf[0].pos[2],
                        s_xf[0].pos[3], s_xf[1].pos[0], s_xf[1].pos[1],
                        s_xf[2].pos[0], s_xf[2].pos[1], 10);
            if (s_gpu.rc_seen) {
                int st;
                fprintf(stderr, "[FTRACE]     rc stages %u ctl %08X prog %05X fin %08X %08X",
                        s_gpu.rc.control & 0xFF, s_gpu.rc.control,
                        s_gpu.rc.stage_program, s_gpu.rc.final0, s_gpu.rc.final1);
                for (st = 0; st < 4; st++)
                    if ((s_gpu.rc.stage_program >> (st * 5)) & 0x1F)
                        fprintf(stderr, " | t%d %08X %ux%u fmt %02X%s", st,
                                s_gpu.texs[st].offset, s_gpu.texs[st].width,
                                s_gpu.texs[st].height, s_gpu.texs[st].color,
                                s_gpu.texs[st].valid ? "" : " (invalid)");
                fputc(10, stderr);
                for (st = 0; st < (int)(s_gpu.rc.control & 0xFF) && st < 8; st++)
                    fprintf(stderr, "[FTRACE]       s%d icw %08X %08X ocw %08X %08X c %08X %08X%c",
                            st, s_gpu.rc.color_icw[st], s_gpu.rc.alpha_icw[st],
                            s_gpu.rc.color_ocw[st], s_gpu.rc.alpha_ocw[st],
                            s_gpu.rc.factor0[st], s_gpu.rc.factor1[st], 10);
            }
            if ((s_gpu.xf_mode & 3) == 2) {
                uint32_t r;
                for (r = 112; r < 116; r++) {
                    const float *c = nv2a_vsh_constant(r);
                    fprintf(stderr, "[FTRACE]     c[%u] %g %g %g %g%c", r,
                            c[0], c[1], c[2], c[3], 10);
                }
            }
        }
    }

    if (getenv("RECOMP_PB_EXEC_VERBOSE")) {
        static int shown;
        if (shown++ < 6) {
            fprintf(stderr, "  [GPU] prim %u, %u indices, pos attr:"
                            " off 0x%08X type %u size %u stride %u\n",
                    s_gpu.prim, s_gpu.idx_count, s_gpu.attr[0].offset,
                    s_gpu.attr[0].type, s_gpu.attr[0].size, s_gpu.attr[0].stride);
            /* The texture stage, for either kind of batch. This used to print
             * only for inline batches, which meant a title drawing through
             * vertex arrays -- Half-Life 2's menu, for one -- showed no
             * texture state at all, and the reason a quad sampled flat was
             * invisible. */
            fprintf(stderr, "  [GPU]   tex: off 0x%08X %ux%u pitch %u"
                            " colour 0x%02X swizzled %d valid %d\n",
                    s_gpu.texs[0].offset, s_gpu.texs[0].width, s_gpu.texs[0].height,
                    s_gpu.texs[0].pitch, s_gpu.texs[0].color,
                    d3d8_format_is_swizzled(s_gpu.texs[0].color), s_gpu.texs[0].valid);
            {
                uint32_t k;
                for (k = 0; k < s_gpu.idx_count && k < 3; k++) {
                    float t[2];
                    if (fetch_texcoord(s_gpu.idx[k], t))
                        fprintf(stderr, "  [GPU]   uv[%u] = %.3f %.3f\n",
                                k, t[0], t[1]);
                }
            }
            /* An inline batch has no guest buffer to go and look at -- the
             * vertices are the payload -- so print the payload too. */
            if (s_gpu.inline_active) {
                uint32_t k;
                fprintf(stderr, "  [GPU]   inline %u dwords:", s_gpu.inline_count);
                for (k = 0; k < s_gpu.inline_count && k < 16; k++)
                    fprintf(stderr, " %08X", s_gpu.inline_buf[k]);
                fprintf(stderr, "\n");
                for (k = 0; k < s_gpu.idx_count && k < 4; k++) {
                    float t[2];
                    if (fetch_texcoord(s_gpu.idx[k], t))
                        fprintf(stderr, "  [GPU]   uv[%u] = %.3f %.3f\n",
                                k, t[0], t[1]);
                }
                fprintf(stderr, "  [GPU]   tex: off 0x%08X %ux%u pitch %u"
                                " colour 0x%02X valid %d\n",
                        s_gpu.texs[0].offset, s_gpu.texs[0].width, s_gpu.texs[0].height,
                        s_gpu.texs[0].pitch, s_gpu.texs[0].color, s_gpu.texs[0].valid);
            }
            {
                /* Every attribute the batch has, not just position. If the
                 * other streams carry data and position does not, the problem
                 * is one buffer rather than the whole vertex path. */
                const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
                uint32_t a, k;
                for (a = 0; a < NV_VERTEX_ATTRS; a++) {
                    const VertexAttr *at = &s_gpu.attr[a];
                    uint32_t nz = 0;
                    if (!at->offset || !at->size)
                        continue;
                    for (k = 0; k < 64; k++)
                        if (mem[at->offset + k]) nz++;
                    fprintf(stderr, "  [GPU]   attr%-2u off 0x%08X type %u"
                                    " size %u stride %-3u  %u/64 bytes set\n",
                            a, at->offset, at->type, at->size, at->stride, nz);
                }
            }
            {
                /* Raw bytes at the array, in case the values read as zero:
                 * that looks the same whether the offset is wrong or the
                 * buffer genuinely has not been filled yet. */
                const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
                uint32_t k;
                fprintf(stderr, "  [GPU]   bytes @0x%08X:", s_gpu.attr[0].offset);
                for (k = 0; k < 32; k++)
                    fprintf(stderr, " %02X", mem[s_gpu.attr[0].offset + k]);
                fprintf(stderr, "\n");
                fprintf(stderr, "  [GPU]   indices:");
                for (k = 0; k < s_gpu.idx_count && k < 8; k++)
                    fprintf(stderr, " %u", s_gpu.idx[k]);
                fprintf(stderr, "\n");
            }
            for (i = 0; i < s_gpu.idx_count && i < 3; i++) {
                if (fetch_attr(&s_gpu.attr[0], s_gpu.idx[i], v))
                    fprintf(stderr, "  [GPU]   v[%u] = %.3f %.3f %.3f %.3f\n",
                            s_gpu.idx[i], v[0], v[1], v[2], v[3]);
            }
        }
    }
}

/* Draw the vertices the title wrote straight into the pushbuffer.
 *
 * INLINE_ARRAY carries no offsets and no indices: the dwords between BEGIN and
 * END *are* the vertex buffer, packed in attribute order using the same
 * SET_VERTEX_DATA_ARRAY_FORMAT registers an ordinary array would use. So the
 * whole batch is describable as a vertex array whose base happens to be that
 * payload, which means synthesising the layout and handing it to the existing
 * path -- rather than a second copy of the topology and rasterisation code.
 *
 * The title's own attribute table is saved and put back: these offsets and
 * strides are ours, and it has not stopped using its.
 *
 * ponytail: each attribute is padded to a whole dword. That is exact for the
 * float and D3DCOLOR formats every inline batch actually uses; a packed
 * sub-dword attribute would need the unpadded layout.
 */
static void draw_inline_array(void)
{
    s_tex_epoch++;                    /* a new batch rechecks textures */
    VertexAttr saved[NV_VERTEX_ATTRS];
    uint32_t off = 0, a, i, vsize, count;

    memcpy(saved, s_gpu.attr, sizeof saved);

    for (a = 0; a < NV_VERTEX_ATTRS; a++) {
        uint32_t bytes;
        if (!s_gpu.attr[a].size)
            continue;
        switch (s_gpu.attr[a].type) {
        case 0:  bytes = 4;                        break;  /* D3DCOLOR   */
        case 2:  bytes = 4 * s_gpu.attr[a].size;   break;  /* float      */
        case 4:  bytes = s_gpu.attr[a].size;       break;  /* ubyte norm */
        default: bytes = 4 * s_gpu.attr[a].size;   break;
        }
        s_gpu.attr[a].offset = off;
        off += (bytes + 3u) & ~3u;
    }
    vsize = off;
    if (!vsize)
        goto out;

    count = (s_gpu.inline_count * 4) / vsize;
    if (count < 3 || count > NV_MAX_INDICES)
        goto out;
    for (a = 0; a < NV_VERTEX_ATTRS; a++)
        if (s_gpu.attr[a].size)
            s_gpu.attr[a].stride = vsize;

    for (i = 0; i < count; i++)
        s_gpu.idx[i] = i;
    s_gpu.idx_count = count;

    s_gpu.inline_active = 1;
    draw_primitive();
    s_gpu.inline_active = 0;

out:
    memcpy(s_gpu.attr, saved, sizeof saved);
    s_gpu.idx_count = 0;
}

/* Draw the vertices SET_VERTEX3F/4F completed.
 *
 * Same trick as draw_inline_array: rather than a second copy of the topology
 * and rasterisation code, describe what was accumulated as an ordinary vertex
 * array and hand it to the existing path. The layout is ours and fixed, so
 * the attribute table is written out here rather than derived from the
 * title's format registers.
 *
 * The title's own table is saved and put back -- it has not stopped using it.
 *
 * ponytail: position, diffuse and texcoord0 only. That is what a 2D quad
 * carries and what this rasteriser samples; a second texcoord set or a normal
 * would need the D3D11 translator, not more slots here.
 */
static void draw_immediate(void)
{
    s_tex_epoch++;                    /* a new batch rechecks textures */
    VertexAttr saved[NV_VERTEX_ATTRS];
    uint32_t i;

    if (s_gpu.imm_count < 3)
        return;

    memcpy(saved, s_gpu.attr, sizeof saved);
    memset(s_gpu.attr, 0, sizeof s_gpu.attr);
    /* Offsets are byte offsets into inline_buf here, not guest addresses --
     * fetch_attr reads them that way while inline_active is set, which is
     * also why 0 is a legal offset for position. Every attribute is a float4;
     * the ones the batch never set carry their standing values, as on the
     * GPU. */
    for (i = 0; i < NV_VERTEX_ATTRS; i++) {
        s_gpu.attr[i].type = 2;
        s_gpu.attr[i].size = 4;
        s_gpu.attr[i].offset = i * 16;
        s_gpu.attr[i].stride = IMM_VERTEX_DWORDS * 4;
    }

    for (i = 0; i < s_gpu.imm_count && i < NV_MAX_INDICES; i++)
        s_gpu.idx[i] = i;
    s_gpu.idx_count = i;

    /* fetch_attr bounds-checks against inline_count dwords. */
    s_gpu.inline_count = s_gpu.imm_count * IMM_VERTEX_DWORDS;
    s_gpu.inline_active = 1;
    draw_primitive();
    s_gpu.inline_active = 0;

    memcpy(s_gpu.attr, saved, sizeof saved);
    s_gpu.idx_count = 0;
    s_gpu.inline_count = 0;
}

/* A vertex is complete: append all 16 attributes in the layout
 * draw_immediate describes. */
static void imm_emit_vertex(void)
{
    uint32_t at = s_gpu.imm_count * IMM_VERTEX_DWORDS;

    if (!s_gpu.prim || at + IMM_VERTEX_DWORDS > NV_MAX_INLINE)
        return;
    memcpy(&s_gpu.inline_buf[at], s_gpu.imm_attr, sizeof s_gpu.imm_attr);
    s_gpu.imm_count++;
}

/* The immediate-mode writes. Returns 1 if `method` was one of them.
 *
 * Split out because it is a range test against five separate bases, and that
 * reads better than five more cases in an already long switch.
 */
/* The immediate-mode attribute methods (xemu pgraph.c SET_VERTEX_DATA*).
 * Each sets an attribute's current value; completing attribute 0 -- the
 * position -- emits a vertex carrying every attribute as it stands. This used
 * to keep position, diffuse and texcoord 0 only, which dropped any quad with
 * a second texture coordinate: Burnout 3 composites its whole 3D scene into
 * the frame with exactly such a quad, and the frame never got the scene. */
static int imm_vertex_method(uint32_t method, uint32_t param)
{
    union { uint32_t u; float f; } v;
    uint32_t attr, c;
    float *a;
    v.u = param;

    if (method >= NV097_SET_VERTEX4F && method < NV097_SET_VERTEX4F + 16) {
        c = (method - NV097_SET_VERTEX4F) / 4;
        s_gpu.imm_attr[0][c] = v.f;
        s_gpu.imm_used |= 1;
        if (c == 3)
            imm_emit_vertex();
        return 1;
    }
    if (method >= NV097_SET_VERTEX3F && method < NV097_SET_VERTEX3F + 12) {
        c = (method - NV097_SET_VERTEX3F) / 4;
        s_gpu.imm_attr[0][c] = v.f;
        s_gpu.imm_used |= 1;
        if (c == 2) {
            s_gpu.imm_attr[0][3] = 1.0f;
            imm_emit_vertex();
        }
        return 1;
    }
    if (method >= NV097_SET_VERTEX_DATA2F_M
            && method < NV097_SET_VERTEX_DATA2F_M + NV_VERTEX_ATTRS * 8) {
        attr = (method - NV097_SET_VERTEX_DATA2F_M) / 8;
        c = ((method - NV097_SET_VERTEX_DATA2F_M) % 8) / 4;
        a = s_gpu.imm_attr[attr];
        a[c] = v.f;
        if (c == 1) {
            a[2] = 0.0f; a[3] = 1.0f;
        }
        s_gpu.imm_used |= (uint16_t)(1u << attr);
        if (attr == 0 && c == 1)
            imm_emit_vertex();
        return 1;
    }
    if (method >= NV097_SET_VERTEX_DATA4F_M
            && method < NV097_SET_VERTEX_DATA4F_M + NV_VERTEX_ATTRS * 16) {
        attr = (method - NV097_SET_VERTEX_DATA4F_M) / 16;
        c = ((method - NV097_SET_VERTEX_DATA4F_M) % 16) / 4;
        s_gpu.imm_attr[attr][c] = v.f;
        s_gpu.imm_used |= (uint16_t)(1u << attr);
        if (attr == 0 && c == 3)
            imm_emit_vertex();
        return 1;
    }
    if (method >= NV097_SET_VERTEX_DATA2S
            && method < NV097_SET_VERTEX_DATA2S + NV_VERTEX_ATTRS * 4) {
        attr = (method - NV097_SET_VERTEX_DATA2S) / 4;
        a = s_gpu.imm_attr[attr];
        a[0] = (float)(int16_t)(param & 0xFFFF);
        a[1] = (float)(int16_t)(param >> 16);
        a[2] = 0.0f; a[3] = 1.0f;
        s_gpu.imm_used |= (uint16_t)(1u << attr);
        if (attr == 0)
            imm_emit_vertex();
        return 1;
    }
    if (method >= NV097_SET_VERTEX_DATA4UB
            && method < NV097_SET_VERTEX_DATA4UB + NV_VERTEX_ATTRS * 4) {
        /* Bytes in register order, x from the low byte (xemu). */
        attr = (method - NV097_SET_VERTEX_DATA4UB) / 4;
        a = s_gpu.imm_attr[attr];
        a[0] = (float)( param        & 0xFF) / 255.0f;
        a[1] = (float)((param >>  8) & 0xFF) / 255.0f;
        a[2] = (float)((param >> 16) & 0xFF) / 255.0f;
        a[3] = (float)( param >> 24        ) / 255.0f;
        s_gpu.imm_used |= (uint16_t)(1u << attr);
        if (attr == 0)
            imm_emit_vertex();
        return 1;
    }
    if (method >= NV097_SET_VERTEX_DATA4S_M
            && method < NV097_SET_VERTEX_DATA4S_M + NV_VERTEX_ATTRS * 8) {
        attr = (method - NV097_SET_VERTEX_DATA4S_M) / 8;
        c = ((method - NV097_SET_VERTEX_DATA4S_M) % 8) / 4;
        a = s_gpu.imm_attr[attr];
        a[c * 2]     = (float)(int16_t)(param & 0xFFFF);
        a[c * 2 + 1] = (float)(int16_t)(param >> 16);
        s_gpu.imm_used |= (uint16_t)(1u << attr);
        if (attr == 0 && c == 1)
            imm_emit_vertex();
        return 1;
    }
    return 0;
}
/* The texture stage registers, all four stages. Stage i's registers are
 * 0x1B00 + 0x40*i onwards; only stage 0 used to be decoded, which was
 * enough for menus drawn one texture at a time and not for a 3D frame that
 * composites its scene through a second stage. */
static void tex_stage_method(uint32_t method, uint32_t param)
{
    s_tex_epoch++;
    uint32_t stage = (method - NV_TEX_FIRST) / 0x40;
    uint32_t reg = NV_TEX_FIRST + ((method - NV_TEX_FIRST) & 0x3F);
    Texture *t = &s_gpu.texs[stage];

    switch (reg) {
    case NV097_SET_TEXTURE_OFFSET:
        /* A DMA-object offset like a surface's: physical, reached through
         * the contiguous window when it names contiguous memory. */
        t->offset = dma_resolve(param);
        break;
    case NV097_SET_TEXTURE_FORMAT:
        t->color = (param >> 8) & 0xFF;
        t->cube = (param >> 2) & 1;
        t->levels = (param >> 16) & 0xF;
        /* A swizzled texture carries its own dimensions here, as log2 in
         * BASE_SIZE_U/V. It has to: IMAGE_RECT describes a linear image, and
         * a title that only uses swizzled textures never sends one. */
        if (tex_size_from_format(t->color)) {
            t->width  = 1u << ((param >> 20) & 0xF);
            t->height = 1u << ((param >> 24) & 0xF);
        }
        break;
    case NV097_SET_TEXTURE_PALETTE:
        /* The low six bits carry the DMA context and the entry count. */
        t->palette = dma_resolve(param & ~0x3Fu);
        break;
    case NV097_SET_TEXTURE_ADDRESS:
        /* Four bits per axis. 1 is wrap, 3 clamp-to-edge; mirror and border
         * fall back to clamp, wrong at an edge rather than everywhere. */
        t->addr_u =  param        & 0xF;
        t->addr_v = (param >>  8) & 0xF;
        break;
    case NV097_SET_TEXTURE_CONTROL1:
        t->pitch = param >> 16;          /* linear formats only */
        break;
    case NV097_SET_TEXTURE_IMAGE_RECT:
        t->width  = param >> 16;
        t->height = param & 0xFFFF;
        break;
    case 0x1B14:                                  /* SET_TEXTURE_FILTER */
        t->filter = param;
        break;
    default:
        break;
    }
    tex_update_valid(t);
    if (stage == 0)
        record_tex_reg(method, param);
}

void nv2a_pb_exec_method(uint32_t subch, uint32_t method, uint32_t param)
{
    t_exec_thread = 1;
    static int inited;
    if (!inited) {
        inited = 1;
        s_gpu.color_mask = 0x01010101u;           /* all channels, as reset */
        {
            int i;
            for (i = 0; i < NV_VERTEX_ATTRS; i++)
                s_gpu.imm_attr[i][3] = 1.0f;
        }
        s_gpu.min_x = s_gpu.min_y = 1e30f;
        s_gpu.max_x = s_gpu.max_y = -1e30f;
    }
    /* Bring-up: the first parameters each surface method carries. A wrong
     * pitch or clip is indistinguishable from a method never arriving unless
     * the values are visible. Cached: this runs for every method, and an
     * uncached getenv here was a quarter of the executor's time. */
    static int verbose = -1;
    if (verbose < 0)
        verbose = getenv("RECOMP_PB_EXEC_VERBOSE") != NULL;
    if (verbose) {
        static int shown[8];
        int slot = -1;
        switch (method) {
        case NV097_SET_SURFACE_CLIP_HORIZONTAL: slot = 0; break;
        case NV097_SET_SURFACE_CLIP_VERTICAL:   slot = 1; break;
        case NV097_SET_SURFACE_FORMAT:          slot = 2; break;
        case NV097_SET_SURFACE_PITCH:           slot = 3; break;
        case NV097_SET_SURFACE_COLOR_OFFSET:    slot = 4; break;
        case NV097_SET_COLOR_CLEAR_VALUE:       slot = 5; break;
        case NV097_CLEAR_SURFACE:               slot = 6; break;
        default: break;
        }
        if (slot >= 0 && shown[slot]++ < 4)
            fprintf(stderr, "  [GPU] subch %u method 0x%04X param 0x%08X\n",
                    subch, method, param);
    }

    if (s_ftrace == 2) {
        /* RECOMP_FRAME_TRACE_METHODS: every method of the traced frame,
         * less the bulk (vertex data, program and constant uploads). */
        static int all = -1;
        if (all < 0)
            all = getenv("RECOMP_FRAME_TRACE_METHODS") != NULL;
        if (all && !(method >= 0x1800 && method < 0x1A00)
            && !(method >= 0x0B00 && method < 0x0C00))
            fprintf(stderr, "[FTRACE]   m %u:%04X = %08X%c", subch, method,
                    param, 10);
    }
    if (gpu_aux_method(subch, method, param)) return;
    if (subch == 0 && method >= NV_TEX_FIRST && method <= NV_TEX_LAST) {
        tex_stage_method(method, param);
        return;
    }
    if (subch != 0) {                      /* 3D class lives on subchannel 0 */
        note_unhandled(method, param);
        return;
    }
    if (method == 0x01A4 || method == 0x1D6C || method == 0x1D70) {
        static unsigned shown;
        if (shown++ < 24)
            fprintf(stderr, "[SEMAPHORE] method=%04X parameter=%08X\n", method, param);
    }
    switch (method) {
    case 0x01A4: /* SET_CONTEXT_DMA_SEMAPHORE */
        s_semaphore_handle = param;
        break;
    case 0x1D6C: /* SET_SEMAPHORE_OFFSET */
        s_semaphore_offset = param;
        break;
    case 0x1D70: /* BACK_END_WRITE_SEMAPHORE_RELEASE */
        if (!semaphore_release(param)) note_unhandled(method, param);
        break;
    case NV097_SET_SURFACE_CLIP_HORIZONTAL:
        s_gpu.clip_x = param & 0xFFFF;
        s_gpu.clip_w = (param >> 16) & 0xFFFF;
        break;
    case NV097_SET_SURFACE_CLIP_VERTICAL:
        s_gpu.clip_y = param & 0xFFFF;
        s_gpu.clip_h = (param >> 16) & 0xFFFF;
        break;
    case NV097_SET_SURFACE_FORMAT:
        s_gpu.format = param;
        break;
    case NV097_SET_SURFACE_PITCH:
        s_gpu.pitch = param & 0xFFFF;      /* colour pitch; zeta is the top half */
        break;
    case NV097_SET_SURFACE_COLOR_OFFSET:
        if (s_ftrace == 2)
            fprintf(stderr, "[FTRACE] color offset -> %08X%c", param, 10);
        s_gpu.color_offset = param;
        break;
    case NV097_SET_COLOR_CLEAR_VALUE:
        s_gpu.clear_color = param;
        break;
    case NV097_SET_BLEND_ENABLE:
        s_gpu.blend_enable = param;
        break;
    case NV097_SET_BLEND_EQUATION:
        s_gpu.blend_equation = param;
        break;
    case 0x0358:                                  /* SET_COLOR_MASK */
        s_gpu.color_mask = param;
        break;

    /* Register combiners. */
    case 0x0288: s_gpu.rc.final0 = param; break;  /* SPECULAR_FOG_CW0 */
    case 0x028C: s_gpu.rc.final1 = param; break;  /* SPECULAR_FOG_CW1 */
    case 0x1E20: s_gpu.rc.final_c0 = param; break;/* SPECULAR_FOG_FACTOR */
    case 0x1E24: s_gpu.rc.final_c1 = param; break;
    case 0x1E60:                                  /* COMBINER_CONTROL */
        s_gpu.rc.control = param;
        s_gpu.rc_seen = 1;
        break;
    case 0x1E70: s_gpu.rc.stage_program = param; break;
    case 0x17F8: s_gpu.clip_plane_mode = param; break;

    /* Alpha test and fog. */
    case 0x0300: s_gpu.alpha_test = param; break;
    case 0x033C: s_gpu.alpha_func = param; break;
    case 0x0340: s_gpu.alpha_ref = param; break;
    case 0x02A4: s_gpu.fog_enable = param; break;
    case 0x02A8: s_gpu.fog_color = param; break;  /* R in bits 0-7 */
    case 0x029C: s_gpu.fog_mode = param; break;
    case 0x09C0: memcpy(&s_gpu.fog_param[0], &param, 4); break;
    case 0x09C4: memcpy(&s_gpu.fog_param[1], &param, 4); break;
    case 0x17C8:                                  /* CLEAR_REPORT_VALUE */
        gpu_surface_sync();                       /* pending counts first */
        s_gpu.zpass_count = 0;
        break;
    case 0x17CC:                                  /* SET_ZPASS_PIXEL_COUNT_ENABLE */
        s_gpu.zpass_enable = param;
        break;
    case 0x17D0: {                                /* GET_REPORT */
        /* The GPU's answer to a visibility test: 16 bytes at the report
         * DMA offset -- a timestamp, the pixel count, and 0 for "done".
         * Never written, a title polling it (D3D's GetVisibilityTestResult
         * starts the slot at 0xFFFFFFFF) sees every test incomplete. Burnout
         * 3 culls its world with these, so the race's main view drew only
         * the sky while the cube-map passes, which do not test, drew it all.
         * The report DMA object starts at physical 0 here, as D3D sets it. */
        uint8_t *mem = (uint8_t *)xbox_GetMemoryOffset();
        uint32_t at;
        gpu_surface_sync();
        at = dma_resolve(param & 0x00FFFFFFu);
        uint64_t stamp = (uint64_t)s_gpu.reports++ * 1000u;
        memcpy(mem + at, &stamp, 8);
        if (xbox_nv2a_draw_disabled)          /* nothing rasterised: report visible */
            s_gpu.zpass_count = 0x00100000u;
        memcpy(mem + at + 8, &s_gpu.zpass_count, 4);
        memset(mem + at + 12, 0, 4);
        break;
    }
    case NV097_SET_BLEND_COLOR:
        s_gpu.blend_color = param;
        break;
    case NV097_SET_BLEND_FUNC_SFACTOR:
        s_gpu.blend_sfactor = param;
        break;
    case NV097_SET_BLEND_FUNC_DFACTOR:
        s_gpu.blend_dfactor = param;
        break;
    case NV097_CLEAR_SURFACE:
        if (xbox_nv2a_draw_disabled)
            break;
        if (s_ftrace == 2)
            fprintf(stderr, "[FTRACE] clear %X surf %08X %ux%u colour %08X%c",
                    param, s_gpu.color_offset, s_gpu.clip_w, s_gpu.clip_h,
                    s_gpu.clear_color, 10);
        if (gpu_clear(param)) {
            clear_surface(param);   /* guest copy and bookkeeping; Z is on the GPU */
        } else {
            gpu_surface_invalidate();
            clear_surface(param);
            if (param & 0x01)                     /* Z */
                zbuf_clear();
        }
        break;

    case NV097_SET_BEGIN_END:
        if (xbox_nv2a_draw_disabled) {
            s_gpu.prim = 0;
            break;
        }
        if (param) {
            s_gpu.prim = param;
            s_gpu.idx_count = 0;
            s_gpu.inline_count = 0;
            s_gpu.imm_count = 0;
        } else {
            /* Three ways a batch can have arrived, and only one is in use at
             * a time: vertices completed by SET_VERTEX4F, a payload written
             * with INLINE_ARRAY, or indices into the title's own arrays. */
            if (s_gpu.imm_count)
                draw_immediate();
            else if (s_gpu.inline_count)
                draw_inline_array();
            else
                draw_primitive();
            s_gpu.prim = 0;
            s_gpu.inline_count = 0;
            s_gpu.imm_count = 0;
        }
        break;

    case NV097_INLINE_ARRAY:
        /* Vertex data, not a pointer to it. Buffered rather than decoded here
         * because the format is only fully known at END. */
        if (s_gpu.prim && s_gpu.inline_count < NV_MAX_INLINE)
            s_gpu.inline_buf[s_gpu.inline_count++] = param;
        break;

    case NV097_DRAW_ARRAYS: {
        /* The method this title actually draws with, and the reason the
         * executor reported zero draws while geometry was being submitted the
         * whole time: BEGIN_END arrived, END arrived, and in between came a
         * run description rather than the index list the draw path wanted, so
         * every batch ended with idx_count == 0 and was dropped in silence.
         *
         * Expanded into indices because that is what the rasteriser consumes,
         * and an implicit run is just the indices start..start+count-1. */
        uint32_t start = param & 0x00FFFFFFu;
        uint32_t count = ((param >> 24) & 0xFFu) + 1u;
        uint32_t i;

        if (!s_gpu.prim)
            break;
        for (i = 0; i < count && s_gpu.idx_count < NV_MAX_INDICES; i++)
            s_gpu.idx[s_gpu.idx_count++] = start + i;
        break;
    }

    case NV097_ARRAY_ELEMENT32:
        if (s_gpu.prim && s_gpu.idx_count < NV_MAX_INDICES)
            s_gpu.idx[s_gpu.idx_count++] = param;
        break;

    case NV097_ARRAY_ELEMENT16:
        /* Two 16-bit indices per parameter word. */
        if (s_gpu.prim && s_gpu.idx_count + 2 <= NV_MAX_INDICES) {
            s_gpu.idx[s_gpu.idx_count++] = (uint16_t)(param & 0xFFFF);
            s_gpu.idx[s_gpu.idx_count++] = (uint16_t)(param >> 16);
        }
        break;

    case NV097_SET_FLIP_READ:
        if (s_ftrace == 2)
            fprintf(stderr, "[FTRACE] P_READ %u%c", param, 10);
        s_gpu.flip_read = param;
        return;

    case NV097_SET_FLIP_WRITE:
        if (s_ftrace == 2)
            fprintf(stderr, "[FTRACE] P_WRITE %u%c", param, 10);
        s_gpu.flip_write = param;
        return;

    case NV097_SET_FLIP_MODULO:
        s_gpu.flip_modulo = param;
        return;

    case NV097_FLIP_INCREMENT_WRITE:
        gpu_surface_sync();
        s_gpu.flip_write = s_gpu.flip_modulo
                         ? (s_gpu.flip_write + 1) % s_gpu.flip_modulo
                         : s_gpu.flip_write + 1;
        s_gpu.flips++;
        frame_trace_flip();
        return;

    case NV097_FLIP_STALL:
        gpu_surface_sync();
        s_span_epoch++;
        if (s_ftrace == 2)
            fprintf(stderr, "[FTRACE] FLIP_STALL presenting %08X (color now %08X)%c",
                    s_gpu.drawn_offset, s_gpu.color_offset, 10);
        /* The stall ends when the buffer being read is the one just finished.
         * There is no scanout here to wait for, so that is now. */
        s_gpu.flip_read = s_gpu.flip_write;
        /* And this is a completed swap, which is what a title's own swap
         * counter counts -- see xbox_Nv2aFrameCounterFlip. */
        xbox_Nv2aFrameCounterFlip();
        /* Hand the window a copy of the frame just finished.
         *
         * The buffer the title has finished is the one the last batch drew
         * into, which is what drawn_offset holds and why it exists: by the
         * flip, color_offset has already moved to the next buffer. Copying
         * here, rather than letting the window read guest memory on its own
         * clock, is what stops it showing a surface the rasteriser is still
         * writing. */
        {
            extern void xbox_FramebufferWindowFrameStats(uint32_t);
            static uint32_t draws_at_flip;
            xbox_FramebufferWindowFrameStats(s_gpu.draws - draws_at_flip);
            draws_at_flip = s_gpu.draws;
        }
        if (s_gpu.pitch && !xbox_nv2a_draw_disabled) {
            extern void xbox_FramebufferWindowPresentSurface(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
            uint32_t done = s_gpu.drawn_offset ? s_gpu.drawn_offset
                                               : s_gpu.color_offset;
            uint32_t pitch = s_gpu.drawn_offset ? s_gpu.drawn_pitch
                                                : s_gpu.pitch;
            if (done) {
                xbox_FramebufferWindowSet(dma_resolve(done), pitch);
                xbox_FramebufferWindowPresentSurface(dma_resolve(done), pitch,
                    s_gpu.drawn_offset ? s_gpu.drawn_bpp : surface_bpp(),
                    s_gpu.drawn_offset ? s_gpu.drawn_samples_x : surface_samples_x(),
                    s_gpu.drawn_offset ? s_gpu.drawn_samples_y : surface_samples_y());
            }
        }
        {
            /* RECOMP_FB_DUMP_FLIPS: dump every presented frame, not one per
             * report -- a consecutive sequence, which is what a headless
             * recording (frames -> ffmpeg) needs. "1" dumps from boot; any
             * other value names a file, and frames are dumped while it
             * exists, so a capture can start mid-game without writing
             * gigabytes of menus first. */
            static const char *spec = (const char *)-1;
            if (spec == (const char *)-1)
                spec = getenv("RECOMP_FB_DUMP_FLIPS");
            if (spec && !xbox_nv2a_draw_disabled) {
                int on = strcmp(spec, "1") == 0;
                if (!on) {
                    FILE *f = fopen(spec, "rb");
                    if (f) { fclose(f); on = 1; }
                }
                s_flip_dumping = on;
                if (on)
                    dump_surface_bmp();
            }
        }
        s_gpu.drawn_stale = 1;

        if (getenv("RECOMP_PB_EXEC_VERBOSE")) {
            static unsigned n;
            if (n++ < 8) {
                fprintf(stderr, "  [GPU] flip %u: read=%u write=%u\n",
                        s_gpu.flips, s_gpu.flip_read, s_gpu.flip_write);
                fflush(stderr);
            }
        }
        return;


    /* Vertex programs and their constants: forwarded as they arrive. */
    case 0x1E94:                                  /* TRANSFORM_EXECUTION_MODE */
        s_gpu.xf_mode = param;
        break;
    case 0x1E98:                                  /* _PROGRAM_CXT_WRITE_EN */
        nv2a_vsh_set_cxt_write(param);
        break;
    case 0x1E9C:                                  /* _PROGRAM_LOAD */
        nv2a_vsh_set_load_slot(param);
        break;
    case 0x1EA0:                                  /* _PROGRAM_START */
        nv2a_vsh_set_start_slot(param);
        break;
    case 0x1EA4:                                  /* _CONSTANT_LOAD */
        nv2a_vsh_set_constant_load(param);
        break;

    /* Depth. */
    case 0x030C: s_gpu.depth_test = param; break; /* DEPTH_TEST_ENABLE */
    case 0x0354: s_gpu.depth_func = param; break; /* DEPTH_FUNC */
    case 0x035C: s_gpu.depth_mask = param; break; /* DEPTH_MASK */
    case 0x0214: s_gpu.zeta_offset = param; break;/* SURFACE_ZETA_OFFSET */
    case 0x1D8C: s_gpu.zstencil_clear = param; break; /* ZSTENCIL_CLEAR_VALUE */

    default:
        if (method >= 0x0260 && method < 0x0280) {         /* ALPHA_ICW(i) */
            s_gpu.rc.alpha_icw[(method - 0x0260) / 4] = param;
            break;
        }
        if (method >= 0x0AC0 && method < 0x0AE0) {         /* COLOR_ICW(i) */
            s_gpu.rc.color_icw[(method - 0x0AC0) / 4] = param;
            break;
        }
        if (method >= 0x1E40 && method < 0x1E60) {         /* COLOR_OCW(i) */
            s_gpu.rc.color_ocw[(method - 0x1E40) / 4] = param;
            break;
        }
        if (method >= 0x0AA0 && method < 0x0AC0) {         /* ALPHA_OCW(i) */
            s_gpu.rc.alpha_ocw[(method - 0x0AA0) / 4] = param;
            break;
        }
        if (method >= 0x0A60 && method < 0x0A80) {         /* FACTOR0(i) */
            s_gpu.rc.factor0[(method - 0x0A60) / 4] = param;
            break;
        }
        if (method >= 0x0A80 && method < 0x0AA0) {         /* FACTOR1(i) */
            s_gpu.rc.factor1[(method - 0x0A80) / 4] = param;
            break;
        }
        /* SET_VIEWPORT_OFFSET / _SCALE. The GPU keeps these in the constant
         * file, at c[59] and c[58] -- exactly where the D3D epilogue reads
         * them -- so they are constants that happen to arrive by method.
         * Dropped, every vertex program put its whole batch at the origin:
         * no 3D anywhere, and a black car on the garage screen. */
        if (method >= 0x0A20 && method < 0x0A30) {
            nv2a_vsh_constant_component(59, (method - 0x0A20) / 4, param);
            break;
        }
        if (method >= 0x0AF0 && method < 0x0B00) {
            nv2a_vsh_constant_component(58, (method - 0x0AF0) / 4, param);
            break;
        }
        if (method >= 0x0B00 && method < 0x0B80) {    /* TRANSFORM_PROGRAM(i) */
            nv2a_vsh_program_word(param);
            break;
        }
        if (method >= 0x0B80 && method < 0x0C00) {    /* TRANSFORM_CONSTANT(i) */
            nv2a_vsh_constant_word(param);
            break;
        }
        if (method >= NV_TEX_FIRST && method <= NV_TEX_LAST)
            record_tex_reg(method, param);
        if (method >= NV097_SET_VERTEX_DATA_ARRAY_OFFSET
                && method < NV097_SET_VERTEX_DATA_ARRAY_OFFSET + NV_VERTEX_ATTRS * 4) {
            /* Resolved here, once, so every consumer -- the rasteriser's
             * attribute reads and the diagnostics alike -- sees the same
             * address. A vertex array offset is a DMA-object offset exactly
             * like a surface offset: physical, and addressable only through
             * the window when it names contiguous memory. */
            s_gpu.attr[(method - NV097_SET_VERTEX_DATA_ARRAY_OFFSET) / 4].offset =
                dma_resolve(param);
        } else if (method >= NV097_SET_VERTEX_DATA_ARRAY_FORMAT
                && method < NV097_SET_VERTEX_DATA_ARRAY_FORMAT + NV_VERTEX_ATTRS * 4) {
            VertexAttr *a = &s_gpu.attr[(method - NV097_SET_VERTEX_DATA_ARRAY_FORMAT) / 4];
            a->type   =  param        & 0x0F;
            a->size   = (param >> 4)  & 0x0F;
            a->stride = (param >> 8)  & 0xFF;
        } else if (!imm_vertex_method(method, param)) {
            note_unhandled(method, param);
        }
        break;
    }
}

/* Find where the title actually wrote its quad.
 *
 * The GPU is pointed at a buffer that stays zero, which says the data went
 * somewhere else -- and the only way to find somewhere else is to look for the
 * data. A screen-space quad for a 640x480 target contains 640.0f and 480.0f as
 * floats, which is a distinctive enough pair to search guest RAM for. Whatever
 * address that turns up is where the title's writes are landing, and the
 * difference from the programmed offset is the bug. */
static void find_quad_vertices(void)
{
    const uint32_t W = 0x44200000u;   /* 640.0f */
    const uint32_t H = 0x43F00000u;   /* 480.0f */
    const uint32_t *ram = (const uint32_t *)xbox_GetMemoryOffset();
    uint32_t i, hits = 0;

    fprintf(stderr, "[GPU] searching guest RAM for 640.0f/480.0f pairs...\n");
    for (i = 0x1000 / 4; i < (0x04000000u / 4) - 8 && hits < 12; i++) {
        if (ram[i] != W && ram[i] != H)
            continue;
        /* Both values within a few words of each other: a lone 640.0f is
         * common, the pair much less so. */
        {
            int has_w = 0, has_h = 0;
            uint32_t k;
            for (k = 0; k < 8; k++) {
                if (ram[i + k] == W) has_w = 1;
                if (ram[i + k] == H) has_h = 1;
            }
            if (!has_w || !has_h)
                continue;
        }
        hits++;
        fprintf(stderr, "  [GPU]   0x%08X:", i * 4);
        {
            uint32_t k;
            for (k = 0; k < 8; k++)
                fprintf(stderr, " %08X", ram[i + k]);
        }
        fprintf(stderr, "\n");
        i += 8;
    }
    if (!hits)
        fprintf(stderr, "  [GPU]   none found -- the quad is not in RAM in"
                        " that form\n");
    fflush(stderr);
}

/* Locate NaN-filled transform matrices in guest RAM.
 *
 * A matrix arriving as NaN says the maths went wrong somewhere upstream, and
 * the only way to find where is to find the matrix and watch who writes it.
 * Three consecutive real-indefinite values is a distinctive enough signature:
 * ordinary data does not contain runs of 0xFFC00000. */
static void find_nan_matrices(void)
{
    const uint32_t NAN_NEG = 0xFFC00000u;
    const uint32_t *ram = (const uint32_t *)xbox_GetMemoryOffset();
    uint32_t i, hits = 0;

    fprintf(stderr, "[GPU] searching guest RAM for NaN matrices...\n");
    for (i = 0x1000 / 4; i < (0x04000000u / 4) - 20 && hits < 10; i++) {
        if (ram[i] != NAN_NEG || ram[i + 1] != NAN_NEG || ram[i + 2] != NAN_NEG)
            continue;
        hits++;
        fprintf(stderr, "  [GPU]   0x%08X:", i * 4);
        {
            uint32_t k;
            for (k = 0; k < 16; k++)
                fprintf(stderr, " %08X", ram[i + k]);
        }
        fprintf(stderr, "\n");
        i += 16;
    }
    if (!hits)
        fprintf(stderr, "  [GPU]   none in RAM -- the NaNs are computed into"
                        " registers, not stored\n");
    fflush(stderr);
}

/* Print guest dwords named by RECOMP_PEEK, as hex and as float.
 *
 * Chasing a value backwards means reading it, and a value that is only wrong
 * for one frame in a thousand cannot be caught by stopping. Both
 * interpretations are printed because the question is usually "is this a
 * pointer or a number", and guessing wrong costs a run. */
static void peek_addresses(void)
{
    const char *spec = getenv("RECOMP_PEEK");
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    char buf[256];
    char *tok, *ctx = NULL;

    if (!spec)
        return;
    strncpy(buf, spec, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (tok = strtok_s(buf, ",", &ctx); tok; tok = strtok_s(NULL, ",", &ctx)) {
        uint32_t va = (uint32_t)strtoul(tok, NULL, 0);
        uint32_t v;
        float f;
        if (va < 0x1000u || va >= 0x04000000u)
            continue;
        v = *(const uint32_t *)(mem + va);
        memcpy(&f, &v, 4);
        fprintf(stderr, "  [PEEK] 0x%08X = %08X  (%g)\n", va, v, f);
    }
    fflush(stderr);
}

/* Walk a pointer chain and print every step.
 *
 * RECOMP_PEEK_CHAIN="0x1315A8,8,0x10,0" starts at that address, and for each
 * offset dereferences the current pointer and adds it. Following a chain by
 * hand costs one run per level; this costs one run for the whole chain, and
 * prints where it goes wrong when a level is null.
 */
static void peek_chain(void)
{
    const char *spec = getenv("RECOMP_PEEK_CHAIN");
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    char buf[256], *tok, *ctx = NULL;
    uint32_t cur = 0;
    int step = 0;

    if (!spec)
        return;
    strncpy(buf, spec, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;

    for (tok = strtok_s(buf, ",", &ctx); tok; tok = strtok_s(NULL, ",", &ctx)) {
        uint32_t off = (uint32_t)strtoul(tok, NULL, 0);
        if (step == 0) {
            cur = off;
            fprintf(stderr, "  [CHAIN] start 0x%08X\n", cur);
        } else {
            if (cur < 0x1000u || cur + 4 >= 0x04000000u) {
                fprintf(stderr, "  [CHAIN] step %d: 0x%08X is not a guest"
                                " pointer -- chain ends\n", step, cur);
                return;
            }
            cur = *(const uint32_t *)(mem + cur) + off;
            fprintf(stderr, "  [CHAIN] step %d: deref +0x%X -> 0x%08X\n",
                    step, off, cur);
        }
        step++;
    }
    if (cur >= 0x1000u && cur + 4 < 0x04000000u)
        fprintf(stderr, "  [CHAIN] final value at 0x%08X = 0x%08X\n",
                cur, *(const uint32_t *)(mem + cur));
    fflush(stderr);
}

void nv2a_pb_exec_report(void)
{
    fprintf(stderr, "[GPU-RES] binds=%llu syncs=%llu reuses=%llu gpu-clears=%llu atlas-reuse=%llu\n",
            s_res_binds, s_res_syncs, s_res_reuses, s_gpu_clears, s_atlas_reuse);
    nv2a_gpu_report();
    fprintf(stderr, "[GPU-DECODE] hits=%llu misses=%llu decoded-texels=%llu\n",
            (unsigned long long)s_decode_hits, (unsigned long long)s_decode_misses,
            (unsigned long long)s_decode_texels);
    for (unsigned i = 0; i < 32; i++)
        if (s_miss[i].misses > 1)
            fprintf(stderr, "[GPU-DECODE] miss %08X fmt %02X %ux%u x%u%s\n", s_miss[i].offset,
                    s_miss[i].color, s_miss[i].width, s_miss[i].height, s_miss[i].misses,
                    s_miss[i].cacheable ? "" : " (too big to cache)");
    memset(s_miss, 0, sizeof s_miss);
    peek_addresses();
    peek_chain();
    if (getenv("RECOMP_FIND_NAN")) {
        /* Every report, not once: the matrix is fine early on and only turns
         * to NaN later, so a single scan at startup finds nothing and says
         * nothing. */
        find_nan_matrices();
    }
    if (getenv("RECOMP_FIND_QUAD")) {
        static int done;
        if (!done) { done = 1; find_quad_vertices(); }
    }
    int i, j;

    {
        /* Frames per second of guest time, from flips between reports. */
        static uint32_t last_flips;
        static unsigned long last_ms;
        unsigned long now = (unsigned long)(clock() * 1000.0 / CLOCKS_PER_SEC);
        if (last_ms && now > last_ms)
            fprintf(stderr, "[GPU] %.2f fps (%u flips)%c",
                    (s_gpu.flips - last_flips) * 1000.0 / (now - last_ms),
                    s_gpu.flips, 10);
        last_flips = s_gpu.flips;
        last_ms = now;
    }
    fprintf(stderr, "[GPU] surface 0x%08X format %08X pitch %u clip %ux%u+%u+%u"
                    " clears %u | %u unhandled methods (%d distinct)\n",
            s_gpu.color_offset, s_gpu.format, s_gpu.pitch, s_gpu.clip_w, s_gpu.clip_h,
            s_gpu.clip_x, s_gpu.clip_y, s_gpu.clears,
            s_gpu.unhandled_total, s_unhandled_count);
    fprintf(stderr, "[GPU] draws %u (%u with coordinates), %u indices;"
                    " x %.1f..%.1f  y %.1f..%.1f\n",
            s_gpu.draws, s_gpu.nonzero_draws, s_gpu.verts,
            s_gpu.min_x, s_gpu.max_x, s_gpu.min_y, s_gpu.max_y);
    /* One picture per report rather than per clear: a title clears hundreds of
     * times a second and nobody wants that many files. */
    if (!s_flip_dumping)
        dump_surface_bmp();

    /* Drawn and skipped separately: "nothing appeared" and "every batch needed
     * a vertex program we do not run" look identical on screen, and only one
     * of them means the rasteriser is broken. */
    fprintf(stderr, "[GPU] brightest pixel written 0x%08X\n", s_gpu.pixel_max);
    fprintf(stderr, "[GPU] %llu pixels written; draw surface 0x%08X"
                    " -> 0x%08X, clear surface 0x%08X -> 0x%08X\n",
            (unsigned long long)s_gpu.pixels, s_gpu.drawn_offset,
            dma_resolve(s_gpu.drawn_offset), s_gpu.color_offset,
            dma_resolve(s_gpu.color_offset));
    fprintf(stderr, "[GPU] rasterised %u triangles; %u batches skipped as not"
                    " screen-space, %u triangles fully off-surface\n",
            s_gpu.tris_drawn, s_gpu.batches_untransformed,
            s_gpu.tris_skipped_offscreen);
    fprintf(stderr, "[GPU] vertex programs: %u batches, %u vertices;"
                    " %u triangles dropped behind the eye\n",
            s_gpu.batches_program, s_gpu.verts_program, s_gpu.tris_behind);
    fprintf(stderr, "[GPU]   of the rest: %u degenerate/NaN, %u off-surface,"
                    " %u drawn; pixels %llu written, %llu depth-failed;"
                    " x %.0f..%.0f y %.0f..%.0f z %g..%g\n",
            s_gpu.xf_degenerate, s_gpu.xf_offscreen, s_gpu.xf_drawn,
            (unsigned long long)s_gpu.xf_pixels,
            (unsigned long long)s_gpu.xf_depth_fail,
            s_gpu.xf_min[0], s_gpu.xf_max[0], s_gpu.xf_min[1], s_gpu.xf_max[1],
            s_gpu.xf_min[2], s_gpu.xf_max[2]);
    {
        fprintf(stderr, "[GPU]   blend pairs (src/dst):");
        for (j = 0; j < s_gpu.blend_npairs; j++)
            fprintf(stderr, " %X/%X", s_gpu.blend_pairs[j] >> 16,
                    s_gpu.blend_pairs[j] & 0xFFFF);
        fputc(10, stderr);
        fprintf(stderr, "[GPU]   program attribute formats (slot:type/size):");
        for (j = 0; j < s_gpu.xf_nfmt; j++)
            fprintf(stderr, " v%u:%u/%u", s_gpu.xf_fmt[j] >> 8,
                    s_gpu.xf_fmt[j] & 15, (s_gpu.xf_fmt[j] >> 4) & 15);
        fprintf(stderr, "\n");
    }

    /* And of the batches that did rasterise, how many sampled anything. A menu
     * that draws its background from one texture and its text from another
     * shows both as flat colour if either half is missing, so the split is
     * what says which half. */
    fprintf(stderr, "[GPU] batches: %u textured, %u with no texcoords,"
                    " %u with texcoords but no usable stage\n",
            s_gpu.batches_textured, s_gpu.batches_no_uv, s_gpu.batches_no_tex);
    for (i = 0; i < s_tex_use_count; i++)
        fprintf(stderr, "  [TEXUSE] 0x%08X %ux%u fmt 0x%02X%s: %u batches\n",
                s_tex_use[i].offset, s_tex_use[i].width, s_tex_use[i].height,
                s_tex_use[i].color,
                d3d8_format_dxt_block_bytes(s_tex_use[i].color) ? " dxt"
                    : d3d8_format_is_swizzled(s_tex_use[i].color) ? " swz" : " lin",
                s_tex_use[i].batches);

    if (getenv("RECOMP_TEX_STATE")) {
        uint32_t k;
        for (k = 0; k < sizeof s_tex_set / sizeof s_tex_set[0]; k++)
            if (s_tex_set[k])
                fprintf(stderr, "  [TEX] 0x%04X = 0x%08X\n",
                        (unsigned)(NV_TEX_FIRST + k * 4), s_tex_reg[k]);
    }

    /* Top ten by frequency: selection sort over a small table, once every few
     * seconds, is not worth a better algorithm.
     *
     * RECOMP_PB_UNHANDLED_ALL lists every one instead. Ten is the right
     * default -- the tail is a long list of state registers nobody needs to
     * read -- but when a title stops and the question is which method it
     * stopped on, the answer is as likely to be the one seen twice as the
     * one seen a thousand times, and ten hides it. */
    {
        int shown = getenv("RECOMP_PB_UNHANDLED_ALL") ? s_unhandled_count : 10;
    for (i = 0; i < shown && i < s_unhandled_count; i++) {
        int best = i;
        for (j = i + 1; j < s_unhandled_count; j++)
            if (s_unhandled[j].count > s_unhandled[best].count)
                best = j;
        if (best != i) {
            PbUnhandled t = s_unhandled[i];
            s_unhandled[i] = s_unhandled[best];
            s_unhandled[best] = t;
        }
        {
            /* The value as well as the count. A method nobody decoded is
             * a guess until you see what it carried: screen coordinates,
             * a 0..1 texcoord and a packed colour are told apart at a
             * glance, and that is what says which vertex encoding a title
             * is using. */
            union { uint32_t u; float f; } v;
            v.u = s_unhandled[i].last_param;
            fprintf(stderr, "  [GPU]   0x%04X x%-8u last=0x%08X (%.4f)\n",
                    s_unhandled[i].method, s_unhandled[i].count,
                    v.u, v.f);
        }
    }
    }    fflush(stderr);
}
