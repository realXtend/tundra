@rem Makes a build directory of the Tundra, so that you can run the NSIS install script
@rem You should have VS2008 redistributable (vcredist_x86.exe) in your viewer trunk root
@rem as well as OpenAL installer (oalinst.exe)
@echo off
rmdir build /S /Q
md build
copy readme.txt build
copy license.txt build
copy vcredist_x86.exe build
copy oalinst.exe build
copy dxwebsetup.exe build
xcopy bin\*.* build /S /C /Y

del build\performancelogger.txt
del build\fullmemoryleaklog.txt
del build\scenestats.txt
del build\ogrestats.txt
del build\telepathy*.dll
del build\caelum*.dll
del build\openal32.dll
del build\wrap_oal.dll
del build\*d4.dll
del build\*_d.dll
del build\Poco*d.dll
@rem del build\dbus-1d.dll
del build\libcurld.dll
del build\audiod.dll
del build\OpenJPEGd.dll
del build\xmlrpcepid.dll
del build\QtSolutions_PropertyBrowser-2.5d.dll
del build\QtPropertyBrowserd.dll
del build\liboggd.dll
del build\libvorbisd.dll
del build\libvorbisfiled.dll
del build\Tundrad.exe
del build\plugins\*d.dll
del build\Hydraxd.dll
del build\SkyXd.dll
@rem for now, delete all python stuff
del build\python*.dll
rmdir build\pymodules /S /Q
@rem del build\pymodules\*.pyc
@rem del build\pymodules\apitest\*.pyc
@rem del build\pymodules\circuits\*.pyc
@rem del build\pymodules\circuits\core\*.pyc
@rem del build\pymodules\circuits\net\*.pyc
@rem del build\pymodules\circuits\tools\*.pyc
@rem del build\pymodules\circuits\web\*.pyc
@rem del build\pymodules\core\*.pyc
@rem del build\pymodules\objectedit\*.pyc
@rem del build\pymodules\lib\*.pyc
@rem del build\pymodules\lib\webdav\*.pyc
@rem del build\pymodules\lib\webdav\acp\*.pyc
@rem del build\pymodules\lib\poster\*.pyc
@rem del build\pymodules\usr\*.pyc
@rem del build\pymodules\webdavinventory\*.pyc
@rem del build\pymodules\webserver\*.pyc
@rem del build\pymodules\mediaurlhandler\*.pyc
@rem del build\pymodules\localscene\*.pyc
@rem del build\pymodules\objecttools\*.pyc
@rem del build\pymodules\estatemanagement\*.pyc
@rem del build\pymodules\loadurlhandler\*.pyc
@rem del build\pymodules\DLLs\*_d.pyd
