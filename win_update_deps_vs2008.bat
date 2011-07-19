@echo off
echo.

:: If this file fails to find git.exe add it Git install 
:: directorys 'bin' folder path to your PATH env variable

:: If you get git curl ssl cert errors 'error: error setting certificate verify locations' you can propably 
:: fix that by updating to a newer git (tested tp work with >=1.7.6) or doing a work around like
:: http://lostechies.com/keithdahlby/2010/09/26/msysgit-error-setting-certificate-verify-locations/
:: Essentialy you will be running a command below and running this batch file again
:: git config --global http.sslcainfo "C:\Program Files (x86)\Git\bin\curl-ca-bundle.crt"
:: this will make git find the curl certification file properly.

:update_vs2008_dependency_submodule
echo -- Updatating git submodule path deps/vs2008
git.exe submodule init deps/vs2008
git.exe submodule update deps/vs2008
if exist "deps\vs2008\README" (
    goto :remove_all_dlls_from_bin
) else (
    echo ERROR: Could not find 'deps\vs2008\README', assuming git submodule init/update failed!
    goto :end
)

:remove_all_dlls_from_bin
echo -- Removing all .dll files from /bin
del bin\*.dll /q
echo -- Removing CMakeCache.txt to get rid of cached dependency paths
del CMakeCache.txt /q
goto :copy_new_dlls_to_bin

:copy_new_dlls_to_bin
echo -- Copying new vs2008 dependencies to /bin
xcopy deps\vs2008\runtime_deps\*.* bin /S /C /Y /Q
goto :end

:end
echo.
pause