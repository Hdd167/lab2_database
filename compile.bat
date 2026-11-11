@echo off
setlocal
cd /d "%~dp0"

call "%~dp0find-qt.bat"
if errorlevel 1 (
    echo.
    pause
    exit /b 1
)

if not exist build mkdir build
cd build

"%QTDIR%\bin\qmake.exe" ..\ProductDatabase.pro -spec win32-g++
if errorlevel 1 goto failed

mingw32-make
if errorlevel 1 goto failed

echo.
echo Compilation successful: build\ProductDatabase.exe
echo Start it with run.bat
pause
exit /b 0

:failed
echo.
echo Compilation failed.
pause
exit /b 1
