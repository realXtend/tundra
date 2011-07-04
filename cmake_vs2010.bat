@echo off
echo.

::SET NAALI_DEP_PATH=<path_to_vc10_build_deps>
SET QTDIR=%NAALI_DEP_PATH%\Qt
SET QMAKESPEC=%QTDIR%\mkspecs\win32-msvc2010

cmake.exe -G "Visual Studio 10"

echo.
pause
