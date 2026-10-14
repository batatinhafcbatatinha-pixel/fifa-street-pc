/* Standalone C interpreter adapter, based on xemu DSP peripheral and C
 * backend code at 478b4f496102379c7eaa7f3ec10e714a703c4300.
 * Copyright (c) 2015 espes, 2020-2025 Matt Borgerson and contributors.
 * This adapter is GPL-2.0-or-later, matching the interpreter backend.
 */
#include "standalone.h"
#include "dsp_runtime.h"
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static uint32_t dma_read_memory(void *opaque, int space, uint32_t addr)
{ return dsp56k_read_memory(opaque, space, addr); }
static void dma_write_memory(void *opaque, int space, uint32_t addr, uint32_t value)
{ dsp56k_write_memory(opaque, space, addr, value); }

static uint32_t read_peripheral(dsp_core_t *core, uint32_t address)
{
    DSPState *dsp = core->opaque;
    switch (address) {
    case 0xFFFFB3: return 0;
    case 0xFFFFC5: return dsp->interrupts | (dsp->dma.eol ? 0x80 : 0);
    case 0xFFFFD4: return dsp_dma_read(&dsp->dma, DMA_NEXT_BLOCK);
    case 0xFFFFD5: return dsp_dma_read(&dsp->dma, DMA_START_BLOCK);
    case 0xFFFFD6: return dsp_dma_read(&dsp->dma, DMA_CONTROL);
    case 0xFFFFD7: return dsp_dma_read(&dsp->dma, DMA_CONFIGURATION);
    default: return 0xABABA;
    }
}

static void write_peripheral(dsp_core_t *core, uint32_t address, uint32_t value)
{
    DSPState *dsp = core->opaque;
    switch (address) {
    case 0xFFFFC4: if (value & 1) core->is_idle = true; break;
    case 0xFFFFC5:
        dsp->interrupts &= ~value;
        if (value & 0x80) dsp->dma.eol = false;
        break;
    case 0xFFFFD4: dsp_dma_write(&dsp->dma, DMA_NEXT_BLOCK, value); break;
    case 0xFFFFD5: dsp_dma_write(&dsp->dma, DMA_START_BLOCK, value); break;
    case 0xFFFFD6: dsp_dma_write(&dsp->dma, DMA_CONTROL, value); break;
    case 0xFFFFD7: dsp_dma_write(&dsp->dma, DMA_CONFIGURATION, value); break;
    }
}

DSPState *dsp_init(void *opaque, dsp_scratch_rw_func scratch, dsp_fifo_rw_func fifo, bool is_gp)
{
    DSPState *dsp = calloc(1, sizeof(*dsp));
    if (!dsp) return NULL;
    dsp->is_gp = dsp->core.is_gp = is_gp;
    const char *native = getenv("RECOMP_DSP_NATIVE");
    dsp->native_enabled = native && !strcmp(native, "1");
    const char *profile = getenv("RECOMP_DSP_PROFILE");
    dsp->profile_enabled = profile && !strcmp(profile, "1");
    const char *dump = getenv("RECOMP_DSP_PROGRAM_DUMP");
    if (dump && *dump) {
        if (strlen(dump) < sizeof(dsp->program_dump_directory))
            memcpy(dsp->program_dump_directory, dump, strlen(dump)+1);
        else fprintf(stderr, "[DSP-DUMP] directory too long; dumping disabled\n");
    }
    dsp->core.opaque = dsp;
    dsp->core.read_peripheral = read_peripheral;
    dsp->core.write_peripheral = write_peripheral;
    dsp->dma.mem_opaque = &dsp->core;
    dsp->dma.mem_read = dma_read_memory;
    dsp->dma.mem_write = dma_write_memory;
    dsp->dma.rw_opaque = opaque;
    dsp->dma.scratch_rw = scratch;
    dsp->dma.fifo_rw = fifo;
    dsp_reset(dsp);
    return dsp;
}

void dsp_reset(DSPState *dsp)
{ dsp56k_reset_cpu(&dsp->core); dsp->save_cycles = 0;
  dsp->native_steps = dsp->interpreted_steps = dsp->native_guard_fallbacks = 0;
  memset(dsp->pc_steps, 0, sizeof(dsp->pc_steps));
  memset(dsp->pc_fallback_steps, 0, sizeof(dsp->pc_fallback_steps)); }

void dsp_run(DSPState *dsp, int cycles)
{
    dsp->save_cycles += cycles;
    while (dsp->save_cycles > 0) {
        const uint32_t profile_pc = dsp->core.pc;
        const bool candidate = dsp->native_enabled && dsp56k_native_pc_covered(&dsp->core);
        if (dsp->profile_enabled && profile_pc < DSP_PRAM_SIZE)
            ++dsp->pc_steps[profile_pc];
        if (candidate && dsp56k_execute_native_instruction(&dsp->core))
            ++dsp->native_steps;
        else {
            if (candidate) ++dsp->native_guard_fallbacks;
            if (dsp->profile_enabled && profile_pc < DSP_PRAM_SIZE)
                ++dsp->pc_fallback_steps[profile_pc];
            dsp56k_execute_instruction(&dsp->core);
            ++dsp->interpreted_steps;
        }
        dsp->save_cycles -= dsp->core.instr_cycle;
        dsp->core.cycle_count += dsp->core.instr_cycle;
        if (dsp->core.is_idle) break;
    }
}

void dsp_bootstrap(DSPState *dsp)
{
    dsp->dma.scratch_rw(dsp->dma.rw_opaque, (uint8_t *)dsp->core.pram, 0, 0x800 * 4, false);
    for (int i = 0; i < 0x800; ++i) dsp->core.pram[i] &= 0xFFFFFF;
    memset(dsp->core.pram_opcache, 0, sizeof(dsp->core.pram_opcache));
    fprintf(stderr, "[DSP] %s bootstrap: program[0]=%06X program[1]=%06X\n",
            dsp->is_gp ? "GP" : "EP", dsp->core.pram[0], dsp->core.pram[1]);
}

static int space_id(char space)
{
    switch (space) { case 'X': return DSP_SPACE_X; case 'Y': return DSP_SPACE_Y; case 'P': return DSP_SPACE_P; }
    assert(!"Invalid DSP memory space"); return DSP_SPACE_X;
}
uint32_t dsp_read_memory(DSPState *dsp, char space, uint32_t address)
{ return dsp56k_read_memory(&dsp->core, space_id(space), address); }
void dsp_write_memory(DSPState *dsp, char space, uint32_t address, uint32_t value)
{ dsp56k_write_memory(&dsp->core, space_id(space), address, value & 0xFFFFFF); }
void dsp_start_frame(DSPState *dsp) { dsp->interrupts |= 2; }
bool dsp_get_halt_requested(DSPState *dsp) { return dsp->core.is_idle; }
void dsp_set_halt_requested(DSPState *dsp, bool idle) { dsp->core.is_idle = idle; }
uint32_t dsp_get_cycle_count(DSPState *dsp) { return dsp->core.cycle_count; }
void dsp_set_cycle_count(DSPState *dsp, uint32_t count) { dsp->core.cycle_count = count; }

/* Read from the serial DSP thread, at the existing periodic report boundary. */
static void dsp_print_profile(const uint64_t *counts, const char *name, const char *prefix)
{
    unsigned pcs[10] = {0}; uint64_t hits[10] = {0}, total = 0;
    for (unsigned pc = 0; pc < DSP_PRAM_SIZE; ++pc) {
        uint64_t count = counts[pc]; total += count;
        if (!count) continue;
        for (unsigned i = 0; i < 10; ++i) {
            if (count > hits[i]) {
                for (unsigned j = 9; j > i; --j) { hits[j] = hits[j-1]; pcs[j] = pcs[j-1]; }
                hits[i] = count; pcs[i] = pc; break;
            }
        }
    }
    char line[512];
    size_t used = (size_t)snprintf(line, sizeof(line), "[%s] %s total=%llu top", prefix, name, (unsigned long long)total);
    for (unsigned i = 0; i < 10 && hits[i] && used < sizeof(line); ++i)
        used += (size_t)snprintf(line + used, sizeof(line) - used, " %03X:%llu", pcs[i], (unsigned long long)hits[i]);
    fprintf(stderr, "%s\n", line);
}

void dsp_profile_report(DSPState *dsp, const char *name)
{
    dsp_dump_program(dsp);
    if (!dsp->profile_enabled) return;
    dsp_print_profile(dsp->pc_steps, name, "DSP-PROFILE");
    dsp_print_profile(dsp->pc_fallback_steps, name, "DSP-FALLBACK-PROFILE");
}

/* Both processors execute on the serial DSP thread. Process-wide lifetime cap
 * survives DSP resets; at most 64 successful 16 KiB writes across GP and EP.
 * FNV is only a lookup filter: byte comparison of the retained file proves dedup.
 */
static unsigned program_dump_total;
static void program_dump_path(const DSPState *dsp, unsigned slot, uint64_t hash,
                              char *path, size_t bytes)
{
    snprintf(path, bytes, "%s/%s-pram-%02u-%016llX.bin", dsp->program_dump_directory,
             dsp->core.is_gp ? "GP" : "EP", slot, (unsigned long long)hash);
}
static bool program_dump_equal(const char *path, const uint32_t *words)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    uint32_t saved[DSP_PRAM_SIZE];
    bool equal = fread(saved, 1, sizeof(saved), f) == sizeof(saved) &&
                 fgetc(f) == EOF && !memcmp(saved, words, sizeof(saved));
    fclose(f); return equal;
}
int dsp_dump_program(DSPState *dsp)
{
    if (!*dsp->program_dump_directory || dsp->program_dump_failed) return 0;
    const unsigned char *bytes = (const unsigned char *)dsp->core.pram;
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < sizeof(dsp->core.pram); ++i) { hash ^= bytes[i]; hash *= UINT64_C(1099511628211); }
    char path[1200];
    for (unsigned i = 0; i < dsp->program_dump_saved; ++i) {
        if (dsp->program_dump_hashes[i] == hash) {
            program_dump_path(dsp, dsp->program_dump_slots[i], hash, path, sizeof(path));
            if (program_dump_equal(path, dsp->core.pram)) return 0;
        }
    }
    if (program_dump_total == 64 || dsp->program_dump_saved == 64) return 0;
#ifdef _WIN32
    int mkdir_result = _mkdir(dsp->program_dump_directory);
#else
    int mkdir_result = mkdir(dsp->program_dump_directory, 0700);
#endif
    if (mkdir_result && errno != EEXIST) goto failed;
    program_dump_path(dsp, program_dump_total, hash, path, sizeof(path));
    /* Exclusive creation preserves any pre-existing diagnostic artifact. */
    FILE *f = fopen(path, "wbx");
    if (!f) goto failed;
    bool written = fwrite(dsp->core.pram, 1, sizeof(dsp->core.pram), f) == sizeof(dsp->core.pram);
    if (fclose(f)) written = false;
    if (!written) { remove(path); goto failed; }
    unsigned i = dsp->program_dump_saved++;
    dsp->program_dump_hashes[i] = hash;
    dsp->program_dump_slots[i] = program_dump_total++;
    fprintf(stderr, "[DSP-DUMP] %s pc=%03X bytes=%zu slot=%u file=%s\n",
            dsp->core.is_gp ? "GP" : "EP", dsp->core.pc, sizeof(dsp->core.pram), program_dump_total-1, path);
    return 1;
failed:
    dsp->program_dump_failed = true;
    fprintf(stderr, "[DSP-DUMP] cannot save program; disabled for this processor (errno=%d)\n", errno);
    return -1;
}
