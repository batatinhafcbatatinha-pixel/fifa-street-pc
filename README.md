# FIFA Street (2005) — Native PC Port | No Emulated Xbox Hardware

> [!WARNING]
> This repository does **not** include the game, ROM/ISO/XBE, EA assets or a precompiled executable.
> You need your own original-Xbox USA or European disc. See [the full notice](#notice) at the end.

> [!TIP]
> **New: play without a controller.** The keyboard now plays as an Xbox controller, with the basic actions of **EA SPORTS FC 27's default WASD layout**: WASD to move, **L** pass, **;** shoot, **K** lob, **O** through ball, **P** sprint, **Esc** pause. See [Keyboard controls](#keyboard-controls).

An experimental original-Xbox FIFA Street port built through static recompilation, with native PC graphics and audio backends. **This public repository is a source and local-build kit, not a ready-to-play game download.** Game instructions are translated locally from your own verified USA or European disc. No game executable or generated game code is distributed here.

## Features

- Native PC execution of statically recompiled game code.
- Hardware-free by default (`RECOMP_NO_HW=1` in every launcher): no model of the NV2A GPU, MCPX APU/DSP, AC97, USB/OHCI or flash is started. Their address ranges are only reserved, and any access is logged as `[NO-HW]` (expected count: 0).
- Native rendering: Direct3D 8 reimplemented in C on DirectX 11 or Vulkan (DXVK), selectable on Windows; Vulkan through DXVK native on Linux.
- Native audio: DirectSound reimplemented in C with its own mixer, output through XAudio2 (Windows) or SDL2 (Linux).
- Native controllers: the XInput API reimplemented in C on host pads (XInput/SDL) and the keyboard; no USB stack.
- Game threads run in parallel on the host CPU, with heap and stream code serialized (`RECOMP_GUEST_SMP=1`).
- Windows x64 and Linux x64 build paths. Experimental [macOS](MACOS.md) build files are included; they have not been compiled or run yet.
- Resolution presets from 720p to 8K, or your monitor's resolution. Performance depends on hardware, scene and build; 4K/60 is not a universal guarantee.
- The game's own 16:9 widescreen mode; other display shapes use letterboxing. Additional aspect settings remain experimental.
- In-game settings with **F1**: display mode, monitor, refresh rate, VSync, frame limit, resolution, aspect ratio, graphics API, GPU, game language (English / Spanish on the USA disc; English / French / German on the European disc), FXAA, MSAA (off, 2x/4x/8x or the 2x the Xbox game asks for), anisotropic filtering and volume.
- Settings default to **English**, with Portuguese available through **Menu language**. The settings language is separate from game language.
- Shader caching, specialized pixel shaders and native audio/frame-pacing improvements. Visual, audio, gameplay and save regressions may remain.

## Why a native port instead of an emulator

An emulator such as [xemu](https://github.com/xemu-project/xemu) recreates the Xbox itself: it interprets or JIT-translates the console's CPU code, and models the NV2A GPU, the MCPX audio DSP and the USB controllers register by register. That is the right approach for running the whole Xbox library accurately. This project takes a different route for one game:

- **No emulated hardware in the hot path.** The game's x86 code is translated ahead of time into C and compiled by a modern optimizing compiler. Direct3D 8, DirectSound and XInput calls go straight to C implementations on DirectX 11/Vulkan, XAudio2/SDL2 and host pads, instead of being lowered to push-buffer commands, DSP microcode and USB packets that are then emulated again.
- **Low CPU requirements.** In our tests on a Ryzen 5 3600 / RTX 3050 Laptop, a match stays at 60 fps even with the process pinned to a single physical core (2 logical threads), with no frame over 50 ms. Match loading takes about 3.5–4 s.
- **Resolution and widescreen beyond the console.** Rendering is done by the PC GPU at any internal resolution (720p up to 8K, or the monitor's), with the game's widescreen mode and ultrawide letterboxing. With specialized pixel shaders, 4K ran at 60 fps with the GPU at about a third of its capacity in our tests.
- **PC-side improvements in one place.** Shader caching, frame pacing, exclusive fullscreen, FXAA, anisotropic filtering, keyboard controls and an in-game settings menu are part of the port, not emulator-wide options.
- **Readable and moddable.** Each part of the game's behaviour exists as C source in your local build. Individual functions can be inspected, replaced with hand-written native versions, or fixed directly.

The trade-off: this approach only covers this one game (USA and European releases), and it is still experimental. An emulator is the more general and better-tested option for the Xbox library as a whole. The figures above come from a single test machine and are not guarantees.

The source tree still contains the older Xbox hardware models (NV2A, APU/DSP, OHCI) used for diagnostics and comparison; the default launch profile does not start any of them. The Xbox kernel and memory layout remain as compatibility layers. The Linux build has been tested under WSL2 (software Vulkan); real Linux GPUs, physical controllers and exclusive fullscreen there are untested.

## Download or clone

```sh
git clone --recursive https://github.com/batatinhafcbatatinha-pixel/fifa-street-pc.git
cd fifa-street-pc
```

The xboxrecomp toolkit is the `xboxrecomp/` submodule ([batatinhafcbatatinha-pixel/xboxrecomp](https://github.com/batatinhafcbatatinha-pixel/xboxrecomp), branch `ports`). If you cloned without `--recursive`, run `git submodule update --init`.

Alternatively, download the **source-kit ZIP** from [Releases](https://github.com/batatinhafcbatatinha-pixel/fifa-street-pc/releases), which includes the toolkit. Extract the entire archive. GitHub's **Code → Download ZIP** does not include the submodule. These downloads contain source and tools only; they do not contain a precompiled game.

The GitHub Actions workflow audits the publication allowlist, tests ISO extraction, and creates a source-kit ZIP on release tags. It never receives a ROM or builds translated game code. Standard hosted runners are [free for public repositories](https://docs.github.com/en/billing/concepts/product-billing/github-actions); the workflow uses a standard Ubuntu runner and publishes the ZIP as a release asset.

## Your game files

Two original-Xbox releases are accepted: the **USA** disc (English / Spanish) and the **European** disc (English / French / German). Xbox 360 FIFA Street titles are not supported.

The European executable is the same build of the game as the USA one, relinked: code and data sit at different addresses and about 90 functions changed. The build detects which executable you have by its hash. For the European one it moves every USA address used by the native sources (D3D, DirectSound, XInput, hooks, routing tables) to its European counterpart using [`street1/regions/europe.json`](street1/regions/europe.json), and translates the European executable with its own function list. Those tables contain only addresses, never game code; they were made with [`street1/regions/make-region-table.py`](street1/regions/make-region-table.py) from both executables, which checks that every moved function is byte-identical apart from pointers. The European release runs at 60 Hz like the USA one, and the game language follows the setting (only languages whose files are on your disc are offered).

Place your own extracted disc contents in this layout:

```text
fifa-street-pc/
  game_files/
    default.xbe
    data/
    ...other original disc files...
```

Accepted `default.xbe` SHA-256:

```text
USA     f43dcca6fbd99e7a04dc94992443b368eefaa9d6bbc88432bf43b2d90e115ef1
Europe  8a67bfb82f8a7b99c957c17803a5626b5f27f93dfff4bc32ea3815941aa220a0
```

If you have an original-Xbox XDVDFS ISO instead, inspect and extract it locally into a **new** directory:

```sh
python street1/project/tools/fifa_setup.py inspect --iso "/path/to/your-disc.iso"
python street1/project/tools/fifa_setup.py install --iso "/path/to/your-disc.iso" --workspace ".local/disc" --extract-only
```

Use `.local/disc/game_files` instead of `game_files` in the build and launch commands below. Do not upload the ISO, extracted files, local analysis output, generated C, caches or compiled game executable to GitHub.

## Build on Windows

Install Python 3.10+ (with Tkinter), Visual Studio 2022 Build Tools with the C++ workload and Windows SDK, LLVM/Clang, CMake and Ninja. The scripts discover installed tools; they do not install system dependencies.

From a terminal in the repository:

```powershell
python -m pip install -r requirements.txt
python tools/build_local.py --game-files game_files
python tools/play.py --game-files game_files
```

The first build disassembles your XBE, identifies functions, translates them into private C files, installs the native hooks and compiles the executable. Function boundaries follow the set used by the tested private build ([`street1/function-set.json`](street1/function-set.json), addresses only): builds made with earlier versions of this kit split some functions differently and crashed during boot, so regenerate from scratch (without `--reuse-generated`) if you built before. It can take considerable time and memory. Build products and private game translations stay under `.local/`, which is excluded from version control.

After a completed generation, `--reuse-generated` skips the translation phase when rebuilding native sources. If a build fails, keep the console output for diagnosis. This local reconstruction pipeline is experimental and should not be assumed identical to a previously tested private build.

DirectX 11 is the default Windows backend. To enable Vulkan, obtain DXVK from its [official releases](https://github.com/doitsujin/dxvk/releases) and place the x64 `d3d11.dll` and `dxgi.dll` in `.local/build/vulkan/` before selecting Vulkan in settings. They are not bundled in this source kit. Windows N editions may need the Media Feature Pack for video playback.

## Build on Linux

Install Python 3.10+, Clang, CMake, Ninja, SDL2, SDL2_ttf, Vulkan drivers and development headers, DXVK native, and vkd3d-shader with HLSL compilation support. Get those dependencies from your distro or their upstream projects; prebuilt shared libraries are not bundled here.

```sh
python3 -m pip install -r requirements.txt
python3 tools/build_local.py --game-files game_files \
  --dxvk-native-dir /path/to/dxvk-native/usr \
  --vkd3d-dir /path/to/vkd3d-install
python3 tools/play.py --game-files game_files
```

The DXVK prefix must contain `include/dxvk` and `lib/libdxvk_d3d11.so`, `lib/libdxvk_dxgi.so` (including their `.so.0` files). The vkd3d prefix must contain the headers and `lib/libvkd3d-shader.so.1`. Mac support is not provided.

## Build on macOS

Experimental and not compiled or run yet: see [MACOS.md](MACOS.md).

## Keyboard controls

No controller needed: the keyboard plays as an Xbox controller, with the keys of the default **WASD** layout of EA SPORTS FC 27 ([EA's layout chart](https://help.ea.com/en/articles/ea-sports-fc/mouse-keyboard/)) on the buttons FIFA used on the original Xbox. Only the basic actions are mapped for now.

| Key | In a match | In menus | Controller button |
| --- | --- | --- | --- |
| **W A S D** | Move the player | Navigate | Left stick |
| **Arrows** | | Navigate | D-pad |
| **L** | Pass | Confirm | A |
| **;** (**Ç** on ABNT2 keyboards) | Shoot | Back | B |
| **K** | Lob pass / cross | | X |
| **O** | Through pass | | Y |
| **P** | Sprint | | Right trigger |
| **Enter** | | Confirm, passes *PRESS START* | A + Start |
| **Esc** | Pause | | Start |
| **Backspace** | | Back | B |
| **Tab** | | | Back |

The keyboard drives player 1 and is merged with a controller plugged in there, so both work at the same time. Tricks (right stick), the white/black buttons and the left trigger are not mapped on the keyboard yet. The launcher turns the keyboard on (`RECOMP_KEYBOARD=1`); set `RECOMP_KEYBOARD=0` to turn it off.

## Controls

Xbox-style controllers use the standard game controls. **F1** or **Back + Start** opens PC settings. **Alt + Enter** switches fullscreen.

The PC settings screen is drawn in the game's own style (orange title, white brush frame, yellow selection band) with the game's fonts, FreeStyle European and Euromode. They are read from your disc (`data/aptanims/aptfntz.big`) when the game starts and are not part of this repository. On the main menu and the Options screens a **BACK · PC SETTINGS** hint shows that the controller's Back button opens it there. The menu is in English by default; *Menu language* switches it to Portuguese.

Settings are saved in `fifastreet.ini` beside the locally built executable. Options marked with `*` require a restart. Logs go under `.local/logs/`.

## Credits and licenses

- Port integration and native backend work: [batatinhafcbatatinha-pixel](https://github.com/batatinhafcbatatinha-pixel).
- [xboxrecomp](https://github.com/sp00nznet/xboxrecomp): **Ned Heller / sp00nz**, and the contributors listed in the preserved [CONTRIBUTORS.md](xboxrecomp/CONTRIBUTORS.md). The modified copy used here is the fork [batatinhafcbatatinha-pixel/xboxrecomp](https://github.com/batatinhafcbatatinha-pixel/xboxrecomp) (branch `ports`, based on revision `193e2995ffaec871bb3be43a43bd66aa5fccfeb1`), with its original MIT license and notices.
- [xemu](https://github.com/xemu-project/xemu): **espes, Jannik Vogel, Matt Borgerson** and contributors for the runtime components identified in [xboxrecomp/NOTICE](xboxrecomp/NOTICE). LGPL and GPL sources retain their original notices and license texts. DSP interpreter lineage also credits the **ARAnyM team** and **Thomas Huth / Hatari**; see [DSP provenance](xboxrecomp/src/apu/dsp/UPSTREAM.md).
- [DXVK / DXVK native](https://github.com/doitsujin/dxvk): Philip Rebohle and contributors, zlib license. Optional external dependency; license reproduced in `licenses/`.
- [vkd3d-shader](https://gitlab.winehq.org/wine/vkd3d): the Wine project and contributors, LGPL; optional Linux dependency.
- [DejaVu fonts](https://dejavu-fonts.github.io/): DejaVu contributors and Bitstream; see the preserved font license. Optional system fonts on Linux.
- [Capstone](https://www.capstone-engine.org/): Nguyen Anh Quynh and contributors; external local-build dependency with its own license.
- Microsoft Visual C++ runtime and Windows SDK: Microsoft; obtained separately, not distributed in this repository.
- FIFA Street, its game code, trademarks and assets belong to Electronic Arts and the respective rights holders. This project is not affiliated with or endorsed by EA or Microsoft.

See [NOTICE.md](NOTICE.md) for the publication boundary and [LICENSE](LICENSE) for the license covering the port's own native source and tooling. Upstream components keep their original licenses; a root license does not relicense them or the game.

---

<a id="notice"></a>

**⚠️ THIS PROJECT DOES NOT INCLUDE THE GAME, A ROM/ISO/XBE, EA GAME ASSETS, GENERATED GAME CODE OR A PRECOMPILED GAME EXECUTABLE. YOU MUST PROVIDE YOUR OWN LAWFULLY OBTAINED ORIGINAL-XBOX USA OR EUROPEAN GAME FILES AND HAVE THE RIGHTS REQUIRED FOR YOUR USE. THIS PROJECT DOES NOT AUTHORIZE COPYRIGHT INFRINGEMENT. ALL GAME RIGHTS BELONG TO THEIR RESPECTIVE OWNERS. ⚠️**
