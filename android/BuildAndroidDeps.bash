#!/bin/bash

set -e # Quit the script immediately if any of the commands fails.

# Test that the user has all the necessary command line tools installed
echo "Testing the presence of wget,svn,git,hg,cmake,make.."
wget --version > /dev/null
svn --version > /dev/null
git --version > /dev/null
hg --version > /dev/null
cmake --version > /dev/null
make --version > /dev/null
echo "OK."

cmake_build_type=Release

tundra=$(dirname $(readlink -f $0))/..
deps=$tundra/deps-android
mkdir -p $deps
deps=$(cd $deps && pwd)
tundra=$(cd $tundra && pwd)
tundra_android=$tundra/android

cd $deps

if [ ! -d bullet ]; then
	echo "Checking out Bullet.."
	svn checkout http://bullet.googlecode.com/svn/tags/bullet-2.78 bullet
	echo "Running cmake for Bullet.."
	cmake -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DCMAKE_TOOLCAHIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type .
	echo "Building Bullet.."
	make
else
	echo "Bullet already checked out. Skipping.."
fi

