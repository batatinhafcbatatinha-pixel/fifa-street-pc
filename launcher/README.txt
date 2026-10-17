FIFA Street (2005) - PC Port
============================

A native PC version of FIFA Street (Xbox, 2005), made by static
recompilation. It is not an emulator: the game's code was translated to
run directly on your PC.

- Native graphics: the game's Direct3D calls run on DirectX 11 or Vulkan.
  The Xbox graphics chip is not emulated.
- Native audio: the game's sound system was rewritten for PC. No emulated
  audio chip, no crackling.
- Any resolution, from 720p up to 8K, or your monitor's own (4K runs at a
  steady 60 fps on a mid-range GPU).
- Real widescreen 16:9: uses the game's own widescreen mode, so the camera,
  the HUD and what you see on the sides are all right.
  (16:10, 21:9 and 32:9 screens get the 16:9 picture with black bars.)
- In-game settings (press F1).
- Windows and Linux.


YOU NEED THE GAME
-----------------
This package does not include the game. You need your own copy of
FIFA Street for the original Xbox (USA version).

Copy everything from the game disc into the "game_files" folder, so that
the file "default.xbe" ends up here:

    game_files\default.xbe

(The disc also has a "data" folder; it goes inside game_files too.)


WINDOWS
-------
Needs: Windows 10 or 11, 64-bit, and a graphics card with DirectX 11.
       "N" editions of Windows also need the Media Feature Pack.

1. Extract the whole zip to a folder (not inside Program Files).
2. Copy the game files into "game_files" (see above).
3. Double-click "Play FIFA Street.exe".

If Windows SmartScreen warns about the file, click "More info" and then
"Run anyway".


LINUX
-----
Needs: a 64-bit PC, a recent distro (Ubuntu 24.04 or newer, Fedora 39 or
newer...), and working Vulkan drivers for your graphics card.

1. Install the libraries the game uses. On Ubuntu / Debian:
       sudo apt install libsdl2-2.0-0 libsdl2-ttf-2.0-0 libepoxy0 libssl3 mesa-vulkan-drivers
   (NVIDIA users: the NVIDIA driver already brings Vulkan.)
2. Extract the whole zip to a folder.
3. Copy the game files into "game_files" (see above).
4. Open a terminal in that folder and run:
       sh play-fifa-street.sh


MAC
---
Not supported for now.


CONTROLS
--------
Any Xbox-style controller works (Xbox 360 / One / Series and most others).

Keyboard:
  Arrows            D-pad
  Numpad 8/4/2/6    Left stick
  I / J / K / L     Right stick
  Z X A S           A B X Y buttons
  Q / E             White / Black buttons
  1 / 3             Left / Right trigger
  Enter or Esc      Start (pause)
  Backspace         Back
  F1                Settings
  Alt+Enter         Window / fullscreen


SETTINGS (F1)
-------------
F1 on the keyboard, or Back+Start on the controller, opens the settings at
any time. You can also open them from the "PC SETTINGS" button in the main
menu and in the pause menu. Settings default to English; "Menu language"
switches between English and Portuguese immediately. Game language is separate.

You can change: window or fullscreen, monitor, refresh rate, VSync, frame
limit, resolution, aspect ratio, anti-aliasing (FXAA), texture filtering,
graphics API (DirectX 11 or Vulkan, Windows only), graphics card, game
language (English or Spanish) and volume.

Items marked with * only change after a restart; the menu can restart the
game for you. Settings are saved in fifastreet.ini, next to the game's exe.


PROBLEMS
--------
- The game does not open: check that game_files\default.xbe exists.
- Screen tearing: turn VSync on in the settings.
- Something looks wrong with DirectX 11: try Vulkan in the settings
  (or the other way around).
- If the game closes, the "logs" folder has the details. Send them along
  if you report a bug.


THIRD-PARTY SOFTWARE
--------------------
- DXVK (Vulkan for DirectX 11) - zlib license - licenses\DXVK-LICENSE.txt
- vkd3d-shader (Linux shader compiler) - LGPL 2.1 -
  licenses\vkd3d-COPYING.txt; source: https://gitlab.winehq.org/wine/vkd3d
- DejaVu fonts (Linux settings menu) - licenses\DejaVu-fonts-LICENSE.txt
- Microsoft Visual C++ runtime (vcruntime140.dll)

FIFA Street is a trademark of Electronic Arts. This project is not made or
approved by EA or Microsoft.
