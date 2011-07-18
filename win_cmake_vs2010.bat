@rem Add -DUPDATE_LANGUAGE_TRANSLATIONS:BOOL=TRUE to cmake parameters to generate translations
@echo off

::SET TUNDRA_DEP_PATH=<path_to_vc10_build_deps>
SET QTDIR=%TUNDRA_DEP_PATH%\Qt
SET QMAKESPEC=%QTDIR%\mkspecs\win32-msvc2010

cmake.exe -G "Visual Studio 10"

echo.
pause
