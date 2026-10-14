cmake_minimum_required(VERSION 3.20)

# ============================================================================
# YOUR_GAME_NAME - Static Recompilation
#
# Copy this template to your game project root and customize:
#   1. Change the project name below
#   2. Set XBOXRECOMP_DIR to where you cloned xboxrecomp
#   3. Adjust source file lists as you add game-specific code
#   4. Add any game-specific compile definitions or link libraries
# ============================================================================

project(fifastreet_recomp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common definitions
add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# ── xboxrecomp toolkit ──────────────────────────────────────────
#
# Option A: xboxrecomp as a subdirectory (simplest)
#   Place xboxrecomp/ next to your game project and use a relative path.
#
# Option B: find_package (if you install xboxrecomp system-wide)
#   find_package(xboxrecomp REQUIRED)
#
# Adjust this path to point to your xboxrecomp clone:
set(XBOXRECOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../xboxrecomp" CACHE PATH
    "Path to the xboxrecomp toolkit root directory")

# Add xboxrecomp as a subdirectory -- this builds all 6 runtime libraries
# and provides the 'xboxrecomp' umbrella target to link against.
add_subdirectory(${XBOXRECOMP_DIR} ${CMAKE_BINARY_DIR}/xboxrecomp)

# ── Recompiled game executable ──────────────────────────────────

# Collect generated recompiled code.
# The recomp pipeline outputs files into src/recomp/gen/.
# With --split 1000, you get gen_0000.c, gen_0001.c, etc.
# CONFIGURE_DEPENDS so re-running the recompiler picks up added/removed files
# without a manual re-configure. A plain GLOB is evaluated once at configure
# time, so a newly generated file (an extra chunk, recomp_stubs_unresolved.c)
# is silently left out of the build and shows up as unresolved externals.
file(GLOB RECOMP_GEN_SOURCES CONFIGURE_DEPENDS "src/recomp/gen/*.c")

# Game logic: guest-side HLE, no host graphics API.
set(GAME_SOURCES
    src/main.c
    src/recomp_manual.c
    src/resource_trace.c
    src/hle_profile.c
    src/hle_d3d8.c
    src/hle_vsh.c
    src/hle_dsound.c
    src/hle_dsound_native.c
    src/hle_sync.c
    src/hle_native_api.c
    src/hle_native_resources.c
    src/hle_native_states.c
    src/hle_native_create.c
    src/game_hooks.c
    src/hle_nv2a_watch.c
    src/hle_device.c
)
# The renderer and the options screen: D3D11/DXGI (the system's or DXVK on
# Windows, DXVK native on Linux).
set(RENDERER_SOURCES
    src/hle_d3d11.c
    src/hle_draw.c
    src/hle_native_misc.c
    src/hle_native_capture.c
    src/hle_native_quality.c
    src/hle_native_present.c
    src/hle_native_logic.c
    src/hle_options.c
    src/hle_gpu_api.c
    src/hle_native_geometry.c
)

# The recomp pipeline also generates a dispatch header and function declarations.
# These are included by the generated .c files:
#   src/recomp/gen/recomp_funcs.h    - extern declarations for all recompiled functions
#   src/recomp/recomp_dispatch.h     - binary search dispatch table
#   src/recomp/recomp_types.h        - register model, ICALL macros, MEM helpers

if(WIN32)
    add_executable(${PROJECT_NAME}
        ${GAME_SOURCES}
        ${RENDERER_SOURCES}
        ${RECOMP_GEN_SOURCES}
    )
else()
    # Linux: the renderer is its own library -- its files see DXVK native's
    # Win32 headers, which cannot meet the runtime's win32_compat in one file.
    find_package(SDL2 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    set(DXVK_NATIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/dxvk-native/usr" CACHE PATH
        "DXVK native release (include/dxvk, lib/libdxvk_d3d11.so, lib/libdxvk_dxgi.so)")
    set(VKD3D_DIR "/opt/vkd3d-inst" CACHE PATH "vkd3d (libvkd3d-shader: HLSL to DXBC for the shaders)")
    add_library(fifa_renderer STATIC ${RENDERER_SOURCES} src/hle_posix_win32.c src/hle_gdi_posix.c)
    target_include_directories(fifa_renderer BEFORE PRIVATE
        ${DXVK_NATIVE_DIR}/include/dxvk
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/recomp
        ${XBOXRECOMP_DIR}/src/d3d
        ${SDL2_TTF_INCLUDE_DIRS}
        ${VKD3D_DIR}/include/vkd3d)
    target_compile_definitions(fifa_renderer PRIVATE DXVK_WSI_SDL2)
    target_compile_options(fifa_renderer PRIVATE -fms-extensions -Wno-microsoft-anon-tag)
    target_link_libraries(fifa_renderer PUBLIC
        SDL2::SDL2 ${SDL2_TTF_LINK_LIBRARIES}
        ${DXVK_NATIVE_DIR}/lib/libdxvk_d3d11.so ${DXVK_NATIVE_DIR}/lib/libdxvk_dxgi.so
        ${VKD3D_DIR}/lib/libvkd3d-shader.so)

    add_executable(${PROJECT_NAME}
        ${GAME_SOURCES}
        ${RECOMP_GEN_SOURCES}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE fifa_renderer m dl pthread)
    # The generated code is machine output: its warnings are noise.
    set_source_files_properties(${RECOMP_GEN_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")
    # DXVK native and vkd3d-shader travel next to the executable (lib/), with
    # a sans-serif font for the options screen (fonts/) when the build
    # machine has DejaVu: the folder runs as it is on another Linux machine.
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN/lib"
        INSTALL_RPATH "$ORIGIN/lib")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DXVK_NATIVE_DIR}/lib/libdxvk_d3d11.so.0" "${DXVK_NATIVE_DIR}/lib/libdxvk_dxgi.so.0"
                "${VKD3D_DIR}/lib/libvkd3d-shader.so.1"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib/")
    foreach(_font DejaVuSans.ttf DejaVuSans-Bold.ttf DejaVuSans-BoldOblique.ttf)
        if(EXISTS "/usr/share/fonts/truetype/dejavu/${_font}")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "/usr/share/fonts/truetype/dejavu/${_font}"
                        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts/")
        endif()
    endforeach()
endif()

# Link against the xboxrecomp umbrella target (all 6 runtime libraries)
target_link_libraries(${PROJECT_NAME} PRIVATE xboxrecomp)

# Include paths for generated recomp headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/recomp
    ${XBOXRECOMP_DIR}/include
    ${XBOXRECOMP_DIR}/src/d3d
)

if(WIN32)
# Windows SDK libraries needed at link time
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    d3dcompiler
    dxgi
    dxguid
    xinput
    winmm
    # Lets the fault handler resolve the host RIP back to the generated
    # function's symbol, i.e. to the guest function that faulted.
    dbghelp
)
endif()

# MSVC-specific: large address aware, increase section limit for big generated files
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        /LARGEADDRESSAWARE
        # Bound on first use, not at load: with Vulkan chosen, hle_gpu_api.c
        # loads DXVK's d3d11.dll/dxgi.dll first and every call goes there.
        /DELAYLOAD:d3d11.dll
        /DELAYLOAD:dxgi.dll
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE delayimp)
    # The generated code files can be very large. Increase object file section limit.
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

# ── Game-specific additions ─────────────────────────────────────
#
# Add any game-specific libraries, loaders, or custom renderers below.
# Examples:
#
# # Custom texture loader for your game's format
# target_sources(${PROJECT_NAME} PRIVATE src/texture_loader.c)
#
# # If your game uses RenderWare, you may need a RW bridge layer
# target_sources(${PROJECT_NAME} PRIVATE src/rw_bridge.c)
#
# # Additional compile definitions for debugging
# target_compile_definitions(${PROJECT_NAME} PRIVATE
#     ENABLE_ICALL_TRACE=1
#     VERBOSE_KERNEL_LOG=1
# )

# D3D8 HLE survey: route every direct call through recomp_call_profile_hit
# (src/hle_profile.c). Slower; use a separate build directory.
option(FIFA_CALL_PROFILE "Build with RECOMP_CALL_PROFILE (HLE survey)" OFF)
if(FIFA_CALL_PROFILE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RECOMP_CALL_PROFILE)
endif()

# Vulkan: DXVK (D3D11/DXGI on Vulkan) in vulkan/ next to the executable.
set(DXVK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/dxvk-3.1.1" CACHE PATH
    "DXVK release (x64/d3d11.dll, x64/dxgi.dll) for the Vulkan option")
if(WIN32 AND EXISTS "${DXVK_DIR}/x64/d3d11.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/vulkan"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXVK_DIR}/x64/d3d11.dll" "${DXVK_DIR}/x64/dxgi.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/vulkan/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXVK_DIR}/LICENSE"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/vulkan/LICENSE.dxvk.txt")
endif()
