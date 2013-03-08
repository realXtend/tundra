@echo off
IF EXIST tundra.sln del /Q tundra.sln
cd tools
call windows-run-cmake.cmd "Visual Studio 9 2008"
cd ..
pause
