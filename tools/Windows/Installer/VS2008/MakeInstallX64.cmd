@rem Builds and deploys Tundra x64 using the INSTALL project, so that you can run the NSIS install script.
@echo off
cd ..\..\..\..

MSBuild INSTALL.vcxproj /p:configuration=RelWithDebInfo /nologo 

copy tools\Windows\Installer\VS2008\vcredist_x64.exe build\bin\vcredist.exe
copy tools\Windows\Installer\oalinst.exe build\bin
copy tools\Windows\Installer\dxwebsetup.exe build\bin

:: Copy contents of \build\bin\ to build\ so that we have identical folder structure to MakeBuild.bat
xcopy build\bin\*.* build /S /C /Y
rd build\bin /S /Q
IF EXIST build\include. rd build\include /S /Q
IF EXIST build\lib. rd build\lib /S /Q

cd tools\Windows\Installer\VS2008
