#!/bin/bash
set -e

fnDisplayHelpAndExit()
{
    echo " "
    echo " USAGE: $0 --deps-path <PATH> [--help --client-path <PATH> --qt-path <PATH> --release-with-debug-info --no-run-cmake"
    echo "                               --no-run-make --number-of-processes <NUMBER>]"
    echo "    or: $0          -d <PATH> [-h -c <PATH> -q <PATH> -rwdi -nc -nm -np <NUMBER>]"
    echo " "
    echo " -h | --help                          Displays this message"
    echo " "
    echo " -d <PATH> | --deps-path <PATH>       Specifies the path in which the dependencies will be built. It is recommended"
    echo "                                      that PATH points to an empty location. (REQUIRED)"
    echo " " 
    echo " -c <PATH> | --client-path <PATH>     Specifies the path that points to the client path. If this is not specified, "
    echo "                                      it will be set as the same level as the 'deps' path with 'tundra2' as working"
    echo "                                      directory"
    echo " "
    echo " -q <PATH> | --qt-path <PATH>         Specifies the path where Qt is located. If this is not specified, it will be "
    echo "                                      set to the default sandboxed Qt installation directory which is              "
    echo "                                      /usr/local/Trolltech/Qt-4.7.1                                                "
    echo "                                      NOTE: This option will overwrite the value in environment variable QTDIR     "
    echo " "
    echo " -rwdi | --release-with-debug-info    Enables debugging information to be included in compile-time"
    echo " "
    echo " -nc | --no-run-cmake                 Do not run 'cmake .' after the dependencies are built. (The default is that  "
    echo "                                      'cmake .' is executed to generate Makefiles after all dependencies have been "
    echo "                                      built)."
    echo " "
    echo " -nm | --no-run-make                  Do not run 'make' after the dependencies are built. (The default is that     "
    echo "                                      'make' is executed to start the compile process)."
    echo " "
    echo " -np | --number-of-processes <NUMBER> The number of processes to be run simultaneously, recommended for multi-core "
    echo "                                      or processors with hyper-threading technology for a faster compile process."
    echo "                                      The default value is set according to the reports by the operating system."
    echo " "
    echo " NOTE: The properties that are in middle brackets are optional if otherwise not specified."
    echo " "
    exit 1
}

echo " "
echo "=============================== realXtend Tundra 2 dependency building script ==============================="
echo "= Script to build most dependencies. For now, the following dependencies are required to run this script:   ="
echo "=                                                                                                           ="
echo "= CMake                                                                                                     ="
echo "= Git                                                                                                       ="
echo "= Ogre SDK 1.7.3                                                                                            ="
echo "= Qt 4.7.1 for Cocoa 64-bit                                                                                 ="
echo "=                                                                                                           ="
echo "============================================================================================================="

# Some helper variables
NO_ARGS="0"
REQUIRED_ARGS=1
REQUIRED_ARGS_COUNT=0
ERRORS_OCCURED="0"

# Default values
RELWITHDEBINFO="0"
RUN_CMAKE="1"
RUN_MAKE="1"
NPROCS=`sysctl -n hw.ncpu`
CLIENT=

if [ $# -eq "$NO_ARGS" ]; then
    echo " ERROR: No options selected"
    echo " "
    fnDisplayHelpAndExit
fi

echo "= Chosen options: $@"
echo " "
while [ "$1" != "" ]; do
    case $1 in
        -h | --help )                       fnDisplayHelpAndExit
                                            ;;

        -rwdi | --release-with-debug-info ) RELWITHDEBINFO="1"
                                            ;;

        -d | --deps-path )                  shift
                                            if [ ! -d $1 ]; then
                                                echo "ERROR: Bad directory for --deps-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            REQUIRED_ARGS_COUNT=$((REQUIRED_ARGS_COUNT+1))
                                            DEPS=$1
                                            ;;

        -c | --client-path )                shift
                                            if [ ! -d $1 ]; then
                                                echo "ERROR: Bad directory for --client-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            CLIENT=$1
                                            ;;

        -q | --qt-path )                    shift
                                            if [ ! -d $1 ]; then
                                                echo "ERROR: Bad directory for --qt-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            export QTDIR=$1
                                            ;;

        -nc | --no-run-cmake )              RUN_CMAKE="0"
                                            ;;

        -nm | --no-run-make )               RUN_MAKE="0"
                                            ;;

        -np | --number-of-processes )       shift
                                            check=`echo $1 | awk '$0 ~/[^0-9]/ { print "NaN" }'`
                                            if [ "$check" == "NaN" ]; then
                                                echo "ERROR: Invalid value for --number-of-processes \"$1\""
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi

                                            if [ $1 -gt $NPROCS ]; then
                                                echo "WARNING: The number of processes that you specified ($1) is larger than the number of cores reported by the operating system ($NPROCS). This may cause slow performance during the compile process"
                                            fi

                                            NPROCS=$1
                                            ;;

        * )                                 echo "ERROR: Invalid option $1"
                                            ERRORS_OCCURED="1"
                                            shift
                                            continue
    esac
    shift
done

if [ "$ERRORS_OCCURED" == "1" ]; then
    fnDisplayHelpAndExit
fi

if [ $REQUIRED_ARGS_COUNT -ne $REQUIRED_ARGS ]; then
    echo "ERROR: One on more required options were omitted. Please try again."
    fnDisplayHelpAndExit
fi

if [ ! -d $CLIENT ]; then
    CLIENT=$DEPS/../tundra2
fi

if [ ! -d $QTDIR ]; then
    export QTDIR=/usr/local/Trolltech/Qt-4.7.1
fi

prefix=$DEPS
build=$DEPS/build
tarballs=$DEPS/tarballs
tags=$DEPS/tags

mkdir -p $tarballs $build $prefix/{lib,share,etc,include} $tags

if [ "$RELWITHDEBINFO" == "1" ]; then
    export CFLAGS="-gdwarf-2 -O0"
    export CXXFLAGS="-gdwarf-2 -O0"
    export CMAKE_C_FLAGS="-gdwarf-2 -O0"
    export CMAKE_CXX_FLAGS="-gdwarf-2 -O0"
fi

export PATH=$prefix/bin:$QTDIR/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include

cd $build
what=boost    
urlbase=http://downloads.sourceforge.net/project/boost/boost/1.46.1
pkgbase=boost_1_46_1
dlurl=$urlbase/$pkgbase.tar.gz    
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    ./bootstrap.sh --prefix=$prefix
    ./bjam --layout=versioned --build-type=complete install
    touch $tags/$what-done
fi

cd $build
what=bullet
urlbase=http://bullet.googlecode.com/files
unzipped=bullet-2.78
pkgbase=bullet-2.78-r2387
dlurl=$urlbase/$pkgbase.tgz
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $unzipped
    zip=$tarballs/$pkgbase.tgz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $unzipped
    cmake . -DCMAKE_INSTALL_PREFIX=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=ogg
urlbase=http://downloads.xiph.org/releases/ogg
pkgbase=libogg-1.3.0
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=vorbis
urlbase=http://downloads.xiph.org/releases/vorbis
pkgbase=libvorbis-1.3.2
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    ./configure --prefix=$prefix --with-ogg=$prefix --build=x86_64
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=theora
urlbase=http://downloads.xiph.org/releases/theora
pkgbase=libtheora-1.1.1
dlurl=$urlbase/$pkgbase.tar.bz2
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.bz2
    test -f $zip || curl -L -o $zip $dlurl
    bzip2 -d $zip
    tar xzf $tarballs/$pkgbase.tar

    cd $pkgbase
    ./configure --prefix=$prefix --with-ogg=$prefix --with-vorbis=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=qtpropertybrowser
if test -f $tags/$what-done; then
    echo $what is done
else
    test -d qt-solutions || git clone git://gitorious.org/qt-solutions/qt-solutions.git
    cd qt-solutions/$what
    echo "CONFIG += release" >> qtpropertybrowser.pro
    echo "CONFIG -= debug" >> qtpropertybrowser.pro
    ./configure -library
    qmake
    make VERBOSE=1 -j$NPROCS
    cp ./lib/* $prefix/lib
    cp ./src/*.h $prefix/include
    cp ./src/Qt* $prefix/include
    touch $tags/$what-done
fi

cd $build
what=protobuf
urlbase=http://protobuf.googlecode.com/files
pkgbase=protobuf-2.4.1
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=celt
urlbase=http://downloads.xiph.org/releases/celt
pkgbase=celt-0.11.1
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

cd $build
what=mumbleclient
if test -f $tags/$what-done; then
    echo $what is done
else
    test -d $what || git clone https://github.com/Adminotech/libmumble.git $what
    cd $what
    cmake .
    make VERBOSE=1 -j$NPROCS
    cp libmumbleclient.dylib $prefix/lib
    cp Mumble.pb.h $prefix/include
    mkdir $prefix/include/$what
    cp ./src/*.h $prefix/include/$what
    touch $tags/$what-done
fi

cd $CLIENT
if [ "$RUN_CMAKE" == "1" ]; then
    TUNDRA_DEP_PATH=$prefix cmake .
fi

if [ "$RUN_MAKE" == "1" ]; then
    make -j$NPROCS VERBOSE=1
fi
