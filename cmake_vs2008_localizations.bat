@echo off
echo.
SET QMAKESPEC=%NAALI_DEP_PATH%\Qt\mkspecs\win32-msvc2008
SET QTDIR=%NAALI_DEP_PATH%\Qt
cmake.exe -G "Visual Studio 9 2008" -DUPDATE_LANGUAGE_TRANSLATIONS:BOOL=TRUE
echo.
pause
