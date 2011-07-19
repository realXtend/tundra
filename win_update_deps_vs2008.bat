@echo off
echo.

:: If this file fails to find git.exe add it Git install 
:: directorys 'bin' folder path to your PATH env variable

:update_vs2008_dependency_submodule
echo -- Updatating git submodule path deps/vs2008
git.exe submodule init deps/vs2008
git.exe submodule init deps/vs2008
if exist "deps\vs2008\README" (
    goto :remove_all_dlls_from_bin
) else (
    echo ERROR: Could not find 'deps\vs2008\README', assuming git submodule init/update failed!
    goto :end
)

:remove_all_dlls_from_bin
echo -- Removing all .dll files from /bin
del bin\*.dll /q
goto :copy_new_dlls_to_bin

:copy_new_dlls_to_bin
echo -- Copying new vs2008 dependencies to /bin
xcopy deps\vs2008\runtime_deps\*.* bin /S /C /Y /Q
goto :end

:end
echo.
pause