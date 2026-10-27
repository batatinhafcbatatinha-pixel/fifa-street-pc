# Redistribution and provenance

This is a source and tooling distribution. No original game assets, executable, generated translation units, disassembly, SDK binaries or compiled game executable are included. The public address seeds and hook metadata describe functional addresses and calling conventions, not instruction bodies.

The independently written native integration and build tooling are offered under the root MIT license. Existing attribution and upstream licenses take precedence for imported files. This license grants no rights to Electronic Arts game content.

`xboxrecomp/` is a submodule: the fork batatinhafcbatatinha-pixel/xboxrecomp, branch `ports`, a modified version of sp00nznet/xboxrecomp revision `193e2995ffaec871bb3be43a43bd66aa5fccfeb1`. Original copyright headers, LICENSE, NOTICE, CONTRIBUTORS and component license texts are retained. Its modifications are the platform/runtime work used by these ports; they are not part of the upstream project.

The runtime contains LGPL components derived from xemu and GPL-2.0-or-later DSP interpreter/debug sources derived through xemu from Hatari/ARAnyM. These files keep those licenses even when the native profile does not execute them. See `xboxrecomp/src/apu/dsp/UPSTREAM.md`, `COPYING.GPL-2.0`, `COPYING.LGPL-2.1`, and individual headers. A future binary distribution must independently satisfy all applicable licensing obligations; this source release does not authorize distribution of proprietary game translations.

DXVK, vkd3d-shader, DejaVu fonts, Capstone and Microsoft tools are external dependencies. Included license texts acknowledge the dependencies; their binaries are not included here.

The publication audit uses an allowlist and rejects executable/image/audio/archive/game-resource files and generated output. It is an engineering safeguard, not a legal guarantee about every jurisdiction or use. Do not submit game content or private build artifacts in pull requests or issues.
