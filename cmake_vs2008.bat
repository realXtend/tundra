@echo off
echo.
SET QTDIR=%NAALI_DEP_PATH%\Qt
cmake.exe -G "Visual Studio 9 2008"
echo.
pause
