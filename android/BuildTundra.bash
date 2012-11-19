set -e # Quit the script immediately if any of the commands fails.

echo "ANDROID_NDK is at '$ANDROID_NDK'"
if [ -z "$ANDROID_NDK" ]; then
	echo "ERROR: Please export environment variable ANDROID_NDK to point to where you installed the Android NDK (r8)! Note that NDK r8b or newer are not (yet) supported!"
	exit 1
fi

echo "Necessitas-Qt is at '$QTDIR'"
if [ -z "$QTDIR" ]; then
	echo "ERROR: Please export environment variable QTDIR to point to where you cloned and built Necessitas-Qt!"
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
tundra_android=$tundra/android

export BOOST_ROOT=$deps/boost
export TUNDRA_DEP_PATH=$deps
export KNET_DIR=$deps/kNet
export BULLET_DIR=$deps/bullet
export OGRE_HOME=$deps/ogre

# Add Necessitas QT bin directory for moc
export PATH=$QTDIR/bin:$PATH

if [ ! -f $tundra_android\local.properties ]; then
    echo "Configuring Tundra Android project.."
    cd $tundra_android
    android update project -p . -t android-10
fi

echo "Preparing Tundra build.."
cd $tundra
cmake -DCMAKE_TOOLCHAIN_FILE=$tundra_android/android.toolchain.cmake -DBOOST_ROOT=$deps/boost -DANDROID=1 -DANDROID_NATIVE_API_LEVEL=$tundra_android_native_api_level -DANDROID_ABI=$TUNDRA_ANDROID_ABI -DCMAKE_BUILD_TYPE=$cmake_build_type

echo "Building Tundra.."
make

echo "Building and signing Tundra APK.."
cd $tundra_android
ant release



