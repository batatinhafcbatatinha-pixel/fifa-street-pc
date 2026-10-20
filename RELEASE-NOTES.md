Experimental native PC port source kit for FIFA Street (2005, original Xbox USA and Europe).

Includes the hardware-free native profile (Direct3D 8, DirectSound and XInput reimplemented in C; no emulated NV2A, APU/DSP, AC97, USB or flash), settings in English and Portuguese, the modified xboxrecomp toolkit, third-party notices, and tools to generate and build privately from your own supported USA or European game files.

**This ZIP contains source and tooling only. It is not a ready-to-play game or precompiled port.** No ISO, XBE, EA game resources, translated game source or compiled game executable is included. Read README.md for requirements and build instructions.

**New: keyboard play.** The keyboard plays as an Xbox controller with EA SPORTS FC 27's default WASD layout (WASD move, L pass, ; shoot, K lob/cross, O through ball, P sprint, Esc pause; Enter passes PRESS START).

The previous version added the European release (English / French / German, detected by its executable hash; addresses moved with street1/regions) and fixes the local generation: function boundaries now follow the tested build's function set, so the generated game boots to the menus and plays instead of crashing at boot. Rebuild from scratch if you built an earlier version.

The reconstruction pipeline and Linux build path are experimental. Do not assume a newly generated build has the same gameplay behavior as the previously tested private build. Credits and licenses are in README.md, NOTICE.md and the preserved upstream notices.
