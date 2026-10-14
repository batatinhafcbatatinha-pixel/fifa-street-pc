#pragma once
/* QEMU's optional trace sink is absent in the standalone runtime. */
#define TRACE_DSP56K_EXECUTE_INSTRUCTION_DISASM 0
#define trace_event_get_state(event) 0
#define trace_dsp56k_execute_instruction(gp,pc) ((void)0)
#define trace_dsp56k_execute_instruction_disasm(text) ((void)0)
