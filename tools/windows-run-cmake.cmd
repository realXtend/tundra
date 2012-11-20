@echo off
echo.

set GENERATOR="Visual Studio 9 2008"

cd ..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps

:: Print user defined variables
cecho {0A}Script configuration:{# #}{\n}
echo CMake Generator   = %GENERATOR%
echo.

:: Make sure we call .Net Framework 3.5 version of msbuild, to be able to build VS2008 solutions.
set PATH=C:\Windows\Microsoft.NET\Framework\v3.5;%PATH%
:: Add qmake from our downloaded Qt to PATH.
set PATH=%DEPS%\Qt\bin;%PATH%

SET BOOST_ROOT=%DEPS%\boost
SET QMAKESPEC=win32-msvc2008
SET QTDIR=%DEPS%\qt
SET TUNDRA_DEP_PATH=%DEPS%
SET KNET_DIR=%DEPS%\kNet
SET BULLET_DIR=%DEPS%\bullet
SET OGRE_HOME=%DEPS%\ogre-safe-nocrashes\SDK
SET SKYX_HOME=%DEPS%\realxtend-tundra-deps\skyx
SET HYDRAX_HOME=%DEPS%\realxtend-tundra-deps\hydrax
SET PROTOBUF_SRC_ROOT_FOLDER=%DEPS%\protobuf
SET OPENSSL_ROOT_DIR=%DEPS%\openssl
SET CELT_ROOT=%DEPS%\celt
SET SPEEX_ROOT=%DEPS%\speex
SET VLC_ROOT=%DEPS%\vlc
SET QXMPP_ROOT=%DEPS%\qxmpp
SET ZZIPLIB_ROOT=%DEPS%\zziplib
SET CRUNCH_ROOT=%DEPS%\crunch

:: Disable python untill it has been fixed to windows-build-deps.cmd!
SET TUNDRA_PYTHON_ENABLED=FALSE
IF %TUNDRA_PYTHON_ENABLED%==FALSE cecho {0E}Disabling Python from the build until deps are automated!{# #}{\n}
echo.

IF NOT EXIST Tundra.sln. (
   del /Q CMakeCache.txt
   cecho {0D}Running CMake for Tundra.{# #}{\n}
   cmake.exe -G %GENERATOR%
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0A}Tundra.sln exists. Skipping CMake call for Tundra.{# #}{\n}
   cecho {0A}Delete %CD%\Tundra.sln to trigger a CMake rerun.{# #}{\n}
)
echo.

:: Finish in same directory we started in.
cd TOOLS
set PATH=%ORIGINAL_PATH%
GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
:: Finish in same directory we started in.
cd tools
set PATH=%ORIGINAL_PATH%
pause
