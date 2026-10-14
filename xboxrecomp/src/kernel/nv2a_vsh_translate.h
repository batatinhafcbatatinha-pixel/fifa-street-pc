#ifndef NV2A_VSH_TRANSLATE_H
#define NV2A_VSH_TRANSLATE_H
#include <stddef.h>
#include <stdint.h>
#include "nv2a_vsh_interp.h"
/* Generates straight-line HLSL, not an opcode interpreter. Constants at b0:
 * float4 c[192], uint4 meta (meta.x = vertex count). Inputs at t0: struct
 * Input {float4 attr[16];}; results at u0: Nv2aVshOutput.
 * Programs without effective context writes dispatch ceil(n/64) groups.
 * Context writers use one GPU thread, preserving input vertex order and
 * constant visibility, then return final constants at u1 (192 float4s).
 * Runtime owns compilation, caching and buffer lifetime.
 * Returns source byte count excluding NUL, or zero and a diagnostic. */
int nv2a_vsh_context_writes(const uint32_t program[NV2A_VSH_SLOTS][4],
                            uint32_t start, int enabled);
size_t nv2a_vsh_translate(const uint32_t program[NV2A_VSH_SLOTS][4],
                          uint32_t start, int context_write,
                          char *source, size_t capacity,
                          char *error, size_t error_capacity);
#endif
