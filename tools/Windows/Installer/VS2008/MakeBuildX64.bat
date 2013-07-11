@rem Makes a build directory of the Tundra, so that you can run the NSIS install script
@echo off
cd ..\..\..\..
rmdir build /S /Q
md build
copy README.md build
copy LICENSE build\LICENSE.txt
copy WhatsNew.txt build
copy tools\Windows\Installer\VS2008\vcredist_x86.exe build\vcredist.exe
copy tools\Windows\Installer\oalinst.exe build
copy tools\Windows\Installer\dxwebsetup.exe build
xcopy bin\*.* build /S /C /Y

:: NOTE! This script only copies content to the build\ folder. If you want to make a full development build with all the content
:: in your bin\ directory, run NSIS after running this bat.
:: If you want to create an end-user client build, run MakeClean.bat after this, and then run NSIS.
