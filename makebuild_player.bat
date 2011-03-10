@rem Makes a buildplayer directory of the viewer, so that you can run the NSIS install script
@rem You should have VS2008 redistributable (vcredist_x86.exe) in your viewer trunk root
@rem as well as OpenAL installer (oalinst.exe)
@echo off
rmdir buildplayer /S /Q
md buildplayer
copy readme.txt buildplayer
copy license.txt buildplayer
copy vcredist_x86.exe buildplayer
copy oalinst.exe buildplayer
copy dxwebsetup.exe buildplayer
xcopy bin\*.* buildplayer /S /C /Y
del buildplayer\performancelogger.txt
del buildplayer\fullmemoryleaklog.txt
del buildplayer\scenestats.txt
del buildplayer\ogrestats.txt
del buildplayer\openal32.dll
del buildplayer\wrap_oal.dll
del buildplayer\*d4.dll
del buildplayer\*_d.dll
del buildplayer\Poco*d.dll
@rem del buildplayer\dbus-1d.dll
del buildplayer\libcurld.dll
del buildplayer\audiod.dll
del buildplayer\OpenJPEGd.dll
del buildplayer\QtPropertyBrowserd.dll
del buildplayer\xmlrpcepid.dll
del buildplayer\liboggd.dll
del buildplayer\libvorbisd.dll
del buildplayer\libvorbisfiled.dll
del buildplayer\pymodules\*.pyc
del buildplayer\pymodules\apitest\*.pyc
del buildplayer\pymodules\circuits\*.pyc
del buildplayer\pymodules\circuits\core\*.pyc
del buildplayer\pymodules\circuits\net\*.pyc
del buildplayer\pymodules\circuits\tools\*.pyc
del buildplayer\pymodules\circuits\web\*.pyc
del buildplayer\pymodules\core\*.pyc
del buildplayer\pymodules\objectedit\*.pyc
del buildplayer\pymodules\lib\*.pyc
del buildplayer\pymodules\lib\webdav\*.pyc
del buildplayer\pymodules\lib\webdav\acp\*.pyc
del buildplayer\pymodules\lib\poster\*.pyc
del buildplayer\pymodules\usr\*.pyc
del buildplayer\pymodules\webdavinventory\*.pyc
del buildplayer\pymodules\webserver\*.pyc
del buildplayer\pymodules\mediaurlhandler\*.pyc
del buildplayer\pymodules\localscene\*.pyc
del buildplayer\pymodules\objecttools\*.pyc
del buildplayer\pymodules\estatemanagement\*.pyc
del buildplayer\pymodules\loadurlhandler\*.pyc
del buildplayer\pymodules\DLLs\*_d.pyd
del buildplayer\plugins\*_d.dll

@rem Player Viewer

rmdir buildplayer\scenes /S /Q
rmdir buildplayer\tmp /S /Q
rmdir buildplayer\jsmodules /S /Q


rmdir buildplayer\pymodules\objectedit /S /Q
rmdir buildplayer\pymodules\objecttools /S /Q
rmdir buildplayer\pymodules\apitest /S /Q
rmdir buildplayer\pymodules\aligntools /S /Q
rmdir buildplayer\pymodules\webdavinventory /S /Q
rmdir buildplayer\pymodules\simiangrid /S /Q
del buildplayer\pymodules\default.ini

del buildplayer\viewer.exe
del buildplayer\viewerd.exe
del buildplayer\viewerpd.exe
del buildplayer\server.exe
del buildplayer\serverp.exe
del buildplayer\serverd.exe
del buildplayer\serverpd.exe

rmdir buildplayer\modules\core /S /Q
md buildplayer\modules\core
rmdir /s /q buildplayer\pymodules\localscene\
rmdir /s /q buildplayer\pymodules\estatemanagement\
xcopy bin\modules\core\*p.dll buildplayer\modules\core\
xcopy bin\modules\core\*.xml buildplayer\modules\core\

del buildplayer\modules\core\ECEditorModule*
del buildplayer\modules\core\OgreAssetEditorModule*
del buildplayer\modules\core\SupportModules*
del buildplayer\modules\core\CommunicationsModule*
del buildplayer\modules\core\SceneStructureModule*
