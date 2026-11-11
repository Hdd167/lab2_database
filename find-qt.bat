@echo off
rem Locates the Qt 6 kit (MinGW) together with the MinGW compiler shipped with
rem it and puts both on PATH. Called by compile.bat and run.bat.
rem Set QTDIR (and optionally MINGWDIR) before running to use another install.

if not defined QTDIR (
    for %%R in ("D:\Qt" "C:\Qt" "%USERPROFILE%\Qt" "%ProgramFiles%\Qt") do (
        for /d %%V in ("%%~R\6*") do (
            for /d %%K in ("%%~V\mingw*") do (
                if not defined QTDIR if exist "%%~K\bin\qmake.exe" set "QTDIR=%%~K"
            )
        )
    )
)

if not defined QTDIR (
    echo Qt 6 with a MinGW kit was not found.
    echo Install it with the Qt Maintenance Tool, or point QTDIR at your kit, e.g.
    echo     set QTDIR=C:\Qt\6.6.0\mingw_64
    exit /b 1
)

if not defined MINGWDIR (
    for /d %%M in ("%QTDIR%\..\..\Tools\mingw*") do (
        if not defined MINGWDIR if exist "%%~M\bin\g++.exe" set "MINGWDIR=%%~M"
    )
)

set "PATH=%QTDIR%\bin;%PATH%"
if defined MINGWDIR set "PATH=%MINGWDIR%\bin;%PATH%"

exit /b 0
