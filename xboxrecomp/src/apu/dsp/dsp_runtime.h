#pragma once
#include "interp/dsp_cpu.h"
#include "dsp_dma.h"

typedef struct DSPState {
    dsp_core_t core;
    DSPDMAState dma;
    int save_cycles;
    uint32_t interrupts;
    bool is_gp;
    char program_dump_directory[1024];
    uint64_t program_dump_hashes[64];
    unsigned program_dump_slots[64], program_dump_saved;
    bool program_dump_failed;
    bool profile_enabled;
    uint64_t pc_fallback_steps[DSP_PRAM_SIZE];
    uint64_t pc_steps[DSP_PRAM_SIZE]; /* Fixed storage, no allocation per instruction. */
    bool native_enabled; /* RECOMP_DSP_NATIVE=1, experimental partial backend. */
    uint64_t native_steps, interpreted_steps;
    uint64_t native_guard_fallbacks; /* Attempts at covered GP/EP PCs rejected by guard/tracing. */
} DSPState;

DSPState *dsp_init(void *, dsp_scratch_rw_func, dsp_fifo_rw_func, bool);
void dsp_reset(DSPState *);
int dsp_dump_program(DSPState *); /* 1 saved, 0 disabled/duplicate/capped, -1 I/O error. */
void dsp_profile_report(DSPState *, const char *name);
void dsp_run(DSPState *, int);
void dsp_bootstrap(DSPState *);
void dsp_start_frame(DSPState *);
uint32_t dsp_read_memory(DSPState *, char, uint32_t);
void dsp_write_memory(DSPState *, char, uint32_t, uint32_t);
bool dsp_get_halt_requested(DSPState *);
void dsp_set_halt_requested(DSPState *, bool);
uint32_t dsp_get_cycle_count(DSPState *);
void dsp_set_cycle_count(DSPState *, uint32_t);

int mcpx_apu_dsp_enabled(void);
