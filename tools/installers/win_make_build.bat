@rem Makes a build directory of the Tundra, so that you can run the NSIS install script
@rem You should have VS2008 redistributable (vcredist_x86.exe) in your <TundraRoot>\tools\installer
@rem as well as OpenAL installer (oalinst.exe)
@echo off
cd ..\..
rmdir build /S /Q
md build
copy README.md build
copy LICENSE build
copy tools\installers\vcredist_x86.exe build
copy tools\installers\oalinst.exe build
copy tools\installers\dxwebsetup.exe build
xcopy bin\*.* build /S /C /Y

:: NOTE! This script only copies content to the build\ folder. If you want to make a full development build with all the content
:: in your bin\ directory, run NSIS after running this bat.
:: If you want to create an end-user client build, run win_make_clean.bat after this, and then run NSIS.