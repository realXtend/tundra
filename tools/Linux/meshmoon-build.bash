#!/bin/bash

set -e

# Imports

TOOLS_PATH=$(dirname $(readlink -f $0))
source ${TOOLS_PATH}/meshmoon-utils.bash

# Functions

print_help()
{
cat << EOF
Usage: $(basename $0) OPTIONS

Options:
  -s, --server           Build Meshmoon server.
  -p, --package          Build .deb package from results.

  --gcc                  Use gcc. Default: clang.

  -np, --no-packages     Skip running OS package manager.
  -nc, --no-cmake        Skip running Tundra CMake
  -nb, --no-build        Skip building Tundra

  -h, --help             Print this help

  Note: Params cannot be combined to a single command eg. -ncnb).

EOF
}

# Settings affected by comman line args

skip_pkg=false
skip_deps=false
skip_cmake=false
skip_build=false
skip_packager=true

# Clang is default. GCC >=4.9 works but
# is not yet in Ubuntu default repos.
# (We need it for c++11 regexp)

build_with_clang=true
build_server=false

# Parse command line args

while [[ $1 = -* ]]; do
    arg=$1; 
    shift

    case $arg in
        --server|-s)
            build_server=true
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
        --package|-p)
            skip_packager=false
            ;;
        --gcc)
            echo "GCC build are not yet supported"
            exit 1
            ;;
        --help|-h)
            print_help
            exit 0
            ;;
    esac
done

# Paths

export TUNDRA=$(cd $TOOLS_PATH/../.. && pwd)
export TUNDRA_BIN=$TUNDRA/bin

export DEPS=$TUNDRA/deps
export DEPS_SRC=$DEPS/src
export DEPS_BIN=$DEPS/bin
export DEPS_LIB=$DEPS/lib
export DEPS_INC=$DEPS/include

# Misc

export num_cpu=`grep -c "^processor" /proc/cpuinfo`

export MESHMOON_ROCKET_BUILD="TRUE"
export MESHMOON_SERVER_BUILD="FALSE"
if [ $build_server = true ] ; then
    export MESHMOON_ROCKET_BUILD="FALSE"
    export MESHMOON_SERVER_BUILD="TRUE"
fi

# Prepare

mkdir -p $DEPS

# Packages

if [ $skip_pkg = false ] ; then

    if [ $build_with_clang = true ] ; then
        print_title "Fetching packages: Build tools and utils"
        sudo apt-get -y install \
            build-essential clang \
            cmake unzip
    else
        echo "GCC build are not yet supported"
        exit 1
        #print_title "Updating apt repositories from GCC 4.9"
        #sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
        #sudo apt-get update

        #print_title "Fetching packages: Build tools and utils"
        #sudo apt-get -y install \
        #    build-essential gcc-4.9 g++-4.9 \
        #    cmake unzip
    fi

    print_title "Fetching packages: Source control"
    sudo apt-get -y install \
        git subversion mercurial

    print_title "Fetching packages: Qt"
    sudo apt-get -y install \
        libqt4-dev libqtwebkit-dev

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

    if [ $build_server = false ] ; then
        print_title "Fetching packages: Rocket dependencies"
        sudo apt-get -y install \
            libxmu-dev libxi-dev
    fi

    print_title "Fetching packages: Ogre dependencies"
    sudo apt-get -y install \
        libfreetype6-dev libfreeimage-dev \
        libzzip-dev
fi

# Export build tool env variables.

if [ $build_with_clang = true ] ; then
    export CXX_FLAGS="-O3 -std=c++11 -Wno-ignored-qualifiers -Wno-unused-parameter -Wno-cast-qual -Wno-deprecated-register -Wno-deprecated-declarations"
    export C_FLAGS="-g0 -O3 -Wno-unused-variable"

    export MESHMOON_BUILDER_TOOLSET="clang"
    export MESHMOON_BUILDER_QMAKE_SPEC="unsupported/linux-clang"
    export CC="clang $C_FLAGS"
    export CXX="clang++"
else
    export CXX_FLAGS="-O3 -std=c++11 -Wno-ignored-qualifiers -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-cast-qual"
    export C_FLAGS="-g0 -O3 -Wno-unused-variable"

    export MESHMOON_BUILDER_TOOLSET="gcc"
    export MESHMOON_BUILDER_QMAKE_SPEC="linux-g++-64"
    export CC="gcc-4.9 $C_FLAGS"
    export CXX="g++-4.9"
fi

export CXXFLAGS=$CXX_FLAGS
export CFLAGS=$CFLAGS
export LDFLAGS="-Wl,-O1 -Wl,--no-undefined"

export PKG_CONFIG_PATH=$DEPS_LIB/pkgconfig

export QTDIR=$(qmake -query QT_INSTALL_PREFIX)

print_title "Build tools config"
echo "    CC=$CC"
echo "    CXX=$CXX"
echo "    CXX_FLAGS=$CXX_FLAGS"
echo "    LDFLAGS=$LDFLAGS"

if [ $skip_deps = false ] ; then

    # Ogre needs to be re-configured and built when going from
    # Meshmoon Rocket to Meshmoon Server mode and vice versa.
    # This enabled us to build a Ogre that does not link to x11,
    # for a truely headless Meshmoon Server.

    if file_exists $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder.json ; then
        rebuild_ogre=false
        if file_exists $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-server.json ; then
            if [ $build_server = false ] ; then
                rebuild_ogre=true
            fi
        fi
        if file_exists $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-client.json ; then
            if [ $build_server = true ] ; then
                rebuild_ogre=true
            fi
        fi
        if [ $rebuild_ogre = true ] ; then
            rm -f $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder.json $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-server.json $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-client.json
            rm -f $DEPS_LIB/libOgre* $DEPS_BIN/Ogre*
            rm -rf $DEPS_LIB/OGRE $DEPS_INC/OGRE $DEPS/share/OGRE
        fi
    fi

    # meshmoon-builder will do most of the heavy lifting for us

    meshmoon-builder -config $TOOLS_PATH/meshmoon-deps.json -prefix $DEPS
    meshmoon-builder -config $TOOLS_PATH/meshmoon-plugins.json -prefix $TUNDRA

    # Mark Ogre build mode

    if [ $build_server = true ] ; then
        touch $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-server.json
    else
        touch $DEPS_SRC/ogre-safe-nocrashes/meshmoon-builder-client.json
    fi

    #### realxtend-tundra-deps/hydrax

    if ! is_built realxtend-tundra-deps/hydrax ; then
        cmake -Wno-dev \
              -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
              -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" \
              -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
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
              -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" \
              -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
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
        qmake -spec ${MESHMOON_BUILDER_QMAKE_SPEC}
        make -j $num_cpu -S
        ./generator --include-paths=`qmake -query QT_INSTALL_HEADERS`

        # Build bindings
        cd ../qtbindings
        sed -i 's/qtscript_phonon //' qtbindings.pro
        qmake -spec ${MESHMOON_BUILDER_QMAKE_SPEC}
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
        qmake -spec ${MESHMOON_BUILDER_QMAKE_SPEC}
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
        cc_temp=$CC
        if [ $build_with_clang = true ] ; then
            # Boost does not like our FLAGS being embedded to $CC
            export CC="clang"
        fi
        ./bootstrap.sh --with-toolset=${MESHMOON_BUILDER_TOOLSET}
        ./b2 install toolset=${MESHMOON_BUILDER_TOOLSET} cxxflags="-O3 -std=c++11" --prefix=$DEPS_SRC/boost/build --with-system --with-regex
        export CC="$cc_temp"

        mark_built boost
    fi

    # Rocket deps

    if [ $build_server = false ] ; then
        $TUNDRA/src/admino-plugins/tools/Linux/Ubuntu/meshmoon-build-deps.bash
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

if [ $skip_cmake = false ] ; then

    export TUNDRA_DEP_PATH=$DEPS

    export OGRE_HOME=$DEPS_SRC/ogre-safe-nocrashes
    export TBB_HOME=$DEPS_SRC/tbb
    export BOOST_ROOT=$DEPS_SRC/boost/build
    export TRITON_PATH=$DEPS_SRC/triton
    export SILVERLINING_PATH=$DEPS_SRC/silverlining

    export KNET_DIR=$DEPS
    export BULLET_DIR=$DEPS
    export SKYX_HOME=$DEPS
    export HYDRAX_HOME=$DEPS

    # Add rpath lookup dir. Otherwise they wont be
    # found as CMAKE_SKIP_RPATH is defined for Tundra.
    # Boost is added separately as we didn't want to
    # pollute $DEPS_LIB with Boost libs.
    export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$DEPS_LIB -Wl,-rpath-link,$DEPS_SRC/boost/build/lib/"

    print_title "Running Tundra CMake"

    cd $TUNDRA
    rm -f CMakeCache.txt
    rm -rf CMakeFiles
    cmake -Wno-dev \
          -DROCKET_OCULUS_ENABLED=FALSE \
          -DMESHMOON_SERVER_BUILD="$MESHMOON_SERVER_BUILD" \
          -DTUNDRA_NO_AUDIO="$MESHMOON_SERVER_BUILD" \
          -DTUNDRA_NO_BOOST=TRUE \
          -DTUNDRA_BOOST_REGEX=TRUE \
          -DTUNDRA_BOOST_SYSTEM=TRUE \
          -DTUNDRA_CPP11_ENABLED=TRUE \
          -DINSTALL_BINARIES_ONLY=TRUE \
          -DCMAKE_SKIP_RPATH=TRUE \
          -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
          -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" \
          -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"
fi

# Tundra build

if [ $skip_build = false ] ; then

    cd $TUNDRA
    make -j $num_cpu -S
fi

# Package

if [ $skip_packager = false ] ; then

    $TOOLS_PATH/meshmoon-packager.bash
fi
