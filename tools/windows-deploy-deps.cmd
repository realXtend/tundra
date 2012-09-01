:: This script makes copy of the dependencies that were built with windows-build-deps.cmd
:: to a output folder. The aim is to make this folder deployable to our deps svn repo or
:: making a compressed archive of it. This output folder should be a drop in replacement for
:: <tundra_git>/deps so that our cmake scripts will run as it was the full built deps. This means
:: you unpack/clone to eg. C:\deps-prebuilt and you can run cmake with TUNDRA_DEPS_PATH=C:\deps-prebuilt
::
:: Notes:
:: - When copying libs/dlls we are excluding idb, ilk, idb and exp and most of the tim exe files,
::   if you need these files please build the deps yourself with windows-build-deps.cmd.
:: - Default target is to have <dependency>/bin /lib and /include folders after we are done.
:: - When copying headers from "src" folders, the destination will be called "include"
:: - When copying .dll or .exe from "lib" folders, the destination will be called "bin"
:: - qtscriptgenerator plugins will be copied directly to qt/plugins instead of qtscriptgenerator/plugins

@echo off
echo.

:: User defined variables
set DEPLOY_TARGET=vs2008-32bit
set DEPLOY_PACKAGE=TRUE
set DEPLOY_VERSION=2.3.3.1
set PREFER_RELEASE=FALSE

:: Populate path variables
cd ..
set ORIGINAL_PATH=%PATH%
set PATH=%PATH%;"%CD%\tools\utils-windows"
set TOOLS=%CD%\tools
set TUNDRA_DIR="%CD%"
set TUNDRA_BIN=%CD%\bin
set DEPS=%CD%\deps
set DEPLOY_DIR=%CD%\deps-%DEPLOY_TARGET%
IF NOT "%DEPLOY_VERSION%" == "" set DEPLOY_DIR=%DEPLOY_DIR%-%DEPLOY_VERSION%

:: Validate user defined variables
IF "%DEPLOY_TARGET%" == "" (
    cecho {0E}DEPLOY_TARGET cannot be empty!{# #}{\n}
    GOTO :ERROR
)
IF NOT %DEPLOY_PACKAGE% == FALSE (
   IF NOT %DEPLOY_PACKAGE% == TRUE (
      cecho {0E}DEPLOY_PACKAGE needs to be either TRUE or FALSE{# #}{\n}
      GOTO :ERROR
   ) 
)
IF NOT %PREFER_RELEASE% == FALSE (
   IF NOT %PREFER_RELEASE% == TRUE (
      cecho {0E}PREFER_RELEASE needs to be either TRUE or FALSE{# #}{\n}
      GOTO :ERROR
   ) 
)

:: Set preferred release dir name. This is used to copy release libs/dlls.
set PREFERRED_RELEASE=RelWithDebInfo
IF %PREFER_RELEASE% == TRUE set PREFERRED_RELEASE=Release

:: Print what will be done and get a comfirmation.
cecho {F0}This script will deploy all Tundra dependencies to a folder           {# #}{\n}
cecho {F0}that can then be distributed and used as is by others to build Tundra.{# #}{\n}
echo.
cecho {0A}Script configuration:{# #}{\n}
cecho {0D}  DEPLOY_TARGET        = %DEPLOY_TARGET%{# #}{\n}
echo      Target, used in deploy path if defined.
cecho {0D}  DEPLOY_PACKAGE       = %DEPLOY_PACKAGE%{# #}{\n}
echo      Compress deployment to .7z package.
cecho {0D}  DEPLOY_VERSION       = %DEPLOY_VERSION%{# #}{\n}
echo      Optional version eg. 2.3.1.0, used in deploy path if defined.
cecho {0D}  PREFER_RELEASE       = %PREFER_RELEASE%{# #}{\n}
echo      Prefer Release .lib and .dll instead of RelWithDebInfo.
cecho {0D}  Source dependencies  = %DEPS%{# #}{\n}
echo      Where dependencies will be searched from
cecho {0D}  Deployment path      = %DEPLOY_DIR%{# #}{\n}
echo      Where dependencies will be copied to
echo.
cecho {0E}Warning: This script will permanently delete everything from{# #}{\n}
cecho {0E}%DEPLOY_DIR% before deployment if you continue.{# #}{\n}
echo.
echo If the configuration is incorrect, press Ctrl-C to abort!
pause
echo.

IF EXIST "%DEPLOY_DIR%". (
    cecho {0D}Cleaning existing %DEPLOY_DIR%{# #}{\n}
    rmdir /S /Q "%DEPLOY_DIR%"
)
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

cecho {0D}Preparing %DEPLOY_DIR%{# #}{\n}
mkdir "%DEPLOY_DIR%"
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Prepare xcopy /EXCLUDE files
cd "%DEPLOY_DIR%"
del /Q *.exclude
echo .exe >> exe.exclude
echo .pdb >> nonlibs.exclude
echo .idb >> nonlibs.exclude
echo .ilk >> nonlibs.exclude
echo .exp >> nonlibs.exclude
echo .obj >> nonlibs.exclude
echo .dep >> nonlibs.exclude
echo .htm >> nonlibs.exclude
echo .manifest >> nonlibs.exclude
echo .cpp >> nonheaders.exclude
echo .txt >> nonheaders.exclude
echo .png >> nonheaders.exclude
echo .cmake >> nonheaders.exclude
echo Makefile >> nonheaders.exclude
echo Doxyfile >> nonheaders.exclude
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
echo.

:: OpenSSL
IF EXIST "%DEPS%\openssl\bin\ssleay32.dll". (
    cecho {0D}Copying OpenSSL{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\openssl\include" "%DEPLOY_DIR%\openssl\include\"
    xcopy /Y /S "%DEPS%\openssl\bin" "%DEPLOY_DIR%\openssl\bin\"
    xcopy /Y /S "%DEPS%\openssl\lib" "%DEPLOY_DIR%\openssl\lib\"
) ELSE (cecho {0E}Could not detect OpenSSL with %DEPS%\openssl\bin\ssleay32.dll, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Qt
IF EXIST "%DEPS%\qt\bin\QtWebKit4.dll". (
    cecho {0D}Copying Qt headers, this will take a while.{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\qt\include" "%DEPLOY_DIR%\qt\include\"
    cecho {0D}Copying rest of Qt{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\common" "%DEPLOY_DIR%\qt\mkspecs\common\"
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\default" "%DEPLOY_DIR%\qt\mkspecs\default\"
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\features" "%DEPLOY_DIR%\qt\mkspecs\features\"
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\win32-msvc2008" "%DEPLOY_DIR%\qt\mkspecs\win32-msvc2008\"
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\win32-msvc2010" "%DEPLOY_DIR%\qt\mkspecs\win32-msvc2010\"
    xcopy /Y /S /Q "%DEPS%\qt\mkspecs\qconfig.pri" "%DEPLOY_DIR%\qt\mkspecs\"
    xcopy /Y /S "%DEPS%\qt\bin\*.dll" "%DEPLOY_DIR%\qt\bin\"
    xcopy /Y /S "%DEPS%\qt\bin\*.exe" "%DEPLOY_DIR%\qt\bin\"
    xcopy /Y /S "%DEPS%\qt\lib\*.lib" "%DEPLOY_DIR%\qt\lib\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude "%DEPS%\qt\plugins" "%DEPLOY_DIR%\qt\plugins\"
    del /Q "%DEPLOY_DIR%\qt\bin\assistant.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\designer.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\linguist.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\qdoc3.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\qt3to4.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\qhelp*.exe"
    del /Q "%DEPLOY_DIR%\qt\bin\qttracereplay.exe"
    :: Create qt.conf for qmake.exe
    cd "%DEPLOY_DIR%\qt\bin"
    echo [Paths] >> qt.conf
    echo Binaries = ./>> qt.conf
    echo Libraries = ../lib>> qt.conf
    echo Headers = ../include>> qt.conf
    cd "%DEPLOY_DIR%"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR

    :: QtScriptGenerator
    IF EXIST "%DEPS%\qtscriptgenerator\plugins\script\qtscript_webkit.dll". (
        cecho {0D}Copying QtScriptGenerator script plugins to %DEPLOY_DIR%\qt\plugins\script{# #}{\n}
        xcopy /Y /S "%DEPS%\qtscriptgenerator\plugins\script\*.dll" "%DEPLOY_DIR%\qt\plugins\script\"
    ) ELSE (cecho {0E}Could not detect QtScriptGenerator with %DEPS%\qtscriptgenerator\plugins\script\qtscript_webkit.dll, skipping...{# #}{\n})

) ELSE (cecho {0E}Could not detect Qt with %DEPS%\qt\bin\QtWebKit4.dll, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: QtPropertyBrowser
IF EXIST "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head.dll". (
    cecho {0D}Copying QtPropertyBrowser{# #}{\n}
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\qt-solutions\qtpropertybrowser\src" "%DEPLOY_DIR%\qtpropertybrowser\include\"
    xcopy /Y /S "%DEPS%\qt-solutions\qtpropertybrowser\lib\*.dll" "%DEPLOY_DIR%\qtpropertybrowser\bin\"
    xcopy /Y /S "%DEPS%\qt-solutions\qtpropertybrowser\lib\*.lib" "%DEPLOY_DIR%\qtpropertybrowser\lib\"
) ELSE (cecho {0E}Could not detect QtPropertyBrowser with %DEPS%\qtscriptgenerator\plugins\script\qtscript_xmlpatterns.dll, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Bullet
IF EXIST "%DEPS%\bullet\lib\release\LinearMath.lib". (
    cecho {0D}Copying Bullet{# #}{\n}
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\bullet\src\src" "%DEPLOY_DIR%\bullet\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude "%DEPS%\bullet\lib" "%DEPLOY_DIR%\bullet\lib\"
) ELSE (cecho {0E}Could not detect Bullet with %DEPS%\bullet\lib\release\LinearMath.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Boost
IF EXIST "%DEPS%\boost\stage". (
    cecho {0D}Copying Boost headers, this will take a while.{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\boost\boost" "%DEPLOY_DIR%\boost\include\boost\"
    cecho {0D}Copying Boost libs{# #}{\n}
    xcopy /Y /S "%DEPS%\boost\stage\lib" "%DEPLOY_DIR%\boost\lib\"
    :: Remove not needed (big) libs, modify if Tundra boost usage changes
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_wave*.lib"
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_serialization*.lib"
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_wserialization*.lib"
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_unit_test*.lib"
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_test_exec_monitor*.lib"
    del /Q "%DEPLOY_DIR%\boost\lib\libboost_math_tr1*.lib"
) ELSE (cecho {0E}Could not detect Boost with %DEPS%\boost\stage, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: kNet
IF EXIST "%DEPS%\kNet\lib\RelWithDebInfo\kNet.lib". (
    cecho {0D}Copying kNet{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\kNet\include" "%DEPLOY_DIR%\kNet\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude+%DEPLOY_DIR%\exe.exclude "%DEPS%\kNet\lib" "%DEPLOY_DIR%\kNet\lib\"
    :: MessageCompiler.exe might be useful for some people. Dont use PREFER_RELEASE here as this is an external tool.
    IF EXIST "%DEPS%\kNet\lib\Release\MessageCompiler.exe". (
        xcopy /Y "%DEPS%\kNet\lib\Release\MessageCompiler.exe" "%DEPLOY_DIR%\kNet\bin\"
    ) ELSE (
        IF EXIST "%DEPS%\kNet\lib\RelWithDebInfo\MessageCompiler.exe". xcopy /Y "%DEPS%\kNet\lib\RelWithDebInfo\MessageCompiler.exe" "%DEPLOY_DIR%\kNet\bin\"
    )
) ELSE (cecho {0E}Could not detect kNet with %DEPS%\kNet\lib\RelWithDebInfo\kNet.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Ogre
IF EXIST "%DEPS%\ogre-safe-nocrashes\SDK\lib\%PREFERRED_RELEASE%\OgreMain.lib". (
    cecho {0D}Copying Ogre headers, this will take a while.{# #}{\n}
    :: Ignore OIS include folder
    xcopy /Y /S /Q "%DEPS%\ogre-safe-nocrashes\SDK\include\OGRE" "%DEPLOY_DIR%\ogre-safe-nocrashes\include\OGRE\"
    cecho {0D}Copying Ogre libs{# #}{\n}
    xcopy /Y /S "%DEPS%\ogre-safe-nocrashes\SDK\bin\Debug\*.dll" "%DEPLOY_DIR%\ogre-safe-nocrashes\bin\"
    xcopy /Y /S "%DEPS%\ogre-safe-nocrashes\SDK\lib\Debug\*.lib" "%DEPLOY_DIR%\ogre-safe-nocrashes\lib\"
    xcopy /Y /S "%DEPS%\ogre-safe-nocrashes\SDK\bin\%PREFERRED_RELEASE%\*.dll" "%DEPLOY_DIR%\ogre-safe-nocrashes\bin\"
    xcopy /Y /S "%DEPS%\ogre-safe-nocrashes\SDK\lib\%PREFERRED_RELEASE%\*.lib" "%DEPLOY_DIR%\ogre-safe-nocrashes\lib\"
    xcopy /Y /S "%DEPS%\ogre-safe-nocrashes\SDK\bin\%PREFERRED_RELEASE%\*.exe" "%DEPLOY_DIR%\ogre-safe-nocrashes\bin\"
    copy /Y "%DEPS%\ogre-safe-nocrashes\Dependencies\bin\Release\cg.dll" "%DEPLOY_DIR%\ogre-safe-nocrashes\bin\"
    :: Copy Cmake, they are sometimes useful for building plugins like skyx/hydrax (they are total <100kb in size)
    xcopy /Y /S /Q "%DEPS%\ogre-safe-nocrashes\SDK\CMake" "%DEPLOY_DIR%\ogre-safe-nocrashes\CMake\"
    :: Remove OIS libs/dlls
    del /Q "%DEPLOY_DIR%\ogre-safe-nocrashes\lib\OIS*dll"
    del /Q "%DEPLOY_DIR%\ogre-safe-nocrashes\lib\OIS*lib"
) ELSE (cecho {0E}Could not detect Ogre with %DEPS%\ogre-safe-nocrashes\SDK\lib\%PREFERRED_RELEASE%\OgreMain.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: SkyX
IF EXIST "%DEPS%\realxtend-tundra-deps\skyx\lib\%PREFERRED_RELEASE%\SkyX.lib". (
    cecho {0D}Copying SkyX{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\realxtend-tundra-deps\skyx\SkyX\Include" "%DEPLOY_DIR%\skyx\include\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\skyx\bin\Debug\*.dll" "%DEPLOY_DIR%\skyx\bin\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\skyx\lib\Debug\*.lib" "%DEPLOY_DIR%\skyx\lib\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\skyx\bin\%PREFERRED_RELEASE%\*.dll" "%DEPLOY_DIR%\skyx\bin\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\skyx\lib\%PREFERRED_RELEASE%\*.lib" "%DEPLOY_DIR%\skyx\lib\"
) ELSE (cecho {0E}Could not detect SkyX with %DEPS%\realxtend-tundra-deps\skyx\lib\%PREFERRED_RELEASE%\SkyX.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Hydrax
IF EXIST "%DEPS%\realxtend-tundra-deps\hydrax\lib\Hydrax.lib". (
    cecho {0D}Copying Hydrax{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\realxtend-tundra-deps\hydrax\include" "%DEPLOY_DIR%\hydrax\include\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\hydrax\lib\*.dll" "%DEPLOY_DIR%\hydrax\bin\"
    xcopy /Y /S "%DEPS%\realxtend-tundra-deps\hydrax\lib\*.lib" "%DEPLOY_DIR%\hydrax\lib\"
) ELSE (cecho {0E}Could not detect Hydrax with %DEPS%\realxtend-tundra-deps\hydrax\lib\Hydrax.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: OpenAL
IF EXIST "%DEPS%\OpenAL\libs\Win32\OpenAL32.lib". (
    cecho {0D}Copying OpenAL{# #}{\n}
    xcopy /Y /S "%DEPS%\OpenAL" "%DEPLOY_DIR%\openal\"
) ELSE (cecho {0E}Could not detect OpenAL with %DEPS%\OpenAL\libs\Win32\OpenAL32.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Ogg
IF EXIST "%DEPS%\ogg\win32\VS2008\Win32\Release\libogg_static.lib". (
    cecho {0D}Copying Ogg{# #}{\n}
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\ogg\include" "%DEPLOY_DIR%\ogg\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude "%DEPS%\ogg\win32\VS2008\Win32" "%DEPLOY_DIR%\ogg\lib\"
) ELSE (cecho {0E}Could not detect Ogg with %DEPS%\ogg\win32\VS2008\Win32\Release\libogg_static.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Vorbis
IF EXIST "%DEPS%\vorbis\win32\VS2008\Win32\Release\libvorbis_static.lib". (
    cecho {0D}Copying Vorbis{# #}{\n}
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\vorbis\include" "%DEPLOY_DIR%\vorbis\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude+%DEPLOY_DIR%\exe.exclude "%DEPS%\vorbis\win32\VS2008\Win32" "%DEPLOY_DIR%\vorbis\lib\"
) ELSE (cecho {0E}Could not detect Vorbis with %DEPS%\vorbis\win32\VS2008\Win32\Release\libvorbis_static.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Theora
IF EXIST "%DEPS%\theora\win32\VS2008\Win32\Release_SSE2\libtheora_static.lib". (
    cecho {0D}Copying Theora{# #}{\n}
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\theora\include" "%DEPLOY_DIR%\theora\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude+%DEPLOY_DIR%\exe.exclude "%DEPS%\theora\win32\VS2008\Win32" "%DEPLOY_DIR%\theora\lib\"
) ELSE (cecho {0E}Could not detect Theora with %DEPS%\theora\win32\VS2008\Win32\Release_SSE2\libtheora_static.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Speex
IF EXIST "%DEPS%\speex\win32\VS2008\Win32\Release\libspeex.lib". (
    cecho {0D}Copying Speex{# #}{\n}
    :: Note: We are copying both /win32 and /lib path for libs.
    xcopy /Y /S /Q /EXCLUDE:%DEPLOY_DIR%\nonheaders.exclude "%DEPS%\speex\include" "%DEPLOY_DIR%\speex\include\"
    xcopy /Y /S /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude+%DEPLOY_DIR%\exe.exclude "%DEPS%\speex\win32\VS2008\Win32" "%DEPLOY_DIR%\speex\win32\VS2008\Win32\"
    xcopy /Y /Q "%DEPS%\speex\lib" "%DEPLOY_DIR%\speex\lib\"
) ELSE (cecho {0E}Could not detect Speex with %DEPS%\speex\win32\VS2008\Win32\Release\libspeex.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Celt
IF EXIST "%DEPS%\celt\lib\libcelt.lib". (
    cecho {0D}Copying Celt{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\celt\include" "%DEPLOY_DIR%\celt\include\"
    xcopy /Y /S "%DEPS%\celt\lib" "%DEPLOY_DIR%\celt\lib\"
) ELSE (cecho {0E}Could not detect Celt with %DEPS%\celt\lib\libcelt.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Protobuf
IF EXIST "%DEPS%\protobuf\vsprojects\Release\libprotobuf.lib". (
    cecho {0D}Copying Protobuf{# #}{\n}
    :: FindProtobuf.cmake is stupid, so (for now) lets keep the directory hierarchy that it understands.
    IF NOT EXIST "%DEPS%\protobuf\vsprojects\include". (
        cd "%DEPS%\protobuf\vsprojects"
        call extract_includes.bat
        cd "%DEPLOY_DIR%"
    )
    xcopy /Y /S /Q "%DEPS%\protobuf\vsprojects\include" "%DEPLOY_DIR%\protobuf\include\"
    xcopy /Y /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude "%DEPS%\protobuf\vsprojects\Debug" "%DEPLOY_DIR%\protobuf\vsprojects\Debug\"
    xcopy /Y /EXCLUDE:%DEPLOY_DIR%\nonlibs.exclude "%DEPS%\protobuf\vsprojects\Release" "%DEPLOY_DIR%\protobuf\vsprojects\Release\"
    xcopy /Y "%DEPS%\protobuf\vsprojects\Release\*.exe" "%DEPLOY_DIR%\protobuf\bin\"
) ELSE (cecho {0E}Could not detect Protobuf with %DEPS%\protobuf\vsprojects\Release\libprotobuf.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: VLC
IF EXIST "%DEPS%\vlc\lib\libvlc.lib". (
    cecho {0D}Copying VLC{# #}{\n}
    xcopy /Y /S /Q "%DEPS%\vlc\include" "%DEPLOY_DIR%\vlc\include\"
    xcopy /Y /S /Q "%DEPS%\vlc\bin" "%DEPLOY_DIR%\vlc\bin\"
    xcopy /Y /S /Q "%DEPS%\vlc\lib" "%DEPLOY_DIR%\vlc\lib\"
) ELSE (cecho {0E}Could not detect VLC with %DEPS%\vlc\lib\libvlc.lib, skipping...{# #}{\n})
IF NOT %ERRORLEVEL%==0 GOTO :ERROR

:: Cleanup
cd "%DEPLOY_DIR%"
del /Q *.exclude

:: Copying is all done
echo.
cecho {0A}Tundra dependencies deployed to %DEPLOY_DIR%.{# #}{\n}
echo.

set PACKAGE_NAME=tundra-deps-%DEPLOY_TARGET%
IF NOT "%DEPLOY_VERSION%" == "" set PACKAGE_NAME=%PACKAGE_NAME%-%DEPLOY_VERSION%
set PACKAGE_NAME=%PACKAGE_NAME%.7z

:: Create .7z archive from the deps.
IF %DEPLOY_PACKAGE%==TRUE (
    cecho {0A}Packaging Tundra dependencies to %DEPLOY_DIR%\%PACKAGE_NAME%{# #}{\n}
    echo This will take a long time, are you sure you want to continue? Press Ctrl-C to abort!
    pause
    cd "%DEPLOY_DIR%"
    IF EXIST "%PACKAGE_NAME%". del /Q "%PACKAGE_NAME%"
    call 7za.exe a -t7z %PACKAGE_NAME% %DEPLOY_DIR% -mx9 -mmt -y -r
) ELSE (
    cecho {0D}Not deploying to %DEPLOY_DIR%\%PACKAGE_NAME%, DEPLOY_PACKAGE is FALSE.{# #}{\n}
)

:: Finish script
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
echo.
GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
cd "%DEPLOY_DIR%"
del /Q *.exclude
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
pause
