:: This script initializes various Visual Studio -related envinronment variables needed for building
@echo off

:: Supported Visual Studio versions
::set GENERATOR_VS2012="Visual Studio 11"
set GENERATOR_VS2010="Visual Studio 10"
set GENERATOR_VS2008="Visual Studio 9 2008"
set GENERATOR_DEFAULT=%GENERATOR_VS2008%

set GENERATOR=%1
IF "%GENERATOR%"=="" (
    set GENERATOR=%GENERATOR_DEFAULT%
    utils-windows\cecho {0E}VSConfig.cmd: Warning: Generator not passed - using the default %GENERATOR_DEFAULT%.{# #}{\n}
)
:: TODO Utilize BUILD_TYPE
::set BUILD_TYPE=%2%
::set BUILD_TYPE_RELEASE=Release
::set BUILD_TYPE_RELWITHDEBINFO=RelWithDebInfo
::set BUILD_TYPE_DEBUG=Debug

:: VS_VER is a convenience define used f.ex. for filenames
::IF %GENERATOR%==%GENERATOR_VS2012% set VS_VER=vs2012
IF %GENERATOR%==%GENERATOR_VS2010% set VS_VER=vs2010
IF %GENERATOR%==%GENERATOR_VS2008% set VS_VER=vs2008

:: VS project file extension differ on different VS versions
IF %GENERATOR%==%GENERATOR_VS2008% (
   set VCPROJ_FILE_EXT=vcproj
) ELSE (
   set VCPROJ_FILE_EXT=vcxproj
)
IF %GENERATOR%==%GENERATOR_VS2008% set QT_PLATFORM=win32-msvc2008
IF %GENERATOR%==%GENERATOR_VS2010% set QT_PLATFORM=win32-msvc2010

:: Populate path variables
cd ..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin

:: Fetch and build the dependencies to a dedicated directory depending on the used VS version.
:: For now, use the old deps dir for VS2008 build, but deps-%VS_VER% for other VS version

IF %GENERATOR%==%GENERATOR_VS2008% (
   set DEPS=%CD%\deps
) ELSE (
   set DEPS=%CD%\deps-%VS_VER%
)

cd %TOOLS%

:: Make sure deps folder exists.
IF NOT EXIST "%DEPS%". mkdir "%DEPS%"
