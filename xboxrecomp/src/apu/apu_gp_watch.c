/*
 * RECOMP_GP_WATCH=1: who reads what the GP DSP writes into guest RAM.
 *
 * The GP's only output a title can see is what its DMA writes to the scratch
 * pages in guest RAM (no CPU read of the GP aperture was ever observed in
 * FIFA Street). To find which guest code consumes it, the APU thread arms
 * those host pages with PAGE_GUARD after each GP frame; the first touch from
 * any other thread raises STATUS_GUARD_PAGE_VIOLATION, which this handler
 * records (host RIP, page, offset, read/write) and lets proceed. The report
 * resolves the RIPs to recompiled function names through dbghelp.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GPW_MAX_PAGES 1024
#define GPW_MAX_HITS  64

static int s_on = -1;
static DWORD s_apu_tid;
static uint8_t *s_pages[GPW_MAX_PAGES];
static uint16_t s_page_index[GPW_MAX_PAGES];
static volatile LONG s_npages;
static struct { uintptr_t rip; unsigned page; uint32_t off; int write; volatile LONG n; } s_hits[GPW_MAX_HITS];
static volatile LONG s_nhits;
static CRITICAL_SECTION s_lock;

static LONG CALLBACK gpw_handler(PEXCEPTION_POINTERS e)
{
    uintptr_t a;
    LONG i, n;
    if (e->ExceptionRecord->ExceptionCode != STATUS_GUARD_PAGE_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    a = (uintptr_t)e->ExceptionRecord->ExceptionInformation[1];
    n = s_npages;
    for (i = 0; i < n; i++) {
        uintptr_t b = (uintptr_t)s_pages[i];
        if (a >= b && a < b + 4096)
            break;
    }
    if (i == n)
        return EXCEPTION_CONTINUE_SEARCH;           /* not ours */
    if (GetCurrentThreadId() != s_apu_tid) {
        uintptr_t rip = (uintptr_t)e->ContextRecord->Rip;
        LONG k, m = s_nhits;
        for (k = 0; k < m; k++)
            if (s_hits[k].rip == rip) {
                InterlockedIncrement(&s_hits[k].n);
                break;
            }
        if (k == m && m < GPW_MAX_HITS) {
            EnterCriticalSection(&s_lock);
            m = s_nhits;
            if (m < GPW_MAX_HITS) {
                s_hits[m].rip = rip;
                s_hits[m].page = s_page_index[i];
                s_hits[m].off = (uint32_t)(a - (uintptr_t)s_pages[i]);
                s_hits[m].write = e->ExceptionRecord->ExceptionInformation[0] != 0;
                s_hits[m].n = 1;
                s_nhits = m + 1;
            }
            LeaveCriticalSection(&s_lock);
        }
    }
    return EXCEPTION_CONTINUE_EXECUTION;            /* the guard is gone; retry the access */
}

int mcpx_gpw_enabled(void)
{
    if (s_on < 0) {
        s_on = getenv("RECOMP_GP_WATCH") != NULL;
        if (s_on) {
            InitializeCriticalSection(&s_lock);
            AddVectoredExceptionHandler(1, gpw_handler);
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
            SymInitialize(GetCurrentProcess(), NULL, TRUE);
            fprintf(stderr, "[GP-WATCH] armed: guest reads of GP scratch pages are recorded\n");
        }
    }
    return s_on;
}

/* APU thread, after the GP ran: (re)arm the pages. host[i] = host address of
 * scratch page index[i]. */
void mcpx_gpw_arm(uint8_t *const host[], const uint16_t index[], int n)
{
    DWORD old;
    int i;
    if (!mcpx_gpw_enabled())
        return;
    s_apu_tid = GetCurrentThreadId();
    if (n > GPW_MAX_PAGES)
        n = GPW_MAX_PAGES;
    for (i = 0; i < n; i++) {
        s_pages[i] = (uint8_t *)((uintptr_t)host[i] & ~(uintptr_t)4095);
        s_page_index[i] = index[i];
    }
    s_npages = n;
    for (i = 0; i < n; i++)
        VirtualProtect(s_pages[i], 4096, PAGE_READWRITE | PAGE_GUARD, &old);
}

/* APU thread, before the GP runs: its own DMA must not trip the guards. */
void mcpx_gpw_disarm(void)
{
    DWORD old;
    LONG i, n = s_npages;
    if (s_on <= 0)
        return;
    for (i = 0; i < n; i++)
        VirtualProtect(s_pages[i], 4096, PAGE_READWRITE, &old);
}

void mcpx_gpw_report(void)
{
    LONG k, m = s_nhits;
    char buf[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO *sym = (SYMBOL_INFO *)buf;
    if (s_on <= 0)
        return;
    fprintf(stderr, "[GP-WATCH] %ld reader site(s):\n", (long)m);
    for (k = 0; k < m; k++) {
        DWORD64 disp = 0;
        memset(buf, 0, sizeof buf);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;
        if (!SymFromAddr(GetCurrentProcess(), s_hits[k].rip, &disp, sym))
            strcpy(sym->Name, "?");
        fprintf(stderr, "[GP-WATCH]   %s+0x%llX scratch p%u+0x%03X %s x%ld\n", sym->Name,
                (unsigned long long)disp, s_hits[k].page, s_hits[k].off,
                s_hits[k].write ? "write" : "read", (long)s_hits[k].n);
    }
}
#else /* !_WIN32: the watch is a Windows diagnostic (PAGE_GUARD + dbghelp). */
#include <stdint.h>
int mcpx_gpw_enabled(void) { return 0; }
void mcpx_gpw_arm(uint8_t *const host[], const uint16_t index[], int n) { (void)host; (void)index; (void)n; }
void mcpx_gpw_disarm(void) {}
void mcpx_gpw_report(void) {}
#endif
