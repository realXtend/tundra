@echo off

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

:: Disable python untill it has been fixed to windows-build-deps.cmd!
SET TUNDRA_PYTHON_ENABLED=FALSE
IF %TUNDRA_PYTHON_ENABLED%==FALSE cecho {0E}Warning: Disabling Python from the build.{# #}{\n}

IF NOT EXIST Tundra.sln. (
   del /Q CMakeCache.txt
   cecho {0D}Running cmake for Tundra.{# #}{\n}
   cmake.exe -G %GENERATOR% -DBOOST_ROOT="%DEPS%\boost"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   echo.
) ELSE (
   cecho {0D}Tundra.sln exists. Skipping cmake call for Tundra. Delete Tundra.sln to force cmake a rerun.{# #}{\n}
)

cecho {0D}Building Tundra.{# #}{\n}
msbuild tundra.sln /p:Configuration=RelWithDebInfo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

echo.
cecho {0A}Tundra build finished.{# #}{\n}
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
