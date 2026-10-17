@echo off
setlocal
where py >nul 2>nul
if errorlevel 1 (
  python "%~dp0fifa_launch.py"
) else (
  py -3 "%~dp0fifa_launch.py"
)
if errorlevel 1 pause
