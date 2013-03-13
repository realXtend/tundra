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
export TUNDRA_ANDROID_ABI=x86 # Possible options 'armeabi', 'armeabi-v7a', 'x86'
echo "Targeting Android ABI '$TUNDRA_ANDROID_ABI'"
 
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

if [ ! -d bullet/.svn ]; then
	echo "Checking out Bullet.."
	svn checkout http://bullet.googlecode.com/svn/tags/bullet-2.78 bullet
	echo "Running cmake for Bullet.."
	cd bullet
	cmake -G "Unix Makefiles" -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$TUNDRA_ANDROID_ABI .
	echo "Building Bullet.."
	make
	cd $deps
else
	echo "Bullet already checked out. Skipping.."
fi

if [ ! -d boost ]; then
	echo "Downloading Boost.."
	cd $deps
	wget http://downloads.sourceforge.net/project/boost/boost/1.49.0/boost_1_49_0.tar.gz
	tar xvf boost_1_49_0.tar.gz
	mv boost_1_49_0 boost
	rm boost_1_49_0.tar.gz
	echo "Patching Boost.."
        cd $deps/boost
	cp $tundra_android/boost-1_49_0.patch .
	patch -l -p1 -i boost-1_49_0.patch
	echo "Building the Boost build engine.."
	chmod a+x bootstrap.sh
	./bootstrap.sh
    	cp $tundra_android/user-config_$TUNDRA_ANDROID_ABI.jam tools/build/v2/user-config.jam
    	cp $tundra_android/project-config.jam .
    	./b2 --with-date_time --with-filesystem --with-program_options --with-regex --with-signals --with-system --with-thread --with-iostreams toolset=gcc-android4.4.3 link=static runtime-link=static target-os=linux --stagedir=. --layout=system
        cd $deps	
else
	echo "Boost already checked out. Skipping.."
fi

if [ ! -d kNet ]; then
	echo "Cloning kNet.."
	git clone https://github.com/juj/kNet
	cd kNet
	echo "Switching to master branch, stable doesn't yet work on Android. (TODO: Remove this after merge)"
	git checkout master
	echo "Running cmake for kNet.."
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$TUNDRA_ANDROID_ABI -DBOOST_ROOT=$deps/boost  .
	echo "Building kNet.."
	make
	cd $deps
else
	echo "kNet already cloned. Skipping.."
fi

if [ ! -d ogre/.hg ]; then
	echo "Cloning OGRE.."
	hg clone -r dd56c87 https://bitbucket.org/sinbad/ogre ogre
else
	echo "OGRE already cloned. Skipping.."
fi

if [ ! -d ogre/AndroidDependenciesBuild/.hg ]; then
	cd ogre
	echo "Cloning OGRE Android dependencies.."
	hg clone https://bitbucket.org/cabalistic/ogredeps AndroidDependenciesBuild
	cd $deps
else
	echo "OGRE Android dependencies already cloned. Skipping.."
fi

if [ ! -f $deps/ogre/AndroidDependenciesBuild/lib/libFreeImage.a ]; then
	cd $deps/ogre/AndroidDependenciesBuild
	echo "Running cmake of OGRE Android dependencies.."
	# IMPORTANT! Use OGRE's modded version of Android-OpenCV toolchain instead of the 'stock'
	# version used in Tundra.
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$TUNDRA_ANDROID_ABI -DCMAKE_INSTALL_PREFIX="$deps/ogre/AndroidDependencies" .
	echo "Building OGRE Android dependencies.."
	make
	echo "Installing OGRE Android dependencies.."
	make install
	cd $deps
        # Rename libFreeImage to libfreeimage, otherwise OGRE build will not find it properly
        mv $deps/ogre/AndroidDependencies/lib/libFreeImage.a $deps/ogre/AndroidDependencies/lib/libfreeimage.a
else
	echo "Ogre Android dependencies already built. Skipping.."
fi

if [ ! -f $deps/ogre/lib/libOgreMainStatic.a ]; then
	cd $deps/ogre
	echo "Running cmake for OGRE.."
	# IMPORTANT! Use OGRE's modded version of Android-OpenCV toolchain instead of the 'stock'
	# version used in Tundra.
	cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=CMake/toolchain/android.toolchain.cmake -DCMAKE_BUILD_TYPE=$cmake_build_type -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$TUNDRA_ANDROID_ABI -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_DEPENDENCIES_DIR=./AndroidDependencies .
	echo "Building OGRE.."
	make VERBOSE=1

	cd $deps	
else
	echo "OGRE already built. Skipping.."
fi

if [ ! -d $tundra/bin/media/RTShaderLib ]; then 
    echo "Copying OGRE RTShader media to Tundra's media directory.."
    cp -r $deps/ogre/Samples/Media/RTShaderLib $tundra/bin/media
fi

