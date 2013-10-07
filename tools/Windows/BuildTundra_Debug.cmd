:: TODO Almost identical file with BuildTundra_RelWithDebInfo.cmd
@echo off
call RunCMake.cmd

cd ..\..

tools\Windows\Utils\cecho {0D}Building Debug Tundra.{# #}{\n}
MSBuild tundra.sln /p:Configuration=Debug %1 %2 %3 %4 %5 %6 %7 %8 %9
IF NOT %ERRORLEVEL%==0 GOTO :Error
echo.

tools\Windows\Utils\cecho {0A}Debug Tundra build finished.{# #}{\n}
goto :End

:Error
echo.
tools\Windows\Utils\cecho {0C}Debug Tundra build failed!{# #}{\n}

:End
:: Finish in same directory we started in.
cd tools\Windows
