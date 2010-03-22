@rem Makes a build directory of the viewer, so that you can run the NSIS install script
@rem You should have VS2008 redistributable (vcredist_x86.exe) in your viewer trunk root
@rem as well as OpenAL installer (oalinst.exe)
@echo off
rmdir build /S /Q
md build
copy readme.txt build
copy license.txt build
copy vcredist_x86.exe build
copy oalinst.exe build
xcopy bin\*.* build /S /C /Y
del build\fullmemoryleaklog.txt
del build\openal32.dll
del build\wrap_oal.dll
del build\*d4.dll
del build\*_d.dll
del build\Poco*d.dll
@rem del build\dbus-1d.dll
del build\libcurld.dll
del build\OpenJPEGd.dll
del build\xmlrpcepid.dll
del build\liboggd.dll
del build\libvorbisd.dll
del build\libvorbisfiled.dll
del build\viewerd.exe
del build\modules\core\*d.dll
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
del build\pymodules\mediaurlhandler\*.pyc
del build\pymodules\DLLs\*_d.pyd
del build\plugins\*_d.dll
