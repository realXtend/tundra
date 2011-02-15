:: This bat will clean all telepathy related files from ./build run this after makebuild.bat
:: Also makes sure no unneeded qt libraries are not included
@echo off

:: Telepathy stuff
rd /s /q build\gstreamer
rd /s /q build\gabble
rd /s /q build\dbus
rd /s /q build\data\dbus_services

del build\startdbus.bat
del build\dbus-1.dll
del build\dbus-glib.dll
del build\libgnutlsxx-26.dll
del build\telepathy-farsight.dll
del build\telepathy-glib.dll

:: Qt stuff
del build\QtDeclarative*.dll
del build\QtMultimedia*.dll
del build\QtXmlPatterns*.dll
del build\QtTest*.dll
del build\QtOpenGLd4.dll
del build\QtSqld4.dll
del build\QtSvgd4.dll

rd /s /q build\qtplugins\accessible
rd /s /q build\qtplugins\bearer
rd /s /q build\qtplugins\sqldrivers
rd /s /q build\qtplugins\designer
rd /s /q build\qtplugins\graphicssystems

del build\qtplugins\script\*d.dll
del build\qtplugins\codecs\*d4.dll
del build\qtplugins\iconengines\*d4.dll
del build\qtplugins\imageformats\*d4.dll
del build\qtplugins\phonon_backend\*d4.dll

:: Random stuff
rd /s /q build\dox
del build\pluginsd.cfg
del build\plugins-*.cfg
del build\resources-*.cfg
del build\data\avatar_*.llm
del build\data\gabble.manager
del build\data\assets\dummy.txt

:: Installers we dont need in .msi
::del build\dxwebsetup.exe
::del build\oalinst.exe
::del build\vcredist_x86.exe