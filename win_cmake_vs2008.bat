@echo off

:check_vs2008_deps
if exist "deps\vs2008\README" (
    goto :set_env_variables
) else (
    echo ERROR: Could not find 'deps\vs2008\README', assuming git submodule has not been fetched yet.
    echo ERROR: Please run win_update_deps_vs2008.bat to fetch the dependencies!
    goto :end
)

:set_env_variables
SET TUNDRA_DEP_PATH=%CD%\deps\vs2008\build_deps
SET QTDIR=%TUNDRA_DEP_PATH%\Qt
SET QMAKESPEC=%QTDIR%\mkspecs\win32-msvc2008
goto :run_cmake

:run_cmake
cmake.exe -G "Visual Studio 9 2008" 
::cmake.exe -G "Visual Studio 9 2008" -DUPDATE_LANGUAGE_TRANSLATIONS:BOOL=TRUE
goto :end

:end
echo.
pause
