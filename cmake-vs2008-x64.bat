@echo off
IF EXIST tundra.sln del /Q tundra.sln
cd tools\Windows\
call RunCMake.cmd "Visual Studio 9 2008 Win64"
cd ..\..
pause
