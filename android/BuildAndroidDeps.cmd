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
cd %ANDROID%

echo Testing existence of required tools.. If you get an error, install the missing software and add it to PATH.

patch --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
wget --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
svn --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
call git --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
hg --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
cmake --version
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
nmake /?
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
sfk ver -own
cmd /C EXIT 0
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR

:: Make sure deps folder exists.
IF NOT EXIST "%DEPS%". mkdir "%DEPS%"

:: Qt, needs manually copied Necessitas for now
IF NOT EXIST "%DEPS%\qt". mkdir "%DEPS%\Qt"

:: Bullet
IF NOT EXIST "%DEPS%\bullet\". (
   cecho {0D}Cloning Bullet into "%DEPS%\bullet".{# #}{\n}
   cd "%DEPS%"
   svn checkout http://bullet.googlecode.com/svn/tags/bullet-2.78 bullet
   IF NOT EXIST "%DEPS%\bullet\.svn". GOTO :ERROR
)
IF NOT EXIST "%DEPS%\bullet\libs\%TUNDRA_ANDROID_ABI%\libBulletCollision.a". (
   cecho {0D}Building Bullet. Please be patient, this will take a while.{# #}{\n}
   cd "%DEPS%\bullet"
   del CMakeCache.txt
   cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID%/android.toolchain.cmake -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DCMAKE_BUILD_TYPE=Release -DANDROID_NATIVE_API_LEVEL=9 -DANDROID_ABI=%TUNDRA_ANDROID_ABI%
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   nmake
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
) ELSE (
   cecho {0D}Bullet already built. Skipping.{# #}{\n}
)

:: Boost
IF NOT EXIST "%DEPS%\boost". (
   cecho {0D}Downloading Boost into "%DEPS%\boost".{# #}{\n}
   cd "%DEPS%"
   wget http://downloads.sourceforge.net/project/boost/boost/1.49.0/boost_1_49_0.tar.gz
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   7za x -y boost_1_49_0.tar.gz
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   7za x -y boost_1_49_0.tar
   CMD /C EXIT 0 :: The boost tar file has 2 failures, so kill the unclean return code! TODO: Remove this once there's a proper boost release.
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   ren boost_1_49_0 boost
   del boost_1_49_0.tar.gz
   IF NOT EXIST "%DEPS%\boost\boost.css". GOTO :ERROR
)
IF NOT EXIST "%DEPS%\boost\boost-1_49_0.patch". (
   cecho {0D}Patching Boost.{# #}{\n}
   cd "%DEPS%\boost"
   copy "%ANDROID%\boost-1_49_0.patch" .
   :: Make sure the patch has Unix line endings
   sfk crlf-to-lf boost-1_49_0.patch
   patch -l -p1 -i boost-1_49_0.patch
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   cecho {0D}Building Boost build engine.{# #}{\n}
   cd "%DEPS%\boost"
   call bootstrap.bat
)

cd "%DEPS%\boost"
b2 --version
cmd /C EXIT 0
IF %ERRORLEVEL% NEQ 0 GOTO :ERROR

IF NOT EXIST "%DEPS%\boost\lib\libboost_date_time.a". (
    cd "%DEPS%\boost"
    cecho {0D}Building Boost.{# #}{\n}
    copy "%ANDROID%\user-config_%TUNDRA_ANDROID_ABI%.jam" tools\build\v2\user-config.jam
    copy "%ANDROID%\project-config.jam" .
    b2 --with-date_time --with-filesystem --with-program_options --with-regex --with-signals --with-system --with-thread --with-iostreams toolset=gcc-android4.4.3 link=static runtime-link=static target-os=linux --stagedir=. --layout=system
) ELSE (
    cecho {0D}Boost already built. Skipping.{# #}{\n}
)

:: kNet
IF NOT EXIST "%DEPS%\kNet". (
   cecho {0D}Cloning kNet from https://github.com/juj/kNet into "%DEPS%\kNet".{# #}{\n}
   cd "%DEPS%"
   call git clone https://github.com/juj/kNet
   IF NOT EXIST "%DEPS%\kNet\.git". GOTO :ERROR
   cd "%DEPS%\kNet"
   call git checkout master
)
IF NOT EXIST "%DEPS%\kNet\lib\libkNet.a". (
   cecho {0D}Building kNet.{# #}{\n}
   cd "%DEPS%\kNet"
   del CMakeCache.txt
   cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID%/android.toolchain.cmake -DBOOST_ROOT=%DEPS%/boost -DCMAKE_BUILD_TYPE=Release -DANDROID_NATIVE_API_LEVEL=9 -DANDROID_ABI=%TUNDRA_ANDROID_ABI%
   nmake
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
) ELSE (
   cecho {0D}kNet already built. Skipping.{# #}{\n}
)

:: OGRE
IF NOT EXIST "%DEPS%\ogre". (
   cd "%DEPS%"
   cecho {0D}Cloning OGRE from https://bitbucket.org/sinbad/ogre into "%DEPS%\ogre".{# #}{\n}
   hg clone -r v1-9 https://bitbucket.org/sinbad/ogre ogre
)
IF NOT EXIST "%DEPS%\ogre\AndroidDependenciesBuild". (
   cecho {0D}Cloning OGRE Android dependencies from https://bitbucket.org/cabalistic/ogredeps{# #}{\n}
   cd "%DEPS%\ogre"
   hg clone https://bitbucket.org/cabalistic/ogredeps AndroidDependenciesBuild
)
IF NOT EXIST "%DEPS%\ogre\AndroidDependencies\lib\libFreeImage.a". (
   cecho {0D}Building OGRE Android dependencies{# #}{\n}
   cd "%DEPS%\ogre\AndroidDependenciesBuild"
   del CMakeCache.txt
   cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_NATIVE_API_LEVEL=9 -DANDROID_ABI=%TUNDRA_ANDROID_ABI% -DCMAKE_INSTALL_PREFIX="%DEPS%\ogre\AndroidDependencies"
   nmake
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
   nmake install
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
)
IF NOT EXIST "%DEPS%\ogre\lib\libOgreMainStatic.a". (
   cecho {0D}Building OGRE.{# #}{\n}
   cd "%DEPS%\ogre"
   del CMakeCache.txt
   cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=CMake/toolchain/android.toolchain.cmake -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_DEPENDENCIES_DIR=./AndroidDependencies -DCMAKE_BUILD_TYPE=Release -DANDROID_NATIVE_API_LEVEL=9 -DANDROID_ABI=%TUNDRA_ANDROID_ABI%
   nmake
   IF %ERRORLEVEL% NEQ 0 GOTO :ERROR
) ELSE (
   cecho {0D}OGRE already built. Skipping.{# #}{\n}
)


echo.
cecho {0A}Tundra dependencies built.{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %ANDROID%
GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %ANDROID%
pause
