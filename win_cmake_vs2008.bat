@echo off
echo.

:: This batch script will determine TUNDRA_DEP_PATH and run cmake for Visual Studio 2008
::   1. If TUNDRA_DEP_PATH is defined on the system, it will proceed with your dep path.
::   2. If TUNDRA_DEP_PATH is not defined it will check if the deps git submodule has been fetched.
::      - If deps are present set TUNDRA_DEP_PATH to point into that and run cmake.
::      - If deps are not present prints insructions how to automatically fetch them.
::
:: Note: The suggested way of going about this is using the win_update_deps_vs2008.bat
:: This is because it will also copy all the needed DLLs into the /bin folder so you wont have
:: to do this manually. You can always run this .bat again to update the dependencies if changes
:: has happened in the git submodule. Be in contact if you have problems with this work flow.

:check_tundra_dep_path
echo Checking for TUNDRA_DEP_PATH on the system 
if NOT DEFINED TUNDRA_DEP_PATH (
    echo -- Not found
    goto :check_submodule_vs2008_deps 
) else (
    echo -- Found
    goto :set_env_variables
)

:check_submodule_vs2008_deps
echo Checking git submodule deps/vs2008 for dependecies
if exist "deps\vs2008\README" (
    echo -- Found
    SET TUNDRA_DEP_PATH=%CD%\deps\vs2008\build_deps
    goto :set_env_variables
) else (
    echo -- Not Found
    echo    Could not find 'deps\vs2008\README', assuming git submodule has not been fetched yet.
    echo    Please run win_update_deps_vs2008.bat to fetch the dependencies!
    echo.
    echo    If you would prefer getting dependencies manually with git, do a git clone from
    echo    https://code.google.com/p/realxtend-tundra-deps/ remember to checkout the prebuild-vs2008 branch.
    echo    Next set TUNDRA_DEP_PATH to point at your <clone_path>\build_deps and run this script again.
    goto :end
)

:set_env_variables
SET QTDIR=%TUNDRA_DEP_PATH%\Qt
SET QMAKESPEC=%QTDIR%\mkspecs\win32-msvc2008
SET TUNDRA_PYTHON_ENABLED=TRUE
echo.
goto :run_cmake

:run_cmake
cmake.exe -G "Visual Studio 9 2008" 
::cmake.exe -G "Visual Studio 9 2008" -DUPDATE_LANGUAGE_TRANSLATIONS:BOOL=TRUE
goto :end

:end
echo.
pause
