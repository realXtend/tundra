@echo off
echo.

SET QTDIR=%NAALI_DEP_PATH%\Qt
SET QMAKESPEC=%QTDIR%\mkspecs\win32-msvc2008

cmake.exe -G "Visual Studio 9 2008"

echo.
pause
