@echo off
setlocal
cd /d "%~dp0"
if not defined HSP_DIR set "HSP_DIR=C:\hsp37"
"%HSP_DIR%\hspcmp.exe" --compath="%HSP_DIR%\common\" -d sample.hsp
if errorlevel 1 exit /b 1
"%HSP_DIR%\hsp3gp.exe" sample.ax %*
