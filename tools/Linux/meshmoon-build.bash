#!/bin/bash

set -e

# Functions

print_help()
{
cat << EOF
Usage: $0 OPTIONS

Options:
  -s, --server           Build Meshmoon server.

  -np, --no-packages     Skip running OS package manager.
  -nc, --no-cmake        Skip running Tundra CMake
  -nb, --no-build        Skip building Tundra

  -h, --help             Print this help

  Note: Params cannot be combined to a single command eg. -ncnb).

EOF
}

print_title()
{
    echo
    echo $1
    echo "----------------------------------------"
}

is_built()
{
    echo
    echo $1

    # Directory exists?
    test -d "$DEPS_SRC/$1"
    if [ $? -ne 0 ] ; then
        echo "Error: Sources not found from $DEPS_SRC/$1"
        exit $?
    fi

    # Build done marker file exists?
    test -f "$DEPS_SRC/$1/meshmoon-builder.json"
    result=$?
    if [ $result -eq 0 ] ; then
        echo "    Build      OK"
        cd $DEPS
    else
        echo "    Building, please wait..."
        cd $DEPS_SRC/$1
    fi
    return $result
}

mark_built()
{
    touch $DEPS_SRC/$1/meshmoon-builder.json
}

# Settings affected by comman line args

skip_pkg=false
skip_deps=false
skip_cmake=false
skip_build=false
build_server="FALSE"

# Parse command line args

while [[ $1 = -* ]]; do
    arg=$1; 
    shift

    case $arg in
        --server|-s)
            build_server="TRUE"
            ;;
        --no-packages|-np)
            skip_pkg=true
            ;;
        --no-deps|-nd)
            skip_deps=true
            ;;
        --no-cmake|-nc)
            skip_cmake=true
            ;;
        --no-build|-nb)
            skip_build=true
            ;;
        --help|-h)
            print_help
            exit 0
            ;;
    esac
done

# Paths

TOOLS_PATH=$(dirname $(readlink -f $0))

TUNDRA=$(cd $TOOLS_PATH/../.. && pwd)
TUNDRA_BIN=$TUNDRA/bin

DEPS=$TUNDRA/deps
DEPS_SRC=$DEPS/src
DEPS_BIN=$DEPS/bin
DEPS_LIB=$DEPS/lib
DEPS_INC=$DEPS/include

# Misc

num_cpu=`grep -c "^processor" /proc/cpuinfo`

# Prepare

mkdir -p $DEPS

# Packages

if [ "$skip_pkg" = false ] ; then

    print_title "Updating apt repositories"
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get update

    print_title "Fetching packages: Build tools and utils"
    sudo apt-get -y install \
        build-essential gcc-4.9 g++-4.9 \
        cmake unzip 

    print_title "Fetching packages: Source control"
    sudo apt-get -y install \
        git subversion mercurial

    print_title "Fetching packages: Qt"
    sudo apt-get -y install \
        libqt4-dev libqt4-opengl-dev libqtwebkit-dev

    print_title "Fetching packages: Network and protocols"
    sudo apt-get -y install \
        openssl libssl-dev \
        libprotobuf-dev libprotobuf-c0 libprotobuf-c0-dev \
        protobuf-c-compiler protobuf-compiler

    print_title "Fetching packages: Audio and video"
    sudo apt-get -y install \
        libalut-dev libogg-dev libvorbis-dev libtheora-dev \
        libspeex-dev libspeexdsp-dev libvlc-dev

    print_title "Fetching packages: Graphics"
    sudo apt-get -y install \
        libgl1-mesa-dev libglu1-mesa-dev \
        freeglut3-dev \
        libxaw7-dev libxrandr-dev \
        nvidia-cg-toolkit

    print_title "Fetching packages: Ogre dependencies"
    sudo apt-get -y install \
        libfreetype6-dev libfreeimage-dev \
        libzzip-dev
fi

export CC="gcc-4.9"
export CXX="g++-4.9"
export PKG_CONFIG_PATH=$DEPS_LIB/pkgconfig
export QTDIR=`qmake -query QT_INSTALL_PREFIX`

CXX_FLAGS="-std=c++11 -O3 -Wno-ignored-qualifiers -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-cast-qual"
SHARED_LINKER_FLAGS="-Wl,-O1 -Wl,--no-undefined -Wl,-rpath-link,$DEPS_LIB"
EXE_LINKER_FLAGS="-Wl,-O1 -Wl,--no-undefined -Wl,-rpath-link,$DEPS_LIB"

if [ "$skip_deps" = false ] ; then

    # meshmoon-builder will do most of the heavy lifting for us

    meshmoon-builder -config $TOOLS_PATH/meshmoon-deps.json -prefix $DEPS
    meshmoon-builder -config $TOOLS_PATH/meshmoon-plugins.json -prefix $TUNDRA

    #### realxtend-tundra-deps/hydrax

    if ! is_built realxtend-tundra-deps/hydrax ; then
        cmake -Wno-dev \
              -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
              -DCMAKE_SHARED_LINKER_FLAGS="$SHARED_LINKER_FLAGS" \
              -DCMAKE_EXE_LINKER_FLAGS="$EXE_LINKER_FLAGS" \
              -DCMAKE_INSTALL_PREFIX=$DEPS \
              -DCMAKE_MODULE_PATH=$DEPS_LIB/OGRE/cmake \
              -DUSE_BOOST=FALSE \
              -DOGRE_HOME=$DEPS \
              -DTBB_HOME=$DEPS_SRC/tbb

        make -j $num_cpu -S -f Makefile
        
        mkdir -p $DEPS_INC/Hydrax
        cp lib/libHydrax.so $DEPS_LIB/
        cp -r include/* $DEPS_INC/Hydrax/

        mark_built realxtend-tundra-deps/hydrax
    fi

    #### realxtend-tundra-deps/skyx

    if ! is_built realxtend-tundra-deps/skyx ; then
        cmake -Wno-dev \
              -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
              -DCMAKE_SHARED_LINKER_FLAGS="$SHARED_LINKER_FLAGS" \
              -DCMAKE_EXE_LINKER_FLAGS="$EXE_LINKER_FLAGS" \
              -DCMAKE_INSTALL_PREFIX=$DEPS \
              -DCMAKE_MODULE_PATH=$DEPS_LIB/OGRE/cmake \
              -DUSE_BOOST=FALSE \
              -DOGRE_HOME=$DEPS \
              -DTBB_HOME=$DEPS_SRC/tbb

        make -j $num_cpu -S
        make install

        mark_built realxtend-tundra-deps/skyx
    fi

    #### qtscriptgenerator

    if ! is_built qtscriptgenerator ; then
        # Build generator
        cd generator
        qmake
        make -j $num_cpu -S
        ./generator --include-paths=`qmake -query QT_INSTALL_HEADERS`

        # Build bindings
        cd ../qtbindings
        sed -i 's/qtscript_phonon //' qtbindings.pro
        qmake
        make -j $num_cpu -S
        cd ..

        # Install
        mkdir -p $DEPS_LIB/qtplugins/script
        mkdir -p $TUNDRA_BIN/qtplugins/script
        cp -lf plugins/script/* $DEPS_LIB/qtplugins/script/
        cp -lf plugins/script/* $TUNDRA_BIN/qtplugins/script/

        mark_built qtscriptgenerator
    fi

    #### qt-solutions/qtpropertybrowser

    if ! is_built qt-solutions/qtpropertybrowser ; then
        echo yes | ./configure -library
        qmake
        make -j $num_cpu -S

        # Note: luckily only extensionless headers under src match Qt*
        cp lib/lib* $DEPS_LIB/
        cp src/qt*.h src/Qt* $DEPS_INC/

        mark_built qt-solutions/qtpropertybrowser
    fi

    #### boost

    if ! is_built boost ; then
        # Minimal build for the websocketpp library.
        # The prefix is hidden inside boost folder so
        # other projects an Tundra won't find it by accident.
        ./bootstrap.sh
        ./b2 install --prefix=$DEPS_SRC/boost/build --with-system

        mark_built boost
    fi
fi

# Sync dynamic libraries

rsync -u -L $DEPS_LIB/*.so* $TUNDRA_BIN/
rsync -u -L $DEPS_SRC/tbb/lib/tbb_release/*.so* $TUNDRA_BIN/
rsync -u -L $DEPS_SRC/boost/build/lib/*.so* $TUNDRA_BIN/
rsync -u -L $DEPS_LIB/OGRE/Plugin_CgProgramManager.so* \
            $DEPS_LIB/OGRE/Plugin_ParticleFX.so* \
            $DEPS_LIB/OGRE/Plugin_OctreeSceneManager.so* \
            $DEPS_LIB/OGRE/RenderSystem_*.so* \
            $TUNDRA_BIN/

# Tundra cmake

if [ "$skip_cmake" = false ] ; then

    export TUNDRA_DEP_PATH=$DEPS
    export OGRE_HOME=$DEPS_SRC/ogre-safe-nocrashes
    export TBB_HOME=$DEPS_SRC/tbb
    export BOOST_ROOT=$DEPS_SRC/boost/build
    export KNET_DIR=$DEPS
    export BULLET_DIR=$DEPS
    export SKYX_HOME=$DEPS
    export HYDRAX_HOME=$DEPS

    cd $TUNDRA
    rm -f CMakeCache.txt
    cmake -Wno-dev \
          -DMESHMOON_SERVER_BUILD="$build_server" \
          -DTUNDRA_NO_BOOST=TRUE \
          -DTUNDRA_CPP11_ENABLED=TRUE \
          -DINSTALL_BINARIES_ONLY=TRUE \
          -DCMAKE_SKIP_RPATH=TRUE \
          -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
          -DCMAKE_SHARED_LINKER_FLAGS="$SHARED_LINKER_FLAGS" \
          -DCMAKE_EXE_LINKER_FLAGS="$EXE_LINKER_FLAGS"
fi

# Tundra build

if [ "$skip_build" = false ] ; then

    cd $TUNDRA
    make -j $num_cpu -S
fi
