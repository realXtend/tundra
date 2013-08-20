:: Deploys Tundra using the INSTALL project, so that you can run the NSIS install script.
:: Must be run from Visual Studio Command Prompt.
@echo off

:: Enable the delayed environment variable expansion needed in VSConfig.cmd.
setlocal EnableDelayedExpansion

:: Make sure we're running in Visual Studio Command Prompt
IF "%VSINSTALLDIR%"=="" (
   ..\Utils\cecho {0C}Batch file not executed from Visual Studio Command Prompt - cannot proceed!{# #}{\n}
   GOTO :EOF
)

cd ..
:: Set up variables depending on the used Visual Studio version.
call VSConfig.cmd %1
:: Guarantee a fresh CMake run
IF EXIST ..\..\tundra.sln. del ..\..\tundra.sln
call RunCMake.cmd %*
cd ..\..

:: Pass in VS_PLATFORM in order to ensure that we're deploying the version we intented to.
MSBuild INSTALL.%VCPROJ_FILE_EXT% /p:configuration=RelWithDebInfo /p:platform="%VS_PLATFORM%" /nologo
IF NOT %ERRORLEVEL%==0 GOTO :End

copy /y tools\Windows\Installer\%VS_VER%\vcredist_%TARGET_ARCH%.exe build\bin\vcredist.exe
copy /y tools\Windows\Installer\oalinst.exe build\bin
copy /y tools\Windows\Installer\dxwebsetup.exe build\bin

:: Copy contents of \build\bin\ to build\ so that we have identical folder structure to MakeBuild.bat
xcopy build\bin\*.* build /S /C /Y
rd build\bin /S /Q
::IF EXIST build\include. rd build\include /S /Q
::IF EXIST build\lib. rd build\lib /S /Q

:End
:: TODO Why isn't the following cd working?
cd tools\Windows\Installer\%VS_VER%
endlocal
