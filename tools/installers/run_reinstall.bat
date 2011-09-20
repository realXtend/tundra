@echo off
echo WARNING: This batch file performs unconfirmable uninstallation of possible existing Tundra 2 installation.
echo Close the window if you wish to abort.
pause
chcp 1252
SET ORIG_PATH=%CD%
SET TUNDRA_PATH=""
IF EXIST "C:\Program Files (x86)\Tundra 2.0\Tundra.exe" GOTO ProgFilesx86
IF EXIST "C:\Program Files\Tundra 2.0\Tundra.exe" GOTO ProgFiles
echo Previous Tundra installation not found from Program Files. Skipping uninstallation phase.
GOTO Install

:ProgFilesx86
set TUNDRA_PATH="C:\Program Files (x86)\Tundra 2.0"
GOTO Uninstall

:ProgFiles
set TUNDRA_PATH="C:\Program Files\Tundra 2.0"
GOTO Uninstall

:Uninstall
echo Unistalling previous Tundra installation...
%TUNDRA_PATH%\uninstaller.exe /S
GOTO Install

:Install
pushd \\ServerAddress\TundraInstallerFolder\
copy realXtend-Tundra-2.0.exe %ORIG_PATH%
popd
realXtend-Tundra-2.0.exe
del realXtend-Tundra-2.0.exe
