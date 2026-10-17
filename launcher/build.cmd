@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
rc /nologo /fo launcher.res launcher.rc || exit /b 1
"C:\Program Files\LLVM\bin\clang-cl.exe" /nologo /O2 /utf-8 -D_CRT_SECURE_NO_WARNINGS /c launcher.c /Folauncher.obj || exit /b 1
link /nologo launcher.obj launcher.res /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup /OUT:"Play FIFA Street.exe" user32.lib kernel32.lib
