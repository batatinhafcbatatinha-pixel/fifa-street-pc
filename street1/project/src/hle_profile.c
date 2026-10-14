/*
 * D3D8 HLE survey profiler (build with -DRECOMP_CALL_PROFILE).
 *
 * Every direct call reports its target here. We count calls into the XDK D3D8
 * library (section D3D, 0x250020..0x263A40) per frame and decode the NV2A
 * command blobs the game submits outside the API -- D3DDevice_RunPushBuffer and
 * the precompiled-blob copier sub_0019E400 -- to learn which methods an HLE
 * renderer must translate. Frames are delimited by D3DDevice_Swap.
 *
 * Output: "[HLE-PROF]" lines on stderr every RECOMP_HLE_PROF_FRAMES frames
 * (default 120) plus a cumulative report.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"

#define D3D_LO 0x00250020u
#define D3D_HI 0x00263A40u
#define VA_SWAP             0x0025DB90u
#define VA_RUN_PUSHBUFFER   0x0025B070u
#define VA_BLOB_COPY        0x0019E400u
#define VA_BEGIN_PUSH       0x00251B80u
#define VA_DRAW_VERTICES    0x0025CA70u
#define VA_DRAW_INDEXED     0x0025CB10u
#define VA_BEGIN            0x0025CE60u

static uint64_t s_api_total[D3D_HI - D3D_LO];   /* sparse by entry offset */
static uint64_t s_method_blob[0x2000 / 4];      /* NV097 method histogram, blobs */
static uint64_t s_method_run[0x2000 / 4];       /* same, RunPushBuffer */
static uint64_t s_frames, s_report_every;
static uint64_t s_blob_calls, s_blob_dwords, s_blob_draws, s_blob_bad;
static uint64_t s_run_calls, s_run_dwords, s_run_draws, s_run_bad;
static uint64_t s_f_blob_draws, s_f_run_draws, s_f_api_draws, s_f_push;

static int readable(uint32_t va, uint32_t len)
{
    /* Guest RAM (low mirror) and the contiguous window are mapped; anything else is
     * rejected so a misread pointer cannot fault the survey. */
    if (va >= 0x00010000u && va + len < 0x04000000u) return 1;
    if (va >= 0x80000000u && va + len < 0x84000000u) return 1;
    return 0;
}

/* Walk an NV2A command stream; count methods and BEGIN_END(non-zero) draws. */
static void walk(uint32_t va, uint32_t dwords, uint64_t *hist, uint64_t *draws, uint64_t *bad)
{
    uint32_t i = 0;
    while (i < dwords) {
        uint32_t h = MEM32(va + i * 4);
        if ((h & 0xE0030003u) == 0 || (h & 0xE0030003u) == 0x40000000u) {
            uint32_t method = h & 0x1FFCu;
            uint32_t count = (h >> 18) & 0x7FFu;
            uint32_t sub = (h >> 13) & 7u;
            int incr = (h & 0x40000000u) == 0;
            for (uint32_t k = 0; k < count && i + 1 + k < dwords; k++) {
                uint32_t m = incr ? method + 4 * k : method;
                if (sub == 0 && m < 0x2000u) {
                    hist[m / 4]++;
                    if (m == 0x17FCu && MEM32(va + (i + 1 + k) * 4) != 0) (*draws)++;
                }
            }
            i += 1 + count;
        } else {
            (*bad)++;   /* jump/call/return inside a blob: note and stop */
            return;
        }
    }
}

static void report(const char *tag)
{
    fprintf(stderr, "[HLE-PROF] %s frames=%llu blobs=%llu(%llu dw, %llu draws, %llu bad) "
            "runpb=%llu(%llu dw, %llu draws, %llu bad)\n", tag,
            (unsigned long long)s_frames, (unsigned long long)s_blob_calls,
            (unsigned long long)s_blob_dwords, (unsigned long long)s_blob_draws,
            (unsigned long long)s_blob_bad, (unsigned long long)s_run_calls,
            (unsigned long long)s_run_dwords, (unsigned long long)s_run_draws,
            (unsigned long long)s_run_bad);
    for (uint32_t o = 0; o < D3D_HI - D3D_LO; o++)
        if (s_api_total[o])
            fprintf(stderr, "[HLE-PROF] api %08X %llu\n", D3D_LO + o,
                    (unsigned long long)s_api_total[o]);
    for (uint32_t m = 0; m < 0x2000 / 4; m++)
        if (s_method_blob[m] || s_method_run[m])
            fprintf(stderr, "[HLE-PROF] method %04X blob=%llu runpb=%llu\n", m * 4,
                    (unsigned long long)s_method_blob[m], (unsigned long long)s_method_run[m]);
    fflush(stderr);
}

void recomp_call_profile_hit(uint32_t va)
{
    if (va < D3D_LO || va >= D3D_HI) {
        if (va == VA_BLOB_COPY) {
            /* sub_0019E400(int *blob, x, y): blob[0] = dword count, commands at blob+8 */
            uint32_t p = MEM32(g_esp + 4);
            if (readable(p, 8)) {
                uint32_t n = MEM32(p);
                if (n && n < 0x100000u && readable(p + 8, n * 4)) {
                    uint64_t before = s_blob_draws;
                    s_blob_calls++; s_blob_dwords += n;
                    walk(p + 8, n, s_method_blob, &s_blob_draws, &s_blob_bad);
                    s_f_blob_draws += s_blob_draws - before;
                } else s_blob_bad++;
            } else s_blob_bad++;
        }
        return;
    }
    s_api_total[va - D3D_LO]++;
    if (va == VA_DRAW_VERTICES || va == VA_DRAW_INDEXED || va == VA_BEGIN) s_f_api_draws++;
    if (va == VA_BEGIN_PUSH) s_f_push++;
    if (va == VA_RUN_PUSHBUFFER) {
        /* RunPushBuffer(D3DPushBuffer *pb, D3DFixup *fixup): {Common, Data, Lock, Size, AllocationSize} */
        uint32_t pb = MEM32(g_esp + 4);
        if (readable(pb, 20)) {
            uint32_t data = MEM32(pb + 4), size = MEM32(pb + 12);
            if (data && data < 0x04000000u) data |= 0x80000000u;
            if (size && size < 0x400000u && readable(data, size)) {
                uint64_t before = s_run_draws;
                s_run_calls++; s_run_dwords += size / 4;
                walk(data, size / 4, s_method_run, &s_run_draws, &s_run_bad);
                s_f_run_draws += s_run_draws - before;
            } else s_run_bad++;
        } else s_run_bad++;
    }
    if (va == VA_SWAP) {
        s_frames++;
        if (!s_report_every) {
            const char *e = getenv("RECOMP_HLE_PROF_FRAMES");
            s_report_every = e ? strtoull(e, NULL, 10) : 120;
            if (!s_report_every) s_report_every = 120;
        }
        if (s_frames % 30 == 0)
            fprintf(stderr, "[HLE-PROF] frame %llu draws: api=%llu blob=%llu runpb=%llu beginpush=%llu\n",
                    (unsigned long long)s_frames, (unsigned long long)s_f_api_draws,
                    (unsigned long long)s_f_blob_draws, (unsigned long long)s_f_run_draws,
                    (unsigned long long)s_f_push);
        s_f_api_draws = s_f_blob_draws = s_f_run_draws = s_f_push = 0;
        if (s_frames % s_report_every == 0) report("cumulative");
    }
}
