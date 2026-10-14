#ifndef NV2A_GPU_COMPUTE_H
#define NV2A_GPU_COMPUTE_H
#include <stddef.h>
#include <stdint.h>
#include "nv2a_vsh_interp.h"
/* Experimental hardware raster path. nv2a_gpu_draw is coherent per call;
 * the resident surface API defers coherence to the caller's sync points. */
typedef struct {
  float pos[3][4], color[3][4], uvw[3][4];
  uint32_t box[4];   /* min x/y, width/height */
  uint32_t tex[4];   /* width/height, address u/v */
  uint32_t blend[4]; /* enable, source factor, destination factor, equation */
  uint32_t state[4]; /* color mask, constant blend color, flat color, 3D */
  uint32_t depth[4]; /* test, func, write, samples x (0 = 1) */
  uint32_t alpha[4]; /* test, func, ref, samples y (0 = 1) */
  float color1[3][4], fog[3][4], stage_coords[3][4][4];
  uint32_t stage_info[4][4]; /* width, height, atlas offset, flags */
  uint32_t rcmeta[4]; /* enable, atlas texel count, clip-plane mode, reserved */
  uint32_t rc_color_icw[8], rc_alpha_icw[8], rc_color_ocw[8], rc_alpha_ocw[8];
  uint32_t rc_factor0[8], rc_factor1[8];
  uint32_t rc_final[4];   /* final0/1, c0/1 */
  uint32_t rc_control[4]; /* control, stage program, packed source formats,
                             reserved */
  uint32_t surf[4]; /* set by the backend: stride, origin x/y, zpass flag */
} Nv2aGpuParams;
typedef struct {
  uint32_t color;
  float depth;
  uint32_t covered, depth_fail;
} Nv2aGpuPixel;
int nv2a_gpu_enabled(void);
/* Translate once per program content, dispatch all vertices on hardware.
 * Returns zero for unsupported programs or device failures; callers preserve
 * the existing interpreter path until coverage has been established.
 * Successful context-writing programs update state->constants atomically
 * with outputs; callers must commit them before subsequent vertex batches. */
int nv2a_gpu_transform(Nv2aVshState *state, const Nv2aVshInput *input,
                       Nv2aVshOutput *output, size_t count);
/* Backend-owned reusable buffers; valid until the next growth or shutdown.
 * Single executor thread, like the D3D immediate context. Never free these. */
int nv2a_gpu_host_buffers(size_t pixels, size_t texels, Nv2aGpuPixel **input,
                          Nv2aGpuPixel **output, uint32_t **texture);
typedef struct {
  uint64_t draws, atlas_uploads, atlas_upload_skips, atlas_upload_bytes,
      host_buffer_grows, readback_bytes, full_capacity_bytes;
} Nv2aGpuStats;
void nv2a_gpu_get_stats(Nv2aGpuStats *stats);
int nv2a_gpu_draw(const Nv2aGpuParams *p, const Nv2aGpuPixel *input,
                  const uint32_t *texture, Nv2aGpuPixel *output);
/* Resident surface: one Nv2aGpuPixel per sample, row stride in samples,
 * kept on the GPU between draws. Draws update it in place in submission
 * order; nothing returns to the host until nv2a_gpu_surface_download.
 * Upload replaces the contents and zeroes the pending statistics. */
int nv2a_gpu_surface_upload(const Nv2aGpuPixel *pixels, size_t count);
/* texture NULL reuses the atlas uploaded by the previous draw unchanged.
 * Box origin, in samples: box.x * samples x, box.y * samples y. */
int nv2a_gpu_surface_draw(const Nv2aGpuParams *p, const uint32_t *texture,
                          uint32_t stride, int zpass);
/* stats: covered, depth-failed, zpass (only while zpass was requested). */
int nv2a_gpu_surface_download(Nv2aGpuPixel *pixels, size_t count,
                              uint64_t stats[3]);
void nv2a_gpu_fallback(const char *reason);
void nv2a_gpu_report(void);
#endif
