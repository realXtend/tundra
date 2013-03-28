:: This is a common utility script that copies runtime libraries.
:: Calling script must have defined TUNDRA_BIN and DEPS.

IF NOT EXIST "%TUNDRA_BIN%\ssleay32.dll". (
   cd "%DEPS%"
   cecho {0D}Deploying OpenSSL DLLs to Tundra bin\{# #}{\n}
   copy /Y "openssl\bin\*.dll" "%TUNDRA_BIN%"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%TUNDRA_BIN%\QtWebKit4.dll". (
   cecho {0D}Deploying Qt DLLs to Tundra bin\.{# #}{\n}
   copy /Y "%DEPS%\qt\bin\*.dll" "%TUNDRA_BIN%"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   mkdir "%TUNDRA_BIN%\qtplugins"
   xcopy /E /I /C /H /R /Y "%DEPS%\qt\plugins\*.*" "%TUNDRA_BIN%\qtplugins"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
   :: Clean out some definately not needed Qt DLLs from bin
   :: QtCLucene does not have a public API and QtDesigner* are for QtCretor etc.
   :: Others we could (should) remove right here: QtSvg, QtSql, QtTest and QtHelp.
   del /Q "%TUNDRA_BIN%\QtCLucene*.dll"
   del /Q "%TUNDRA_BIN%\QtDesigner*.dll"
)

IF NOT EXIST "%TUNDRA_BIN%\qtplugins\script\qtscript_core.dll". (
   cecho {0D}Deploying QtScript plugin DLLs.{# #}{\n}
   mkdir "%TUNDRA_BIN%\qtplugins\script"
   xcopy /Q /E /I /C /H /R /Y "%DEPS%\qtscriptgenerator\plugins\script\*.dll" "%TUNDRA_BIN%\qtplugins\script"
)

IF NOT EXIST "%TUNDRA_BIN%\OgreMain.dll". (
    cecho {0D}Deploying Ogre DLLs to Tundra bin\ directory.{# #}{\n}
    :: Support both fullbuild and prebuilt paths.
    IF EXIST "%DEPS%\ogre-safe-nocrashes\bin". copy /Y "%DEPS%\ogre-safe-nocrashes\bin\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\ogre-safe-nocrashes\bin\debug". copy /Y "%DEPS%\ogre-safe-nocrashes\bin\debug\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\ogre-safe-nocrashes\bin\relwithdebinfo". copy /Y "%DEPS%\ogre-safe-nocrashes\bin\relwithdebinfo\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\ogre-safe-nocrashes\bin\release". copy /Y "%DEPS%\ogre-safe-nocrashes\bin\release\*.dll" "%TUNDRA_BIN%")
    IF EXIST "%DEPS%\ogre-safe-nocrashes\Dependencies\bin\Release". copy /Y "%DEPS%\ogre-safe-nocrashes\Dependencies\bin\Release\cg.dll" "%TUNDRA_BIN%"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%TUNDRA_BIN%\SkyX.dll". (
    cecho {0D}Deploying SkyX DLLs to Tundra bin\.{# #}{\n}
    :: Support both fullbuild and prebuilt paths.
    IF EXIST "%DEPS%\skyx\bin". copy /Y "%DEPS%\skyx\bin\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\realxtend-tundra-deps\skyx\bin\debug". copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\debug\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\realxtend-tundra-deps\skyx\bin\relwithdebinfo". copy /Y "%DEPS%\realxtend-tundra-deps\skyx\relwithdebinfo\debug\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\realxtend-tundra-deps\skyx\bin\release". copy /Y "%DEPS%\realxtend-tundra-deps\skyx\bin\release\*.dll" "%TUNDRA_BIN%"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%TUNDRA_BIN%\Hydrax.dll". (
    cecho {0D}Deploying Hydrax DLLs to Tundra bin\.{# #}{\n}
    :: Support both fullbuild and prebuilt paths.
    IF EXIST "%DEPS%\hydrax\bin". copy /Y "%DEPS%\hydrax\bin\*.dll" "%TUNDRA_BIN%"
    IF EXIST "%DEPS%\realxtend-tundra-deps\hydrax\lib". copy /Y "%DEPS%\realxtend-tundra-deps\hydrax\lib\*.dll" "%TUNDRA_BIN%"
    IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

IF NOT EXIST "%TUNDRA_BIN%\QtSolutions_PropertyBrowser-head.dll". (
   cecho {0D}Deploying QtPropertyBrowser DLLs.{# #}{\n}
   copy /Y "%DEPS%\qt-solutions\qtpropertybrowser\lib\QtSolutions_PropertyBrowser-head*.dll" "%TUNDRA_BIN%"
)

IF NOT EXIST "%TUNDRA_BIN%\libvlc.dll". (
   cecho {0D}Deploying VLC 2.0.1 DLLs to Tundra bin\{# #}{\n}
   xcopy /E /I /C /H /R /Y "%DEPS%\vlc\bin\*.*" "%TUNDRA_BIN%"
   IF NOT %ERRORLEVEL%==0 GOTO :ERROR
)

GOTO :EOF

:ERROR
echo.
cecho {0C}An error occurred! Aborting!{# #}{\n}
set PATH=%ORIGINAL_PATH%
cd %TOOLS%
pause
