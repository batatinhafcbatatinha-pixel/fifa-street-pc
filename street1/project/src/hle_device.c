/*
 * Direct3D_CreateDevice without the NV2A.
 *
 * CreateDevice (0x25D080 -> 0x255BF0) is CPU work -- the device structure,
 * the pushbuffer ring, fences, back buffers, the display mode through the
 * kernel -- plus the miniport's hardware bring-up: register writes that
 * program the GPU, reads of its configuration (clock PLLs, memory size, PCI
 * id) and the instance-memory objects (DMA contexts, graphics classes, the
 * FIFO channel) the pushbuffer refers to. With the native renderer and the
 * native KickOff nothing executes that pushbuffer, so only what the
 * bring-up leaves in CPU-side state matters. The functions below are the
 * bring-up's hardware layer, routed by hle/sync_functions.json:
 *
 *   0x259D01  register base, interrupt enables      -> base only
 *   0x259D2C  PCI id, revision, memory size         -> the values, as facts
 *   0x259D6D  clocks, instance memory layout, ...   -> the values, as facts
 *   0x259DF0  interrupt/PGRAPH/PFIFO initialisation -> its CPU fields
 *   0x259EE2  DMA context object                    -> instance allocation
 *   0x259FB1  graphics class object                 -> instance allocation
 *   0x259C6A  FIFO channel 0                        -> allocation; the "user
 *                                                      registers" (PUT/GET)
 *                                                      become a block of
 *                                                      ordinary guest memory
 *   0x259CCD  hash-table entry                      -> nothing
 *   0x253AD0  "GPU busy?" poll                      -> never busy
 *   0x25A356  SetTile: PFB/PGRAPH tile registers    -> nothing (the CPU sees
 *   0x25A544  SetTile(NULL): clear them                linear memory either
 *                                                      way; the library's tile
 *                                                      table at device+0x1AC0
 *                                                      is kept by its caller)
 *
 * Default with RECOMP_HLE_NOGPU=1. RECOMP_HLE_LIBDEVICE=1 runs the original
 * bodies (A/B); RECOMP_HLE_DEVLOG=1
 * then logs every CPU-side dword each of them changes in the device and
 * miniport (0x261240..0x2636E0), which is where the facts below come from.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "recomp/gen/recomp_funcs.h"

#define DEV_BASE   0x261240u
#define DEV_END    0x2636E0u
#define MINIPORT   0x262E68u
#define MP(i)      MEM32(MINIPORT + 4u * (i))

static int s_mode = -1;     /* 0 original, 1 native */
static int s_log;

static int dev_mode(void)
{
    if (s_mode < 0) {
        const char *lib = getenv("RECOMP_HLE_LIBDEVICE"), *nat = getenv("RECOMP_HLE_NATIVEDEVICE");
        const char *lg = getenv("RECOMP_HLE_DEVLOG");
        const char *nogpu = getenv("RECOMP_HLE_NOGPU");
        /* Native with the native KickOff (nothing else would consume the
         * hardware it programs); RECOMP_HLE_NATIVEDEVICE=0/1 overrides. */
        s_mode = (lib && *lib == '1') ? 0 : nat ? (*nat == '1') : (nogpu && *nogpu == '1');
        s_log = lg && *lg == '1';
        fprintf(stderr, "[HLE-DEV] GPU bring-up in CreateDevice: %s\n",
                s_mode ? "native (no NV2A register or instance memory access)" : "original library code");
    }
    return s_mode;
}

int hle_device_native(void)
{
    return dev_mode();
}

/* Original body, logging the CPU-side state it changes. */
static void original(const char *name, void (*gen)(void))
{
    static uint8_t snaps[4][DEV_END - DEV_BASE];
    static int depth;
    uint8_t *before;
    uint32_t va;
    if (!s_log || depth >= 4) {
        gen();
        return;
    }
    before = snaps[depth++];
    memcpy(before, (const void *)XBOX_PTR(DEV_BASE), DEV_END - DEV_BASE);
    gen();
    depth--;
    for (va = DEV_BASE; va < DEV_END; va += 4) {
        uint32_t old, now = MEM32(va);
        memcpy(&old, before + (va - DEV_BASE), 4);
        if (va >= DEV_BASE + 0x1C28 + 0x1DC && va < DEV_BASE + 0x1C28 + 0x4DC && s_log < 2)
            continue;                    /* the gamma ramps: summarised */
        if (old != now)
            fprintf(stderr, "[HLE-DEV] %s: MP%+05X (dev+%04X) %08X -> %08X\n", name,
                    (int)(va - MINIPORT), va - DEV_BASE, old, now);
    }
    fprintf(stderr, "[HLE-DEV] %s: eax=%08X\n", name, g_eax);
}

static void ret_pop(uint32_t args, uint32_t eax)
{
    g_eax = eax;
    g_esp += 4 + 4 * args;
}

static uint32_t arg(int i)
{
    return MEM32(g_esp + 4 + 4 * i);
}

/* ------------------------------------------------- bring-up (miniport) */

void sub_00259D01(void)
{
    if (!dev_mode()) { original("RegisterBase", sub_00259D01_gen); return; }
    MEM32(g_ecx) = 0xFD000000u;          /* kept as a value; nothing dereferences it */
    ret_pop(0, 1);
}

/* PCI id/revision (MP 0x2A, 0x30) and memory size (0x2B) read as 0 from the
 * emulated registers and stay 0; only the constant at 0x2F is set. */
void sub_00259D2C(void)
{
    if (!dev_mode()) { original("Config", sub_00259D2C_gen); return; }
    MP(0x2F) = 0xFE502Au;
    ret_pop(0, 1);
}

/* Clock PLLs, instance memory layout, gamma ramps. Values as the original
 * computes them on this hardware model (RECOMP_HLE_DEVLOG, run
 * 20261012-192206-664); the instance memory itself is still claimed from the
 * kernel (MmClaimGpuInstanceMemory, import thunk 0x2772A0), as the original
 * does, so the memory map is the same. */
void sub_00259D6D(void)
{
    uint32_t pad, top, i;
    if (!dev_mode()) { original("Clocks+Instance", sub_00259D6D_gen); return; }
    {
        extern uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args);
        uint32_t a[2];
        g_esp -= 4;
        MEM32(g_esp) = 0;
        a[0] = 0x5000;
        a[1] = g_esp;
        top = hle_guest_call(MEM32(0x2772A0u), 2, a);
        pad = MEM32(g_esp);
        g_esp += 4;
    }
    MP(0x04) = top - 0x5000;              /* instance memory base (VA) */
    MP(0x2C) = 3;
    MP(0x2D) = 1;
    MP(0x36) = 1;                         /* MPLL/VPLL/NVPLL dividers: the */
    MP(0x3A) = 1;                         /* emulated PLL registers read 0 */
    MP(0x3E) = 1;
    MP(0x47) = 0x01111111u;
    MP(0x48) = 0xFF;
    MP(0x49) = 0x800000u;
    MP(0x4C) = 0x700000u + pad;           /* RAMIN layout */
    MP(0x4A) = MP(0x4C) + 0x1000;
    MP(0x4B) = MP(0x4A) + 0x80;
    MP(0x4D) = 2;
    MP(0x50) = 0x10A;
    MP(0x58) = 0x112;                     /* next free instance block */
    for (i = 0; i < 0x300; i++)           /* identity gamma ramps R, G, B */
        MEM8(MINIPORT + 0x1DC + i) = (uint8_t)i;
    ret_pop(0, 1);
}

/* PGRAPH/PFIFO initialisation: the engine defaults it keeps in the miniport. */
void sub_00259DF0(void)
{
    if (!dev_mode()) { original("EngineInit", sub_00259DF0_gen); return; }
    MP(0x1FC) = 0x00118700u;
    MP(0x1FD) = 0x0028C3FFu;
    MP(0x1FE) = 0xF3DE0479u;
    MP(0x200) = 4;
    MP(0x201) = 0x45EAD10Eu;
    MP(0x203) = 0x78;
    MP(0x204) = 0x40;
    MP(0x208) = 0;
    MP(0x209) = 0;
    MP(0x210) = 0xFFFFFFFFu;
    MP(0x211) = 0xFFFFFFFFu;
    ret_pop(0, 1);
}

/* ------------------------------------------------- instance objects */

/* DMA context (class, target, base, limit, out object). */
void sub_00259EE2(void)
{
    uint32_t mp = g_ecx, out = arg(4), inst;
    if (!dev_mode()) { original("DmaObject", sub_00259EE2_gen); return; }
    inst = MEM32(mp + 0x160);
    MEM32(mp + 0x160) = inst + 1;
    MEM32(out + 0) = arg(0);
    MEM32(out + 4) = 0;
    MEM32(out + 8) = arg(1);
    MEM32(out + 12) = inst;
    ret_pop(5, 1);
}

/* Graphics object (handle, class, out object). Class 0x97 (Kelvin) takes
 * 0x330 bytes of instance memory and is the channel's graphics context. */
void sub_00259FB1(void)
{
    uint32_t mp = g_ecx, cls = arg(1), out = arg(2), inst;
    if (!dev_mode()) { original("GrObject", sub_00259FB1_gen); return; }
    inst = MEM32(mp + 0x160);
    MEM32(mp + 0x160) = inst + (cls == 0x97 ? 0x33u : 0x1u);
    if (cls == 0x97)
        MEM32(mp + 0x144 + 4 * MEM32(mp + 0x100)) = inst;
    MEM32(out + 0) = arg(0);
    MEM32(out + 4) = 0x10000;            /* 16-bit field at +6 = 1 */
    MEM32(out + 8) = cls;
    MEM32(out + 12) = inst;
    ret_pop(3, 1);
}

/* FIFO channel 0. The out pointer is where D3D finds the channel's user
 * registers (+0x40 PUT, +0x44 GET): plain guest memory now, read and written
 * only by the native KickOff and the library's own GET reads. */
void sub_00259C6A(void)
{
    uint32_t mp = g_ecx, out = arg(4), ch, block;
    if (!dev_mode()) { original("Channel", sub_00259C6A_gen); return; }
    MP(0x40) = 0;
    MP(0x41) = 0;
    MP(0x44) = 0;
    MP(0x46) = 0;
    MP(0x43) = MP(0x58);
    MP(0x58) = MP(0x58) + 0x37F;
    ch = MP(0x40);
    MP(0x4E + ch) = MP(0x43);
    MP(0x41) |= 1u << ch;
    MP(0x42) |= 1u << ch;
    {
        extern uint32_t hle_guest_call(uint32_t va, int n, const uint32_t *args);
        uint32_t a[2] = { 0x100, 0xAC800000u };   /* the allocator CreateDevice uses */
        block = hle_guest_call(0x1609A7u, 2, a);    /* XMemAlloc-style contiguous block */
        if (block)
            memset((void *)XBOX_PTR(block), 0, 0x100);
    }
    MEM32(out) = block;
    fprintf(stderr, "[HLE-DEV] FIFO channel %u user registers at %08X (guest memory)\n", ch, block);
    (void)mp;
    ret_pop(5, 1);
}

void sub_00259CCD(void)
{
    if (!dev_mode()) { original("HashEntry", sub_00259CCD_gen); return; }
    ret_pop(1, 1);
}

void sub_00253AD0(void)
{
    if (!dev_mode()) { sub_00253AD0_gen(); return; }
    ret_pop(0, 0);
}

void sub_0025A356(void)
{
    if (!dev_mode()) { sub_0025A356_gen(); return; }
    ret_pop(7, 1);
}

void sub_0025A544(void)
{
    if (!dev_mode()) { sub_0025A544_gen(); return; }
    ret_pop(2, 1);
}
