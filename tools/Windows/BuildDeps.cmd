@echo off
echo.

:: Enable the delayed environment variable expansion needed in VSConfig.cmd.
setlocal EnableDelayedExpansion

:: Make sure we're running in Visual Studio Command Prompt
IF "%VSINSTALLDIR%"=="" (
   Utils\cecho {0C}Batch file not executed from Visual Studio Command Prompt - cannot proceed!{# #}{\n}
   GOTO :ERROR
)

:: Set up variables depending on the used Visual Studio version
call VSConfig.cmd %1

:: Set up variables depending on the used build type.
set BUILD_TYPE=%2

:: Possible build types provided by CMake
set BUILD_TYPE_MINSIZEREL=MinSizeRel
set BUILD_TYPE_RELEASE=Release
set BUILD_TYPE_RELWITHDEBINFO=RelWithDebInfo
set BUILD_TYPE_DEBUG=Debug
set BUILD_TYPE_DEFAULT=%BUILD_TYPE_RELWITHDEBINFO%
IF "!BUILD_TYPE!"=="" (
    set BUILD_TYPE=%BUILD_TYPE_DEFAULT%
    Utils\cecho {0E}VSConfig.cmd: Warning: BUILD_TYPE not specified - using the default %BUILD_TYPE_DEFAULT%{# #}{\n}
    pause
)
IF NOT !BUILD_TYPE!==%BUILD_TYPE_MINSIZEREL% IF NOT !BUILD_TYPE!==%BUILD_TYPE_RELEASE% IF NOT !BUILD_TYPE!==%BUILD_TYPE_RELWITHDEBINFO% IF NOT !BUILD_TYPE!==%BUILD_TYPE_DEBUG% (
    Utils\cecho {0C}VSConfig.cmd: Invalid or unsupported CMake build type passed: !BUILD_TYPE!. Cannot proceed, aborting!{# #}{\n}
    pause
    GOTO :EOF
)
:: DEBUG_OR_RELEASE and DEBUG_OR_RELEASE_LOWERCASE are "Debug" and "debug" for Debug build and "Release" and "release"
:: for all of the Release variants. Lowercase version exists for Qt/nmake/jom.
:: POSTFIX_D, POSTFIX_UNDERSCORE_D and POSTFIX_UNDERSCORE_DEBUG are helpers for performing file copies and checking
:: for existence of files. In release build these variables are empty.
set DEBUG_OR_RELEASE=Release
set DEBUG_OR_RELEASE_LOWERCASE=release
set POSTFIX_D=
set POSTFIX_UNDERSCORE_D=
set POSTFIX_UNDERSCORE_DEBUG=
IF %BUILD_TYPE%==Debug (
    set DEBUG_OR_RELEASE=Debug
    set DEBUG_OR_RELEASE_LOWERCASE=debug
    set POSTFIX_D=d
    set POSTFIX_UNDERSCORE_D=_d
    set POSTFIX_UNDERSCORE_DEBUG=_debug
)

:: Make sure deps folder exists.
IF NOT EXIST "%DEPS%". mkdir "%DEPS%"

:: User-defined variables
set BUILD_OPENSSL=TRUE
set USE_JOM=TRUE
set USE_BOOST=FALSE

:: Validate user-defined variables
IF NOT %BUILD_OPENSSL%==FALSE IF NOT %BUILD_OPENSSL%==TRUE (
   cecho {0E}BUILD_OPENSSL needs to be either TRUE or FALSE!{# #}{\n}
   GOTO :ERROR
)
IF NOT %USE_JOM%==FALSE IF NOT %USE_JOM%==TRUE (
   cecho {0E}USE_JOM needs to be either TRUE or FALSE!{# #}{\n}
   GOTO :ERROR
)
IF NOT %USE_BOOST%==FALSE IF NOT %USE_BOOST%==TRUE (
   cecho {0E}USE_BOOST needs to be either TRUE or FALSE!{# #}{\n}
   GOTO :ERROR
)

IF %GENERATOR%=="" (
   cecho {0C}GENERATOR not specified - cannot proceed!{# #}{\n}
   GOTO :ERROR
)

:: If we use VS2008, framework path (for msbuild) may not be correctly set. Manually attempt to add in that case
IF %VS_VER%==vs2008 set PATH=C:\Windows\Microsoft.NET\Framework\v3.5;%PATH%

:: Print user-defined variables
cecho {F0}This script fetches and builds all Tundra dependencies{# #}{\n}
echo.
cecho {0A}Script configuration:{# #}{\n}
cecho {0D}  CMake Generator      = %GENERATOR%{# #}{\n}
echo    - Passed to CMake -G option.
cecho {0D}  Target Architecture  = %TARGET_ARCH%{# #}{\n}
echo    - Whether were doing 32-bit (x86) or 64-bit (x64) build.
cecho {0D}  Dependency Directory = %DEPS%{# #}{\n}
echo    - The directory where Tundra dependencies are fetched and built.
cecho {0D}  Use Boost            = %USE_BOOST%{# #}{\n}
echo    - Configures whether dependencies kNet, Ogre, and AssImp are built using Boost.
cecho {0D}  Build Type           = %BUILD_TYPE%{# #}{\n}
echo    - The used build type for the dependencies.
echo      Defaults to RelWithDebInfo if not specified.
IF %BUILD_TYPE%==MinSizeRel cecho {0E}     WARNING: MinSizeRel build can suffer from a significant performance loss.{# #}{\n}
cecho {0D}  Build OpenSSL        = %BUILD_OPENSSL%{# #}{\n}
echo    - Build OpenSSL, requires Active Perl.
cecho {0D}  Build Qt with JOM    = %USE_JOM%{# #}{\n}
echo    - Use jom.exe instead of nmake.exe to build qmake projects.
echo      Default enabled as jom is significantly faster by usin all CPUs.
echo.

:: Print scripts usage information
cecho {0A}Requirements for a successful execution:{# #}{\n}
echo    1. Install SVN and make sure 'svn' is accessible from PATH.
echo     - http://tortoisesvn.net/downloads.html, install with command line tools!
echo    2. Install Hg and make sure 'hg' is accessible from PATH.
echo     - http://tortoisehg.bitbucket.org/
echo    3. Install Git and make sure 'git' is accessible from PATH.
echo     - http://code.google.com/p/tortoisegit/
echo    4. Install DirectX SDK June 2010.
echo     - http://www.microsoft.com/download/en/details.aspx?id=6812
echo    5. Install CMake and make sure 'cmake' is accessible from PATH.
echo     - http://www.cmake.org/
echo    6. Install Visual Studio 2008/2010 (Express is ok, but see section 7).
echo     - http://www.microsoft.com/visualstudio/eng/downloads
cecho {0E}   7. Optional: Make sure you have the Visual Studio x64 tools installed{# #}{\n}
cecho {0E}      before installing the Visual Studio Service Pack 1 (section 8), if{# #}{\n}
cecho {0E}      wanting to build Tundra as a 64-bit application.{# #}{\n}
cecho {0E}      NOTE: The x64 tools are not available for the Express editions.{# #}{\n}
echo    8. Install Visual Studio 2008/2010 Service Pack 1.
echo     - http://www.microsoft.com/en-us/download/details.aspx?id=23691
echo    9. Install Windows SDK.
echo     - http://www.microsoft.com/download/en/details.aspx?id=8279

IF %BUILD_OPENSSL%==TRUE (
   echo   10. To build OpenSSL install Active Perl and set perl.exe to PATH.
   echo     - http://www.activestate.com/activeperl/downloads
   cecho {0E}      NOTE: Perl needs to be before git in PATH, otherwise the git{# #}{\n}
   cecho {0E}      provided perl.exe will be used and OpenSSL build will fail.{# #}{\n}
   echo   11. Execute this file from Visual Studio 2008/2010 ^(x64^) Command Prompt.
) ELSE (
   echo   10. Execute this file from Visual Studio 2008/2010 ^(x64^) Command Prompt.
)
echo.

cecho {0A}Assuming Tundra git trunk is found at %TUNDRA_DIR%.{# #}{\n}
cecho {0E}Warning: The path %TUNDRA_DIR% may not contain spaces! (qmake breaks on them).{# #}{\n}
cecho {0E}Warning: You will need roughly 15GB of disk space to proceed.{# #}{\n}
cecho {0E}Warning: This script is not fully unattended once you continue.{# #}{\n}
cecho {0E}         When building Qt, you must press 'y' once for the script to proceed.{# #}{\n}
echo.

echo If you are not ready with the above, press Ctrl-C to abort!
pause
echo.

:: OpenSSL
IF %BUILD_OPENSSL%==FALSE (
    cecho {0D}Building OpenSSL disabled. Skipping.{# #}{\n}
    GOTO :SKIP_OPENSSL
)

set OPENSSL_VERSION=0.9.8x
IF NOT EXIST "%DEPS%\openssl\src". (
    cd "%DEPS%"
    IF NOT EXIST openssl-%OPENSSL_VERSION%.tar.gz. (
        cecho {0D}Downloading OpenSSL %OPENSSL_VERSION%.{# #}{\n}
        wget http://www.openssl.org/source/openssl-%OPENSSL_VERSION%.tar.gz
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    )

    mkdir openssl
    cecho {0D}Extracting OpenSSL %OPENSSL_VERSION% sources to "%DEPS%\openssl\src".{# #}{\n}
    7za e -y openssl-%OPENSSL_VERSION%.tar.gz
    7za x -y -oopenssl openssl-%OPENSSL_VERSION%.tar
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    cd openssl
    ren openssl-%OPENSSL_VERSION% src
    cd ..
    IF NOT EXIST "%DEPS%\openssl\src". (
        cecho {0E}Failed to rename %DEPS%\openssl\openssl-%OPENSSL_VERSION% to %DEPS%\openssl\src. Permission denied for your account?{# #}{\n}
        GOTO :ERROR
    )
    del openssl-%OPENSSL_VERSION%.tar
) ELSE (
    cecho {0D}OpenSSL already downloaded. Skipping.{# #}{\n}
)

set OPENSSL_OUTPUT_PREFIX=%DEPS%\openssl\out-%VS_VER%-%TARGET_ARCH%
IF NOT EXIST "%OPENSSL_OUTPUT_PREFIX%\bin\ssleay32.dll". (
    cd "%DEPS%\openssl\src"
    cecho {0D}Configuring OpenSSL build.{# #}{\n}
    IF %TARGET_ARCH%==x64 (
        perl Configure VC-WIN64A --prefix=%OPENSSL_OUTPUT_PREFIX%
    ) ELSE (
        perl Configure VC-WIN32 --prefix=%OPENSSL_OUTPUT_PREFIX%
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    REM Build Makefiles  with assembly language files. ml.exe is a part of Visual Studio
    cecho {0D}Building OpenSSL. Please be patient, this will take a while.{# #}{\n}    
    if %TARGET_ARCH%==x64 (
        call ms\do_win64a.bat
    ) else (
        call ms\do_masm.bat
    )
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak install
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    xcopy /E /I /C /H /R /Y "%OPENSSL_OUTPUT_PREFIX%\*.*" "%DEPS%\openssl"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}OpenSSL already built. Skipping.{# #}{\n}
)

cecho {0D}Deploying %TARGET_ARCH% OpenSSL DLLs to Tundra bin\{# #}{\n}
copy /Y "%DEPS%\openssl\bin\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:SKIP_OPENSSL

:: Qt
:: NOTE For VS2012 support Qt 4.8.3>= needed: http://stackoverflow.com/questions/12113400/compiling-qt-4-8-x-for-visual-studio-2012
:: In theory, 4.8.4 should support VS2012 out-of-the-box, but in practice doesn't...
:: NOTE ftp://ftp.qt-project.org/qt/source/ Can be used for Qt < 4.8.2, for Qt >= 4.8.2 use http://releases.qt-project.org/qt4/source/
::set QT_VER=4.7.4
::set QT_URL=ftp://ftp.qt-project.org/qt/source/qt-everywhere-opensource-src-%QT_VER%.zip
set QT_VER=4.8.4
set QT_URL=http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-%QT_VER%.zip
IF NOT EXIST "%DEPS%\qt". (
   cd "%DEPS%"
   IF NOT EXIST qt-everywhere-opensource-src-%QT_VER%.zip. (
      cecho {0D}Downloading Qt %QT_VER%. Please be patient, this will take a while.{# #}{\n}
      wget %QT_URL%
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   )

   cecho {0D}Extracting Qt %QT_VER% sources to "%DEPS%\qt".{# #}{\n}
   mkdir qt
   7za x -y -oqt qt-everywhere-opensource-src-%QT_VER%.zip
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd qt
   ren qt-everywhere-opensource-src-%QT_VER% qt-src-%QT_VER%
   IF NOT EXIST "%DEPS%\qt" GOTO :ERROR
) ELSE (
   cecho {0D}Qt already downloaded. Skipping.{# #}{\n}
)

IF %USE_JOM%==FALSE GOTO :SKIP_JOM
IF NOT EXIST "%DEPS%\qt\jom\jom.exe". (
   cd "%DEPS%"
   IF NOT EXIST jom_1_0_11.zip. (
      cecho {0D}Downloading JOM build tool for Qt.{# #}{\n}
      wget http://releases.qt-project.org/jom/jom_1_0_11.zip
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   )

   cecho {0D}Installing JOM build tool for to %DEPS%\qt\jom.{# #}{\n}
   mkdir %DEPS%\qt\jom
   7za x -y -oqt\jom jom_1_0_11.zip
) ELSE (
   cecho {0D}JOM already installed to %DEPS%\qt\jom. Skipping.{# #}{\n}
)

:SKIP_JOM

:: Enable OpenSSL in the Qt if OpenSSL build is enabled. For some reason if you 
:: echo QT_OPENSSL_CONFIGURE inside the IF statement it will always be empty. 
:: Hence the secondary IF to print it out when build is enabled. Only print these if Qt will be built
SET QT_INSTALL_WEBKIT_DLL_FILENAME="%DEPS%\qt\lib\QtWebKit%POSTFIX_D%4.dll"
SET QT_OPENSSL_CONFIGURE=
IF %BUILD_OPENSSL%==TRUE (
   IF NOT EXIST %QT_INSTALL_WEBKIT_DLL_FILENAME%. cecho {0D}Configuring OpenSSL into the Qt build with:{# #}{\n}
   SET QT_OPENSSL_CONFIGURE=-openssl -I "%DEPS%\openssl\include" -L "%DEPS%\openssl\lib"
) ELSE (
   IF NOT EXIST %QT_INSTALL_WEBKIT_DLL_FILENAME%. cecho {0D}OpenSSL build disabled, not confguring OpenSSL to Qt.{# #}{\n}
)
IF %BUILD_OPENSSL%==TRUE (
   IF NOT EXIST %QT_INSTALL_WEBKIT_DLL_FILENAME%. echo '%QT_OPENSSL_CONFIGURE%'
)

:: Set QMAKESPEC and QTDIR in case we are going to build qt. If we don't do this
:: a system set QMAKESPEC might take over the build in some bizarre fashion.
:: Note 1: QTDIR is not used while build, neither should QMAKESPEC be used when -platform is given to configure.
:: Note 2: We cannot do this inside the qt IF without @setlocal EnableDelayedExpansion.
set QTDIR=%DEPS%\qt\qt-src-%QT_VER%
set QMAKESPEC=%QTDIR%\mkspecs\%QT_PLATFORM%

IF NOT EXIST %QT_INSTALL_WEBKIT_DLL_FILENAME%. (
    IF NOT EXIST "%QTDIR%". (
        cecho {0E}Warning: %QTDIR% does not exist, extracting Qt failed?.{# #}{\n}
        GOTO :ERROR
    )

    cd %QTDIR%
    IF EXIST configure.cache. del /Q configure.cache
    cecho {0D}Configuring Qt build. Please answer 'y'!{# #}{\n}
    configure -platform %QT_PLATFORM% -%DEBUG_OR_RELEASE_LOWERCASE% -opensource -prefix "%DEPS%\qt" -shared -ltcg ^
        -no-qt3support -no-opengl -no-openvg -no-dbus -no-phonon -no-phonon-backend -no-multimedia -no-audio-backend ^
        -no-declarative -no-xmlpatterns -nomake examples -nomake demos ^
        -qt-zlib -qt-libpng -qt-libmng -qt-libjpeg -qt-libtiff %QT_OPENSSL_CONFIGURE%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    IF %USE_JOM%==TRUE (
        cecho {0D}Building %DEBUG_OR_RELEASE% Qt with jom. Please be patient, this will take a while.{# #}{\n}
        "%DEPS%\qt\jom\jom.exe"
    ) ELSE (
        cecho {0D}Building %DEBUG_OR_RELEASE% Qt with nmake. Please be patient, this will take a while.{# #}{\n}
        nmake /nologo
    )

    IF NOT EXIST "%QTDIR%\lib\QtWebKit%POSTFIX_D%4.dll". (
        cecho {0E}Warning: %QTDIR%\lib\QtWebKit%POSTFIX_D%4.dll not present, Qt build failed?.{# #}{\n}
        GOTO :ERROR
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    :: Don't use jom for install. It seems to hang easily, maybe beacuse it tries to use multiple cores.
    cecho {0D}Installing Qt to %DEPS%\qt{# #}{\n}
    nmake install
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    IF NOT EXIST %QT_INSTALL_WEBKIT_DLL_FILENAME%. (
        cecho {0E}Warning: %QT_INSTALL_WEBKIT_DLL_FILENAME% not present, Qt install failed?.{# #}{\n}
        GOTO :ERROR
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}Qt already built. Skipping.{# #}{\n}
)

:: Setup now built Qt to PATH (for qmake.exe), QTDIR and QMAKESPEC.
:: These will be utilized by other dependencies that need Qt in this script.
set PATH=%DEPS%\qt\bin;%PATH%
set QMAKESPEC=%DEPS%\qt\mkspecs\%QT_PLATFORM%
set QTDIR=%DEPS%\qt

cecho {0D}Deploying %DEBUG_OR_RELEASE% Qt DLLs to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\qt\bin\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
IF NOT EXIST "%TUNDRA_BIN%\qtplugins". mkdir "%TUNDRA_BIN%\qtplugins"
xcopy /E /I /C /H /R /Y "%DEPS%\qt\plugins\*.*" "%TUNDRA_BIN%\qtplugins"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
:: Clean out some definately not needed Qt DLLs from bin
del /Q "%TUNDRA_BIN%\QtCLucene*.dll"
del /Q "%TUNDRA_BIN%\QtDesigner*.dll"
del /Q "%TUNDRA_BIN%\QtHelp*.dll"
del /Q "%TUNDRA_BIN%\QtScriptTools*.dll"
del /Q "%TUNDRA_BIN%\QtSql*.dll"
del /Q "%TUNDRA_BIN%\QtSvg*.dll"
del /Q "%TUNDRA_BIN%\QtTest*.dll"

:: Bullet physics engine
:: version 2.81 sp1, svn rev 2613
IF NOT EXIST "%DEPS%\bullet\". (
    cecho {0D}Cloning Bullet into "%DEPS%\bullet".{# #}{\n}
    cd "%DEPS%"
    svn checkout http://bullet.googlecode.com/svn/trunk@2613 bullet
    IF NOT EXIST "%DEPS%\bullet\.svn" GOTO :ERROR
)

IF NOT EXIST "%DEPS%\bullet\lib\%BUILD_TYPE%\BulletCollision.lib". (
    cd "%DEPS%\bullet\"
    :: If the CMake generator has changed from the previous run, delete the cache file.
    :: TODO This logic is not probably necessary as the deps for different generators are already in different dirs.
    findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
    IF NOT %ERRORLEVEL%==0 (
        IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
        IF EXIST BULLET_PHYSICS.sln. del /Q BULLET_PHYSICS.sln
    )
    IF NOT EXIST BULLET_PHYSICS.sln. (
        cecho {0D}Running CMake for Bullet.{# #}{\n}
        IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
        cmake . -G %GENERATOR% -DBUILD_DEMOS:BOOL=OFF -DBUILD_EXTRAS:BOOL=OFF -DBUILD_INTEL_OPENCL_DEMOS:BOOL=OFF ^
            -DBUILD_NVIDIA_OPENCL_DEMOS:BOOL=OFF -DBUILD_UNIT_TESTS:BOOL=OFF -DUSE_DX11:BOOL=OFF -DBUILD_AMD_OPENCL_DEMOS:BOOL=OFF ^
            -DCMAKE_DEBUG_POSTFIX= -DCMAKE_MINSIZEREL_POSTFIX= -DCMAKE_RELWITHDEBINFO_POSTFIX=
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    )

    cecho {0D}Building %BUILD_TYPE% Bullet. Please be patient, this will take a while.{# #}{\n}
    MSBuild BULLET_PHYSICS.sln /p:configuration=%BUILD_TYPE% /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}Bullet already built. Skipping.{# #}{\n}
)

set BOOST_ROOT=%DEPS%\boost
set BOOST_INCLUDEDIR=%DEPS%\boost
set BOOST_LIBRARYDIR=%DEPS%\boost\stage\lib

IF %USE_BOOST%==FALSE (
   cecho {0D}USE_BOOST set to FALSE. Skipping Boost.{# #}{\n}
   GOTO :SKIP_BOOST
)

IF NOT EXIST "%DEPS%\boost". (
    cecho {0D}Cloning Boost into "%DEPS%\boost".{# #}{\n}
    cd "%DEPS%"
    svn checkout http://svn.boost.org/svn/boost/tags/release/Boost_1_49_0 boost
    IF NOT EXIST "%DEPS%\boost\.svn" GOTO :ERROR
    IF NOT EXIST "%DEPS%\boost\boost.css" GOTO :ERROR
    cd "%DEPS%\boost"
    cecho {0D}Building Boost build script.{# #}{\n}
    call bootstrap

    :: Copy proper config file depending on the used VS version.
    copy /Y "%TOOLS%\Mods\boost-user-config-%VS_VER%.jam" "%DEPS%\boost\tools\build\v2\user-config.jam"

    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    cd "%DEPS%\boost"
    cecho {0D}Building Boost. Please be patient, this will take a while.{# #}{\n}
    :: Building boost with single core takes ages, so utilize all cores for the build process
    call .\b2 -j %NUMBER_OF_PROCESSORS% --without-mpi thread regex stage
) ELSE (
    ::TODO Even if %DEPS%\boost exists, we have no guarantee that boost is built successfully for real
    cecho {0D}Boost already built. Skipping.{# #}{\n}
)

:SKIP_BOOST

IF NOT EXIST "%DEPS%\assimp\". (
    cecho {0D}Checking out OpenAssetImport library from https://assimp.svn.sourceforge.net/svnroot/assimp/trunk into "%DEPS%\assimp".{# #}{\n}
    cd "%DEPS%"
    :: Note the fixed revision number. OpenAssetImport does not have an up-to-date tagged release, so fix to a recent revision of trunk.
    svn checkout -r 1300 https://assimp.svn.sourceforge.net/svnroot/assimp/trunk assimp
)

IF NOT EXIST "%DEPS%\assimp\bin\%BUILD_TYPE%\assimp.dll". (
    cd "%DEPS%\assimp"
    IF %USE_BOOST%==FALSE (
        :: Tweaks CMakeLists.txt to set ASSIMP_ENABLE_BOOST_WORKAROUND on.
        sed s/"ASSIMP_ENABLE_BOOST_WORKAROUND OFF"/"ASSIMP_ENABLE_BOOST_WORKAROUND ON"/g <CMakeLists.txt >CMakeLists.txt.sed
        del CMakeLists.txt
        rename CMakeLists.txt.sed CMakeLists.txt
    )

    :: If the CMake generator has changed from the previous run, delete the cache file.
    findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
    IF NOT %ERRORLEVEL%==0 (
        IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
        IF EXIST Assimp.sln. del /Q Assimp.sln
    )

    IF NOT EXIST Assimp.sln. (
        cecho {0D}Running CMake for OpenAssetImport.{# #}{\n}
        cmake -G %GENERATOR%
    )

    cecho {0D}Building %BUILD_TYPE% OpenAssetImport. Please be patient, this will take a while.{# #}{\n}
    MSBuild Assimp.sln /p:configuration=%BUILD_TYPE% /nologo /m:%NUMBER_OF_PROCESSORS%
) ELSE (
    cecho {0D}OpenAssetImport already built. Skipping.{# #}{\n}
)

:: Copy the correct runtime to /bin for this run
cecho {0D}Deploying %BUILD_TYPE% OpenAssetImporter DLL to Tundra bin\ directory.{# #}{\n}
copy /Y "%DEPS%\assimp\bin\%BUILD_TYPE%\assimp%POSTFIX_D%.dll" "%TUNDRA_BIN%"

:: TODO Remove BUILD_KNET when we don't need to hack kNet's CMakeLists.txt anymore
set BUILD_KNET=TRUE
IF NOT EXIST "%DEPS%\kNet\". (
    cecho {0D}Cloning kNet from https://github.com/juj/kNet into "%DEPS%\kNet".{# #}{\n}
    cd "%DEPS%"
    call git clone https://github.com/juj/kNet
    IF NOT EXIST "%DEPS%\kNet\.git" GOTO :ERROR
) ELSE (
    cecho {0D}Updating kNet to newest version from https://github.com/juj/kNet.{# #}{\n}
    cd "%DEPS%\kNet"
    call git pull >"%DEPS%\kNet\git-check"
    set /p KNET_GIT_CHECK=<"%DEPS%\kNet\git-check"
    echo !KNET_GIT_CHECK!
    IF "!KNET_GIT_CHECK!"=="Already up-to-date." set BUILD_KNET=FALSE
    del "%DEPS%\kNet\git-check"
)

cd "%DEPS%\kNet"
:: If the CMake generator has changed from the previous run, delete the cache file.
findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
IF NOT %ERRORLEVEL%==0 (
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    IF EXIST kNet.sln. del /Q kNet.sln
)
IF NOT EXIST kNet.sln. (
    cecho {0D}Running cmake for kNet.{# #}{\n}

    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    cmake . -G %GENERATOR% -DBOOST_ROOT=%BOOST_ROOT% -DUSE_BOOST:BOOL=%USE_BOOST% -DUSE_TINYXML:BOOL=FALSE
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    set BUILD_KNET=TRUE
)

IF NOT EXIST "%DEPS%\kNet\lib\%BUILD_TYPE%\kNet.lib". set BUILD_KNET=TRUE

IF %BUILD_KNET%==TRUE (
    cecho {0D}Building %BUILD_TYPE% kNet. Please be patient, this will take a while.{# #}{\n}
    MSBuild kNet.sln /p:configuration=%BUILD_TYPE% /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}%BUILD_TYPE% kNet already built and up to date. Skipping.{# #}{\n}
)

:: QtScriptGenerator
IF NOT EXIST "%DEPS%\qtscriptgenerator\.git". (
   cecho {0D}Cloning QtScriptGenerator into "%DEPS%\qtscriptgenerator".{# #}{\n}
   cd "%DEPS%"
   call git clone https://git.gitorious.org/qt-labs/qtscriptgenerator
   IF NOT EXIST "%DEPS%\qtscriptgenerator\.git" GOTO :ERROR
) ELSE (
   cecho {0D}QtScriptGenerator already cloned. Skipping.{# #}{\n}
)

:: 1) QtScriptGenerator's generator executable.
:: For QtScriptGenerator's generator.exe we only need the release build.
IF NOT EXIST "%DEPS%\qtscriptgenerator\generator\release\generator.exe". (
    cd "%DEPS%\qtscriptgenerator\generator"
    cecho {0D}Running qmake for QtScriptGenerator.{# #}{\n}
    :: We need to patch pp-iterator.h in order to make it compile with newer Visual Studio versions:
    :: http://stackoverflow.com/questions/2791525/stl-operator-behavior-change-with-visual-studio-2010
    :: Also cannot use QMake as it results in linker errors, so instead generate vcproj files and build using MSBuild.
    IF NOT %VS_VER%==vs2008 (
        copy /Y "%TOOLS%\Mods\QtScriptGenerator_pp-iterator.h" "%DEPS%\qtscriptgenerator\generator\parser\rpp\pp-iterator.h"
        qmake -tp vc
        cecho {0D}Building QtScriptGenerator. Please be patient, this will take a while.{# #}{\n}
        MSBuild generator.vcxproj /p:configuration=Release /nologo /m:%NUMBER_OF_PROCESSORS%
    ) ELSE (
        qmake
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
        cecho {0D}Building QtScriptGenerator.{# #}{\n}
        IF %USE_JOM%==TRUE (
            cecho {0D}- Building QtScriptGenerator with jom. Please be patient, this will take a while.{# #}{\n}
            "%DEPS%\qt\jom\jom.exe"
        ) ELSE (
            cecho {0D}- Building QtScriptGenerator with nmake. Please be patient, this will take a while.{# #}{\n}
            nmake /nologo
        )
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR    
)
:: 2) QtScriptGenerator's qtbindings - the actual bindings.
IF NOT EXIST "%DEPS%\qtscriptgenerator\plugins\script\qtscript_webkit%POSTFIX_D%.dll". (
    cd "%DEPS%\qtscriptgenerator\generator"
    cecho {0D}Executing QtScriptGenerator.{# #}{\n}
    call release\generator
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cd ..
    cd qtbindings

    :: Do not generate bindings for unneeded Qt modules.
    sed -e "s/qtscript_phonon //" -e "s/qtscript_opengl //" -e "s/qtscript_uitools //" -e "s/qtscript_xmlpatterns //" < qtbindings.pro > qtbindings.pro.sed
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    del /Q qtbindings.pro
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    ren qtbindings.pro.sed qtbindings.pro
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    REM Fix bad script generation for webkit.
    REM TODO: Could try some sed replacement, but can't make the regex escaping rules work from command line.
    REM sed -e s/"QWebPluginFactory_Extension_values[] = "/"QWebPluginFactory_Extension_values[1] = "// -e "s/qtscript_QWebPluginFactory_Extension_keys[] = /qtscript_QWebPluginFactory_Extension_keys[1] = //" < "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit\qtscript_QWebPluginFactory.cpp" > "%DEPS%\qtscript_QWebPluginFactory.cpp"
    REM IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    del "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit\qtscript_QWebPluginFactory.cpp"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    copy /Y "%TOOLS%\Mods\qtscript_QWebPluginFactory.cpp" "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Running qmake for QtBindings plugins.{# #}{\n}
    qmake
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF %USE_JOM%==TRUE (
        cecho {0D}- Building %DEBUG_OR_RELEASE% QtBindings with jom. Please be patient, this will take a while.{# #}{\n}
        "%DEPS%\qt\jom\jom.exe" %DEBUG_OR_RELEASE_LOWERCASE%
    ) ELSE (
        cecho {0D}- Building %DEBUG_OR_RELEASE% QtBindings with nmake. Please be patient, this will take a while.{# #}{\n}
        nmake /nologo %DEBUG_OR_RELEASE_LOWERCASE%
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}QtScriptGenerator already built. Skipping.{# #}{\n}
)

cecho {0D}Deploying QtScript binding plugin DLLs.{# #}{\n}
IF NOT EXIST "%TUNDRA_BIN%\qtplugins\script" mkdir "%TUNDRA_BIN%\qtplugins\script"
xcopy /Q /E /I /C /H /R /Y "%DEPS%\qtscriptgenerator\plugins\script\*.dll" "%TUNDRA_BIN%\qtplugins\script"

IF NOT EXIST "%DEPS%\realxtend-tundra-deps\.git". (
    cecho {0D}Cloning realxtend-tundra-deps repository into "%DEPS%\realxtend-tundra-deps".{# #}{\n}
    REM Only clone/fetch the sources branch. Skipping the prebuilt binary branches as they are huge.
    cd "%DEPS%"
    call git init realxtend-tundra-deps
    cd realxtend-tundra-deps
    call git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    call git remote add origin https://code.google.com/p/realxtend-tundra-deps/
    call git checkout sources
    IF NOT EXIST "%DEPS%\realxtend-tundra-deps\.git" GOTO :ERROR
) ELSE (
    cecho {0D}Updating realxtend-tundra-deps to newest.{# #}{\n}
    cd "%DEPS%\realxtend-tundra-deps"
    call git fetch origin sources:refs/remotes/origin/sources
    git rebase origin/sources
)

set OGRE_HOME=%DEPS%\ogre-safe-nocrashes\SDK

IF NOT EXIST "%DEPS%\ogre-safe-nocrashes\.hg". (
   cecho {0D}Cloning Ogre from https://bitbucket.org/clb/ogre-safe-nocrashes into "%DEPS%\ogre-safe-nocrashes".{# #}{\n}
   cd "%DEPS%"
   hg clone https://bitbucket.org/clb/ogre-safe-nocrashes
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   IF NOT EXIST "%DEPS%\ogre-safe-nocrashes\.hg" GOTO :ERROR
   cd ogre-safe-nocrashes
   hg checkout v1-8
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Updating ogre-safe-nocrashes to newest version from https://bitbucket.org/clb/ogre-safe-nocrashes.{# #}{\n}
   cd "%DEPS%\ogre-safe-nocrashes"
   hg pull -u
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%DEPS%\ogre-safe-nocrashes\RenderSystems\RenderSystem_NULL". (
   cecho {0D}Attaching RenderSystem_NULL to be built with ogre-safe-nocrashes.{# #}{\n}
   mkdir "%DEPS%\ogre-safe-nocrashes\RenderSystems\RenderSystem_NULL"
   copy /Y "%TOOLS%\Mods\Ogre_RenderSystems_CMakeLists.txt" "%DEPS%\ogre-safe-nocrashes\RenderSystems\CMakeLists.txt"
)

REM Instead of the copying above, would like to do the line below, but IF () terminates prematurely on the ) below!
REM   echo add_subdirectory(RenderSystem_NULL) >> "%DEPS%\ogre-safe-nocrashes\RenderSystems\CMakeLists.txt"

cecho {0D}Updating RenderSystem_NULL to the newest version in ogre-safe-nocrashes.{# #}{\n}
xcopy /Q /E /I /C /H /R /Y "%DEPS%\realxtend-tundra-deps\RenderSystem_NULL" "%DEPS%\ogre-safe-nocrashes\RenderSystems\RenderSystem_NULL"

:: Ogre dependencies
IF NOT EXIST "%DEPS%\ogre-safe-nocrashes\ogredeps\.hg". (
    cecho {0D}Cloning Ogre dependencies from https://bitbucket.org/cabalistic/ogredeps into "%DEPS%\ogre-safe-nocrashes\ogredeps".{# #}{\n}
    cd "%DEPS%\ogre-safe-nocrashes\"
    hg clone https://bitbucket.org/cabalistic/ogredeps
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF NOT EXIST "%DEPS%\ogre-safe-nocrashes\ogredeps\.hg" GOTO :ERROR
) ELSE (
    cecho {0D}Updating Ogre dependencies to the newest version from https://bitbucket.org/cabalistic/ogredeps.{# #}{\n}
    cd "%DEPS%\ogre-safe-nocrashes\ogredeps"
    hg pull -u
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cd "%DEPS%\ogre-safe-nocrashes\ogredeps"
findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
IF NOT %ERRORLEVEL%==0 (
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    IF EXIST OGREDEPS.sln. del /Q OGREDEPS.sln
)
IF NOT EXIST OGREDEPS.sln. (
    :: We need modified version for couple of the ogredeps CMakeLists.txts in order to support MinSizeRel and RelWithDebInfo builds.
    copy /Y "%TOOLS%\Mods\ogredeps_CMakeLists.txt" src\CMakeLists.txt
    copy /Y "%TOOLS%\Mods\ogredeps_Cg_CMakeLists.txt" src\Cg\CMakeLists.txt
    cecho {0D}Running CMake for Ogre dependencies.{# #}{\n}
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    cmake . -G %GENERATOR% -DOGREDEPS_BUILD_OIS:BOOL=OFF -DCMAKE_INSTALL_PREFIX="%DEPS%\ogre-safe-nocrashes\Dependencies"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building %BUILD_TYPE% Ogre dependencies. Please be patient, this will take a while.{# #}{\n}
MSBuild OGREDEPS.sln /p:configuration=%BUILD_TYPE% /nologo /m:%NUMBER_OF_PROCESSORS% /clp:ErrorsOnly
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

MSBuild INSTALL.%VCPROJ_FILE_EXT% /p:configuration=%BUILD_TYPE% /nologo /clp:ErrorsOnly
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Use Intel Thread Building Blocks for Ogre's threading if Boost is not used.
:: Latest 4.0 is used as 4.1 introduces WIN32 API calls that do not work on windows xp.
:: If we are to update this can be fixed in tbb windows_api.h by using the xp workaround on all platforms.
set TBB_VERSION=tbb40_20120613oss
set TBB_HOME=%DEPS%\ogre-safe-nocrashes\Dependencies\tbb
IF %USE_BOOST%==FALSE (
    cd "%DEPS%\ogre-safe-nocrashes"
    IF NOT EXIST %TBB_VERSION%_win.zip. (
        cecho {0D}Downloading Intel Thread Building Blocks prebuilt package.{# #}{\n}
        wget "http://threadingbuildingblocks.org/sites/default/files/software_releases/windows/%TBB_VERSION%_win.zip"
        IF NOT EXIST %TBB_VERSION%_win.zip. (
            cecho {0C}Error downloading Intel Thread Building Blocks! Aborting!{# #}{\n}
            GOTO :ERROR
        )
    )
    IF NOT EXIST "%TBB_HOME%". (
        cecho {0D}Extracting Intel Thread Building Blocks package.{# #}{\n}
        7za x -y %TBB_VERSION%_win.zip -oDependencies
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
        cd Dependencies
        rename %TBB_VERSION% tbb
        cd..

        REM nedmalloc messes up the _WIN32_WINNT define which causes TBB headers not to compile on VS2008
        REM (and reportedly on VS2012 too), so fix that. See https://ogre3d.atlassian.net/browse/OGRE-119
        REM TODO Remove when possible (when moving to newer Ogre).
        cd OgreMain\src\nedmalloc
        sed s/"#define _WIN32_WINNT 0x403"//g <malloc.c.h >malloc.c.h.sed
        del malloc.c.h
        rename malloc.c.h.sed malloc.c.h

        REM TODO Apparently the above fix is not enough and VC9 build fails to TBB header due erroneously defined _WIN32_WINNT.
        REM Hack around the error thrown in _tbb_windef.h.
        cd "%TBB_HOME%\include\tbb\internal"
        sed s@"#error TBB is unable to run on old Windows versions;"@"//#error TBB is unable to run on old Windows versions;"@g <_tbb_windef.h >_tbb_windef.h.sed
        del _tbb_windef.h
        rename _tbb_windef.h.sed _tbb_windef.h
    )
)

:: Copy TBB DLLs.
cecho {0D}Deploying %DEBUG_OR_RELEASE% %INTEL_ARCH% %VC_VER% TBB DLL to Tundra bin\ directory.{# #}{\n}
copy /Y "%TBB_HOME%\bin\%INTEL_ARCH%\%VC_VER%\tbb%POSTFIX_UNDERSCORE_DEBUG%.dll" "%TUNDRA_BIN%"

cd "%DEPS%\ogre-safe-nocrashes"
:: If the CMake generator has changed from the previous run, delete the cache file.
findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
IF NOT %ERRORLEVEL%==0 (
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    IF EXIST OGRE.sln. del /Q OGRE.sln
)
IF NOT EXIST OGRE.sln. (
    :: If not wanting to use Boost with Ogre, we need slightly tweaked version of Ogre's Dependencies.cmake
    :: which doesn't enforce usage of Boost if it's found regardless of the value of OGRE_USE_BOOST
    IF %USE_BOOST%==FALSE (
        copy /Y "%TOOLS%\Mods\OgreNoBoost_Dependencies.cmake_" "%DEPS%\ogre-safe-nocrashes\CMake\Dependencies.cmake"
    )
    cecho {0D}Running cmake for ogre-safe-nocrashes.{# #}{\n}
    cmake -G %GENERATOR% -DTBB_HOME=%TBB_HOME% -DOGRE_USE_BOOST:BOOL=%USE_BOOST% -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF ^
        -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF -DOGRE_CONFIG_THREADS:INT=1
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building %BUILD_TYPE% ogre-safe-nocrashes. Please be patient, this will take a while.{# #}{\n}
MSBuild OGRE.sln /p:configuration=%BUILD_TYPE% /nologo /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying %BUILD_TYPE% ogre-safe-nocrashes SDK directory.{# #}{\n}
MSBuild INSTALL.%VCPROJ_FILE_EXT% /p:configuration=%BUILD_TYPE% /nologo /clp:ErrorsOnly
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying %BUILD_TYPE% Ogre DLLs to Tundra bin\ directory.{# #}{\n}
copy /Y "%DEPS%\ogre-safe-nocrashes\bin\%BUILD_TYPE%\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
del /Q "%TUNDRA_BIN%\OgrePaging*.dll"
del /Q "%TUNDRA_BIN%\OgreRTShaderSystem*.dll"
del /Q "%TUNDRA_BIN%\OgreTerrain*.dll"

cecho {0E}NOTE: Skipping PythonQt build for now!{# #}{\n}
REM IF NOT EXIST "%DEPS%\realxtend-tundra-deps\PythonQt\lib\PythonQt.lib". (
REM    cd "%DEPS%\realxtend-tundra-deps\PythonQt"
REM    IF NOT EXIST PythonQt.sln. (
REM       cecho {0D}Running qmake for PythonQt.{# #}{\n}
REM       qmake -tp vc -r PythonQt.pro
REM    )
REM    cecho {0D}Building PythonQt. Please be patient, this will take a while.{# #}{\n}
REM    msbuild PythonQt.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
REM    msbuild PythonQt.sln /p:configuration=Release /clp:ErrorsOnly /nologo
REM    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
REM ) ELSE (
REM    echo PythonQt already built. Skipping.{# #}{\n}
REM )

:: SkyX
cd "%DEPS%\realxtend-tundra-deps\skyx"
:: If the CMake generator has changed from the previous run, delete the cache file.
findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
IF NOT %ERRORLEVEL%==0 (
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    IF EXIST SKYX.sln. del /Q SKYX.sln
)
IF NOT EXIST SKYX.sln. (
   cecho {0D}Running cmake for SkyX.{# #}{\n}
   IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
   cmake . -G %GENERATOR% -DUSE_BOOST:BOOL=%USE_BOOST%
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building %BUILD_TYPE% SkyX. Please be patient, this will take a while.{# #}{\n}
MSBuild SKYX.sln /p:configuration=%BUILD_TYPE% /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying %BUILD_TYPE% SkyX DLL to Tundra bin\.{# #}{\n}

copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\%BUILD_TYPE%\SkyX%POSTFIX_UNDERSCORE_D%.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Hydrax
cd "%DEPS%\realxtend-tundra-deps\hydrax"
:: If the CMake generator has changed from the previous run, delete the cache file.
findstr /x /c:"CMAKE_GENERATOR:INTERNAL=%GENERATOR_NO_DOUBLEQUOTES%" CMakeCache.txt>NUL
IF NOT %ERRORLEVEL%==0 (
    IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
    IF EXIST Hydrax.sln. del /Q Hydrax.sln
)
IF NOT EXIST Hydrax.sln. (
  cecho {0D}Running cmake for Hydrax.{# #}{\n}
  IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
  cmake . -G %GENERATOR%
  IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building %BUILD_TYPE% Hydrax. Please be patient, this will take a while.{# #}{\n}
MSBuild Hydrax.sln /p:configuration=%BUILD_TYPE% /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying %BUILD_TYPE% Hydrax DLL to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\realxtend-tundra-deps\hydrax\bin\%BUILD_TYPE%\Hydrax%POSTFIX_D%.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: QtPropertyBrowser
IF NOT EXIST "%DEPS%\qt-solutions". (
    cecho {0D}Cloning QtPropertyBrowser into "%DEPS%\qt-solutions".{# #}{\n}
    cd "%DEPS%"
    call git clone https://git.gitorious.org/qt-solutions/qt-solutions.git
    IF NOT EXIST "%DEPS%\qt-solutions\.git" GOTO :ERROR
)

IF NOT EXIST "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head%POSTFIX_D%.dll" (
    cd "%DEPS%\qt-solutions\qtpropertybrowser"

    REM Don't build examples.
    sed -e "s/SUBDIRS+=examples//" < qtpropertybrowser.pro > qtpropertybrowser.pro.sed
    del qtpropertybrowser.pro
    ren qtpropertybrowser.pro.sed qtpropertybrowser.pro

    call configure -library
    qmake
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF %USE_JOM%==TRUE (
        cecho {0D}- Building %DEBUG_OR_RELEASE% QtPropertyBrowser with jom{# #}{\n}
        "%DEPS%\qt\jom\jom.exe" %DEBUG_OR_RELEASE_LOWERCASE%
    ) ELSE (
        cecho {0D}- Building %DEBUG_OR_RELEASE% QtPropertyBrowser with nmake{# #}{\n}
        nmake /nologo %DEBUG_OR_RELEASE_LOWERCASE%
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}QtPropertyBrowser already built. Skipping.{# #}{\n}
)

cecho {0D}Deploying %DEBUG_OR_RELEASE% QtPropertyBrowser DLLs.{# #}{\n}
copy /Y "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head%POSTFIX_D%.dll" "%TUNDRA_BIN%"

:: OpenAL
IF NOT EXIST "%DEPS%\OpenAL\libs\Win32\OpenAL32.lib". (
   cecho {0D}OpenAL does not exist. Unzipping a prebuilt package.{# #}{\n}
   copy "%TOOLS%\Utils\OpenAL.zip" "%DEPS%\"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd "%DEPS%\"
   7za x OpenAL.zip
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   del /Q OpenAL.zip
) ELSE (
   cecho {0D}OpenAL already prepared. Skipping.{# #}{\n}
)

:: Ogg
IF NOT EXIST "%DEPS%\ogg". (
    cecho {0D}Cloning Ogg into "%DEPS%\ogg".{# #}{\n}
    svn checkout http://svn.xiph.org/tags/ogg/libogg-1.3.0/ "%DEPS%\ogg"
)

IF NOT EXIST "%DEPS%\ogg\win32\%VS2008_OR_VS2010%\%VS_PLATFORM%\%DEBUG_OR_RELEASE%\libogg_static.lib". (
    cd "%DEPS%\ogg\win32\%VS2008_OR_VS2010%"

    cecho {0D}Building %DEBUG_OR_RELEASE% Ogg. Please be patient, this will take a while.{# #}{\n}
    MSBuild libogg_static.sln /p:configuration=%DEBUG_OR_RELEASE% /p:platform="%VS_PLATFORM%" /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}%DEBUG_OR_RELEASE% Ogg already built. Skipping.{# #}{\n}
)

:: Vorbis
IF NOT EXIST "%DEPS%\vorbis". (
    cecho {0D}Cloning Vorbis into "%DEPS%\vorbis".{# #}{\n}
    svn checkout http://svn.xiph.org/tags/vorbis/libvorbis-1.3.3/ "%DEPS%\vorbis"
)

IF NOT EXIST "%DEPS%\vorbis\win32\%VS2008_OR_VS2010%\%VS_PLATFORM%\%DEBUG_OR_RELEASE%\libvorbis_static.lib". (
    cd "%DEPS%\vorbis\win32\%VS2008_OR_VS2010%"
    cecho {0D}Building %DEBUG_OR_RELEASE% Vorbis. Please be patient, this will take a while.{# #}{\n}
    MSBuild vorbis_static.sln /p:configuration=%DEBUG_OR_RELEASE% /p:platform="%VS_PLATFORM%" /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}%DEBUG_OR_RELEASE% Vorbis already built. Skipping.{# #}{\n}
)

:: Theora
IF NOT EXIST "%DEPS%\theora". (
    cecho {0D}Cloning Theora into "%DEPS%\theora".{# #}{\n}
    svn checkout http://svn.xiph.org/tags/theora/libtheora-1.1.1/ "%DEPS%\theora"
)

IF %BUILD_TYPE%==Debug (
    set THEORA_BUILD_TYPE=Debug
) ELSE (
    set THEORA_BUILD_TYPE=Release_SSE2
)
IF NOT EXIST "%DEPS%\theora\win32\VS2008\%VS_PLATFORM%\%THEORA_BUILD_TYPE%\libtheora_static.lib". (
    cd "%DEPS%\theora\win32\VS2008"
    cecho {0D}Building %THEORA_BUILD_TYPE% Theora. Please be patient, this will take a while.{# #}{\n}
    MSBuild libtheora_static.sln /p:configuration=%THEORA_BUILD_TYPE%  /p:platform="%VS_PLATFORM%" /t:libtheora_static /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}%THEORA_BUILD_TYPE% Theora already built. Skipping.{# #}{\n}
)

:: Speex
IF NOT EXIST "%DEPS%\speex". (
    cd "%DEPS%"
    :: Speex does not have a tagged release for VS2008! So, check out trunk instead.
    cecho {0D}Cloning Speex into "%DEPS%\speex".{# #}{\n}
    svn checkout http://svn.xiph.org/trunk/speex/ speex
)

IF NOT EXIST "%DEPS%\speex\lib\%DEBUG_OR_RELEASE%\libspeexdsp.lib". (
    cd "%DEPS%\speex\win32\VS2008"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Building %DEBUG_OR_RELEASE% Speex. Please be patient, this will take a while.{# #}{\n}
    :: The default libspeex.sln has the libspeexdsp project disabled, so we must use our own custom solution file.
    copy /Y "%TOOLS%\Mods\libspeex.sln" libspeex.sln
    :: Also, the libspeexdsp.vcproj poorly outputs the resulted library to the same directory using the same filename
    :: regardless of the used configuration so we must work around that too.
    copy /Y "%TOOLS%\Mods\libspeexdsp.vcproj" libspeexdsp\libspeexdsp.vcproj
    MSBuild libspeex.sln /p:configuration=%DEBUG_OR_RELEASE%  /p:platform="%VS_PLATFORM%" /t:libspeex;libspeexdsp /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}%DEBUG_OR_RELEASE% Speex already built. Skipping.{# #}{\n}
)

:: Google Protocol Buffer
IF NOT EXIST "%DEPS%\protobuf". (
   cd "%DEPS%"
   IF NOT EXIST protobuf-2.4.1.zip. (
      cecho {0D}Downloading Google Protobuf 2.4.1{# #}{\n}
      wget http://protobuf.googlecode.com/files/protobuf-2.4.1.zip
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   )
   
   cecho {0D}Extracting Google Protobuf 2.4.1 sources to "%DEPS%\protobuf".{# #}{\n}
   7za x -y protobuf-2.4.1.zip
   REM IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   ren protobuf-2.4.1 protobuf
   IF NOT EXIST "%DEPS%\protobuf". (
      cecho {0E}Failed to rename %DEPS%\protobuf-2.4.1 to %DEPS%\protobuf. Permission denied for your account?{# #}{\n}
      GOTO :ERROR
   )
) ELSE (
   cecho {0D}Google Profobuf already downloaded. Skipping.{# #}{\n}
)

:: This project builds both release and debug as libprotobuf.lib but CMake >=2.8.5
:: will know this and find them properly from vsprojects\Release|Debug as long as PROTOBUF_SRC_ROOT_FOLDER
:: is set properly. Because of this we can skip copying things to /lib /bin /include folders.
IF NOT EXIST "%DEPS%\protobuf\vsprojects\%DEBUG_OR_RELEASE%\libprotobuf.lib". (
    cd "%DEPS%\protobuf\vsprojects"
    :: Must use custom solution and project files in order to be able to build with VC10 and/or as 64-bit.
    copy /Y "%TOOLS%\Mods\%VS2008_OR_VS2010%-protobuf.sln" protobuf.sln
    copy /Y "%TOOLS%\Mods\%VS2008_OR_VS2010%-libprotobuf.%VCPROJ_FILE_EXT%" libprotobuf.%VCPROJ_FILE_EXT%
    copy /Y "%TOOLS%\Mods\%VS2008_OR_VS2010%-libprotoc.%VCPROJ_FILE_EXT%" libprotoc.%VCPROJ_FILE_EXT%
    copy /Y "%TOOLS%\Mods\%VS2008_OR_VS2010%-protoc.%VCPROJ_FILE_EXT%" protoc.%VCPROJ_FILE_EXT%
    echo.

    cecho {0D}Building %DEBUG_OR_RELEASE% Google Protobuf. Please be patient, this will take a while.{# #}{\n}
    MSBuild protobuf.sln /p:configuration=%DEBUG_OR_RELEASE%  /p:platform="%VS_PLATFORM%" /t:libprotobuf;libprotoc;protoc /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}%DEBUG_OR_RELEASE% Google Protobuf already built. Skipping.{# #}{\n}
)

:: Celt
IF NOT EXIST "%DEPS%\celt\.git" (
   cd "%DEPS%"
   cecho {0D}Cloning Celt 0.11.1 into "%DEPS%\celt".{# #}{\n}
   call git clone http://git.xiph.org/celt.git celt
   :: Copy config.h from head to the 0.11.1 tag.
   cd celt
   copy /Y msvc\config.h libcelt\config.h
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   call git checkout -b v0.11.1 v0.11.1
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   mkdir msvc
   copy /Y libcelt\config.h msvc\config.h
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Celt already cloned. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\celt\lib\%DEBUG_OR_RELEASE%\libcelt.lib" (
    cd "%DEPS%\celt\libcelt"
    :: We need custom project files that have also x64 configurations.
    :: Also, the project does not even provide a VS2008 project file to begin with.
    copy /Y "%TOOLS%\Mods\libcelt.%VCPROJ_FILE_EXT%" "%DEPS%\celt\libcelt"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Building %DEBUG_OR_RELEASE% Celt 0.11.1.{# #}{\n}
    MSBuild libcelt.%VCPROJ_FILE_EXT% /p:configuration=%DEBUG_OR_RELEASE%  /p:platform="%VS_PLATFORM%" /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%

    :: Copy libs
    IF NOT EXIST "%DEPS%\celt\lib". (
        mkdir "%DEPS%\celt\lib\Debug\"
        mkdir "%DEPS%\celt\lib\Release\"
    )
    IF %TARGET_ARCH%==x64 (
        copy /Y X64\%DEBUG_OR_RELEASE%\libcelt.lib "%DEPS%\celt\lib\%DEBUG_OR_RELEASE%\"
    ) ELSE (
        copy /Y %DEBUG_OR_RELEASE%\libcelt.lib "%DEPS%\celt\lib\%DEBUG_OR_RELEASE%\"
        REM copy /Y lib\%DEBUG_OR_RELEASE%\libcelt.lib "%DEPS%\celt\lib\%DEBUG_OR_RELEASE%\"
    )

    :: Copy headers
    IF NOT EXIST "%DEPS%\celt\include\celt". mkdir "%DEPS%\celt\include\celt"
    copy /Y "*.h" "%DEPS%\celt\include\celt\"
) ELSE (
    cecho {0D}%DEBUG_OR_RELEASE% Celt already built. Skipping.{# #}{\n}
)

:: VLC, only usable with 32-bit VC9 builds.
IF %VS_VER%==vs2008 IF %TARGET_ARCH%==x86 (
    IF NOT EXIST "%DEPS%\vlc-2.0.1-win32.zip". (
        CD "%DEPS%"
        rmdir /S /Q "%DEPS%\vlc"
        cecho {0D}Downloading VLC 2.0.1{# #}{\n}
        wget http://sourceforge.net/projects/vlc/files/2.0.1/win32/vlc-2.0.1-win32.zip/download
        IF NOT EXIST "%DEPS%\vlc-2.0.1-win32.zip". GOTO :ERROR
    ) ELSE (
        cecho {0D}VLC 2.0.1 already downloaded. Skipping.{# #}{\n}
    )

    IF NOT EXIST "%DEPS%\vlc". (
        CD "%DEPS%"
        mkdir vlc
        cecho {0D}Extracting VLC 2.0.1 package to "%DEPS%\vlc\vlc-2.0.1"{# #}{\n}
        7za x -y -ovlc vlc-2.0.1-win32.zip
        cd vlc
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
        mkdir lib
        mkdir include
        mkdir bin\plugins\vlcplugins
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
        :: Copy from extraced location to our subfolders
        cecho {0D}Copying needed VLC 2.0.1 files to \bin \lib and \include{# #}{\n}
        copy /Y vlc-2.0.1\*.dll bin\
        xcopy /E /I /C /H /R /Y vlc-2.0.1\plugins\*.* bin\plugins\vlcplugins
        xcopy /E /I /C /H /R /Y vlc-2.0.1\sdk\include\*.* include
        copy /Y vlc-2.0.1\sdk\lib\*.lib lib\
        :: Remove extracted folder, not needed anymore
        rmdir /S /Q vlc-2.0.1
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
        :: Force deployment and clean vlc plugins cache file
        del /Q "%TUNDRA_BIN%\libvlc.dll"
        rmdir /S /Q "%TUNDRA_BIN%\plugins\vlcplugins"
        del /Q "%TUNDRA_BIN%\plugins\plugins*.dat"
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    ) ELSE (
        cecho {0D}VLC 2.0.1 already extracted. Skipping.{# #}{\n}
    )

    IF NOT EXIST "%TUNDRA_BIN%\libvlc.dll". (
        cecho {0D}Deploying VLC 2.0.1 DLLs to Tundra bin\{# #}{\n}
        xcopy /E /I /C /H /R /Y "%DEPS%\vlc\bin\*.*" "%TUNDRA_BIN%"
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    ) ELSE (
        cecho {0D}VLC 2.0.1 already deployed. Skipping.{# #}{\n}
    )
) ELSE (
   cecho {0D}VLC is not binary-compatible with non-32-bit non-VS2008 binaries, skipping.{# #}{\n}
)

:: QXmpp
IF NOT EXIST "%DEPS%\qxmpp\". (
    cecho {0D}Cloning qxmpp into "%DEPS%\qxmpp".{# #}{\n}
    cd "%DEPS%"
    svn checkout http://qxmpp.googlecode.com/svn/trunk@r1671 qxmpp
    IF NOT EXIST "%DEPS%\qxmpp\.svn" GOTO :ERROR

    cecho {0D}Building %DEBUG_OR_RELEASE% QXmpp.{# #}{\n}
    cd qxmpp
    sed 's/# DEFINES += QXMPP_USE_SPEEX/DEFINES += QXMPP_USE_SPEEX/g' < src\src.pro > src\temp
    sed 's/# LIBS += -lspeex/LIBS += -L"..\\\..\\\speex\\\lib\\\libspeex.lib -L"..\\\.\\\speex\\\lib\\\libspeexdsp.lib"/g' < src\temp > src\src.pro
    sed 's/INCLUDEPATH += $$QXMPP_INCLUDE_DIR $$QXMPP_INTERNAL_INCLUDES/INCLUDEPATH += $$QXMPP_INCLUDE_DIR $$QXMPP_INTERNAL_INCLUDES ..\\\..\\\speex\\\include\nDEPENDPATH += ..\\\..\\\speex/g' < src\src.pro > src\temp
    mv src\temp src\src.pro
    sed 's/LIBS += $$QXMPP_LIBS/LIBS += $$QXMPP_LIBS -L"..\\\..\\\speex\\\lib\\\libspeex.lib" -L"..\\\..\\\speex\\\lib\\\libspeexdsp.lib"/g' < tests\tests.pro > tests\temp
    mv tests\temp tests\tests.pro
    qmake
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF %USE_JOM%==TRUE (
        cecho {0D}Building %DEBUG_OR_RELEASE% Qxmpp with jom{# #}{\n}
        "%DEPS%\qt\jom\jom.exe" sub-src-all-ordered %DEBUG_OR_RELEASE_LOWERCASE%
    ) ELSE (
        cecho {0D}Building %DEBUG_OR_RELEASE% Qxmpp with nmake{# #}{\n}
        nmake /nologo sub-src-all-ordered  %DEBUG_OR_RELEASE_LOWERCASE%
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF NOT EXIST "%DEPS%\qxmpp\include\qxmpp". mkdir %DEPS%\qxmpp\include\qxmpp
    copy /Y "src\*.h" "%DEPS%\qxmpp\include\qxmpp\"
) ELSE (
    cecho {0D}%DEBUG_OR_RELEASE% QXmpp already built. Skipping.{# #}{\n}
)

:: ZLIB
set ZLIB_VERSION=1.2.8
IF NOT EXIST "%DEPS%\zlib-%ZLIB_VERSION%.tar.gz". (
   CD "%DEPS%"
   rmdir /S /Q "%DEPS%\zlib"
   cecho {0D}Downloading zlib %ZLIB_VERSION%{# #}{\n}
   wget http://zlib.net/zlib-%ZLIB_VERSION%.tar.gz
   IF NOT EXIST "%DEPS%\zlib-%ZLIB_VERSION%.tar.gz". GOTO :ERROR
) ELSE (
   cecho {0D}zlib %ZLIB_VERSION% already downloaded. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\zlib". (
    cd "%DEPS%"
    cecho {0D}Extracting zlib %ZLIB_VERSION% package to "%DEPS%\zlib"{# #}{\n}
    mkdir zlib
    7za e -y zlib-%ZLIB_VERSION%.tar.gz
    7za x -y -ozlib zlib-%ZLIB_VERSION%.tar
    del /Q zlib-%ZLIB_VERSION%.tar
)

IF NOT EXIST "%DEPS%\zlib\lib\%DEBUG_OR_RELEASE%\zlibstat.lib". (
    cd "%DEPS%\zlib"
    IF NOT EXIST lib. mkdir lib
    IF NOT EXIST lib\Release. mkdir lib\Release
    IF NOT EXIST lib\Debug. mkdir lib\Debug
    IF NOT EXIST include. mkdir include
    cd zlib-%ZLIB_VERSION%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    IF %TARGET_ARCH%==x64 (
        cd contrib\masmx64
        call bld_ml64.bat
    ) ELSE (
        cd contrib\masmx86
        call bld_ml32.bat
    )

    cd ..\..
    cd contrib\vstudio\%VC_VER%
    cecho {0D}Building %DEBUG_OR_RELEASE% zlib %ZLIB_VERSION%{# #}{\n}
    MSBuild zlibvc.sln /p:configuration=%DEBUG_OR_RELEASE% /p:platform="%VS_PLATFORM%" /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%

    cd ..\..\..
    copy /Y contrib\vstudio\%VC_VER%\%TARGET_ARCH%\ZlibStat%DEBUG_OR_RELEASE%\zlibstat.lib ..\lib\%DEBUG_OR_RELEASE%
    copy /Y *.h ..\include\
) ELSE (
   cecho {0D}%DEBUG_OR_RELEASE% zlib %ZLIB_VERSION% already built. Skipping.{# #}{\n}
)

:: ZZIPLIB
set ZZIPLIB_VERSION=0.13.59
IF NOT EXIST "%DEPS%\zziplib-%ZZIPLIB_VERSION%.tar.bz2". (
    cd "%DEPS%"
    rmdir /S /Q "%DEPS%\zziplib"
    cecho {0D}Downloading zziplib %ZZIPLIB_VERSION%{# #}{\n}
    wget http://sourceforge.net/projects/zziplib/files/zziplib13/%ZZIPLIB_VERSION%/zziplib-%ZZIPLIB_VERSION%.tar.bz2/download
    IF NOT EXIST "%DEPS%\zziplib-%ZZIPLIB_VERSION%.tar.bz2". GOTO :ERROR
) ELSE (
    cecho {0D}zziplib %ZZIPLIB_VERSION% already downloaded. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\zziplib". (
    CD "%DEPS%"
    cecho {0D}Extracting zziplib %ZZIPLIB_VERSION% package to "%DEPS%\zziplib"{# #}{\n}
    mkdir zziplib
    7za e -y zziplib-%ZZIPLIB_VERSION%.tar.bz2
    7za x -y -ozziplib zziplib-%ZZIPLIB_VERSION%.tar
    del /Q zziplib-%ZZIPLIB_VERSION%.tar
)

IF NOT EXIST "%DEPS%\zziplib\lib\zziplib%POSTFIX_D%.lib". (
    cd "%DEPS%\zziplib"
    IF NOT EXIST lib. mkdir lib
    IF NOT EXIST include\zzip. mkdir include\zzip
    cd zziplib-%ZZIPLIB_VERSION%\msvc8

    :: Use a custom project file as zziblib does not ship with vs2008 project files.
    :: Additionally its include/lib paths are not proper for it to find our zlib build and it has weird lib name postfixes.
    :: It's nicer to use a tailored file rathern than copy duplicates under the zziblib source tree.
    cecho {0D}Building %DEBUG_OR_RELEASE% zziplib from premade project %TOOLS%\Mods\vs2008-zziplib.vcproj{# #}{\n}
    copy /Y "%TOOLS%\Mods\vs2008-zziplib.vcproj" zziplib.vcproj
    IF NOT %VS_VER%==vs2008 IF NOT EXIST zziplib.%VCPROJ_FILE_EXT%. VCUpgrade /nologo zziplib.vcproj
    MSBuild zziplib.%VCPROJ_FILE_EXT% /p:configuration=%DEBUG_OR_RELEASE% /p:platform="%VS_PLATFORM%" /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%

    :: Copy results to lib/include
    copy /Y zziplib%POSTFIX_D%.lib ..\..\lib
    copy /Y ..\zzip\*.h ..\..\include\zzip
) ELSE (
    cecho {0D}%DEBUG_OR_RELEASE% zziplib %ZZIPLIB_VERSION% already built. Skipping.{# #}{\n}
)

echo.
%TOOLS%\Utils\cecho {0A}Tundra dependencies built.{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
GOTO :EOF

:ERROR
echo.
%TOOLS%\Utils\cecho {0C}An error occurred! Aborting!{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
pause

endlocal
