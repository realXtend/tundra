#!/bin/bash
cd ..
make clean

rm -f bin/Tundra
rm -f bin/plugins/*.dylib
rm -f CMakeCache.txt
rm -f CMakeBuildConfig.txt
rm -rf tundra.build
rm -rf tundra.xcodeproj
cd tools

