@rem Makes a build directory of the viewer, so that you can run the NSIS install script
@rem You should have a directory ..\viewerbuilddlls which has release versions of
@rem all dependency dlls, and also VS2008 redistributable (vcredist_x86.exe) 
@echo off
rmdir build /S /Q
md build
copy readme.txt build
xcopy bin\*.* build /S /C /Y
del build\*.dll
del build\viewerd.exe
del build\modules\core\*d.dll
xcopy ..\viewerbuilddlls\*.* build /S /C /Y
del build\pymodules\*.pyc
del build\pymodules\apitest\*.pyc
del build\pymodules\circuits\*.pyc
del build\pymodules\circuits\core\*.pyc
del build\pymodules\circuits\net\*.pyc
del build\pymodules\circuits\tools\*.pyc
del build\pymodules\circuits\web\*.pyc
del build\pymodules\core\*.pyc
del build\pymodules\editgui\*.pyc
del build\pymodules\lib\*.pyc
del build\pymodules\lib\webdav\*.pyc
del build\pymodules\lib\webdav\acp\*.pyc
del build\pymodules\usr\*.pyc
del build\pymodules\webdavinventory\*.pyc
del build\pymodules\webserver\*.pyc
del build\pymodules\DLLs\*_d.pyd
cd build\data
rmdir configuration /S /Q
mkdir configuration
cd ..\..


