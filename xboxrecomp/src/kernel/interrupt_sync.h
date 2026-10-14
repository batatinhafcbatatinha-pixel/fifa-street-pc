#ifndef XBOX_INTERRUPT_SYNC_H
#define XBOX_INTERRUPT_SYNC_H
#include <stdint.h>
/* All host paths delivering a connected ISR must share its synchronization
 * lock with KeSynchronizeExecution. Locks are recursive for nested calls. */
void xbox_InterruptAcquire(uint32_t interrupt_va);
void xbox_InterruptRelease(uint32_t interrupt_va);
int xbox_InterruptIrql(uint32_t interrupt_va);
int guest_cpu_callback_enter(void);
int guest_cpu_is_serialized(void);
int guest_cpu_interrupt_allowed(int device_irql);
void guest_cpu_callback_leave(int token);
int guest_cpu_suspend(void);
void guest_cpu_resume(int token);
#endif
