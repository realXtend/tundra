:: TODO Almost identical file with BuildTundra_Debug.cmd
@echo off
call RunCMake.cmd

cd ..\..

tools\Windows\Utils\cecho {0D}Building RelWithDebInfo Tundra.{# #}{\n}
MSBuild tundra.sln /p:Configuration=RelWithDebInfo %1 %2 %3 %4 %5 %6 %7 %8 %9
IF NOT %ERRORLEVEL%==0 GOTO :Error
echo.

tools\Windows\Utils\cecho {0A}RelWithDebInfo Tundra build finished.{# #}{\n}
goto :End

:Error
echo.
tools\Windows\Utils\cecho {0C}RelWithDebInfo Tundra build failed!{# #}{\n}

:End
:: Finish in same directory we started in.
cd tools\Windows
