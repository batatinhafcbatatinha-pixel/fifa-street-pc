/*
 * Stall sampler: where the game is when frames stop.
 *
 * Every boot has two black pauses of almost exactly 4.0 s (swaps ~290 and
 * ~3300), and about one demo in twenty froze the main thread for 6-10 s while
 * audio and file streaming went on. Neither leaves a trace in the log: no
 * kernel wait times out, nothing faults. This thread watches the swap count;
 * when no frame has been presented for RECOMP_HLE_STALLDIAG ms (default 1000,
 * 0 = off) it samples the presenting thread -- and the thread holding the
 * guest CPU lock, if another one -- every 10 ms, and when frames resume it
 * prints the most frequent call chains (host functions; the recompiled ones
 * are named sub_<guest address>).
 *
 * The target is suspended only while its registers and unwind chain are read
 * (RtlLookupFunctionEntry/RtlVirtualUnwind, which do not allocate). Samples
 * are kept as chains of function start addresses; dbghelp names them only in
 * the report, so sampling costs microseconds and never waits on a lock the
 * suspended thread might hold.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dbghelp.h>

extern void guest_cpu_report(void);
extern unsigned long guest_cpu_owner_thread(void);

#define ST_DEPTH   24
#define ST_KEYLEN  10
#define ST_KEYS    64
#define ST_SHOWN   12

static volatile LONG64 s_swaps;
static volatile LONG s_swap_tid;
static int s_threshold_ms = -1;

typedef struct { DWORD64 fn[ST_KEYLEN]; unsigned n; } StallKey;
static StallKey s_keys[2][ST_KEYS];
static unsigned s_nkeys[2], s_samples[2];

static int capture(DWORD tid, DWORD64 *pcs, int max)
{
    HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, tid);
    CONTEXT c;
    int n = 0;
    if (!h)
        return 0;
    if (SuspendThread(h) == (DWORD)-1) {
        CloseHandle(h);
        return 0;
    }
    memset(&c, 0, sizeof c);
    c.ContextFlags = CONTEXT_FULL;
    if (GetThreadContext(h, &c)) {
        while (n < max && c.Rip) {
            DWORD64 base = 0, frame = 0;
            PVOID handler = NULL;
            PRUNTIME_FUNCTION f;
            f = RtlLookupFunctionEntry(c.Rip, &base, NULL);
            pcs[n++] = f ? base + f->BeginAddress : c.Rip;
            if (!f) {                     /* leaf: return address on top */
                c.Rip = *(DWORD64 *)c.Rsp;
                c.Rsp += 8;
            } else {
                RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, c.Rip, f, &c, &handler, &frame, NULL);
            }
        }
    }
    ResumeThread(h);
    CloseHandle(h);
    return n;
}

static void name_of(DWORD64 pc, char *out, size_t size)
{
    char buf[sizeof(SYMBOL_INFO) + 128];
    SYMBOL_INFO *sym = (SYMBOL_INFO *)buf;
    DWORD64 disp = 0;
    memset(buf, 0, sizeof buf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = 127;
    if (SymFromAddr(GetCurrentProcess(), pc, &disp, sym))
        snprintf(out, size, "%s", sym->Name);
    else
        snprintf(out, size, "%llX", (unsigned long long)pc);
}

static void record(int which, const DWORD64 *pcs, int n)
{
    DWORD64 fn[ST_KEYLEN];
    unsigned k;
    int i, m = 0;
    memset(fn, 0, sizeof fn);
    for (i = 0; i < n && m < ST_KEYLEN; i++)
        if (!m || fn[m - 1] != pcs[i])
            fn[m++] = pcs[i];
    s_samples[which]++;
    for (k = 0; k < s_nkeys[which]; k++)
        if (!memcmp(s_keys[which][k].fn, fn, sizeof fn)) {
            s_keys[which][k].n++;
            return;
        }
    if (s_nkeys[which] < ST_KEYS) {
        memcpy(s_keys[which][s_nkeys[which]].fn, fn, sizeof fn);
        s_keys[which][s_nkeys[which]++].n = 1;
    }
}

/* "leaf < caller < ...", cut after the first two recompiled (sub_)
 * functions: that is the guest code doing the waiting. */
static void describe(const DWORD64 *fn, char *key, size_t size)
{
    char name[128];
    size_t len = 0;
    int subs = 0, i;
    key[0] = 0;
    for (i = 0; i < ST_KEYLEN && fn[i] && subs < 2 && len < size - 1; i++) {
        name_of(fn[i], name, sizeof name);
        if (!strncmp(name, "sub_", 4))
            subs++;
        len += (size_t)snprintf(key + len, size - len, "%s%s", len ? " < " : "", name);
    }
}

static void report(int which, const char *who)
{
    unsigned shown, k;
    if (!s_samples[which])
        return;
    fprintf(stderr, "[HLE-STALL]   %s: %u samples\n", who, s_samples[which]);
    for (shown = 0; shown < ST_SHOWN; shown++) {
        int best = -1;
        for (k = 0; k < s_nkeys[which]; k++)
            if (s_keys[which][k].n && (best < 0 || s_keys[which][k].n > s_keys[which][best].n))
                best = (int)k;
        if (best < 0)
            break;
        {
            char key[768];
            describe(s_keys[which][best].fn, key, sizeof key);
            fprintf(stderr, "[HLE-STALL]     %3u%%  %s\n",
                    s_keys[which][best].n * 100 / s_samples[which], key);
        }
        s_keys[which][best].n = 0;
    }
}

static DWORD WINAPI stall_thread(LPVOID arg)
{
    LONG64 seen = 0;
    DWORD seen_at = GetTickCount();
    (void)arg;
    for (;;) {
        LONG64 now_swaps = InterlockedCompareExchange64(&s_swaps, 0, 0);
        DWORD now = GetTickCount(), stalled_since;
        if (now_swaps != seen) {
            seen = now_swaps;
            seen_at = now;
            Sleep(100);
            continue;
        }
        if (now - seen_at < (DWORD)s_threshold_ms) {
            Sleep(50);
            continue;
        }
        /* Stalled: sample until a frame arrives. */
        stalled_since = seen_at;
        s_nkeys[0] = s_nkeys[1] = s_samples[0] = s_samples[1] = 0;
        fprintf(stderr, "[HLE-STALL] swap %lld: no frame for %lu ms, sampling\n",
                (long long)seen, (unsigned long)(now - seen_at));
        guest_cpu_report();
        while (InterlockedCompareExchange64(&s_swaps, 0, 0) == seen && s_samples[0] < 100000) {
            DWORD64 pcs[ST_DEPTH];
            DWORD main_tid = (DWORD)s_swap_tid, owner = (DWORD)guest_cpu_owner_thread();
            int n = capture(main_tid, pcs, ST_DEPTH);
            if (n)
                record(0, pcs, n);
            if (owner && owner != main_tid && owner != GetCurrentThreadId()) {
                n = capture(owner, pcs, ST_DEPTH);
                if (n)
                    record(1, pcs, n);
            }
            Sleep(10);
        }
        now = GetTickCount();
        fprintf(stderr, "[HLE-STALL] swap %lld: stalled %lu ms\n",
                (long long)seen, (unsigned long)(now - stalled_since));
        report(0, "presenting thread");
        report(1, "guest CPU owner (other thread)");
        guest_cpu_report();
        fflush(stderr);
        seen = InterlockedCompareExchange64(&s_swaps, 0, 0);
        seen_at = GetTickCount();
    }
    return 0;
}

/* Called by the renderer on every presented frame. */
void hle_stall_tick(void)
{
    if (s_threshold_ms < 0) {
        const char *e = getenv("RECOMP_HLE_STALLDIAG");
        HANDLE t;
        s_threshold_ms = e ? atoi(e) : 1000;
        if (s_threshold_ms > 0) {
            if (s_threshold_ms < 100)
                s_threshold_ms = 100;
            s_swap_tid = (LONG)GetCurrentThreadId();
            t = CreateThread(NULL, 0, stall_thread, NULL, 0, NULL);
            if (t)
                CloseHandle(t);
        }
    }
    s_swap_tid = (LONG)GetCurrentThreadId();
    InterlockedIncrement64(&s_swaps);
}

#else
void hle_stall_tick(void) {}
#endif
