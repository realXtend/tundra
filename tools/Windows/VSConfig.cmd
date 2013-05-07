:: This script initializes various Visual Studio -related envinronment variables needed for building
:: NOTE: The delayed environment variable expansion needs to be enabled before calling this.

@echo off

set GENERATOR=%1
:: Supported Visual Studio versions:
::set GENERATOR_VS2012="Visual Studio 11"
::set GENERATOR_VS2012_WIN64="Visual Studio 11 Win64"
set GENERATOR_VS2010="Visual Studio 10"
set GENERATOR_VS2010_WIN64="Visual Studio 10 Win64"
set GENERATOR_VS2008="Visual Studio 9 2008"
set GENERATOR_VS2008_WIN64="Visual Studio 9 2008 Win64"
:: TODO VS2008 is ancient, change GENERATOR_DEFAULT to VS2010 at some point.
set GENERATOR_DEFAULT=%GENERATOR_VS2008%

IF "!GENERATOR!"=="" (
    set GENERATOR=%GENERATOR_DEFAULT%
    Utils\cecho {0E}VSConfig.cmd: Warning: Generator not passed - using the default %GENERATOR_DEFAULT%.{# #}{\n}
)

:: Figure out the build configuration from the CMake generator string.
:: Are we building 32-bit or 64-bit version.
set TARGET_ARCH=x86
:: Visual Studio platform name.
set VS_PLATFORM=Win32

:: DEPS_POSTFIX is appended to the build deps directory. Note that this is currently only appended when doing x64 build.
:: TODO set DEPS_POSTFIX=-x86 when 32-bit build is not the default on Windows anymore.
set DEPS_POSTFIX=
:: VS_VER and VC_VER are convenience variables used f.ex. for filenames

:: Split the string for closer inspection.
set GENERATOR_SPLIT=%GENERATOR:"=%
set GENERATOR_SPLIT=%GENERATOR_SPLIT: =,%
FOR %%i IN (%GENERATOR_SPLIT%) DO (
    REM IF %%i==11 (
    REM set VS_VER=vs2012
    REM set VC_VER=vc11
    REM )
    IF %%i==10 (
        set VS_VER=vs2010
        set VC_VER=vc10
    )
    IF %%i==2008 (
        set VS_VER=vs2008
        set VC_VER=vc9
    )
    REM Are going to perform a 64-bit build?
    IF %%i==Win64 (
        set TARGET_ARCH=x64
        set VS_PLATFORM=x64
        set DEPS_POSTFIX=-x64
    )
)

:: VS project file extension differs on different VS versions
:: VS2008_OR_VS2010 is "vs2008" when building with VS 2008 and "vs2010" on all newer VS versions.
IF %VS_VER%==vs2008 (
    set VCPROJ_FILE_EXT=vcproj
    set VS2008_OR_VS2010=vs2008
) ELSE (
    set VCPROJ_FILE_EXT=vcxproj
    set VS2008_OR_VS2010=vs2010
)

:: QT_PLATFORM is used when building Qt.
IF %VS_VER%==vs2008 set QT_PLATFORM=win32-msvc2008
IF %VS_VER%==vs2010 set QT_PLATFORM=win32-msvc2010
::IF %VS_VER%==vs2012 set QT_PLATFORM=win32-msvc2012

:: Populate path variables
cd ..\..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\Windows\Utils"
:: TOOLS is the path where the Windows build scripts reside.
set TOOLS=%CD%\tools\Windows
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin

:: Fetch and build the dependencies to a dedicated directory depending on the used VS version.
:: For now, use the old deps dir for VS2008 build, but deps-%VS_VER% for other VS version
:: TODO consider if the existing VS2008 should be renamed by the script to deps-vs2008 for consistency.
IF %VS_VER%==vs2008 (
    set DEPS=%CD%\deps
) ELSE (
    set DEPS=%CD%\deps-%VS_VER%
)

set DEPS=%DEPS%%DEPS_POSTFIX%

cd %TOOLS%
