#pragma once
/* Outside the APU lock, let pending host MMIO finish before the frame
 * thread reacquires an unfair Windows critical section. Bound the handoff
 * so continuous guest traffic cannot starve audio processing instead. */
static inline void apu_frame_handoff(volatile LONG *waiters)
{
    if (InterlockedCompareExchange(waiters, 0, 0) == 0) {
        SwitchToThread();
        return;
    }
    for (unsigned i = 0; i < 4 &&
         InterlockedCompareExchange(waiters, 0, 0) != 0; ++i)
        Sleep(1);
}
