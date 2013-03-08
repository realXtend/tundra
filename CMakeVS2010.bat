@echo off
IF EXIST tundra.sln del /Q tundra.sln
cd tools
call windows-run-cmake "Visual Studio 10"
cd ..
pause
