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
:: Make sure deps folder exists.
IF NOT EXIST "%DEPS%". mkdir "%DEPS%"

:: User-defined variables
set BUILD_RELEASE=FALSE
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
IF NOT %BUILD_RELEASE%==FALSE IF NOT %BUILD_RELEASE%==TRUE (
   cecho {0E}BUILD_RELEASE needs to be either TRUE or FALSE!{# #}{\n} 
   GOTO :ERROR
)
IF NOT %USE_BOOST%==FALSE IF NOT %USE_BOOST%==TRUE (
   cecho {0E}USE_BOOST needs to be either TRUE or FALSE!{# #}{\n}
   GOTO :ERROR
)
:: TODO remove BUILD_RELEASE and instead use BUILD_TYPE
::set BUILD_TYPE=%2
:: IF %BUILD_TYPE%=="" (
::   set BUILD_TYPE=RelWithDebInfo
::   cecho {0D}BUILD_TYPE not specified - using the default %BUILD_TYPE%{# #}{\n}
:: )
::

set BUILD_TYPE_MINSIZEREL=MinSizeRel
set BUILD_TYPE_RELEASE=Release
set BUILD_TYPE_RELWITHDEBINFO=RelWithDebInfo
set BUILD_TYPE_DEBUG=Debug
IF %BUILD_RELEASE%==TRUE (
    set BUILD_TYPE=%BUILD_TYPE_RELEASE%
) ELSE (
    set BUILD_TYPE=%BUILD_TYPE_RELWITHDEBINFO%
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
cecho {0D}  Build Release        = %BUILD_RELEASE%{# #}{\n}
echo    - Build Release mode in addition to RelWithDebInfo when possible.
echo      Default is disabled, enable if you are planning to deploy
echo      Tundra in Release mode.
cecho {0D}  Build OpenSSL        = %BUILD_OPENSSL%{# #}{\n}
echo    - Build OpenSSL, requires Active Perl.
cecho {0D}  Build Qt with JOM    = %USE_JOM%{# #}{\n}
echo    - Use jom.exe instead of nmake.exe to build qmake projects.
echo      Default enabled as jom is significantly faster by usin all CPUs.
echo.

:: Print scripts usage information
cecho {0A}Requirements for a successful execution:{# #}{\n}
echo   1. Install SVN and make sure 'svn' is accessible from PATH.
echo    - http://tortoisesvn.net/downloads.html, install with command line tools!
echo   2. Install Hg and make sure 'hg' is accessible from PATH.
echo    - http://tortoisehg.bitbucket.org/
echo   3. Install Git and make sure 'git' is accessible from PATH.
echo    - http://code.google.com/p/tortoisegit/
echo   4. Install DirectX SDK June 2010.
echo    - http://www.microsoft.com/download/en/details.aspx?id=6812
echo   5. Install CMake and make sure 'cmake' is accessible from PATH.
echo    - http://www.cmake.org/
echo   6. Install Visual Studio 2008/2010 with SP1 (Express is ok).
echo    - http://www.microsoft.com/download/en/details.aspx?id=14597
echo   7. Install Windows SDK.
echo    - http://www.microsoft.com/download/en/details.aspx?id=8279

IF %BUILD_OPENSSL%==TRUE (
   echo   8. To build OpenSSL install Active Perl and set perl.exe to PATH.
   echo    - http://www.activestate.com/activeperl/downloads
   cecho {0E}     NOTE: Perl needs to be before git in PATH, otherwise the git{# #}{\n}
   cecho {0E}     provided perl.exe will be used and OpenSSL build will fail.{# #}{\n}
   echo   9. Execute this file from Visual Studio 2008/2010 Command Prompt.
) ELSE (
   echo   8. Execute this file from Visual Studio 2008/2010 Command Prompt.
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

IF NOT EXIST "%DEPS%\openssl\src". (
   cd "%DEPS%"
   IF NOT EXIST openssl-0.9.8x.tar.gz. (
      cecho {0D}Downloading OpenSSL 0.9.8x.{# #}{\n}
      wget http://www.openssl.org/source/openssl-0.9.8x.tar.gz
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   )

   mkdir openssl
   cecho {0D}Extracting OpenSSL 0.9.8x sources to "%DEPS%\openssl\src".{# #}{\n}
   7za e -y openssl-0.9.8x.tar.gz
   7za x -y -oopenssl openssl-0.9.8x.tar
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd openssl
   ren openssl-0.9.8x src
   cd ..
   IF NOT EXIST "%DEPS%\openssl\src". (
      cecho {0E}Failed to rename %DEPS%\openssl\openssl-0.9.8x to %DEPS%\openssl\src. Permission denied for your account?{# #}{\n}
      GOTO :ERROR
   )
   del openssl-0.9.8x.tar
) ELSE (
   cecho {0D}OpenSSL already downloaded. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\openssl\bin\ssleay32.dll". (
    cd "%DEPS%\openssl\src"
    cecho {0D}Configuring OpenSSL build.{# #}{\n}
    IF %TARGET_ARCH%==x64 (
        perl Configure VC-WIN64A --prefix=%DEPS%\openssl
    ) ELSE (
        perl Configure VC-WIN32 --prefix=%DEPS%\openssl
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
    REM We (re)built OpenSSL, so delete ssleay32.dll in Tundra bin\ to force DLL deployment below.
    del /Q "%TUNDRA_BIN%\ssleay32.dll"
) ELSE (
   cecho {0D}OpenSSL already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%TUNDRA_BIN%\ssleay32.dll". (
   cd "%DEPS%"
   cecho {0D}Deploying OpenSSL DLLs to Tundra bin\{# #}{\n}
   copy /Y "openssl\bin\*.dll" "%TUNDRA_BIN%"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

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
SET QT_OPENSSL_CONFIGURE=
IF %BUILD_OPENSSL%==TRUE (
   IF NOT EXIST "%DEPS%\qt\lib\QtWebKit4.dll". cecho {0D}Configuring OpenSSL into the Qt build with:{# #}{\n}
   SET QT_OPENSSL_CONFIGURE=-openssl -I "%DEPS%\openssl\include" -L "%DEPS%\openssl\lib"
) ELSE (
   IF NOT EXIST "%DEPS%\qt\lib\QtWebKit4.dll". cecho {0D}OpenSSL build disabled, not confguring OpenSSL to Qt.{# #}{\n}
)
IF %BUILD_OPENSSL%==TRUE (
   IF NOT EXIST "%DEPS%\qt\lib\QtWebKit4.dll". echo '%QT_OPENSSL_CONFIGURE%'
)

:: Set QMAKESPEC and QTDIR in case we are going to build qt. If we don't do this
:: a system set QMAKESPEC might take over the build in some bizarre fashion.
:: Note 1: QTDIR is not used while build, neither should QMAKESPEC be used when -platform is given to configure.
:: Note 2: We cannot do this inside the qt IF without @setlocal EnableDelayedExpansion.
set QTDIR=%DEPS%\qt\qt-src-%QT_VER%
set QMAKESPEC=%QTDIR%\mkspecs\%QT_PLATFORM%

IF NOT EXIST "%DEPS%\qt\lib\QtWebKit4.dll". (
   IF NOT EXIST "%QTDIR%". (
      cecho {0E}Warning: %QTDIR% does not exist, extracting Qt failed?.{# #}{\n}
      GOTO :ERROR
   )

   cd %QTDIR%

   IF NOT EXIST "configure.cache". (
      cecho {0D}Configuring Qt build. Please answer 'y'!{# #}{\n}
      configure -platform %QT_PLATFORM% -debug-and-release -opensource -prefix "%DEPS%\qt" -shared -ltcg ^
        -no-qt3support -no-opengl -no-openvg -no-dbus -no-phonon -no-phonon-backend -no-multimedia -no-audio-backend ^
        -no-declarative -no-xmlpatterns -nomake examples -nomake demos ^
        -qt-zlib -qt-libpng -qt-libmng -qt-libjpeg -qt-libtiff %QT_OPENSSL_CONFIGURE%

      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   ) ELSE (
      cecho {0D}Qt already configured. Remove %QTDIR%\configure.cache to trigger a reconfigure.{# #}{\n}
   )
   
   cecho {0D}Building Qt. Please be patient, this will take a while.{# #}{\n}
   IF %USE_JOM%==TRUE (
      cecho {0D}- Building Qt with jom{# #}{\n}
      "%DEPS%\qt\jom\jom.exe"
   ) ELSE (
      cecho {0D}- Building Qt with nmake{# #}{\n}
      nmake /nologo
   )

   IF NOT EXIST "%QTDIR%\lib\QtWebKit4.dll". (
      cecho {0E}Warning: %QTDIR%\lib\QtWebKit4.dll not present, Qt build failed?.{# #}{\n}
      GOTO :ERROR
   )
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   :: Don't use jom for install. It seems to hang easily, maybe beacuse it tries to use multiple cores.
   cecho {0D}Installing Qt to %DEPS%\qt{# #}{\n}
   nmake install
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   IF NOT EXIST "%DEPS%\qt\lib\QtWebKit4.dll". (
      cecho {0E}Warning: %DEPS%\qt\lib\QtWebKit4.dll not present, Qt install failed?.{# #}{\n}
      GOTO :ERROR
   )
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   :: We (re)built Qt, so delete QtWebKit4.dll in Tundra bin\ to force DLL deployment below.
   IF EXIST "%TUNDRA_BIN%\QtWebKit4.dll". (
      del /Q "%TUNDRA_BIN%\QtWebKit4.dll"
   )
) ELSE (
   cecho {0D}Qt already built. Skipping.{# #}{\n}
)

:: Setup now built Qt to PATH (for qmake.exe), QTDIR and QMAKESPEC.
:: These will be utilized by other dependencies that need Qt in this script.
set PATH=%DEPS%\qt\bin;%PATH%
set QMAKESPEC=%DEPS%\qt\mkspecs\%QT_PLATFORM%
set QTDIR=%DEPS%\qt

IF NOT EXIST "%TUNDRA_BIN%\QtWebKit4.dll". (
    cecho {0D}Deploying Qt DLLs to Tundra bin\.{# #}{\n}
    copy /Y "%DEPS%\qt\bin\*.dll" "%TUNDRA_BIN%"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    mkdir "%TUNDRA_BIN%\qtplugins"
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
)

:: Bullet physics engine
:: version 2.81 sp1, svn rev 2613
IF NOT EXIST "%DEPS%\bullet\". (
    cecho {0D}Cloning Bullet into "%DEPS%\bullet".{# #}{\n}
    cd "%DEPS%"
    svn checkout http://bullet.googlecode.com/svn/trunk@2613 bullet
    IF NOT EXIST "%DEPS%\bullet\.svn" GOTO :ERROR
    cd bullet
    IF NOT EXIST BULLET_PHYSICS.sln. (
        cecho {0D}Running CMake for Bullet.{# #}{\n}
        IF EXIST CMakeCache.txt. del /Q CMakeCache.txt
        cmake . -G %GENERATOR% -DBUILD_DEMOS:BOOL=OFF -DBUILD_EXTRAS:BOOL=OFF -DBUILD_INTEL_OPENCL_DEMOS:BOOL=OFF ^
            -DBUILD_NVIDIA_OPENCL_DEMOS:BOOL=OFF -DBUILD_UNIT_TESTS:BOOL=OFF -DUSE_DX11:BOOL=OFF -DBUILD_AMD_OPENCL_DEMOS:BOOL=OFF ^
            -DCMAKE_DEBUG_POSTFIX= -DCMAKE_MINSIZEREL_POSTFIX= -DCMAKE_RELWITHDEBINFO_POSTFIX=
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    )

    cecho {0D}Building Bullet. Please be patient, this will take a while.{# #}{\n}
    MSBuild BULLET_PHYSICS.sln /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild BULLET_PHYSICS.sln /p:configuration=Release /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild BULLET_PHYSICS.sln /p:configuration=RelWithDebInfo /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
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

::   cd "%DEPS%\boost\tools\build\v2"
::   sed s/"# using msvc ;"/"using msvc : 9.0 ;"/g <user-config.jam >user-config.new.jam
::
::   del user-config.jam
::   rename user-config.new.jam user-config.jam
   copy /Y "%TOOLS%\Mods\boost-user-config-%VS_VER%.jam" "%DEPS%\boost\tools\build\v2\user-config.jam"

   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd "%DEPS%\boost"
   cecho {0D}Building Boost. Please be patient, this will take a while.{# #}{\n}
   :: Building boost takes ages, so utilize all cores for the build process
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

IF NOT EXIST "%DEPS%\assimp\bin\Release\assimp.dll". (
    cd cd "%DEPS%\assimp"
    IF %USE_BOOST%==FALSE (
        :: Tweaks CMakeLists.txt to set ASSIMP_ENABLE_BOOST_WORKAROUND on.
        sed s/"ASSIMP_ENABLE_BOOST_WORKAROUND OFF"/"ASSIMP_ENABLE_BOOST_WORKAROUND ON"/g <CMakeLists.txt >CMakeLists.txt.sed
        del CMakeLists.txt
        rename CMakeLists.txt.sed CMakeLists.txt
    )

    cmake -G %GENERATOR%

    MSBuild Assimp.sln /p:configuration=Debug /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild Assimp.sln /p:configuration=Release /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild Assimp.sln /p:configuration=RelWithDebInfo /nologo /m:%NUMBER_OF_PROCESSORS%
) ELSE (
    cecho {0D}OpenAssetImport already built. Skipping.{# #}{\n}
)

:: Copy the correct runtime to /bin for this run
copy /Y "%DEPS%\assimp\bin\%BUILD_TYPE%\assimp.dll" "%TUNDRA_BIN%"
copy /Y "%DEPS%\assimp\bin\Debug\assimpD.dll" "%TUNDRA_BIN%"

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
IF NOT EXIST kNet.sln. (
    cecho {0D}Running cmake for kNet.{# #}{\n}

    REM TODO/NOTE: USE_BOOST not possible to configure from command-line with kNet's
    REM default (stable) branch yet, so tweak the CMakeLists.txt manually for now.
    sed s/"set(USE_BOOST TRUE)"/"option(USE_BOOST \"Specifies whether Boost is used.\" TRUE)"/g <CMakeLists.txt >CMakeLists.txt.sed
    del CMakeLists.txt
    rename CMakeLists.txt.sed CMakeLists.txt

    del /Q CMakeCache.txt
    cmake . -G %GENERATOR% -DBOOST_ROOT=%BOOST_ROOT% -DUSE_BOOST:BOOL=%USE_BOOST%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    set BUILD_KNET=TRUE
)

IF %BUILD_KNET%==TRUE (
    cecho {0D}Building kNet. Please be patient, this will take a while.{# #}{\n}
    MSBuild kNet.sln /p:configuration=Debug /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild kNet.sln /p:configuration=Release /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild kNet.sln /p:configuration=RelWithDebInfo /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}kNet already built and up to date. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\qtscriptgenerator\.git". (
   cecho {0D}Cloning QtScriptGenerator into "%DEPS%\qtscriptgenerator".{# #}{\n}
   cd "%DEPS%"
   call git clone https://git.gitorious.org/qt-labs/qtscriptgenerator
   IF NOT EXIST "%DEPS%\qtscriptgenerator\.git" GOTO :ERROR
) ELSE (
   cecho {0D}QtScriptGenerator already cloned. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\qtscriptgenerator\plugins\script\qtscript_xml.dll". (
   cd "%DEPS%\qtscriptgenerator\generator"
   cecho {0D}Running qmake for QtScriptGenerator.{# #}{\n}
   :: We need to patch pp-iterator.h in order to make it compile with newer Visual Studio versions:
   :: http://stackoverflow.com/questions/2791525/stl-operator-behavior-change-with-visual-studio-2010
   :: Also cannot use QMake as it results in linker errors, so instead generate vcproj files and build using MSBuild.
   IF NOT %VS_VER%==vs2008 (
      copy /Y "%TOOLS%\Mods\QtScriptGenerator_pp-iterator.h" "%DEPS%\qtscriptgenerator\generator\parser\rpp\pp-iterator.h"
      qmake -tp vc
      cecho {0D}Building qtscript plugins. Please be patient, this will take a while.{# #}{\n}
      MSBuild generator.vcxproj /p:configuration=Debug /nologo /m:%NUMBER_OF_PROCESSORS%
      MSBuild generator.vcxproj /p:configuration=Release /nologo /m:%NUMBER_OF_PROCESSORS%
   ) ELSE (
      qmake
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
      cecho {0D}Building QtScriptGenerator.{# #}{\n}
      IF %USE_JOM%==TRUE (
         cecho {0D}- Building QtScriptGenerator with jom{# #}{\n}
         "%DEPS%\qt\jom\jom.exe"
      ) ELSE (
         cecho {0D}- Building QtScriptGenerator with nmake{# #}{\n}
         nmake /nologo
      )
   )
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cecho {0D}Executing QtScriptGenerator.{# #}{\n}
   call release\generator
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd ..
   cd qtbindings

   sed -e "s/qtscript_phonon //" -e "s/qtscript_opengl //" -e "s/qtscript_uitools //" < qtbindings.pro > qtbindings.pro.sed
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   del /Q qtbindings.pro
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   ren qtbindings.pro.sed qtbindings.pro
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   REM Fix bad script generation for webkit.
   REM TODO: Could try some sed replacement, but can't make the regex escaping rules work from command line.
   REM sed -e s/"QWebPluginFactory_Extension_values[] = "/"QWebPluginFactory_Extension_values[1] = "// -e "s/qtscript_QWebPluginFactory_Extension_keys[] = /qtscript_QWebPluginFactory_Extension_keys[1] = //" < "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit\qtscript_QWebPluginFactory.cpp" > "%DEPS%\qtscript_QWebPluginFactory.cpp"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   del "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit\qtscript_QWebPluginFactory.cpp"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   REM move "%DEPS%\qtscript_QWebPluginFactory.cpp" "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit"
   copy /Y "%TOOLS%\Mods\qtscript_QWebPluginFactory.cpp" "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   cecho {0D}Running qmake for qtbindings plugins.{# #}{\n}
   qmake
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cecho {0D}Building qtscript plugins. Please be patient, this will take a while.{# #}{\n}
   IF %USE_JOM%==TRUE (
      cecho {0D}- Building qtscript plugins with jom{# #}{\n}
      "%DEPS%\qt\jom\jom.exe" debug
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
      "%DEPS%\qt\jom\jom.exe" release
   ) ELSE (
      cecho {0D}- Building qtscript plugins with nmake{# #}{\n}
      nmake debug /nologo
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
      nmake release /nologo
   )
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}QtScriptGenerator already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%TUNDRA_BIN%\qtplugins\script\qtscript_core.dll". (
   cecho {0D}Deploying QtScript plugin DLLs.{# #}{\n}
   mkdir "%TUNDRA_BIN%\qtplugins\script"
   xcopy /Q /E /I /C /H /R /Y "%DEPS%\qtscriptgenerator\plugins\script\*.dll" "%TUNDRA_BIN%\qtplugins\script"
) ELSE (
   cecho {0D}QtScript plugin DLLs already deployed. Skipping.{# #}{\n}
)

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

cd "%DEPS%\ogre-safe-nocrashes"
:: TODO Use newer dependencies when updating Ogre to newer version
:: http://sourceforge.net/projects/ogre/files/ogre-dependencies-vc%%2B%%2B/1.9/OgreDependencies_MSVC_20120819.zip/download
IF NOT EXIST OgreDependencies_MSVC_20101231.zip. (
    cecho {0D}Downloading Ogre prebuilt dependencies package.{# #}{\n}
    wget "http://garr.dl.sourceforge.net/project/ogre/ogre-dependencies-vc%%2B%%2B/1.7/OgreDependencies_MSVC_20101231.zip"
    IF NOT EXIST OgreDependencies_MSVC_20101231.zip. (
        cecho {0C}Error downloading Ogre depencencies! Aborting!{# #}{\n}
        GOTO :ERROR
    )

    cecho {0D}Extracting Ogre prebuilt dependencies package.{# #}{\n}
    7za x -y OgreDependencies_MSVC_20101231.zip
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Building Ogre prebuilt dependencies package. Please be patient, this will take a while.{# #}{\n}
    MSBuild Dependencies\src\OgreDependencies.%VS_VER%.sln /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild Dependencies\src\OgreDependencies.%VS_VER%.sln /p:configuration=Release /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    REM TODO For some reason zlib x64 libs end up to wrong directories, so must copy them manually.
    IF %TARGET_ARCH%==x64 (
        copy /Y Dependencies\src\zlib-1.2.3\projects\visualc6\Win32_LIB_Debug\zlibd.lib Dependencies\lib\Debug
        copy /Y Dependencies\src\zlib-1.2.3\projects\visualc6\Win32_LIB_Release\zlib.lib Dependencies\lib\Release
    )
)

:: Use Intel Thread Building Blocks for Ogre's threading if Boost is not used.
:: Latest 4.0 is used as 4.1 introduces WIN32 API calls that do not work on windows xp.
:: If we are to update this can be fixed in tbb windows_api.h by using the xp workaround on all platforms.
set TBB_VERSION=tbb40_20120613oss
set TBB_HOME=%DEPS%\ogre-safe-nocrashes\Dependencies\tbb
IF %USE_BOOST%==FALSE (
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
copy /Y "%TBB_HOME%\bin\%INTEL_ARCH%\%VC_VER%\tbb_debug.dll" "%TUNDRA_BIN%"
copy /Y "%TBB_HOME%\bin\%INTEL_ARCH%\%VC_VER%\tbb.dll" "%TUNDRA_BIN%"

cd "%DEPS%\ogre-safe-nocrashes"
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

cecho {0D}Building ogre-safe-nocrashes. Please be patient, this will take a while.{# #}{\n}
MSBuild OGRE.sln /p:configuration=Debug /nologo /m:%NUMBER_OF_PROCESSORS%
MSBuild OGRE.sln /p:configuration=%BUILD_TYPE% /nologo /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying ogre-safe-nocrashes SDK directory.{# #}{\n}
MSBuild INSTALL.%VCPROJ_FILE_EXT% /p:configuration=Debug /nologo
MSBuild INSTALL.%VCPROJ_FILE_EXT% /p:configuration=%BUILD_TYPE% /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying Ogre DLLs to Tundra bin\ directory.{# #}{\n}
copy /Y "%DEPS%\ogre-safe-nocrashes\bin\debug\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\ogre-safe-nocrashes\bin\%BUILD_TYPE%\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\ogre-safe-nocrashes\Dependencies\bin\Release\cg.dll" "%TUNDRA_BIN%"
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
IF NOT EXIST SKYX.sln. (
   cecho {0D}Running cmake for SkyX.{# #}{\n}
   del /Q CMakeCache.txt
   cmake . -G %GENERATOR% -DUSE_BOOST:BOOL=%USE_BOOST%
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building SkyX. Please be patient, this will take a while.{# #}{\n}
MSBuild SKYX.sln /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
MSBuild SKYX.sln /p:configuration=%BUILD_TYPE% /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying SkyX DLLs to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\debug\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\%BUILD_TYPE%\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Hydrax
cd "%DEPS%\realxtend-tundra-deps\hydrax"
IF NOT EXIST Hydrax.sln. (
  cecho {0D}Running cmake for Hydrax.{# #}{\n}
  del /Q CMakeCache.txt
  cmake . -G %GENERATOR%
  IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building Hydrax. Please be patient, this will take a while.{# #}{\n}
MSBuild Hydrax.sln /p:configuration=Debug /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
MSBuild Hydrax.sln /p:configuration=%BUILD_TYPE% /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying Hydrax DLLs to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\realxtend-tundra-deps\hydrax\bin\Debug\Hydraxd.dll" "%TUNDRA_BIN%"
copy /Y "%DEPS%\realxtend-tundra-deps\hydrax\bin\%BUILD_TYPE%\Hydrax.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: QtPropertyBrowser
IF NOT EXIST "%DEPS%\qt-solutions". (
    cecho {0D}Cloning QtPropertyBrowser into "%DEPS%\qt-solutions".{# #}{\n}
    cd "%DEPS%"
    call git clone https://git.gitorious.org/qt-solutions/qt-solutions.git
    IF NOT EXIST "%DEPS%\qt-solutions\.git" GOTO :ERROR
)

IF NOT EXIST "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head.dll" (
    cd "%DEPS%\qt-solutions\qtpropertybrowser"

    REM Don't build examples.
    sed -e "s/SUBDIRS+=examples//" < qtpropertybrowser.pro > qtpropertybrowser.pro.sed
    del qtpropertybrowser.pro
    ren qtpropertybrowser.pro.sed qtpropertybrowser.pro

    call configure -library
    qmake
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    IF %USE_JOM%==TRUE (
        cecho {0D}- Building QtPropertyBrowser with jom{# #}{\n}
        "%DEPS%\qt\jom\jom.exe"
    ) ELSE (
        cecho {0D}- Building QtPropertyBrowser with nmake{# #}{\n}
        nmake /nologo
    )
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    :: Force deployment
    del /Q "%TUNDRA_BIN%\QtSolutions_PropertyBrowser-head*.dll"
) ELSE (
    cecho {0D}QtPropertyBrowser already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%TUNDRA_BIN%\QtSolutions_PropertyBrowser-head.dll". (
    cecho {0D}Deploying QtPropertyBrowser DLLs.{# #}{\n}
    copy /Y "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head*.dll" "%TUNDRA_BIN%"
) ELSE (
    cecho {0D}QtPropertyBrowser DLLs already deployed. Skipping.{# #}{\n}
)

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

IF NOT EXIST "%DEPS%\ogg\win32\%VS2008_OR_VS2010%\%VS_PLATFORM%\Release\libogg_static.lib". (
    cd "%DEPS%\ogg\win32\%VS2008_OR_VS2010%"
    cecho {0D}Building Ogg. Please be patient, this will take a while.{# #}{\n}
    MSBuild libogg_static.sln /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild libogg_static.sln /p:configuration=Release /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}Ogg already built. Skipping.{# #}{\n}
)

:: Vorbis
IF NOT EXIST "%DEPS%\vorbis". (
    cecho {0D}Cloning Vorbis into "%DEPS%\vorbis".{# #}{\n}
    svn checkout http://svn.xiph.org/tags/vorbis/libvorbis-1.3.3/ "%DEPS%\vorbis"
)

IF NOT EXIST "%DEPS%\vorbis\win32\%VS2008_OR_VS2010%\%VS_PLATFORM%\Release\libvorbis_static.lib". (
    cd "%DEPS%\vorbis\win32\%VS2008_OR_VS2010%"
    cecho {0D}Building Vorbis. Please be patient, this will take a while.{# #}{\n}
    MSBuild vorbis_static.sln /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild vorbis_static.sln /p:configuration=Release /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
    cecho {0D}Vorbis already built. Skipping.{# #}{\n}
)

:: Theora
IF NOT EXIST "%DEPS%\theora". (
    cecho {0D}Cloning Theora into "%DEPS%\theora".{# #}{\n}
    svn checkout http://svn.xiph.org/tags/theora/libtheora-1.1.1/ "%DEPS%\theora"
)

IF NOT EXIST "%DEPS%\theora\win32\VS2008\%VS_PLATFORM%\Release_SSE2\libtheora_static.lib". (
    cd "%DEPS%\theora\win32\VS2008"
    cecho {0D}Building Theora. Please be patient, this will take a while.{# #}{\n}
    MSBuild libtheora_static.sln /p:configuration=Debug /t:libtheora_static /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild libtheora_static.sln /p:configuration=Release_SSE2 /t:libtheora_static /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    REM IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Theora already built. Skipping.{# #}{\n}
)

:: Speex
IF NOT EXIST "%DEPS%\speex". (
    cd "%DEPS%"
    :: Speex does not have a tagged release for VS2008! So, check out trunk instead.
    cecho {0D}Cloning Speex into "%DEPS%\speex".{# #}{\n}
    svn checkout http://svn.xiph.org/trunk/speex/ speex
)

IF NOT EXIST "%DEPS%\speex\lib\Release\libspeexdsp.lib". (
    cd "%DEPS%\speex\win32\VS2008"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Building Speex. Please be patient, this will take a while.{# #}{\n}
    :: The default libspeex.sln has the libspeexdsp project disabled, so we must use our own custom solution file.
    copy /Y "%TOOLS%\Mods\libspeex.sln" libspeex.sln
    :: Also, the libspeexdsp.vcproj poorly outputs the resulted library to the same directory using the same filename
    :: regardless of the used configuration so we must work around that too.
    copy /Y "%TOOLS%\Mods\libspeexdsp.vcproj" libspeexdsp\libspeexdsp.vcproj
    MSBuild libspeex.sln /p:configuration=Debug /t:libspeex;libspeexdsp /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild libspeex.sln /p:configuration=Release /t:libspeex;libspeexdsp /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Speex already built. Skipping.{# #}{\n}
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
IF NOT EXIST "%DEPS%\protobuf\vsprojects\Debug\libprotobuf.lib". (
    cd "%DEPS%\protobuf\vsprojects"
    IF %VS_VER%==vs2008 (
        :: Upgrade the VS2005 files to VS2008
        :: TODO 64-bit VS2008 not yet possible on VS2008!
        cecho {0D}Upgrading Google Protobuf project files.{# #}{\n}
        vcbuild /c /upgrade libprotobuf.vcproj $ALL
        vcbuild /c /upgrade libprotoc.vcproj Release
        vcbuild /c /upgrade protoc.vcproj Release
        IF NOT %ERRORLEVEL%==0 GOTO :ERROR
    ) ELSE (
        :: Command-line upgrading from VS2005 format to VS2010 (or newer) format fails,
        :: so must use files converted by the Visual Studio Conversion Wizard.
        copy /Y "%TOOLS%\Mods\vs2010-protobuf.sln_" protobuf.sln
        copy /Y "%TOOLS%\Mods\vs2010-libprotobuf.vcxproj_" libprotobuf.vcxproj
        copy /Y "%TOOLS%\Mods\vs2010-libprotoc.vcxproj_" libprotoc.vcxproj
        copy /Y "%TOOLS%\Mods\vs2010-protoc.vcxproj_" protoc.vcxproj
    )
    echo.
    cecho {0D}Building Google Protobuf. Please be patient, this will take a while.{# #}{\n}
    MSBuild protobuf.sln /p:configuration=Debug /t:libprotobuf;libprotoc;protoc /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild protobuf.sln /p:configuration=Release /t:libprotobuf;libprotoc;protoc /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Google Protobuf already built. Skipping.{# #}{\n}
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

IF NOT EXIST "%DEPS%\celt\lib\Release\libcelt.lib" (
    cd "%DEPS%\celt\libcelt"
    :: We need custom project files that have also x64 configurations.
    :: Also, the project does not even provide a VS2008 project file to begin with.
    copy /Y "%TOOLS%\Mods\libcelt.%VCPROJ_FILE_EXT%" "%DEPS%\celt\libcelt"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    cecho {0D}Building Celt 0.11.1.{# #}{\n}
    MSBuild libcelt.%VCPROJ_FILE_EXT% /p:configuration=Debug /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%
    MSBuild libcelt.%VCPROJ_FILE_EXT% /p:configuration=Release /clp:ErrorsOnly /nologo /m:%NUMBER_OF_PROCESSORS%

    :: Copy libs
    IF NOT EXIST "%DEPS%\celt\lib". (
        mkdir "%DEPS%\celt\lib\Debug\"
        mkdir "%DEPS%\celt\lib\Release\"
    )
    IF %TARGET_ARCH%==x64 (
        copy /Y X64\Debug\libcelt.lib "%DEPS%\celt\lib\Debug\"
        copy /Y X64\Release\libcelt.lib "%DEPS%\celt\lib\Release\"
    ) ELSE (
        copy /Y lib\Debug\libcelt.lib "%DEPS%\celt\lib\Release\"
        copy /Y lib\Release\libcelt.lib "%DEPS%\celt\lib\Release\"
    )

    :: Copy headers
    IF NOT EXIST "%DEPS%\celt\include\celt". mkdir "%DEPS%\celt\include\celt"
    copy /Y "*.h" "%DEPS%\celt\include\celt\"
) ELSE (
    cecho {0D}Celt already built. Skipping.{# #}{\n}
)

:: VLC
IF NOT %VS_VER%==vs2008 (
   cecho {0D}VLC is not binary-compatible with non-VS2008 binaries, skipping.{# #}{\n}
   GOTO :SKIP_VLC
)

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

:SKIP_VLC

::qxmpp
IF NOT EXIST "%DEPS%\qxmpp\". (
   cecho {0D}Cloning qxmpp into "%DEPS%\qxmpp".{# #}{\n}
   cd "%DEPS%"
   svn checkout http://qxmpp.googlecode.com/svn/trunk@r1671 qxmpp
   IF NOT EXIST "%DEPS%\qxmpp\.svn" GOTO :ERROR
   cecho {0D}Building qxmpp.{# #}{\n}
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
      cecho {0D}- Building qxmpp with jom{# #}{\n}
      "%DEPS%\qt\jom\jom.exe" sub-src-all-ordered
   ) ELSE (
      cecho {0D}- Building qxmpp with nmake{# #}{\n}
      nmake /nologo sub-src-all-ordered
   )
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   IF NOT EXIST "%DEPS%\qxmpp\include\qxmpp". mkdir %DEPS%\qxmpp\include\qxmpp
   copy /Y "src\*.h" "%DEPS%\qxmpp\include\qxmpp\"
) ELSE (
   cecho {0D}qxmpp already built. Skipping.{# #}{\n}
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

IF NOT EXIST "%DEPS%\zlib\lib\Release\zlibstat.lib". (
    cd "%DEPS%\zlib"
    mkdir lib
    mkdir lib\Release
    mkdir lib\Debug
    mkdir include
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
    cecho {0D}Building zlib %ZLIB_VERSION%{# #}{\n}
    MSBuild zlibvc.sln /p:configuration="Release" /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
    MSBuild zlibvc.sln /p:configuration="Debug" /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
    cd ..\..\..
    copy /Y contrib\vstudio\%VC_VER%\%TARGET_ARCH%\ZlibStatRelease\zlibstat.lib ..\lib\Release
    copy /Y contrib\vstudio\%VC_VER%\%TARGET_ARCH%\ZlibStatDebug\zlibstat.lib ..\lib\Debug   
    copy /Y *.h ..\include\
) ELSE (
   cecho {0D}zlib %ZLIB_VERSION% already built. Skipping.{# #}{\n}
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

IF NOT EXIST "%DEPS%\zziplib\lib\zziplib.lib". (
   cd "%DEPS%\zziplib"
   mkdir lib
   mkdir include\zzip
   cd zziplib-%ZZIPLIB_VERSION%\msvc8

   :: Use a custom project file as zziblib does not ship with vs2008 project files.
   :: Additionally its include/lib paths are not proper for it to find our zlib build and it has weird lib name postfixes.
   :: It's nicer to use a tailored file rathern than copy duplicates under the zziblib source tree.
   cecho {0D}Building zziplib from premade project %TOOLS%\Mods\vs2008-zziplib.vcproj{# #}{\n}
   copy /Y "%TOOLS%\Mods\vs2008-zziplib.vcproj" zziplib.vcproj
   IF NOT %VS_VER%==vs2008 VCUpgrade /nologo zziplib.vcproj
   MSBuild zziplib.%VCPROJ_FILE_EXT% /p:configuration=Release /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
   MSBuild zziplib.%VCPROJ_FILE_EXT% /p:configuration=Debug /nologo /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS%
   
   :: Copy results to lib/include
   copy /Y zziplib.lib ..\..\lib
   copy /Y zziplibd.lib ..\..\lib
   copy /Y ..\zzip\*.h ..\..\include\zzip
) ELSE (
   cecho {0D}zziplib %ZZIPLIB_VERSION% already built. Skipping.{# #}{\n}
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
