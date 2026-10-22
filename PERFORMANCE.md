# Renderer update — 2026-10-22

The native renderer now avoids rebinding unchanged D3D11 state and uploading
identical constant buffers. Changed textures with the same dimensions, mip
count and format receive new texels in place. Uncompressed texture conversion
selects the format outside the pixel loop; YUV video converts pixel pairs with
shared chroma arithmetic. Pixel calculations, rendering resolution and game
timing defaults are preserved.

## Validation

Windows, 1920×1080 internal resolution, VSync off, frame limit off, monitor
index 1 (75 Hz). A 20-second warm-up preceded a 160-second run. Six consecutive
pairs of 600-frame gameplay windows alternated the state/constant-buffer
caches and texture reuse within the same demo match:

| Measurement | Optimizations off | Optimizations on |
|---|---:|---:|
| Mean frame time | 7.250 ms | 6.577 ms |
| Throughput | 137.93 fps | 152.05 fps |
| Draw cost per call | 4.247 µs | 3.723 µs |
| Worst frame in selected windows | 38.1 ms | 29.2 ms |
| Frames over 20 ms | 4 / 3600 | 5 / 3600 |

Draw cost fell 12.34%; measured throughput rose approximately 10.2%. These are
results from one machine and one demo match. They do not establish the same
gain in every scene, resolution or graphics backend. Uncapped mode advances
this game's simulation faster; the normal 60-fps default remains available.

Both production converters matched the pre-update native converter exactly
in 798 synthetic cases each, including odd YUV widths, missing palettes,
padded rows and output guards. Existing captures cover intro, title,
gameplay and replay with correct scene geometry and HUD. The updated Windows
executable compiled and completed the runtime test without unresolved native
calls or hardware aperture accesses. Publication audit and five installer
tests passed. Linux and the European release were not rerun for this update.

## Diagnostics

- `RECOMP_HLE_PROFILE=1`: timings and CPU use every 600 frames.
- `RECOMP_HLE_AB=1` with profiling: alternate optimization modes each window.
- `RECOMP_HLE_NO_STATE_CACHE=1`: rebind state every draw.
- `RECOMP_HLE_NO_CB_SKIP=1`: upload constants even when unchanged.
- `RECOMP_HLE_NO_TEX_REUSE=1`: recreate changed textures.
- `RECOMP_HLE_MONITOR=1`: select the second monitor for tests.
- `RECOMP_HLE_FPS=0` and `RECOMP_HLE_VSYNC=0`: uncapped throughput testing.

Per-draw clock queries run only while profiling. CPU accounting uses the
Windows or POSIX clock APIs for the selected platform. No new post-processing
toggles were added: the depth-of-field and blur paths still require an
independently verified control before exposing them in F1.
