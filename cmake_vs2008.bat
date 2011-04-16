@echo off
echo.
SET QMAKESPEC=%TUNDRAQT47_DEP_PATH%\Qt\mkspecs\win32-msvc2008
SET QTDIR=%TUNDRAQT47_DEP_PATH%\Qt
cmake.exe -G "Visual Studio 9 2008"
echo.
pause
