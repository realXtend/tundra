@rem Makes a build directory of the viewer, so that you can run the NSIS install script
@rem You should have a directory ..\viewerbuilddlls which has release versions of
@rem all dependency dlls, including Qt core/ui/webkit dll's, and also VS2008 
@rem redistributable (vcredist_x86.exe) 
@echo off
rmdir build /S /Q
md build
xcopy bin\*.* build /S /C
del build\data\configuration\*.xml
rmdir build\testing /S /Q
del build\*.dll
del build\viewerd.exe
del build\modules\core\*d.dll
del build\modules\core\OpenAL*.*
del build\modules\core\DebugStats*.*
del build\modules\core\Gtkmm*.*
del build\modules\core\CommunicationUI*.*
del build\modules\test\*.xml
del build\modules\test\*.dll
xcopy ..\viewerbuilddlls\*.* build /S /C

