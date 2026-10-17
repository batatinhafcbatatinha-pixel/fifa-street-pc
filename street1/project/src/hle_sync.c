/*
 * Native GPU synchronisation for the D3D8 HLE renderer.
 *
 * With the HLE backend drawing every call as it is made (hle_d3d11.c,
 * hle_draw.c), the pushbuffer the original library still writes has no
 * rendering left in it for the NV2A; what remains that the CPU can observe is
 * synchronisation:
 *   - SEMAPHORE_RELEASE (0x1D70): the fence "time" the library waits on
 *     (BlockOnTime / BlockOnFence / IsBusy compare it with *device[0xC]);
 *   - NO_OPERATION (0x100) with a nonzero parameter: a software method the
 *     library's graphics ISR services (sub_00259690: 1 = flip, 5 = BlockOnTime
 *     event, 6/7 = InsertCallback, 8/9 = register writes, ...).
 *
 * sub_00251BB0 is CDevice_KickOff (thiscall, ecx = device), the one place the
 * library hands commands to the hardware (PFB flush bit, then PUT into the
 * FIFO's user registers). Routed here by hle/sync_functions.json.
 *
 * RECOMP_HLE_SYNC_CENSUS=1  read each kicked range and count what it holds,
 *                           then run the original body (NV2A path unchanged).
 * RECOMP_HLE_NOGPU=1        native KickOff: nothing reaches the NV2A. The kicked
 *                           range is read once, in order: semaphore releases
 *                           complete their fence at once (the backend has
 *                           already drawn everything before them) and software
 *                           methods go straight to the library's handler. GET
 *                           is set to PUT, as the library's own GPU-less path
 *                           (flag 0x2000 in the same function) does.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "recomp/gen/recomp_funcs.h"

#define D3D_DEVICE_PTR   0x261238u
#define MINIPORT         0x262E68u   /* g_Miniport: [0] = NV2A register base */
#define DEV_PUT          0x000u
#define DEV_FLAGS        0x008u
#define DEV_SEMAPHORE    0x030u      /* -> fence time written by the GPU */
#define DEV_RECORD_PUT   0x770u
#define DEV_FIFO         0x1C20u     /* -> FIFO user registers: +0x40 PUT, +0x44 GET */
#define GPULESS_FLAG     0x260D90u

static int s_mode = -1;              /* 0 off, 1 census, 2 native */
static int sync_mode(void);
static uint32_t s_scan;              /* VA of the next unread command */

#define MAX_NOTIFY 64
static uint32_t s_notify[MAX_NOTIFY];
static int s_notify_count;
static int s_in_kickoff;

static uint64_t s_census_kicks, s_census_words, s_census_sema, s_census_jumps, s_census_calls;
static uint64_t s_census_notify[32];
static uint64_t s_census_methods[0x800];

/* Last commands seen, for context around rare software methods. */
static uint32_t s_hist_m[8], s_hist_v[8];
static unsigned s_hist_n;

/* The library's vblank service (CMiniport, thiscall, no arguments): counts the
 * vblank, completes a due flip, signals the vblank event and calls the
 * title's VBlank callback. With no NV2A behind it, the runtime's timer calls
 * this instead of faking PCRTC/PMC status and raising the graphics ISR. */
extern void (*volatile xbox_native_vblank)(void);
extern volatile int xbox_nv2a_native;

void hle_api_vblank(void);
int hle_api_native_vblank(void);

static void native_vblank(void)
{
    if (hle_api_native_vblank()) {
        hle_api_vblank();
        return;
    }
    g_esp -= 4; MEM32(g_esp) = 0;
    g_ecx = MINIPORT;
    sub_00259390();
}

/* Direct3D_CreateDevice entry: decide the mode before the bring-up, so the
 * runtime stops polling NV2A registers from the start. */
int hle_device_native(void);
static int sync_mode(void);

void hle_sync_early(void)
{
    if (hle_device_native())
        sync_mode();
}

uint32_t hle_sync_scan_pos(void)
{
    return s_scan;
}

void hle_sync_rebase(uint32_t pos)
{
    if (sync_mode() != 2) return;
    s_scan = pos;
    uint32_t dev = MEM32(D3D_DEVICE_PTR), fifo = MEM32(dev + DEV_FIFO);
    if (fifo) MEM32(fifo + 0x44) = pos & 0x0FFFFFFFu;
    MEM32(dev + 0x5C) = pos;
    MEM32(dev + 0x60) = MEM32(dev + 0x40);
}

static int sync_mode(void)
{
    if (s_mode < 0) {
        const char *n = getenv("RECOMP_HLE_NOGPU"), *c = getenv("RECOMP_HLE_SYNC_CENSUS");
        const char *vb = getenv("RECOMP_HLE_NATIVE_VBLANK");
        s_mode = n && *n == '1' ? 2 : c && *c == '1' ? 1 : 0;
        if (s_mode == 2) {
            xbox_nv2a_native = 1;
            if (!(vb && *vb == '0'))
                xbox_native_vblank = native_vblank;
        }
        if (s_mode)
            fprintf(stderr, "[HLE-SYNC] %s%s; miniport %08X regs %08X\n",
                    s_mode == 2 ? "native KickOff (NV2A gets no commands)" : "census",
                    xbox_native_vblank ? ", native vblank" : "",
                    MINIPORT, MEM32(MINIPORT));
    }
    return s_mode;
}

static uint64_t s_notify_original;    /* software methods left to 0x259690 */
static uint64_t s_foreign_kicks;      /* native mode: kicks with commands nobody native wrote */

void hle_sync_report(void)
{
    if (s_mode != 1 && s_mode != 2)
        return;
    fprintf(stderr, "[HLE-SYNC] kicks %llu words %llu semaphores %llu jumps %llu calls %llu\n",
            (unsigned long long)s_census_kicks, (unsigned long long)s_census_words,
            (unsigned long long)s_census_sema, (unsigned long long)s_census_jumps,
            (unsigned long long)s_census_calls);
    if (s_mode == 2) {
        extern long xbox_HardwareAccessCount(void);
        fprintf(stderr, "[HLE-SYNC]   kicks carrying non-native commands: %llu\n",
                (unsigned long long)s_foreign_kicks);
        fprintf(stderr, "[NO-HW] hardware aperture accesses so far: %ld\n", xbox_HardwareAccessCount());
    }
    for (int i = 0; i < 32; i++)
        if (s_census_notify[i])
            fprintf(stderr, "[HLE-SYNC]   software method %2d x%llu\n", i,
                    (unsigned long long)s_census_notify[i]);
    if (s_mode == 1)
        for (int i = 0; i < 0x800; i++)
            if (s_census_methods[i] && (i < 0x60 || (i >= 0x5B0 && i < 0x5F8) || i >= 0x740))
                fprintf(stderr, "[HLE-SYNC]   method %04X x%llu\n", i * 4,
                        (unsigned long long)s_census_methods[i]);
    fflush(stderr);
}

static void on_method(uint32_t dev, uint32_t m, uint32_t v)
{
    s_hist_m[s_hist_n & 7] = m;
    s_hist_v[s_hist_n & 7] = v;
    s_hist_n++;
    if (s_mode == 1)
        s_census_methods[(m >> 2) & 0x7FF]++;
    if (m == 0x1D70) {
        s_census_sema++;
        if (s_mode == 2)
            MEM32(MEM32(dev + DEV_SEMAPHORE)) = v;
    } else if (m == 0x100 && v) {
        unsigned kind = v & 0x1F;
        s_census_notify[kind]++;
        if (kind == 9 || (kind != 1 && kind != 5 && kind != 8 && s_census_notify[kind] <= 4)) {
            static unsigned shown;
            if (shown++ < 16) {
                fprintf(stderr, "[HLE-SYNC] software method %08X (kind %u) after:", v, kind);
                for (unsigned i = 8; i > 1; i--) {
                    unsigned k = (s_hist_n - i) & 7;
                    fprintf(stderr, " %04X=%08X", s_hist_m[k], s_hist_v[k]);
                }
                fprintf(stderr, "\n");
            }
        }
        if (s_mode == 2 && s_notify_count < MAX_NOTIFY)
            s_notify[s_notify_count++] = v;
    }
}

/* Read the commands in [s_scan, put). The ring's wrap is a JUMP the library
 * writes at the old end; RunPushBuffer would be a CALL (this title has none). */
static void scan_to(uint32_t dev, uint32_t put)
{
    uint32_t va = s_scan, budget = 0x400000u;
    if (!va) {
        s_scan = put;
        return;
    }
    while (va != put && budget) {
        uint32_t w = MEM32(va), count, method, i;
        if ((w & 3u) == 1u || (w & 0xE0000003u) == 0x20000000u) {
            uint32_t target = (w & 3u) == 1u ? (w & 0x0FFFFFFCu) : (w & 0x1FFFFFFCu);
            s_census_jumps++;
            va = 0x80000000u | (target & 0x0FFFFFFFu);
            budget--;
            continue;
        }
        if ((w & 3u) == 2u || (w & 0xFFFF0003u) == 0x00020000u) {
            s_census_calls++;
            va += 4;
            budget--;
            continue;
        }
        count = (w >> 18) & 0x7FFu;
        method = w & 0x1FFCu;
        for (i = 0; i < count; i++)
            on_method(dev, (w & 0xE0000000u) == 0x40000000u ? method : method + 4 * i,
                      MEM32(va + 4 + 4 * i));
        va += 4 + 4 * count;
        s_census_words += 1 + count;
        budget -= budget > count ? count + 1 : budget;
    }
    if (!budget) {
        fprintf(stderr, "[HLE-SYNC] lost track of the pushbuffer at %08X (put %08X); resyncing\n", va, put);
        va = put;
    }
    s_scan = va;
}

/* The graphics ISR's software-method handler, called directly. thiscall,
 * one stack argument, ret 4. */
static void call_notify_handler(uint32_t v)
{
    uint32_t ebx_ = g_ebx, esi_ = g_esi, edi_ = g_edi, ebp_ = g_ebp;
    g_esp -= 4; MEM32(g_esp) = v;
    g_esp -= 4; MEM32(g_esp) = 0;
    g_ecx = MINIPORT;
    sub_00259690();
    g_ebx = ebx_; g_esi = esi_; g_edi = edi_; g_ebp = ebp_;
}

/* Every KickOff, whatever the mode: hle_d3d8.c only rewinds a call's
 * pushbuffer writes if none happened during it (a ring wrap kicks off). */
volatile uint32_t hle_sync_kick_count;
int hle_api_software_method(uint32_t v);

static void kickoff(void);
int guest_cpu_native_enter(void);
void guest_cpu_native_leave(int token);

void sub_00251BB0(void)
{
    int smp = guest_cpu_native_enter();
    kickoff();
    guest_cpu_native_leave(smp);
}

static void kickoff(void)
{
    uint32_t dev = g_ecx, flags, put;
    int mode = sync_mode();

    hle_sync_kick_count++;

    if (!mode || s_in_kickoff) {
        sub_00251BB0_gen();
        return;
    }
    flags = MEM32(dev + DEV_FLAGS);
    if (flags & 0x2000) {                 /* the library's own GPU-less path */
        uint32_t fifo = 0x260D40u, p = MEM32(dev + DEV_PUT) & 0x0FFFFFFFu;
        if (mode != 2) {
            sub_00251BB0_gen();
            return;
        }
        /* 0x251C26 in C: a FIFO block in the library's data, PUT = GET, and
         * every fence up to the current time complete. */
        MEM32(dev + DEV_FIFO) = fifo;
        MEM32(fifo + 0x40) = p;
        MEM32(fifo + 0x44) = p;
        MEM32(MEM32(dev + DEV_SEMAPHORE)) = MEM32(dev + 0x2C) - 2;
        MEM32(dev + 0x1DE4) = MEM32(dev + 0x2478);
        s_scan = MEM32(dev + DEV_PUT);
        g_esp += 4;                       /* ret */
        return;
    }
    put = (flags & 4) ? MEM32(dev + DEV_RECORD_PUT) : MEM32(dev + DEV_PUT);
    s_census_kicks++;
    if (mode == 1) {
        s_in_kickoff = 1;
        s_notify_count = 0;
        scan_to(dev, put);
        s_in_kickoff = 0;
        sub_00251BB0_gen();
        return;
    }
    /* Native: the backend has already drawn every call and the native API
     * completed its fences and queued its flips itself, so nothing is read
     * back from the ring. Bytes between the last kick and PUT can only come
     * from library code that is not native; they are counted, not run. */
    if (s_scan && s_scan != put) {
        s_foreign_kicks++;
        if (s_foreign_kicks <= 8)
            fprintf(stderr, "[HLE-SYNC] commands from non-native code in %08X..%08X (dropped)\n",
                    s_scan, put);
    }
    s_scan = put;
    MEM32(MEM32(dev + DEV_FIFO) + 0x44) = put & 0x0FFFFFFFu;     /* GET = PUT */
    if (MEM32(GPULESS_FLAG))
        MEM32(dev + DEV_FLAGS) |= 0x2000;
    g_esp += 4;                           /* ret */
}
