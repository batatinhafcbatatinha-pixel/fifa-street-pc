@echo off
setlocal
if exist "%~dp0..\..\..\recomp-python\Scripts\pythonw.exe" (
  start "" "%~dp0..\..\..\recomp-python\Scripts\pythonw.exe" "%~dp0fifa_installer.py"
  exit /b
)
if exist "%~dp0..\..\..\..\recomp-python\Scripts\pythonw.exe" (
  start "" "%~dp0..\..\..\..\recomp-python\Scripts\pythonw.exe" "%~dp0fifa_installer.py"
  exit /b
)
where pyw >nul 2>nul
if not errorlevel 1 (
  start "" pyw -3 "%~dp0fifa_installer.py"
  exit /b
)
where pythonw >nul 2>nul
if not errorlevel 1 (
  start "" pythonw "%~dp0fifa_installer.py"
  exit /b
)
echo Python 3 com Tkinter nao encontrado. Instale Python e abra novamente.
pause
