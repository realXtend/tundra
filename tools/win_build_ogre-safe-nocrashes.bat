@echo off

:: This batch file will do its best to checkout Tundras custom Ogre fork with skyx, hydrax and null renderer plugins.
:: Initially this is for VS2008, but with small tweaks it can be done for VS2010 also.
:: Do note that this is experimental and might not work for everyone out of the box. Also this might not be the 
:: most readable thing in the world, but hey its a batch script, what did you expect?

:: See your setup is up to the following requirements before running.
:: 1. In order for this path to work you need to the following in PATH: hg.exe, git.exe and cmake.exe
:: 2. In order for the git sources branch checkout to work deps/sources must not exist or be completely empty the first time you run this scripts.
:: 3. If you don't have VS2008 installed to C:\Program Files (x86)\Microsoft Visual Studio 9.0 (default) modify the VS_ROOT variable and cross your fingers
:: 4. It's recommended to have DirectX SDK installer, you can find it from microsoft with a simple google search.

:: Current main features
:: - Tries its best to detect needed steps and gives instructions if 'manual' work is needed (eg. Ogre Dependencies)
:: - Does also update all source repos, so when deps change in their respective repos running this script again will update everything.
:: - Sets Visual Studio 2008 cmd line environment
:: - Fetches Tundra deps sources branch from google-code via git
::   + Clones to /deps/sources 
:: - Fetches Tundra Ogre fork 'ogre-safe-nocrashes' v1-8 branch from bitbucket via hg
::   + Clones to /deps/sources/ogre-safe-nocrashes
:: - Builds and installs 'ogre-safe-nocrashes' in both RelWithDebInfo and Debug mode
::   + Installs to /deps/sources/ogre-safe-nocrashes/SDK (Ogre build default)

:: Todo list
:: - Build SkyX from deps/sources/skyx
:: - Build HydraX from deps/sources/hydrax
:: - Build Null Rendersystem from deps/sources/RenderSystem_Null
:: - Modify win_cmake_vs2008.bat to detect what ogre and its plugins have been built in
::   deps/sources and set <DEP>_ROOT to point to the folders.
:: - Handle copying needed runtime DLLs from the built projects to bin every time this script is ran

for /f %%i in ("%0") do set RUN_PATH=%%~dpi

:setup_script_env
set START_PATH=%CD%
set TUNDRA_DEP_SOURCES=%RUN_PATH%..\deps\sources
set OGRE_SRC_DIR=ogre-safe-nocrashes
set OGRE_SRC_PATH=%TUNDRA_DEP_SOURCES%\%OGRE_SRC_DIR%
set VS_ROOT=C:\Program Files (x86)\Microsoft Visual Studio 9.0

:setup_2008_build_env
echo.
echo VS 2008 BUILD ENV SETUP
echo ==================================================
if not exist "%VS_ROOT%\" (
    echo -- Failed to detect VS2008 installed from "%VS_ROOT%"
    echo -- Set your VS2008 install directory to the VS_ROOT variable
    goto :exit
)
echo -- Setting up Visual Studio 2008 env to %VS_ROOT%
@set DevEnvDir=%VS_ROOT%\Common7\IDE
@set PATH=%VS_ROOT%\Common7\IDE;%VS_ROOT%\VC\BIN;%VS_ROOT%\Common7\Tools;C:\Windows\Microsoft.NET\Framework\v3.5;C:\Windows\Microsoft.NET\Framework\v2.0.50727;%VS_ROOT%\VC\VCPackages;%PATH%
@set INCLUDE=%VS_ROOT%\VC\ATLMFC\INCLUDE;%VS_ROOT%\VC\INCLUDE;
@set LIB=%VS_ROOT%\VC\ATLMFC\LIB;%VS_ROOT%\VC\LIB;
@set LIBPATH=C:\Windows\Microsoft.NET\Framework\v3.5;C:\Windows\Microsoft.NET\Framework\v2.0.50727;%VS_ROOT%\VC\ATLMFC\LIB;%VS_ROOT%\VC\LIB;
echo ---- INCLUDE: %INCLUDE%
echo ---- LIB: %LIB%
goto :update_tundra_sources

:update_tundra_sources
if not exist "%TUNDRA_DEP_SOURCES%\" mkdir %TUNDRA_DEP_SOURCES%
echo.
echo TUNDRA DEPENDENCY SOURCES
echo ==================================================
cd %TUNDRA_DEP_SOURCES%
echo -- Checking %CD% for Tundra dependency sources sources
if exist "README" (
    echo -- Checking out sources branch
    git checkout sources
    echo -- Fetching latest commits
    git fetch origin
    echo -- Updating Tundra dependency sources
    git rebase origin/sources
) else (
    echo -- Cloning Tundra dependency sources. Be patient, this will take a while...
    cd ..
    git clone https://code.google.com/p/realxtend-tundra-deps/ sources
    echo -- Checking out sources branch
    cd sources
    git checkout -b sources origin/sources
)
goto :update_ogre_repo

:update_ogre_repo
echo.
echo OGRE SOURCES
echo ==================================================
cd %TUNDRA_DEP_SOURCES%
echo -- Checking %CD%\%OGRE_SRC_DIR% for sources
if exist "%OGRE_SRC_PATH%\README" (
    echo -- Checking out v1-8 branch
    cd %OGRE_SRC_DIR%
    hg checkout v1-8
    echo -- Updating ogre-safe-nocrashes sources
    hg pull -u
) else (
    echo -- Cloning ogre-safe-nocrashes. Be patient, this will take a while...
    hg clone https://bitbucket.org/clb/ogre-safe-nocrashes %OGRE_SRC_DIR%
    echo -- Checking out v1-8 branch
    cd %OGRE_SRC_DIR%
    hg checkout v1-8
)
goto :check_ogre_dependencies

:check_ogre_dependencies
echo.
echo OGRE BUILD DEPENDENCIES
echo ==================================================
cd %OGRE_SRC_PATH%
echo -- Checking %CD%\Dependencies for Ogre build dependencies
if not exist "Dependencies\include\zlib.h" (
    echo   1. You need do download 'Microsoft Visual C++ Dependencies Package' 
    echo      from http://www.ogre3d.org/download/source
    echo   2. Extract the containing Dependency folder to %CD%
    echo   3. Open and build %CD%\src\OgreDependencies.VS2008.sln 
    echo      in both Release and Debug modes
    echo   4. Once done run this script again
    goto :exit
) else (
    goto :build_ogre
)

:build_ogre
echo.
echo BUILDING OGRE
echo ==================================================
cd %OGRE_SRC_PATH%
:: Ogre home and additional path for boost. 
:: <ogre>/Dependencies will be automatically examined
echo -- Setting OGRE_HOME and OGRE_DEPENDENCIES_DIR
set OGRE_HOME=%CD%
set OGRE_DEPENDENCIES_DIR=%RUN_PATH%..\deps\vs2008\build_deps\Boost
:: CMake
echo -- Running cmake
cmake -G "Visual Studio 9 2008" -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF
:: RelWithDebInfo
echo -- Building RelWithDebInfo
msbuild OGRE.sln /nologo /m:4 /p:BuildInParallel=true /consoleloggerparameters:PerformanceSummary /verbosity:d /p:Configuration=RelWithDebInfo
:: Debug
echo -- Building Debug
msbuild OGRE.sln /nologo /m:4 /p:BuildInParallel=true /consoleloggerparameters:PerformanceSummary /verbosity:d /p:Configuration=Debug
:: Install both
echo.
echo -- Installing RelWithDebInfo to %OGRE_HOME%\SDK
msbuild INSTALL.vcproj /nologo /verbosity:q /p:Configuration=RelWithDebInfo
echo.
echo -- Installing Debug to %OGRE_HOME%\SDK
msbuild INSTALL.vcproj /nologo /verbosity:q /p:Configuration=Debug
goto :exit

:exit
echo.
cd %START_PATH%
pause
exit /B