@echo off
:: Assume that Necessitas-Qt exists in deps-android/Qt. Change this as necessary.
cd ..
set QTDIR=%CD%\deps-android\Qt
cd android

:: Set Android ABI
set TUNDRA_ANDROID_ABI=armeabi-v7a

:: Build
call BuildTundra.cmd