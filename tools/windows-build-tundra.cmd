@echo off

set GENERATOR="Visual Studio 9 2008"
cd ..
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps

set PATH=C:\Windows\Microsoft.NET\Framework\v3.5;%PATH%

:: Make sure we call .Net Framework 3.5 version of msbuild, to be able to build VS2008 solutions.
set PATH=C:\Windows\Microsoft.NET\Framework\v3.5;%PATH%

:: Add qmake from our downloaded Qt to PATH.
set PATH=%DEPS%\Qt\bin;%PATH%

cecho {0D}Running cmake for Tundra.{# #}{\n}

SET BOOST_ROOT=%DEPS%\boost
::SET BOOST_INCLUDEDIR=%DEPS%\boost
::SET BOOST_LIBRARYDIR=%DEPS%\boost\stage\lib
SET QMAKESPEC=win32-msvc2008
SET QTDIR=%DEPS%\qt
SET TUNDRA_DEP_PATH=%DEPS%
SET KNET_DIR=%DEPS%\kNet
SET KNET_DIR_QT47=%DEPS%\kNet
SET BULLET_DIR=%DEPS%\bullet
SET OGRE_HOME=%DEPS%\ogre-safe-nocrashes\SDK
SET SKYX_HOME=%DEPS%\realxtend-tundra-deps\skyx
SET HYDRAX_HOME=%DEPS%\realxtend-tundra-deps\hydrax
cmake.exe -G %GENERATOR% -DBOOST_ROOT="%DEPS%\boost"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Building Tundra.{# #}{\n}
msbuild tundra.sln /p:Configuration=RelWithDebInfo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0A}Tundra build finished.{# #}{\n}
:: Finish in same directory we started in.
cd tools
GOTO :EOF

:ERROR
cecho {0C}An error occurred! Aborting!{# #}{\n}
:: Finish in same directory we started in.
cd tools
pause
