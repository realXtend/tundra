@echo off
echo %0
if (%1) == () GOTO HELP

FORFILES /M %1 /C "cmd /c %~dp0\OgreMeshUpgrader.exe @file"
GOTO END

:HELP
echo Usage: UpgradeOgreMesh filefilter
echo Converts all files matching the given filter to the newest Ogre format.
echo Examples:
echo    UpgradeOgreMesh mymeshfile.mesh
echo       Converts the single file in the current folder to the newest Ogre mesh format.
echo    UpgradeOgreMesh *.mesh
echo       Converts all files in the current folder to the newest Ogre mesh format.
pause

:END
