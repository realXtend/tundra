@echo off

set GENERATOR="Visual Studio 9 2008"
cd ..
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps
cd %TOOLS%

cecho {0A}This script fetches and builds all Tundra dependencies.{# #}{\n}
echo Requirements:
echo 1. Install SVN and set svn.exe to PATH.
echo  - http://tortoisesvn.net/downloads.html, install with command line tools!
echo 2. Install Hg and set hg.exe to PATH.
echo  - http://tortoisehg.bitbucket.org/
echo 3. Install git and set git.exe to PATH.
echo  - http://tortoisehg.bitbucket.org/
echo 4. Execute this file from Visual Studio 2008/2010 Command Prompt.
echo.

cecho {0D}Assuming Tundra git trunk is found at %TUNDRA_DIR%.{# #}{\n}
cecho {0E}Warning: The path %TUNDRA_DIR% may not contain spaces! (qmake breaks on them).{# #}{\n}
cecho {0E}Warning: This script is not fully unattended once you continue.{# #}{\n}
cecho {0E}         When building Qt, you must press 'y' once for the script to proceed.{# #}{\n}
echo.

echo If you are not ready with the above, press Ctrl-C to abort!
pause

:: Make sure we call .Net Framework 3.5 version of msbuild, to be able to build VS2008 solutions.
set PATH=C:\Windows\Microsoft.NET\Framework\v3.5;%PATH%

:: Add qmake from our downloaded Qt to PATH.
set PATH=%DEPS%\Qt\bin;%PATH%

set QMAKESPEC=%DEPS%\Qt\mkspecs\win32-msvc2008
set QTDIR=%DEPS%\Qt

IF NOT EXIST "%DEPS%\Qt". (
   cd "%DEPS%"
   IF NOT EXIST qt-everywhere-opensource-src-4.7.4.zip. (
      cecho {0D}Downloading Qt 4.7.4. Please be patient, this will take a while.{# #}{\n}
      wget http://download.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.4.zip
      IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   )

   cecho {0D}Extracting Qt 4.7.4 sources to "%DEPS%\Qt".{# #}{\n}
   7za x -y qt-everywhere-opensource-src-4.7.4.zip
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   ren qt-everywhere-opensource-src-4.7.4 qt
   IF NOT EXIST "%DEPS%\Qt" GOTO :ERROR
) ELSE (
   cecho {0D}Qt already downloaded. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\Qt\lib\QtCore4.dll". (
   cd "%DEPS%\Qt"
   cecho {0D}Configuring Qt build. Please answer 'y'!.{# #}{\n}
   configure -debug-and-release -opensource -shared -ltcg -no-qt3support -no-opengl -no-openvg -platform win32-msvc2008 -no-dbus -nomake examples -nomake demos
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cecho {0D}Building Qt. Please be patient, this will take a while.{# #}{\n}
   nmake /nologo
   :: Qt build system is slightly broken: see https://bugreports.qt-project.org/browse/QTBUG-6470. Work around the issue.
   set ERRORLEVEL=0
   del /s /q mocinclude.tmp
   nmake /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   REM We (re)built Qt, so delete QtCore4.dll in Tundra bin\ to force DLL deployment below.
   del /Q "%TUNDRA_BIN%\QtCore4.dll"
) ELSE (
   cecho {0D}Qt already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%TUNDRA_BIN%\QtCore4.dll". (
   cecho {0D}Deploying Qt DLLs to Tundra bin\.{# #}{\n}
   copy /Y "%DEPS%\qt\bin\*.dll" "%TUNDRA_BIN%"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   mkdir "%TUNDRA_BIN%\qtplugins"
   xcopy /E /I /C /H /R /Y "%DEPS%\qt\plugins\*.*" "%TUNDRA_BIN%\qtplugins"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%DEPS%\bullet\". (
   cecho {0D}Cloning Bullet into "%DEPS%\bullet".{# #}{\n}
   cd "%DEPS%"
   svn checkout http://bullet.googlecode.com/svn/tags/bullet-2.78 bullet
   IF NOT EXIST "%DEPS%\bullet\.svn" GOTO :ERROR
   cecho {0D}Building Bullet. Please be patient, this will take a while.{# #}{\n}
   msbuild bullet\msvc\2008\BULLET_PHYSICS.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
   msbuild bullet\msvc\2008\BULLET_PHYSICS.sln /p:configuration=Release /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Bullet already built. Skipping.{# #}{\n}
)

set BOOST_ROOT=%DEPS%\boost
set BOOST_INCLUDEDIR=%DEPS%\boost
set BOOST_LIBRARYDIR=%DEPS%\boost\stage\lib

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
   copy /Y "%TOOLS%\utils-windows\boost-user-config-vs2008.jam" "%DEPS%\boost\tools\build\v2\user-config.jam"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd "%DEPS%\boost"
   cecho {0D}Building Boost. Please be patient, this will take a while.{# #}{\n}
   call .\b2 --without-mpi thread regex stage

) ELSE (
   cecho {0D}Boost already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\kNet\". (
   cecho {0D}Cloning kNet from https://github.com/juj/kNet into "%DEPS%\kNet".{# #}{\n}
   cd "%DEPS%"
   call git clone https://github.com/juj/kNet
   IF NOT EXIST "%DEPS%\kNet\.git" GOTO :ERROR
) ELSE (
   cecho {0D}Updating kNet to newest version from https://github.com/juj/kNet.{# #}{\n}
   cd "%DEPS%\kNet"
   call git pull
)

cd "%DEPS%\kNet"
IF NOT EXIST kNet.sln. (
   cecho {0D}Running cmake for kNet.{# #}{\n}
   del /Q CMakeCache.txt
   cmake . -G %GENERATOR% -DBOOST_ROOT="%DEPS%\boost"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)
cecho {0D}Building kNet. Please be patient, this will take a while.{# #}{\n}
msbuild kNet.sln /p:configuration=Debug /nologo
::msbuild kNet.sln /p:configuration=MinSizeRel /nologo
::msbuild kNet.sln /p:configuration=Release /nologo
msbuild kNet.sln /p:configuration=RelWithDebInfo /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

IF NOT EXIST "%DEPS%\qtscriptgenerator\.git". (
   cecho {0D}Cloning QtScriptGenerator into "%DEPS%\qtscriptgenerator".{# #}{\n}
   cd "%DEPS%"
   call git clone git://gitorious.org/qt-labs/qtscriptgenerator
   IF NOT EXIST "%DEPS%\qtscriptgenerator\.git" GOTO :ERROR
) ELSE (
   cecho {0D}QtScriptGenerator already cloned. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\qtscriptgenerator\plugins\script\qtscript_xmlpatterns.dll". (
   cd "%DEPS%\qtscriptgenerator\generator"
   cecho {0D}Running qmake for QtScriptGenerator.{# #}{\n}
   qmake
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cecho {0D}Building QtScriptGenerator.{# #}{\n}
   nmake /nologo
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
   copy /Y "%TOOLS%\utils-windows\qtscript_QWebPluginFactory.cpp" "%DEPS%\qtscriptgenerator\generated_cpp\com_trolltech_qt_webkit"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   cecho {0D}Running qmake for qtbindings plugins.{# #}{\n}
   qmake
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cecho {0D}Building qtscript plugins. Please be patient, this will take a while.{# #}{\n}
   nmake debug /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   nmake release /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}QtScriptGenerator already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\realxtend-tundra-deps\.git". (
   cecho {0D}Cloning realxtend-tundra-deps repository into "%DEPS%\realxtend-tundra-deps".{# #}{\n}
   cd "%DEPS%"
   call git clone https://code.google.com/p/realxtend-tundra-deps
   IF NOT EXIST "%DEPS%\realxtend-tundra-deps\.git" GOTO :ERROR

   cd "%DEPS%\realxtend-tundra-deps"
   call git checkout sources
) ELSE (
   cecho {0D}Updating realxtend-tundra-deps to newest.{# #}{\n}
   cd "%DEPS%\realxtend-tundra-deps"
   call git pull origin
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
   echo add_subdirectory(RenderSystem_Null) >> "%DEPS%\ogre-safe-nocrashes\RenderSystems\CMakeLists.txt"
)

cecho {0D}Updating RenderSystem_NULL to the newest version in ogre-safe-nocrashes.{# #}{\n}
xcopy /Q /E /I /C /H /R /Y "%DEPS%\realxtend-tundra-deps\RenderSystem_NULL" "%DEPS%\ogre-safe-nocrashes\RenderSystems\RenderSystem_NULL"

cd "%DEPS%\ogre-safe-nocrashes"
IF NOT EXIST OgreDependencies_MSVC_20101231.zip. (
   cecho {0D}Downloading Ogre prebuilt dependencies package.{# #}{\n}
   wget "http://garr.dl.sourceforge.net/project/ogre/ogre-dependencies-vc%%2B%%2B/1.7/OgreDependencies_MSVC_20101231.zip"
   IF NOT EXIST OgreDependencies_MSVC_20101231.zip. (
      cecho {0C}Error downloading Ogre depencencies! Aborting!{# #}{\n}
      GOTO:EOF
   )

   cecho {0D}Extracting Ogre prebuilt dependencies package.{# #}{\n}
   7za x -y OgreDependencies_MSVC_20101231.zip Dependencies
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR

   cecho {0D}Building Ogre prebuilt dependencies package. Please be patient, this will take a while.{# #}{\n}
   msbuild Dependencies\src\OgreDependencies.VS2008.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
   msbuild Dependencies\src\OgreDependencies.VS2008.sln /p:configuration=Release /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST OGRE.sln. (
   cecho {0D}Running cmake for ogre-safe-nocrashes.{# #}{\n}
   cmake -G %GENERATOR% -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF -DOGRE_CONFIG_THREADS:INT=1
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

cecho {0D}Building ogre-safe-nocrashes. Please be patient, this will take a while.{# #}{\n}
msbuild OGRE.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
::msbuild OGRE.sln /p:configuration=MinSizeRel /clp:ErrorsOnly /nologo
::msbuild OGRE.sln /p:configuration=Release /clp:ErrorsOnly /nologo
msbuild OGRE.sln /p:configuration=RelWithDebInfo /clp:ErrorsOnly /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying ogre-safe-nocrashes SDK directory.{# #}{\n}
msbuild INSTALL.vcproj /p:configuration=Debug /clp:ErrorsOnly /nologo
::msbuild INSTALL.vcproj /p:configuration=MinSizeRel /clp:ErrorsOnly /nologo
::msbuild INSTALL.vcproj /p:configuration=Release /clp:ErrorsOnly /nologo
msbuild INSTALL.vcproj /p:configuration=RelWithDebInfo /clp:ErrorsOnly /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying Ogre DLLs to Tundra bin\ directory.{# #}{\n}
copy /Y "%DEPS%\ogre-safe-nocrashes\bin\debug\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\ogre-safe-nocrashes\bin\relwithdebinfo\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\ogre-safe-nocrashes\Dependencies\bin\Release\cg.dll" "%TUNDRA_BIN"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0C}NOTE: Skipping PythonQt build for now!{# #}{\n}
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

cd "%DEPS%\realxtend-tundra-deps\skyx"
IF NOT EXIST SKYX.sln. (
   cecho {0D}Running cmake for SkyX.{# #}{\n}
   del /Q CMakeCache.txt
   cmake . -G %GENERATOR%
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)
cecho {0D}Building SkyX. Please be patient, this will take a while.{# #}{\n}
msbuild SKYX.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
::msbuild SKYX.sln /p:configuration=MinSizeRel /clp:ErrorsOnly /nologo
::msbuild SKYX.sln /p:configuration=Release /clp:ErrorsOnly /nologo
msbuild SKYX.sln /p:configuration=RelWithDebInfo /clp:ErrorsOnly /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying SkyX DLLs to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\debug\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\relwithdebinfo\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Building Hydrax. Please be patient, this will take a while.{# #}{\n}
cd "%DEPS%\realxtend-tundra-deps\hydrax\msvc9"
msbuild Hydrax.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
msbuild Hydrax.sln /p:configuration=Release /clp:ErrorsOnly /nologo
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Deploying Hydrax DLLs to Tundra bin\.{# #}{\n}
copy /Y "%DEPS%\realxtend-tundra-deps\hydrax\lib\*.dll" "%TUNDRA_BIN%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

IF NOT EXIST "%DEPS%\qt-solutions". (
   cecho {0D}Cloning QtPropertyBrowser into "%DEPS%\qt-solutions".{# #}{\n}
   cd "%DEPS%"
   call git clone git://gitorious.org/qt-solutions/qt-solutions.git
   IF NOT EXIST "%DEPS%\qt-solutions\.git" GOTO :ERROR
   cd qt-solutions\qtpropertybrowser

   REM Don't build examples.
   sed -e "s/SUBDIRS+=examples//" < qtpropertybrowser.pro > qtpropertybrowser.pro.sed
   del qtpropertybrowser.pro
   ren qtpropertybrowser.pro.sed qtpropertybrowser.pro

   call configure -library
   qmake
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   nmake /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   copy /Y "%DEPS%\qt-solutions\qtpropertybrowser\lib\*.dll" "%TUNDRA_BIN%"
) ELSE (
   cecho {0D}qtpropertybrowser already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\OpenAL\libs\Win32\OpenAL32.lib". (
   cecho {0D}OpenAL does not exist. Unzipping a prebuilt package.{# #}{\n}
   copy "%TOOLS%\utils-windows\OpenAL.zip" "%DEPS%\"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   cd "%DEPS%\"
   7za x OpenAL.zip
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   del /Q OpenAL.zip
) ELSE (
   cecho {0D}OpenAL already prepared. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\ogg". (
   cecho {0D}Cloning Ogg into "%DEPS%\ogg".{# #}{\n}
   svn checkout http://svn.xiph.org/tags/ogg/libogg-1.3.0/ "%DEPS%\ogg"
   cd "%DEPS%\ogg\win32\VS2008"
   cecho {0D}Building Ogg. Please be patient, this will take a while.{# #}{\n}
   msbuild libogg_static.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
   msbuild libogg_static.sln /p:configuration=Release /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Ogg already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\vorbis". (
   cecho {0D}Cloning Vorbis into "%DEPS%\vorbis".{# #}{\n}
   svn checkout http://svn.xiph.org/tags/vorbis/libvorbis-1.3.3/ "%DEPS%\vorbis"
   cd "%DEPS%\vorbis\win32\VS2008"
   cecho {0D}Building Vorbis. Please be patient, this will take a while.{# #}{\n}
   msbuild vorbis_static.sln /p:configuration=Debug /clp:ErrorsOnly /nologo
   msbuild vorbis_static.sln /p:configuration=Release /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Vorbis already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\theora". (
   cecho {0D}Cloning Theora into "%DEPS%\theora".{# #}{\n}
   svn checkout http://svn.xiph.org/tags/theora/libtheora-1.1.1/ "%DEPS%\theora"
   cd "%DEPS%\theora\win32\VS2008"
   cecho {0D}Building Theora. Please be patient, this will take a while.{# #}{\n}
   msbuild libtheora_static.sln /p:configuration=Debug /t:libtheora_static /clp:ErrorsOnly /nologo
   msbuild libtheora_static.sln /p:configuration=Release_SSE2 /t:libtheora_static /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Theora already built. Skipping.{# #}{\n}
)

IF NOT EXIST "%DEPS%\speex". (
   cecho {0D}Cloning Speex into "%DEPS%\speex".{# #}{\n}
   :: speex does not have a tagged release for MSVC2008! So, check out trunk instead.
   svn checkout http://svn.xiph.org/trunk/speex/ "%DEPS%\speex"
   cd "%DEPS%\speex\win32\VS2008"
   cecho {0D}Building Speex. Please be patient, this will take a while.{# #}{\n}
   msbuild libspeex.sln /p:configuration=Debug /t:libspeex /clp:ErrorsOnly /nologo
   msbuild libspeex.sln /p:configuration=Release /t:libspeex /clp:ErrorsOnly /nologo
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
) ELSE (
   cecho {0D}Speex already built. Skipping.{# #}{\n}
)

cecho {0A}Tundra dependencies built.{# #}{\n}
cd %TOOLS%
GOTO :EOF

:ERROR
cecho {0C}An error occurred! Aborting!{# #}{\n}
pause
