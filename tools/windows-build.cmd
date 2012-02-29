call windows-build-deps.cmd

cd ..

cecho {0D}Running cmake for Tundra.{# #}{\n}

SET QMAKESPEC=win32-msvc2008
SET QTDIR=%DEPS%\qt
SET TUNDRA_DEP_PATH=%DEPS%
SET KNET_DIR=%DEPS%\kNet
SET OGRE_HOME=%DEPS%\ogre-safe-nocrashes
SET HYDRAX_HOME=%DEPS%\realxtend-tundra-deps\hydrax
cmake.exe -G %GENERATOR%
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
pause
