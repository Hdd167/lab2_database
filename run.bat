@echo off
setlocal
cd /d "%~dp0"

call "%~dp0find-qt.bat"
if errorlevel 1 exit /b 1

if not exist "build\ProductDatabase.exe" (
    echo build\ProductDatabase.exe not found - run compile.bat first.
    pause
    exit /b 1
)

start "" "build\ProductDatabase.exe"
