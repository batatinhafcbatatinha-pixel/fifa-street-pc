/*
 * D3D8 HLE entry layer.
 *
 * street1/hle/apply-hle-wrappers.py renames the generated XDK D3D8 library
 * functions listed in hle_d3d8_table.h to sub_X_gen; this file defines sub_X.
 * Every call from the game -- direct, dispatch table or indirect -- lands here
 * first. Stage 1 ("shadow"): observe and count, then run the original library
 * body so the existing NV2A path keeps rendering. Native implementations will
 * replace the pass-through function by function.
 *
 * Stage 2: hle_d3d11.c renders the same frame natively in a shadow window
 * (RECOMP_HLE_SHADOW=1); hle_native_call() feeds it the calls it mirrors.
 *
 * RECOMP_HLE_STATS=<frames> prints per-API call counts every N swaps.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "recomp/gen/recomp_funcs.h"
#include "hle_d3d11.h"

void hle_sync_report(void);
void hle_api_report(void);
void hle_nv2a_watch_start(void);
void hle_nv2a_watch_report(void);
int hle_api_call(int id, const uint32_t *a, void (*gen)(void));
extern volatile uint32_t hle_sync_kick_count;

/* Pushbuffer rewind (primary renderer, hle_native_owns_state).
 *
 * The backend mirrors every routed call itself, so what a library body pushes
 * for the NV2A is never needed -- except synchronisation, which hle_sync.c
 * reads at KickOff. After a game call to any other API, the pusher's put is
 * set back to where the call found it: the library's CPU-side state is exactly
 * what its own code left, and its commands are dropped where they were written
 * (the same few KB, cache-resident) instead of filling the ring, being kicked
 * off and scanned. A call that kicked off on the way (ring wrap) is left alone.
 * The game's own BeginPush..EndPush batch is decoded by the backend at EndPush
 * and rewound to where BeginPush started (its deferred-state flush included).
 * RECOMP_HLE_LIBSTATE=1 turns this off with the native setters. */
static int push_is_sync(int id)
{
    switch (id) {
    case HLE_D3DDevice_Swap:
    case HLE_D3DDevice_InsertFence:
    case HLE_D3DDevice_BlockOnFence:
    case HLE_D3DDevice_IsFencePending:
    case HLE_D3DDevice_InsertCallback:
    case HLE_D3DDevice_BeginVisibilityTest:
    case HLE_D3DDevice_EndVisibilityTest:
    case HLE_D3DDevice_GetVisibilityTestResult:
    case HLE_D3DDevice_PersistDisplay:
    case HLE_D3DDevice_RunPushBuffer:
    case HLE_D3DResource_IsBusy:
    case HLE_D3DResource_BlockUntilNotBusy:
    case HLE_D3DDevice_BeginPush_4:
    case HLE_D3DDevice_Reset:
    case HLE_Direct3D_CreateDevice:
        return 1;
    default:
        return 0;
    }
}

static uint32_t s_push_put0, s_push_kicks0;
static int s_push_open;
static uint64_t s_rewound_words;


static const char *const s_names[HLE_COUNT] = {
#define HLE_API(va, name, conv, args) #name,
#include "hle_d3d8_table.h"
#undef HLE_API
};

static uint64_t s_calls[HLE_COUNT];
static uint64_t s_swaps;
static int s_stats_every = -1;
/* Trace window [s_trace_first, s_trace_last) in swaps; -2 = not parsed yet.
 * RECOMP_HLE_TRACE=auto arms it after 900 consecutive frames with more than
 * 200 indexed draws (15 s into a match), and dumps that frame too. */
static int64_t s_trace_first = -2, s_trace_last;
static int s_trace_auto;
static uint32_t s_trace_push;
static int s_trace_count;
static uint32_t s_trace_push_ret;
static uint64_t s_trace_quiet_until;

static void hle_report(void)
{
    fprintf(stderr, "[HLE] swaps=%llu\n", (unsigned long long)s_swaps);
    for (int i = 0; i < HLE_COUNT; i++)
        if (s_calls[i])
            fprintf(stderr, "[HLE]   %-48s %llu\n", s_names[i], (unsigned long long)s_calls[i]);
    fflush(stderr);
}

/* RECOMP_HLE_PBSTAT=<swaps>: pushbuffer dwords each routed API's original
 * body writes (the pusher's put pointer, D3D global 0x261240), per swap
 * window -- what is still left for the NV2A path to consume. A put that moves
 * backwards (ring wrap) counts in s_pb_wraps. */
#define HLE_PB_PUT 0x261240u
static int s_pbstat_every = -1;
static uint64_t s_pb_words[HLE_COUNT], s_pb_calls[HLE_COUNT], s_pb_wraps, s_pb_game;
static uint32_t s_pb_last;

static void hle_pbstat_report(void)
{
    uint64_t total = 0;
    for (int i = 0; i < HLE_COUNT; i++)
        total += s_pb_words[i];
    fprintf(stderr, "[HLE-PB] swap %llu: %llu dwords in %d swaps, %llu wraps, %llu written by the game itself, %llu rewound\n",
            (unsigned long long)s_swaps, (unsigned long long)total, s_pbstat_every,
            (unsigned long long)s_pb_wraps, (unsigned long long)s_pb_game,
            (unsigned long long)s_rewound_words);
    s_rewound_words = 0;
    for (int i = 0; i < HLE_COUNT; i++)
        if (s_pb_words[i])
            fprintf(stderr, "[HLE-PB]   %-48s %8llu dwords %7llu calls\n", s_names[i],
                    (unsigned long long)s_pb_words[i], (unsigned long long)s_pb_calls[i]);
    memset(s_pb_words, 0, sizeof s_pb_words);
    memset(s_pb_calls, 0, sizeof s_pb_calls);
    s_pb_wraps = s_pb_game = 0;
}

static void hle_enter(int id)
{
    s_calls[id]++;
    if (id == HLE_Direct3D_CreateDevice) {
        extern void hle_sync_early(void);
        hle_nv2a_watch_start();
        hle_sync_early();
    }
    if (id == HLE_D3DDevice_Swap) {
        s_swaps++;
        if (s_pbstat_every < 0) {
            const char *e = getenv("RECOMP_HLE_PBSTAT");
            s_pbstat_every = e ? atoi(e) : 0;
        }
        if (s_pbstat_every > 0 && s_swaps % (uint64_t)s_pbstat_every == 0)
            hle_pbstat_report();
        if (s_stats_every < 0) {
            const char *e = getenv("RECOMP_HLE_STATS");
            s_stats_every = e ? atoi(e) : 0;
        }
        if (s_stats_every > 0 && s_swaps % (uint64_t)s_stats_every == 0)
            hle_report();
        if (s_swaps % 1800 == 0) {
            hle_sync_report();
            hle_api_report();
            hle_nv2a_watch_report();
        }
        if (s_trace_auto == 2 && (int64_t)s_swaps == s_trace_last) {
            extern volatile int hle_dump_request;
            hle_dump_request = 1;
            s_trace_auto = 3;
            /* push:<n> keeps watching: up to 6 traces, 600 swaps apart. */
            if (s_trace_push && ++s_trace_count < 6) {
                s_trace_auto = 5;
                s_trace_quiet_until = s_swaps + 600;
            }
        }
        if (s_trace_auto == 1) {
            static uint64_t last_draws, streak;
            uint64_t n = s_calls[HLE_D3DDevice_DrawIndexedVertices] - last_draws;
            last_draws = s_calls[HLE_D3DDevice_DrawIndexedVertices];
            streak = n > 120 ? streak + 1 : 0;
            if (streak == 900) {
                s_trace_first = (int64_t)s_swaps;
                s_trace_last = s_trace_first + 1;
                s_trace_auto = 2;
                fprintf(stderr, "[HLE-T] auto trace armed at swap %llu\n", (unsigned long long)s_swaps);
            }
        }
    }
}

static float f32_bits(uint32_t v)
{
    float f;
    memcpy(&f, &v, sizeof f);
    return f;
}

/* Runs the original library body, telling the native backend about the calls
 * it mirrors. Stack arguments are read at entry (g_esp -> return address),
 * before the stdcall body pops them. */
/* RECOMP_HLE_TRACE=<first swap>[,<count>]: print every routed call with its
 * first stack arguments and the fastcall registers for those frames. */
/* RECOMP_HLE_TRACE=black: hle_d3d11.c found a frame whose middle is black
 * despite many draws; trace the next frame and dump it (once). */
int hle_trace_black_wanted(void)
{
    return s_trace_auto == 4;
}

void hle_trace_arm_next(void)
{
    if (s_trace_auto != 4 && s_trace_auto != 5)
        return;
    s_trace_first = (int64_t)s_swaps + 1;
    s_trace_last = s_trace_first + 1;
    s_trace_auto = 2;
    fprintf(stderr, "[HLE-T] trigger at swap %llu: tracing the next frame\n",
            (unsigned long long)s_swaps);
}

int hle_trace_on(void)
{
    return s_trace_first >= 0 && (int64_t)s_swaps >= s_trace_first && (int64_t)s_swaps < s_trace_last;
}

static void hle_trace(int id)
{
    if (s_trace_first == -2) {
        const char *e = getenv("RECOMP_HLE_TRACE");
        s_trace_first = -1;
        if (e && !strcmp(e, "auto")) {
            s_trace_auto = 1;
        } else if (e && !strcmp(e, "black")) {
            s_trace_auto = 4;                 /* armed by the backend's probe */
        } else if (e && !strncmp(e, "push:", 5)) {
            s_trace_auto = 5;                 /* armed by BeginPush(<dwords>) */
            s_trace_push = (uint32_t)strtoul(e + 5, NULL, 0);
            if (strchr(e, '@'))               /* push:<n>@<caller return address> */
                s_trace_push_ret = (uint32_t)strtoul(strchr(e, '@') + 1, NULL, 0);
        } else if (e) {
            const char *c = strchr(e, ',');
            s_trace_first = atoll(e);
            s_trace_last = s_trace_first + (c ? atoll(c + 1) : 1);
        }
    }
    if (s_trace_first < 0 || (int64_t)s_swaps < s_trace_first || (int64_t)s_swaps >= s_trace_last)
        return;
    fprintf(stderr, "[HLE-T] %llu %-44s ecx=%08X edx=%08X a=%08X %08X %08X %08X %08X %08X\n",
            (unsigned long long)s_swaps, s_names[id], g_ecx, g_edx,
            MEM32(g_esp + 4), MEM32(g_esp + 8), MEM32(g_esp + 12),
            MEM32(g_esp + 16), MEM32(g_esp + 20), MEM32(g_esp + 24));
}

/* Library bodies call other routed functions (Swap blits the multisampled
 * back buffer with Begin/End, SetRenderTarget sets the viewport, ...). The
 * backend mirrors state at any depth but draws only the game's own calls
 * (depth 0); what the library draws internally it implements itself. */
static RECOMP_TLS int s_depth;

/* The game's caller somewhere in the first 8 stack dwords (its frame). */
static int push_caller_matches(void)
{
    int i;
    if (!s_trace_push_ret)
        return 1;
    for (i = 0; i < 8; i++)
        if (MEM32(g_esp + 4 * i) == s_trace_push_ret)
            return 1;
    return 0;
}

static void hle_native_call(int id, void (*gen)(void))
{
    uint32_t a[8], ecx = g_ecx, edx = g_edx;
    hle_trace(id);
    if (s_trace_auto == 5 && id == HLE_D3DDevice_BeginPush_4 && MEM32(g_esp + 4) == s_trace_push &&
        s_swaps >= s_trace_quiet_until && push_caller_matches())
        hle_trace_arm_next();
    if (!hle_native_enabled()) {
        gen();
        return;
    }
    for (int i = 0; i < 8; i++)
        a[i] = MEM32(g_esp + 4 + 4 * i);
    if (s_depth) {
        int depth = s_depth++;
        hle_native_observe(id, depth, 0, a, ecx, edx, 0);
        if (!hle_api_call(id, a, gen)) gen();
        s_depth--;
        hle_native_observe(id, depth, 1, a, ecx, edx, g_eax);
        return;
    }
    hle_native_observe(id, 0, 0, a, ecx, edx, 0);
    uint32_t put0 = MEM32(HLE_PB_PUT), kicks0 = hle_sync_kick_count;
    int rewind = hle_native_owns_state();
    if (rewind && id == HLE_D3DDevice_BeginPush_4) {
        s_push_put0 = put0;
        s_push_kicks0 = kicks0;
        s_push_open = 1;
    }
    if (s_pbstat_every > 0 && s_pb_last && put0 > s_pb_last)
        s_pb_game += (put0 - s_pb_last) / 4;
    s_depth++;
    switch (id) {
    case HLE_D3DDevice_Swap:
        hle_native_swap(a[0]);
        if (!hle_api_call(id, a, gen))
            gen();
        hle_native_pace();
        break;
    /* Native bodies (hle_native_api.c): library state kept in C, no push. */
    case HLE_D3DDevice_SetRenderTarget:
        if (hle_trace_on()) {
            extern void hle_trace_dump_rt(void);
            hle_trace_dump_rt();
        }
        if (!hle_api_call(id, a, gen))
            gen();
        break;
    case HLE_D3DDevice_SetVertexShader:
    case HLE_D3DDevice_SetPixelShader:
    case HLE_D3DDevice_SetRenderState_Simple:
    case HLE_D3DDevice_SetTexture:
    case HLE_D3DDevice_SetPalette:
    case HLE_D3DDevice_SetStreamSource:
    case HLE_D3DDevice_SetIndices:
    case HLE_D3DDevice_SetViewport:
    case HLE_D3DDevice_SetPixelShaderConstant:
    case HLE_D3DDevice_SetVertexData2f:
    case HLE_D3DDevice_SetVertexData4f:
    case HLE_D3DDevice_SetVertexDataColor:
        if (!hle_api_call(id, a, gen))
            gen();
        break;
    case HLE_D3DDevice_CopyRects:
        if (!hle_api_call(id, a, gen)) gen();
        hle_native_copy_rects(a[0], a[1], a[2], a[3], a[4]);
        break;
    case HLE_D3DSurface_LockRect:
        if (!hle_api_call(id, a, gen)) gen();
        hle_native_lock_rect(a[0], a[1]);
        break;
    case HLE_D3DDevice_Clear:
        /* 0x258D70 writes only NV2A commands: native, the backend clears. */
        if (!hle_api_call(id, a, gen))
            gen();
        hle_native_clear(a[0], a[1], a[2], a[3], f32_bits(a[4]), a[5]);
        break;
    case HLE_Direct3D_CreateDevice:
        /* The host renderer comes first so the device bring-up itself can be
         * native (hle_native_create.c) instead of the library's body. */
        hle_native_device_created(a[4]);
        if (!hle_api_call(id, a, gen)) gen();
        {
            /* RECOMP_HLE_DEVDUMP=<file>: the device and miniport as CreateDevice
             * left them, to compare the original and native bring-up. */
            const char *dump = getenv("RECOMP_HLE_DEVDUMP");
            FILE *f = dump ? fopen(dump, "wb") : NULL;
            if (f) {
                fwrite((const void *)XBOX_PTR(0x261240u), 1, 0x2636E0u - 0x261240u, f);
                fclose(f);
            }
        }
        break;
    case HLE_D3DDevice_Reset:
        if (!hle_api_call(id, a, gen)) gen();
        hle_native_device_created(a[0]);
        break;
    case HLE_D3DDevice_DrawVertices:
    case HLE_D3DDevice_DrawIndexedVertices:
        /* Primary: the native backend drew it in observe (vertex and index
         * data are copied at the call); the library body would only encode
         * the same draw into the pushbuffer for the NV2A. stdcall, 3 args. */
        if (hle_native_owns_draws())
            g_esp += 16;
        else
            gen();
        break;
    /* Vertex shader constants: the library keeps a CPU copy at 0x260180 +
     * register * 16 (register already biased by 96) and pushes
     * SET_TRANSFORM_CONSTANT_LOAD + SET_TRANSFORM_CONSTANT. The backend took
     * the values in observe; keep the copy, skip the push. fastcall
     * (ecx = register, edx = data); NotInline also takes a dword count on the
     * stack (ret 4) and skips its copy while device flag 0x10 is set. */
    case HLE_D3DDevice_SetVertexShaderConstant1:
    case HLE_D3DDevice_SetVertexShaderConstant4:
    case HLE_D3DDevice_SetVertexShaderConstantNotInline:
        if (!hle_native_owns_state()) {
            gen();
            break;
        }
        {
            uint32_t dwords = id == HLE_D3DDevice_SetVertexShaderConstant1 ? 4
                            : id == HLE_D3DDevice_SetVertexShaderConstant4 ? 16 : a[0];
            if (id != HLE_D3DDevice_SetVertexShaderConstantNotInline || !(MEM8(0x261248) & 0x10))
                for (uint32_t i = 0; i < dwords; i++)
                    MEM32(0x260180 + ecx * 16 + 4 * i) = MEM32(edx + 4 * i);
            g_esp += id == HLE_D3DDevice_SetVertexShaderConstantNotInline ? 8 : 4;
        }
        break;
    default:
        /* Render-state setters with native bodies (hle_native_api.c). */
        if (!hle_api_call(id, a, gen))
            gen();
        break;
    }
    s_depth--;
    if (rewind) {
        uint32_t put1 = MEM32(HLE_PB_PUT);
        if (id == HLE_D3DDevice_EndPush && s_push_open) {
            s_push_open = 0;
            if (hle_sync_kick_count == s_push_kicks0 && put1 >= s_push_put0) {
                s_rewound_words += (put1 - s_push_put0) / 4;
                MEM32(HLE_PB_PUT) = put1 = s_push_put0;
            }
        } else if (!s_push_open && !push_is_sync(id) && hle_sync_kick_count == kicks0 && put1 > put0) {
            s_rewound_words += (put1 - put0) / 4;
            MEM32(HLE_PB_PUT) = put1 = put0;
        }
    }
    if (s_pbstat_every > 0) {
        uint32_t put1 = MEM32(HLE_PB_PUT);
        s_pb_calls[id]++;
        if (put1 >= put0)
            s_pb_words[id] += (put1 - put0) / 4;
        else
            s_pb_wraps++;
        s_pb_last = put1;
    }
    hle_native_observe(id, 0, 1, a, ecx, edx, g_eax);
}

int guest_cpu_native_enter(void);
void guest_cpu_native_leave(int token);
/* RECOMP_GUEST_SMP: the API runs under the guest kernel lock (no-op otherwise). */
#define HLE_API(va, name, conv, args) \
    void sub_##va(void) { int smp_ = guest_cpu_native_enter(); hle_enter(HLE_##name); \
                          hle_native_call(HLE_##name, sub_##va##_gen); guest_cpu_native_leave(smp_); }
#include "hle_d3d8_table.h"
#undef HLE_API
