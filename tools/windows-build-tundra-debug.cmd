call windows-run-cmake.cmd

cd ..

cecho {0D}Building Tundra.{# #}{\n}
msbuild tundra.sln /p:Configuration=Debug %1 %2 %3 %4 %5 %6 %7 %8 %9
IF NOT %ERRORLEVEL%==0 GOTO :ERROR
echo.

cecho {0A}Tundra build finished.{# #}{\n}

:: Finish in same directory we started in.
cd tools
