/* Win32 for the game-logic files: <windows.h> on Windows, the runtime's POSIX
 * layer (xboxrecomp platform/win32_compat) elsewhere. The renderer files use
 * hle_d3d11_internal.h instead (DXVK native's headers on Linux). */
#ifndef HLE_WIN32_H
#define HLE_WIN32_H
#ifdef _WIN32
#include <windows.h>
#else
#include "platform/xbox_winnt.h"
#include "platform/win32_compat.h"
#endif
#endif
