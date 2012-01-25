:: This bat will clean all telepathy related files from ./build run this after makebuild.bat
:: Also makes sure no unneeded qt libraries are not included
@echo off
cd ..\..\build

:: Delete temporary developer content
del *.bat
del performancelogger.txt
del fullmemoryleaklog.txt
del scenestats.txt
del ogrestats.txt

:: Delete debug DLLs of various libraries that might exist in the bin\ tree.
del *d4.dll
del *_d.dll
del Poco*d.dll
del dbus-1d.dll
del libcurld.dll
del audiod.dll
del OpenJPEGd.dll
del xmlrpcepid.dll
del QtSolutions_PropertyBrowser-2.5d.dll
del QtPropertyBrowserd.dll
del liboggd.dll
del libvorbisd.dll
del libvorbisfiled.dll
del Tundrad.exe
del TundraConsoled.exe
del plugins\*d.dll
del qtplugins\codecs\*d4.dll
del qtplugins\graphicssystems\*d4.dll
del qtplugins\iconengines\*d4.dll
del qtplugins\imageformats\*d4.dll
del qtplugins\phonon_backend\*d4.dll
del qtplugins\script\*d.dll
del caelum_d.dll
del Hydraxd.dll
del SkyXd.dll

:: We don't use Caelum anymore! Delete everything related to it.
del caelum.dll
del caelum_d.dll

:: Remove OpenAL, since it is installed to the system with an installer
del openal32.dll
del wrap_oal.dll

:: Remove everything Telepathy and dbus related
del telepathy*.dll
rd /s /q gstreamer
rd /s /q gabble
rd /s /q dbus
rd /s /q data\dbus_services
del startdbus.bat
del dbus-1.dll
del dbus-glib.dll
del libgnutlsxx-26.dll
del telepathy-farsight.dll
del telepathy-glib.dll

:: Remove unused Qt modules
del QtMultimedia*.dll
del QtXmlPatterns*.dll
del QtTest*.dll
del QtOpenGLd4.dll
del QtSqld4.dll
del QtSvgd4.dll
rd /s /q qtplugins\accessible
rd /s /q qtplugins\bearer
rd /s /q qtplugins\sqldrivers
rd /s /q qtplugins\designer
rd /s /q qtplugins\graphicssystems
del qtplugins\script\*d.dll
del qtplugins\codecs\*d4.dll
del qtplugins\iconengines\*d4.dll
del qtplugins\imageformats\*d4.dll
del qtplugins\phonon_backend\*d4.dll

:: Random stuff
::rd /s /q dox
del pluginsd.cfg
del plugins-*.cfg
del resources-*.cfg
del data\avatar_*.llm
del data\gabble.manager
del data\assets\dummy.txt
rd /s /q media\models\blend

rd /s /q qtplugins\phonon_backend
del qtplugins\script\qtscript_opengl.dll
del qtplugins\script\qtscript_phonon.dll
::del qtplugins\script\qtscript_webkit.dll
del qtplugins\script\qtscriptdbus4.dll
del qtplugins\script\qtscriptdbusd4.dll
del qtplugins\script\qtscript_sql.dll
del qtplugins\script\qtscript_svg.dll
del qtplugins\script\qtscript_xmlpatterns.dll
::del libeay32.dll :: MumbleVoipModule depends on this.
::del ssleay32.dll :: MumbleVoipModule depends on this.
del libexpat.dll
del libiconv2.dll
del libjpeg.dll
del libtheorad.dll
::del QtDBus4.dll
::del QtOpenGL4.dll :: PythonScriptModule depends on this via PythonQt_QtAll.dll
::del QtSql4.dll :: PythonScriptModule depends on this via PythonQt_QtAll.dll
::del QtSvg4.dll :: PythonScriptModule depends on this via PythonQt_QtAll.dll

del Qt3Support4.dll
del QtCLucene4.dll
del QtDesigner4.dll
del QtDesignerComponents4.dll
del QtHelp4.dll
rd /s /q qtplugins\phonon_backend

:: Delete unused Ogre DLLs and media content
::del /q media\fonts\*.*
::del /q media\models\avatar.*
::del /q media\models\Jack.*
del OgrePaging.dll
del OgreProperty.dll
del OgreRTShaderSystem.dll
del OgreTerrain.dll
del OIS.dll
del Plugin_BSPSceneManager.dll
del Plugin_PCZSceneManager.dll
del Plugin_OctreeZone.dll
del RenderSystem_Direct3D10.dll
del RenderSystem_Direct3D11.dll
::del RenderSystem_GL.dll
::del RenderSystem_NULL.dll

:: Comment to enable QtDeclarative support in the build
::del QtDeclarative4.dll

:: Comment to enable QTScriptTools support in the build
::del QtScriptTools4.dll

:: Comment to enable VLC in the build
::rmdir /S /Q vlcplugins
::del libvlc.dll
::del libvlccore.dll

:: Comment to enable QtWebkit in the build. NOTE: QtWebkit depends on Phonon4 to work!
:: del phonon4.dll

:: Comment to enable developer content in the build (EC editor, asset editors, etc.)
::rd /s /q jsmodules\apitest
::del /q media\materials\templates\*.*
::del QtSolutions_PropertyBrowser-2.5.dll
del viewerautomove.xml
::del jsmodules\startup\RandomAvatarMovement.js
::del data\ui\audio_preview.ui
::del data\ui\eceditor.ui
::del data\ui\EditorSettings.ui
::del data\ui\MaterialEditorPassTab.ui
::del data\ui\MaterialEditorTuTab.ui
::del data\ui\materialwizard.ui
::del data\ui\mesh_preview.ui
::del data\ui\ogrescripteditor.ui
::del data\ui\profiler.ui
::del data\ui\texture_preview.ui

:: Comment to enable the addition of sample scenes to the build
::rd /s /q scenes

:: Comment to enable Python in the build
::del python*.dll
::rd /s /q pyplugins

:: Comment to enable web browser UI in the build
::rd /s /q jsmodules\browserui
::rd /s /q data\ui\images\browser
::rd /s /q data\ui\images\comm
::del viewer-browser.xml
::del data\ui\images\icon\browser.ico
::del data\ui\images\icon\update.ico
::del data\ui\LoginWebWidget.ui

pause