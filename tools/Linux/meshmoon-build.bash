#!/bin/bash

set -e

# Functions

print_help()
{
cat << EOF
Usage: $0 OPTIONS

Options:
  -np, --no-packages     Skip running OS package manager.
  -nc, --no-cmake        Skip running Tundra CMake
  -nb, --no-build        Skip building Tundra

  -h, --help             Print this help

  Note: Params cannot be combined to a single command eg. -ncnb).

EOF
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

# Parse command line args

while [[ $1 = -* ]]; do
    arg=$1; 
    shift

    case $arg in
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
    sudo aptitude -y install \
        build-essential g++ cmake \
        git subversion mercurial \
        unzip \
        libqt4-dev libqt4-opengl-dev libqtwebkit-dev \
        libboost-all-dev \
        libogg-dev libvorbis-dev libtheora-dev libspeex-dev libspeexdsp-dev libvlc-dev \
        openssl libssl-dev \
        libprotobuf-dev libprotobuf-c0 libprotobuf-c0-dev protobuf-c-compiler protobuf-compiler \
        nvidia-cg-toolkit freeglut3-dev libxml2-dev libalut-dev \
        liboil0.3-dev libxrandr-dev \
        libfreetype6-dev libfreeimage-dev libzzip-dev \
        libxaw7-dev libgl1-mesa-dev libglu1-mesa-dev
fi

export PKG_CONFIG_PATH=$DEPS_LIB/pkgconfig
export QTDIR=`qmake -query QT_INSTALL_PREFIX`

if [ "$skip_deps" = false ] ; then

    # meshmoon-builder will do most of the heavy lifting for us

    meshmoon-builder -config $TOOLS_PATH/meshmoon-deps.json -prefix $DEPS

    #### bullet

    if ! is_built bullet ; then
        cmake -DCMAKE_INSTALL_PREFIX=$DEPS -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF \
              -DBUILD_CPU_DEMOS=OFF -DINSTALL_EXTRA_LIBS=ON -DCMAKE_CXX_FLAGS_RELEASE="-O2 -g -fPIC" \
              -DCMAKE_DEBUG_POSTFIX= -DCMAKE_MINSIZEREL_POSTFIX= -DCMAKE_RELWITHDEBINFO_POSTFIX=

        make -j $num_cpu -S
        make install
        
        mark_built bullet
    fi

    #### knet

    if ! is_built kNet ; then
        sed -e "s/kNet STATIC/kNet SHARED/" < CMakeLists.txt > x
        mv x CMakeLists.txt

        cmake -DUSE_TINYXML:BOOL=FALSE

        make -j $num_cpu -S
        cp lib/libkNet.so $DEPS_LIB/
        rsync -r include/* $DEPS_INC/

        mark_built kNet
    fi

    #### realxtend-tundra-deps/hydrax

    if ! is_built realxtend-tundra-deps/hydrax ; then
        sed -i 's!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(shell pkg-config OGRE --cflags)!' makefile
        sed -i 's!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(shell pkg-config OGRE --libs)!' makefile
        
        make -j $num_cpu -S PREFIX=$DEPS
        make PREFIX=$DEPS install

        mark_built realxtend-tundra-deps/hydrax
    fi

    #### realxtend-tundra-deps/skyx

    if ! is_built realxtend-tundra-deps/skyx ; then
        env OGRE_HOME=$DEPS/lib/OGRE cmake -DCMAKE_INSTALL_PREFIX=$DEPS .
        make -j $num_cpu -S PREFIX=$DEPS
        make PREFIX=$DEPS install
        
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
fi

# tundra cmake

if [ "$skip_cmake" = false ] ; then
    export TUNDRA_DEP_PATH=$DEPS
    export OGRE_HOME=$DEPS_SRC/ogre-safe-nocrashes
    export KNET_DIR=$DEPS
    export BULLET_DIR=$DEPS
    export SKYX_HOME=$DEPS
    export HYDRAX_HOME=$DEPS

    export QTDIR=`qmake -query QT_INSTALL_PREFIX`
    export BOOST_ROOT=/usr

    cd $TUNDRA
    rm -f CMakeCache.txt
    cmake -DINSTALL_BINARIES_ONLY=TRUE
fi

# tundra build

if [ "$skip_build" = false ] ; then
    cd $TUNDRA
    make -j $num_cpu -S
fi
