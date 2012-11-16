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

echo "ANDROID_NDK is at '$ANDROID_NDK'"
if [ -z "$ANDROID_NDK" ]; then
	echo "ERROR: Please export environment variable ANDROID_NDK to point to where you installed the Android NDK (r8)! Note that NDK r8b or newer are not (yet) supported!"
	exit 1
fi

tundra_android_native_api_level=9 # This is the minimum API level we can possibly support, we require NativeActivity and AAssetManager.
echo "Targeting Android Native API level $tundra_android_native_api_level"
tundra_android_abi=x86 # Possible options 'armeabi', 'armeabi-v7a', 'x86'
echo "Targeting Android ABI '$tundra_android_abi'"
 
cmake_build_type=Release

tundra=$(dirname $(readlink -f $0))/..
echo "Tundra root directory is '$tundra'"
deps=$tundra/deps-android
echo "Tundra deps root directory is '$deps'"
mkdir -p $deps
deps=$(cd $deps && pwd)
tundra=$(cd $tundra && pwd)
tundra_android=$tundra/android

cd $deps

if [ ! -d bullet ]; then
	echo "Checking out Bullet.."
	svn checkout http://bullet.googlecode.com/svn/tags/bullet-2.78 bullet
	echo "Running cmake for Bullet.."
	cd bullet
	cmake -G "Unix Makefiles" -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$tundra_android_abi .
	echo "Building Bullet.."
	make
	cd $deps
else
	echo "Bullet already checked out. Skipping.."
fi

if [ ! -d kNet ]; then
	echo "Cloning kNet.."
	git clone https://github.com/juj/kNet
	cd kNet
	echo "Switching to master branch, stable doesn't yet work on Android. (TODO: Remove this after merge)"
	git checkout master
	echo "Running cmake for kNet.."
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DBOOST_ROOT=$deps/boost -DCMAKE_BUILD_TYPE=$cmake_build_type .
	echo "Building kNet.."
	make
	cd $deps
else
	echo "kNet already cloned. Skipping.."
fi

if [ ! -d ogre ]; then
	echo "Cloning OGRE.."
	hg clone -r v1-9 https://bitbucket.org/sinbad/ogre ogre
	cd ogre
	
	echo "Cloning OGRE Android dependencies.."
	hg clone https://bitbucket.org/cabalistic/ogredeps AndroidDependenciesBuild
	cd AndroidDependenciesBuild
	echo "Running cmake of OGRE Android dependencies.."
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$tundra_android_abi -DCMAKE_INSTALL_PREFIX="$deps/ogre/AndroidDependencies" .
	echo "Building OGRE Android dependencies.."
	make
	echo "Installing OGRE Android dependencies.."
	make install

	cd $deps/ogre
	echo "Running cmake for OGRE.."
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$tundra_android_abi -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_DEPENDENCIES_DIR=./AndroidDependencies .
	echo "Building OGRE.."
	make

	cd $deps	
else
	echo "OGRE already cloned. Skipping.."
fi

