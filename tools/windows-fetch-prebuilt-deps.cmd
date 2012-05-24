@echo off
echo.

set GENERATOR="Visual Studio 9 2008"

:: Populate path variables
cd ..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps-prebuilt
cd %TOOLS%

:: Make sure target folder exists.
IF NOT EXIST "%DEPS%". mkdir "%DEPS%"

cecho {F0}This script fetches and sets up the prebuilt Tundra dependencies{# #}{\n}
echo.

:: Validate user defined variables
IF NOT %GENERATOR%=="Visual Studio 9 2008" (
  cecho {0E}GENERATOR only supports 'Visual Studio 9 2008' at the moment.{# #}{\n}
  GOTO :ERROR
)

echo The download and extraction will take a while, press Ctrl-C to abort!
pause
echo.

IF %GENERATOR%==Visual Studio 9 2008 (
    cd "%DEPS%"
    IF NOT EXIST "vs2008-32bit". (
       IF NOT EXIST tundra-deps-vs2008-32bit.7z. (
          cecho {0D}Downloading tundra-deps-vs2008-32bit.7z.{# #}{\n}
          wget http://realxtend-tundra-prebuilt-deps.googlecode.com/files/tundra-deps-vs2008-32bit.7z
          IF NOT %ERRORLEVEL%==0 GOTO :ERROR
       )

       cecho {0D}Extracting tundra-deps-vs2008-32bit.7z.{# #}{\n}
       7za x -y tundra-deps-vs2008-32bit.7z
       IF NOT %ERRORLEVEL%==0 GOTO :ERROR
       ren deps-vs2008-32bit vs2008-32bit
    ) ELSE (
       cecho {0D}%DEPS%\vs2008-32bit already downloaded. Delete it and tundra-deps-vs2008-32bit.7z to refetch.{# #}{\n}
    )
    
    :: Run cmake. Todo: make this a generic thing below once more GENERATORs are supported.
    :: mind the @setlocal EnableDelayedExpansion thing, as without you cannot SET inside IF statements.
    
    SET DEPS=%DEPS%\vs2008-32bit
    cecho {0D}Using %DEPS% as dependency path, running cmake.{# #}{\n}
    
    SET BOOST_ROOT=%DEPS%\boost
    SET QMAKESPEC=win32-msvc2008
    SET QTDIR=%DEPS%\qt
    SET TUNDRA_DEP_PATH=%DEPS%
    SET KNET_DIR=%DEPS%\kNet
    SET BULLET_DIR=%DEPS%\bullet
    SET OGRE_HOME=%DEPS%\ogre-safe-nocrashes
    SET SKYX_HOME=%DEPS%\realxtend-tundra-deps\skyx
    SET HYDRAX_HOME=%DEPS%\realxtend-tundra-deps\hydrax
    SET PROTOBUF_SRC_ROOT_FOLDER=%DEPS%\protobuf
    SET OPENSSL_ROOT_DIR=%DEPS%\openssl
    SET CELT_ROOT=%DEPS%\celt
    SET SPEEX_ROOT=%DEPS%\speex
    SET VLC_ROOT=%DEPS%\vlc

    :: Disable python untill it has been fixed to windows-build-deps.cmd!
    SET TUNDRA_PYTHON_ENABLED=FALSE
    IF %TUNDRA_PYTHON_ENABLED%==FALSE cecho {0E}Disabling Python from the build until deps are automated!{# #}{\n}
    echo.
    
    cd "%TUNDRA_DIR%"

    IF NOT EXIST Tundra.sln. (
        del /Q CMakeCache.txt
        cecho {0D}Running CMake for Tundra.{# #}{\n}
        cmake.exe -G %GENERATOR% -DBOOST_ROOT="%DEPS%\boost"
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    ) ELSE (
        cecho {0A}Tundra.sln exists. Skipping CMake call for Tundra.{# #}{\n}
        cecho {0A}Delete %CD%\Tundra.sln to trigger a CMake rerun.{# #}{\n}
    )
    echo.
)

echo.
cecho {0D}All done. Open tundra.sln and build.{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
pause
