/* Optional read-only provenance for the resource loader and allocator.
 * Reads through ReadProcessMemory so diagnostics do not cause a guest fault.
 * Never changes guest registers, memory, thread scheduling, or return values.
 */
#ifdef _WIN32
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
extern ptrdiff_t g_xbox_mem_offset;
static INIT_ONCE trace_once = INIT_ONCE_STATIC_INIT;
static CRITICAL_SECTION trace_lock;
static FILE *trace_file;
static unsigned trace_count;
static BOOL CALLBACK trace_init(PINIT_ONCE once, PVOID parameter, PVOID *context)
{
    const char *path = getenv("RECOMP_RESOURCE_TRACE");
    (void)once; (void)parameter; (void)context;
    InitializeCriticalSection(&trace_lock);
    if (path && *path) trace_file = fopen(path, "w");
    return TRUE;
}
void recomp_resource_trace(const char *event, uint32_t address,
                           uint32_t auxiliary, uint32_t result)
{
    uint8_t bytes[64]; SIZE_T count = 0;
    InitOnceExecuteOnce(&trace_once, trace_init, NULL, NULL);
    if (!trace_file) return;
    EnterCriticalSection(&trace_lock);
    /* Preserve loader evidence and allocation failures even after the
     * high-volume allocator history reaches its normal limit. */
    if (trace_count < 100000 || strcmp(event, "free-insert") != 0 &&
        (strcmp(event, "allocation-result") != 0 || address == 0)) {
        ++trace_count;
        BOOL readable = (uint64_t)address + sizeof bytes <= 0x100000000ULL &&
            ReadProcessMemory(GetCurrentProcess(),
                (void *)((uintptr_t)address + g_xbox_mem_offset),
                bytes, sizeof bytes, &count);
        fprintf(trace_file, "%u tick=%llu thread=%lu %s address=%08X auxiliary=%08X result=%08X read=%d bytes=%llu data=",
            trace_count, GetTickCount64(), GetCurrentThreadId(), event,
            address, auxiliary, result, readable, (unsigned long long)count);
        for (SIZE_T i = 0; i < count; ++i) fprintf(trace_file, "%02X", bytes[i]);
        fputc('\n', trace_file);
        fflush(trace_file);
    }
    LeaveCriticalSection(&trace_lock);
}
#else /* !_WIN32: RECOMP_RESOURCE_TRACE is a Windows diagnostic */
#include <stdint.h>
void recomp_resource_trace(const char *event, uint32_t address, uint32_t auxiliary, uint32_t result)
{
    (void)event; (void)address; (void)auxiliary; (void)result;
}
#endif
