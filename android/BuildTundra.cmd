@echo off
echo.

:: Populate path variables
cd ..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\utils-windows"
set ANDROID=%CD%\android
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps-android

:: Add qmake from our downloaded Qt to PATH.
set PATH=%DEPS%\Qt\bin;%PATH%

SET BOOST_ROOT=%DEPS%\boost
SET QTDIR=%DEPS%\qt
SET TUNDRA_DEP_PATH=%DEPS%
SET KNET_DIR=%DEPS%\kNet
SET BULLET_DIR=%DEPS%\bullet
SET OGRE_HOME=%DEPS%\ogre

IF NOT EXIST "%ANDROID%\local.properties". (
    cecho {0D}Configuring Tundra Android project.{# #}{\n}
    cd %ANDROID%
    call android update project -p . -t android-10
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cd %TUNDRA_DIR%
cecho {0D}Configuring Tundra build.{# #}{\n}
del CMakeCache.txt
cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID%/android.toolchain.cmake -DBOOST_ROOT=%DEPS%/boost -DANDROID=1 -DANDROID_NATIVE_API_LEVEL=9 -DANDROID_ABI=%TUNDRA_ANDROID_ABI% -DCMAKE_BUILD_TYPE=Release
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Building Tundra.{# #}{\n}
nmake
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Building and signing Tundra APK.{# #}{\n}
cd %ANDROID%
call ant release

GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %ANDROID%
pause
