/*
 * RECOMP_NV2A_WATCH=1: who still touches the NV2A register aperture.
 *
 * The goal of the native renderer is that nothing reads or writes the
 * emulated GPU's registers (0xFD000000, 16 MB) any more. To measure it, the
 * aperture's host pages carry PAGE_GUARD from Direct3D_CreateDevice on; a
 * thread re-arms them every 20 ms. Each first touch of a page per arming
 * raises STATUS_GUARD_PAGE_VIOLATION, recorded here by host RIP (resolved to
 * the recompiled or runtime function through dbghelp), register offset and
 * direction, and the access then proceeds. Counts are therefore lower bounds
 * (one per page per 20 ms), but a site that never appears never ran.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"

#define NW_BASE      0xFD000000u
#define NW_SIZE      0x01000000u
#define NW_MAX_HITS  128

static int s_on = -1;
static uint8_t *s_host;
static struct { uintptr_t rip; uint32_t off; int write; volatile LONG n; } s_hits[NW_MAX_HITS];
static volatile LONG s_nhits, s_total;
static CRITICAL_SECTION s_lock;

static LONG CALLBACK nw_handler(PEXCEPTION_POINTERS e)
{
    uintptr_t a, rip;
    LONG k, m;
    if (e->ExceptionRecord->ExceptionCode != STATUS_GUARD_PAGE_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    a = (uintptr_t)e->ExceptionRecord->ExceptionInformation[1];
    if (a < (uintptr_t)s_host || a >= (uintptr_t)s_host + NW_SIZE)
        return EXCEPTION_CONTINUE_SEARCH;
    InterlockedIncrement(&s_total);
    rip = (uintptr_t)e->ContextRecord->Rip;
    m = s_nhits;
    for (k = 0; k < m; k++)
        if (s_hits[k].rip == rip) {
            InterlockedIncrement(&s_hits[k].n);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    EnterCriticalSection(&s_lock);
    m = s_nhits;
    if (m < NW_MAX_HITS) {
        s_hits[m].rip = rip;
        s_hits[m].off = (uint32_t)(a - (uintptr_t)s_host);
        s_hits[m].write = e->ExceptionRecord->ExceptionInformation[0] != 0;
        s_hits[m].n = 1;
        s_nhits = m + 1;
    }
    LeaveCriticalSection(&s_lock);
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void nw_arm(void)
{
    uint8_t *p = s_host, *end = s_host + NW_SIZE;
    while (p < end) {
        MEMORY_BASIC_INFORMATION mi;
        uint8_t *next;
        DWORD old;
        if (!VirtualQuery(p, &mi, sizeof mi))
            break;
        next = (uint8_t *)mi.BaseAddress + mi.RegionSize;
        if (next > end)
            next = end;
        if (mi.State == MEM_COMMIT && !(mi.Protect & PAGE_GUARD))
            VirtualProtect(p, (SIZE_T)(next - p), PAGE_READWRITE | PAGE_GUARD, &old);
        p = next;
    }
}

static DWORD WINAPI nw_thread(LPVOID unused)
{
    (void)unused;
    for (;;) {
        nw_arm();
        Sleep(20);
    }
    return 0;
}

void hle_nv2a_watch_start(void)
{
    if (s_on >= 0)
        return;
    s_on = getenv("RECOMP_NV2A_WATCH") != NULL;
    if (!s_on)
        return;
    s_host = (uint8_t *)XBOX_PTR(NW_BASE);
    InitializeCriticalSection(&s_lock);
    AddVectoredExceptionHandler(1, nw_handler);
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    nw_arm();
    CreateThread(NULL, 0, nw_thread, NULL, 0, NULL);
    fprintf(stderr, "[NV2A-WATCH] armed at CreateDevice: accesses to the NV2A registers are recorded\n");
}

void hle_nv2a_watch_report(void)
{
    LONG k, m = s_nhits;
    char buf[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO *sym = (SYMBOL_INFO *)buf;
    if (s_on <= 0)
        return;
    fprintf(stderr, "[NV2A-WATCH] %ld access(es) seen at %ld site(s)\n", (long)s_total, (long)m);
    for (k = 0; k < m; k++) {
        DWORD64 disp = 0;
        memset(buf, 0, sizeof buf);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;
        if (!SymFromAddr(GetCurrentProcess(), s_hits[k].rip, &disp, sym))
            strcpy(sym->Name, "?");
        fprintf(stderr, "[NV2A-WATCH]   %s+0x%llX reg %06X %s x%ld\n", sym->Name,
                (unsigned long long)disp, s_hits[k].off, s_hits[k].write ? "write" : "read",
                (long)s_hits[k].n);
    }
    fflush(stderr);
}
#else /* !_WIN32: a Windows diagnostic (PAGE_GUARD + dbghelp) */
void hle_nv2a_watch_start(void) {}
void hle_nv2a_watch_report(void) {}
#endif
