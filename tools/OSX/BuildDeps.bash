#!/bin/bash
set -e

fnDisplayHelpAndExit()
{
    echo " "
    echo " USAGE: $0 [--help --client-path <PATH> --qt-path <PATH> --ogre-path <PATH> --use-boost --release-with-debug-info"
    echo "            --configuration <CONFIG> --no-xcode --no-run-cmake --no-run-make --number-of-processes <NUMBER>"
    echo "            --version-postfix <STRING>]"
    echo "    or: $0 [-h -c <PATH> -q <PATH> -o <PATH> -ub -rwdi -cfg <CONFIG> -nx -nc -nm -np <NUMBER> -vp <STRING>]"
    echo " "
    echo " -h | --help                          Displays this message"
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
    echo " -o <PATH> | --ogre-path <PATH>       Specifies the path where a custom Ogre root directory is located. If this is "
    echo "                                      not specified, the default-installed Ogre framework will be used.            "
    echo " "
    echo " -ub | --use-boost                    Build Tundra (and its dependencies where applicable) with boost." 
    echo "                                      If this is not specified, Tundra will be built without boost"
    echo " "
    echo " -nc | --no-run-cmake                 Do not run 'cmake .' after the dependencies are built. (The default is that  "
    echo "                                      'cmake .' is executed to generate Makefiles after all dependencies have been "
    echo "                                      built)."
    echo " "
    echo " -nm | --no-run-make                  Do not run 'make' after the dependencies are built, or if --xcode is defined,"
    echo "                                      do not run 'xcodebuild' command (The default is that 'make' is executed to "
    echo "                                      start the compile process)."
    echo " "
    echo " -np | --number-of-processes <NUMBER> The number of processes to be run simultaneously, recommended for multi-core "
    echo "                                      or processors with hyper-threading technology for a faster compile process."
    echo "                                      The default value is set according to the reports by the operating system."
    echo " "
    echo " -nx | --no-xcode                     By default, this script will create Xcode project. Use this to override"
    echo "                                      creating Xcode project in favor of Makefiles"
    echo " "
    echo " -cfg | --configuration <CONFIG>      The build configuration for realXtend Tundra. CONFIG can have Debug,"
    echo "                                      RelWithDebInfo or Release values"
    echo " "
    echo " -vp | --version-postfix <STRING>     Append version postfix <STRING> to Tundra version, which will be shown in the"
    echo "                                      main window title. Needed when doing release candidates or nightly builds."
    echo " "
    echo "DEPRECATED:"
    echo " "
    echo " -rwdi | --release-with-debug-info    Enables debugging information to be included in compile-time. RelWithDebInfo"
    echo "                                      is a default configuration. To override, use -cfg <CONFIG> or --configuration"
    echo "                                      <CONFIG> option)"
    echo " "
    echo " NOTE: The properties that are in middle brackets are optional if otherwise not specified."
    echo " "
    exit 1
}

echoInfo()
{
    COLOR="\033[32;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR INFO: $@ $ENDCOLOR
}

echoWarn()
{
    COLOR="\033[33;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR WARNING: $@ $ENDCOLOR
}

echoError()
{
    COLOR="\033[31;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR ERROR: $@ $ENDCOLOR
}

echo " "
echo "=============================== realXtend Tundra 2 dependency building script ==============================="
echo "= Script to build most dependencies. For now, the following dependencies are required to run this script:   ="
echo "=                                                                                                           ="
echo "= Xcode     for Mac OS X 10.6.x:           available on Apple Developer website                             ="
echo "=           for Mac OS X 10.7.x and newer: available on App Store. Command-line developer tools which       ="
echo "=                                          are part of Xcode must be installed too                          =" 
echo "= CMake     http://www.cmake.org/cmake/resources/software.html                                              ="
echo "= Git       http://git-scm.com/download/mac                                                                 ="
echo "= Mercurial http://mercurial.selenic.com/downloads                                                          ="
echo "= XQuartz   for Mac OS X 10.6.x-10.7.x:    NOT required                                                     ="
echo "=           for Mac OS X 10.8.x and newer: http://xquartz.macosforge.org/landing/                           ="
echo "============================================================================================================="

# Some helper variables
NO_ARGS="0"
REQUIRED_ARGS=0
REQUIRED_ARGS_COUNT=0
ERRORS_OCCURED="0"

# Default values
BUILD_CONFIGURATION=RelWithDebInfo
RUN_CMAKE="1"
RUN_MAKE="1"
MAKE_XCODE="1"
USE_BOOST="OFF"
NO_BOOST="ON" # The variable used in Tundra is called TUNDRA_NO_BOOST, which is opposite of USE_BOOST. We will use this also to specify c++11 usage, since not using boost and using c++11 on Mac are mutually exclusive
NPROCS=`sysctl -n hw.ncpu`
TUNDRA_VERSION_POSTFIX=
viewer=

if [ $# -eq "$NO_ARGS" ]; then
    echoError "No options selected"
    echo " "
    fnDisplayHelpAndExit
fi

echoInfo "Chosen options: $@"
echo " "
while [ "$1" != "" ]; do
    case $1 in
        -h | --help )                       fnDisplayHelpAndExit
                                            ;;

        -rwdi | --release-with-debug-info ) BUILD_CONFIGURATION=RelWithDebInfo
                                            ;;

        -cfg | --configuration )            shift
                                            if [ "$1" == "Debug" ]; then
                                                BUILD_CONFIGURATION=Debug
                                            elif [ "$1" == "RelWithDebInfo" ]; then
                                                BUILD_CONFIGURATION=RelWithDebInfo
                                            elif [ "$1" == "Release" ]; then
                                                BUILD_CONFIGURATION=Release
                                            else
                                                echoWarn "Unavailable build configuration: \"$1\" was requested, but \"Debug\", \"RelWithDebInfo\" and \"Release\" are available only. Defaulting to RelWithDebInfo"
                                                BUILD_CONFIGURATION=RelWithDebInfo
                                            fi
                                            ;;

        -c | --client-path )                shift
                                            if [ ! -d "$1" ]; then
                                                echoError "Bad directory for --client-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            viewer=$1
                                            ;;

        -q | --qt-path )                    shift
                                            if [ ! -d "$1" ]; then
                                                echoError "Bad directory for --qt-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            export QTDIR=$1
                                            ;;

        -o | --ogre-path )                  shift
                                            if [ ! -d "$1" ]; then
                                                echoError "Bad directory for --ogre-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            export OGRE_HOME=$1
                                            ;;

        -nx | --no-xcode )                  MAKE_XCODE="0"
                                            ;;

        -nc | --no-run-cmake )              RUN_CMAKE="0"
                                            ;;

        -nm | --no-run-make )               RUN_MAKE="0"
                                            ;;

        -ub | --use-boost )                 USE_BOOST="ON"
                                            NO_BOOST="OFF"
                                            ;;

        -np | --number-of-processes )       shift
                                            check=`echo $1 | awk '$0 ~/[^0-9]/ { print "NaN" }'`
                                            if [ "$check" == "NaN" ]; then
                                                echoError "Invalid value for --number-of-processes \"$1\""
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi

                                            if [ $1 -gt $NPROCS ]; then
                                                echoWarn "The number of processes that you specified ($1) is larger than the number of cores reported by the operating system ($NPROCS). This may cause slow performance during the compile process"
                                            fi

                                            NPROCS=$1
                                            ;;

        -vp | --version-postfix )           shift
                                            TUNDRA_VERSION_POSTFIX=$1
                                            ;;

        * )                                 echoError "Invalid option: $1"
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
    echoError "One on more required options were omitted. Please try again."
    fnDisplayHelpAndExit
fi

# If the path to the Tundra root directory was not specified, assume the script
# is being run from (gittrunk)/tools, so viewer=(gittrunk).
if [ ! -d "$viewer" ]; then
    cwd=$(pwd)          # Temporarily save this path to the build script.
    viewer=$(pwd)/../.. # Assume the build script lies at gittrunk/tools/OSX.
    cd $viewer
    viewer=$(pwd)
    cd $cwd        # Go back to not alter cwd.
fi

DEPS=$viewer/deps
prefix=$DEPS/osx-64
build=$DEPS/build
tarballs=$DEPS/tarballs
tags=$DEPS/tags
frameworkpath=$DEPS/Frameworks
patches=$viewer/tools/OSX/Mods

if [ ! -f "$QTDIR/bin/qmake" ]; then
    echoWarn "QTDIR does not point to a valid qt installation. Defaulting to $prefix/qt. You can specify a path with --qt-path parameter."
    export QTDIR=$prefix/qt
fi

mkdir -p $tarballs $build $prefix $tags $frameworkpath

export PATH=$QTDIR/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
#export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
#export LIBRARY_PATH=$prefix/lib
#export C_INCLUDE_PATH=$prefix/include
#export CPLUS_INCLUDE_PATH=$prefix/include

# Stupid workaround in case of Mountain Lion, where sed yields "invalid byte sequence" error
OSX_VERSION=`sw_vers -productVersion`
if [[ $OSX_VERSION == 10.8.* ]]; then
    LC_CTYPE_BAK=$LC_CTYPE
    LC_CTYPE_OVERRIDE='export LC_CTYPE="C"'
    LC_CTYPE_RESTORE="export LC_CTYPE=$LC_CTYPE_BAK"
fi

cd $build

what=qt
qtversion=4.8.5
pkgbase=qt-everywhere-opensource-src-$qtversion
dlurl=http://download.qt-project.org/official_releases/qt/4.8/$qtversion/$pkgbase.tar.gz
zip=$tarballs/$pkgbase.tar.gz

if [ ! -d $prefix/$what ]; then
    if [ ! -f $zip ]; then
        echoInfo "Fetching qt, this could take a while..."
        curl -L -o $zip $dlurl
    fi

    if [ ! -d $build/$pkgbase ]; then
        tar xzf $zip
    fi

    cd $pkgbase
    echoInfo "Building $what, version number $qtversion"
    ./configure -arch x86_64 -cocoa -debug-and-release -opensource -prefix $prefix/$what -no-qt3support -no-opengl -no-openvg -no-dbus -no-phonon -no-phonon-backend -no-multimedia -no-audio-backend -no-declarative -no-xmlpatterns -nomake examples -nomake demos -qt-zlib  -qt-libpng -qt-libmng -qt-libjpeg -qt-libtiff
    make -j$NPROCS
    make install

    if [[ $OSX_VERSION == 10.8.* ]]; then
        if [ -f $prefix/$what/mkspecs/common/g++-macx.conf ]; then
            cp $prefix/$what/mkspecs/common/g++-macx.conf $prefix/$what/mkspecs/common/g++-macx.conf.bak
            sed -e "s/-mmacosx-version-min=10.5/-mmacosx-version-min=10.6/g" < $prefix/$what/mkspecs/common/g++-macx.conf > $prefix/$what/mkspecs/common/g++-macx.conf.MODIFIED
            mv $prefix/$what/mkspecs/common/g++-macx.conf.MODIFIED $prefix/$what/mkspecs/common/g++-macx.conf
        fi
    fi
    cp LICENSE.LGPL $prefix/$what
else
    echoInfo "$what is done"
fi

cd $build

what=qjson
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    if [ -d $build/$what ]; then
        cd $what
        echoInfo "Fetching latest code for $what, this could take a while..."
        git pull
        rm -r CMakeCache.txt
    else
        echoInfo "Cloning repository of $what, this could take a while..."
        git clone https://github.com/flavio/qjson
        cd $what
    fi

    echoInfo "Building $what, this could take a while... "
    cmake . -DCMAKE_INSTALL_PREFIX=$prefix/$what -DCMAKE_BUILD_TYPE=RELWITHDEBINFO -DQJSON_BUILD_TESTS=no
    make -j$NPROCS
    make install

    cd $prefix/$what/lib
    install_name_tool -id $PWD/libqjson.dylib libqjson.dylib
    touch $tags/$what-done
fi

cd $build

if [ $USE_BOOST == "ON" ]; then
    what=boost
    urlbase=http://downloads.sourceforge.net/project/boost/boost/1.49.0
    pkgbase=boost_1_49_0
    dlurl=$urlbase/$pkgbase.tar.gz    
    if test -f $tags/$what-done; then
        echoInfo "$what is done"
    else
        rm -rf $pkgbase
        zip=$tarballs/$pkgbase.tar.gz
        test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
        tar xzf $zip

        cd $pkgbase
        echoInfo "Building $what"
        ./bootstrap.sh --prefix=$prefix/$what
        ./bjam toolset=darwin link=static threading=multi --with-thread --with-regex --with-system --with-date_time install
        cp LICENSE_1_0.txt $prefix/$what
        touch $tags/$what-done
    fi
else
    echoInfo "'--use-boost' or '-ub' was not specified. Skipping boost."
fi

cd $build
what=bullet
urlbase=http://bullet.googlecode.com/files
unzipped=bullet-2.81-rev2613
pkgbase=bullet-2.81-rev2613
dlurl=$urlbase/$pkgbase.tgz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $unzipped
    zip=$tarballs/$pkgbase.tgz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $unzipped
    echoInfo "Building $what:"
    cmake . -DCMAKE_INSTALL_PREFIX=$prefix/$what -DCMAKE_DEBUG_POSTFIX= -DCMAKE_MINSIZEREL_POSTFIX= -DCMAKE_RELWITHDEBINFO_POSTFIX=
    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$what
    touch $tags/$what-done
fi

cd $build
what=ogg
urlbase=http://downloads.xiph.org/releases/ogg
pkgbase=libogg-1.3.0
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase

    # Fix a bug in libogg with Xcode 4.5.2 and GCC 4.2.1 that causes the overly aggressive -O4 compilation flag
    # that is used to fail build with a "configure: error: No 16 bit type found on this platform!"
    # config.log will show an error "ld: lto: could not merge in /var/folders/sc/grqjwkcn75qf1z0fjz5b9mnm0000gn/T//ccYcUZbs.o because Unknown instruction for architecture x86_64"
    echoInfo "Workaround an ogg configure bug: replacing -O4 with -O2"
    sed -e "s/-O4/-O2/" < configure > configure_replaced
    mv configure_replaced configure
    chmod +x configure

    echoInfo "Building $what:"
    if [ $NO_BOOST == "ON" ]; then
        ./configure --prefix=$prefix/$what CC="/usr/bin/clang -stdlib=libc++" CXX="/usr/bin/clang -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what
    fi

    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$what
    touch $tags/$what-done
fi

cd $build
what=vorbis
urlbase=http://downloads.xiph.org/releases/vorbis
pkgbase=libvorbis-1.3.2
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
    if [ $NO_BOOST == "ON" ]; then
        ./configure --prefix=$prefix/$what --with-ogg=$prefix/ogg --build=x86_64 CC="/usr/bin/clang -stdlib=libc++" CXX="/usr/bin/clang -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what --with-ogg=$prefix/ogg --build=x86_64
    fi

    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$what
    touch $tags/$what-done
fi

cd $build
what=theora
urlbase=http://downloads.xiph.org/releases/theora
pkgbase=libtheora-1.1.1
dlurl=$urlbase/$pkgbase.tar.bz2
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.bz2
    tarball=$tarballs/$pkgbase.tar
    test -f $tarball || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl && bzip2 -d $zip
    tar xzf $tarballs/$pkgbase.tar

    echoInfo "Building $what:"
    cd $pkgbase

    if [ $NO_BOOST == "ON" ]; then
        ./configure --prefix=$prefix/$what --with-ogg=$prefix/ogg --with-vorbis=$prefix/vorbis CC="/usr/bin/clang -stdlib=libc++" CXX="/usr/bin/clang -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what --with-ogg=$prefix/ogg --with-vorbis=$prefix/vorbis
    fi
    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$whatw
    touch $tags/$what-done
fi

cd $build
what=qtpropertybrowser
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf qt-solutions
    echoInfo "Fetching $what, this may take a while... " && git clone git://gitorious.org/qt-solutions/qt-solutions.git
    cd qt-solutions/$what
    echo "CONFIG += release" >> qtpropertybrowser.pro
    echo "CONFIG -= debug" >> qtpropertybrowser.pro

    echoInfo "Building $what"
    ./configure -library
    qmake
    make VERBOSE=1 -j$NPROCS

    mkdir -p $prefix/$what/{lib,include}
    cp ./lib/* $prefix/$what/lib
    cp ./src/*.h $prefix/$what/include
    cp ./src/Qt* $prefix/$what/include
    touch $tags/$what-done
fi

cd $build
what=protobuf
urlbase=http://protobuf.googlecode.com/files
pkgbase=protobuf-2.4.1
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
    if [ $NO_BOOST == "ON" ]; then
        patch src/google/protobuf/message.cc < $viewer/tools/OSX/Mods/protobuf-message.cc.patch
        ./configure --prefix=$prefix/$what CC="/usr/bin/clang++ -stdlib=libc++" CXX="/usr/bin/clang++ -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what
    fi

    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING.txt $prefix/$what
    touch $tags/$what-done
fi

cd $build
what=celt
urlbase=http://downloads.xiph.org/releases/celt
pkgbase=celt-0.11.1
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
    if [ $NO_BOOST == "ON" ]; then
        ./configure --prefix=$prefix/$what CC="/usr/bin/clang -stdlib=libc++" CXX="/usr/bin/clang -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what
    fi
    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$what
    touch $tags/$what-done
fi

cd $build
what=speex
urlbase=http://downloads.xiph.org/releases/speex
pkgbase=speex-1.2rc1
dlurl=$urlbase/$pkgbase.tar.gz
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
    if [ $NO_BOOST == "ON" ]; then
        ./configure --prefix=$prefix/$what --enable-shared=NO CC="/usr/bin/clang -stdlib=libc++" CXX="/usr/bin/clang -stdlib=libc++"
    else
        ./configure --prefix=$prefix/$what --enable-shared=NO
    fi

    make VERBOSE=1 -j$NPROCS
    make install
    cp COPYING $prefix/$what
    touch $tags/$what-done
fi

what=qxmpp
urlbase=http://qxmpp.googlecode.com/svn/trunk@r1671
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    cd $build
    rm -rf $what
    echoInfo "Fetching $what, this may take a while... "
    svn checkout $urlbase $what
    cd $what
    echo "DEFINES += QXMPP_USE_SPEEX" >> src/src.pro
    echo "INCLUDEPATH += $prefix/speex/include" >> src/src.pro
    echo "LIBS += -L$prefix/speex/lib -lspeex" >> src/src.pro
    echo "CONFIG += debug_and_release" >> src/src.pro

    echoInfo "Building $what:"
    qmake
    make sub-src-all-ordered -j$NPROCS
    mkdir -p $prefix/$what/include/qxmpp $prefix/$what/lib
    cp src/*.h $prefix/$what/include/qxmpp
    cp lib/lib$what*.a $prefix/$what/lib
    touch $tags/$what-done
fi

what=qtscriptgenerator
if test -f $tags/$what-done; then 
   echoInfo "$what is done"
else
    cd $build
    rm -rf $what
    echoInfo "Fetching $what, this may take a while... "
    git clone git://gitorious.org/qt-labs/$what.git
    cd $what

    echoInfo "Building generator:"
    cd generator
    qmake
    make all -j$NPROCS
    ./generator --include-paths=$QTDIR/include/
    cd ..

    echoInfo "Building Qt bindings:"
    cd qtbindings
    sed -e "s/qtscript_phonon //" -e "s/qtscript_xmlpatterns //" -e "s/qtscript_opengl //" -e "s/qtscript_uitools //" < qtbindings.pro > x
    mv x qtbindings.pro  
    qmake
    make all -j$NPROCS
    cd ..
    cd ..
    mkdir -p $viewer/bin/qtplugins $prefix/$what
    cp -Rf $build/$what/plugins/script $viewer/bin/qtplugins
    cp $build/$what/LICENSE.LGPL $prefix/$what
    touch $tags/$what-done
fi

what=kNet
if test -f $tags/$what-done; then
   echoInfo "$what is done"
else
    cd $build
    rm -rf kNet
    echoInfo "Cloning $what repository, this may take a while... "
    git clone https://github.com/juj/kNet
    cd kNet

    if [ $NO_BOOST == "ON" ]; then
        $LC_CTYPE_OVERRIDE
        sed -e "s/kNet STATIC/kNet SHARED/" -e "s/# Always compile with maximum warning level/set(CMAKE_CXX_FLAGS -stdlib=libc++)/" < CMakeLists.txt > x
        $LC_CTYPE_RESTORE
        mv x CMakeLists.txt
    else
        $LC_CTYPE_OVERRIDE
        sed -e "s/kNet STATIC/kNet SHARED/" < CMakeLists.txt > x
        $LC_CTYPE_RESTORE
        mv x CMakeLists.txt
    fi

    echoInfo "Building $what:"
    cmake . -DUSE_BOOST:BOOL=FALSE -DUSE_TINYXML:BOOL=FALSE
    make VERBOSE=1

    mkdir -p $prefix/$what/{lib,include}
    cp lib/libkNet.dylib $prefix/$what/lib/
    rsync -r include/* $prefix/$what/include/
    cp LICENSE.txt $prefix/$what
    touch $tags/$what-done
fi

what=NVIDIA_Cg
baseurl=http://developer.download.nvidia.com/cg/Cg_3.1
dmgname=Cg-3.1_April2012.dmg
if test -d $frameworkpath/Cg.framework; then
    echoInfo "$what is done"
else
    dmg=$tarballs/$dmgname
    tarballname=$what.tgz
    rm -f $dmg
    rm -f $tarballs/$tarballname
    rm -rf $build/$what

    echoInfo "Fetching $what, this may take a while... "
    curl -L -o $dmg $baseurl/$dmgname
    hdiutil attach $dmg

    mountpoint=/Volumes/Cg-3.1.0013
    tarball="$mountpoint/Cg-3.1.0013.app/Contents/Resources/Installer\ Items/$tarballname"

    eval cp $tarball $tarballs
    cd $build
    hdiutil detach $mountpoint
    mkdir $what
    cd $what
    tar --gzip --extract --verbose -f $tarballs/$tarballname

    cd $build
    chmod -R +w $what
    echoInfo "Installing $what into $frameworkpath:"
    mv $what/Library/Frameworks/Cg.framework $frameworkpath
fi

what=ogre-safe-nocrashes
baseurl=https://bitbucket.org/clb

if test -d $prefix/$what/lib/Ogre.framework; then
    echoInfo "$what is done"
    if [ ! -d "$OGRE_HOME" ]; then      # If OGRE_HOME points to invalid location, force it to deps/build/ogre-safe-nocrashes
        export OGRE_HOME=$build/$what # If Ogre is built, then Hydrax and SkyX might be not and OGRE_HOME is needed still
        echoInfo "Setting up OGRE_HOME='$OGRE_HOME'"
    fi
else
    cd $build

    if ! test -d $what; then    
        echoInfo "Cloning $what repository, this may take a while..."
        hg clone $baseurl/$what
        cd $what
        hg checkout v1-9
    else
        cd $what
    fi

    if ! test -d Dependencies; then
        if test -d DependenciesBuild; then
            rm -rf DependenciesBuild
        fi
        hg clone -r 62 https://bitbucket.org/cabalistic/ogredeps DependenciesBuild
        cd DependenciesBuild
        $LC_CTYPE_OVERRIDE
        if [ $NO_BOOST == "ON" ]; then 
            sed -e "s/set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)/set(CMAKE_CXX_FLAGS -stdlib=libc++)/" < CMakeLists.txt > x
        else
            sed -e "s/set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)/ /" < CMakeLists.txt > x
        fi
        $LC_CTYPE_RESTORE
        mv x CMakeLists.txt
        cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../Dependencies" .
        make
        make install
        cd $build/$what
    fi    

    export OGRE_HOME=$build/$what
    echoInfo "Building $what:"
    if test -f CMakeCache.txt; then
        rm CMakeCache.txt
    fi
 
    cmake -G Xcode -DCMAKE_FRAMEWORK_PATH=$frameworkpath -DOGRE_USE_BOOST:BOOL=$USE_BOOST -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF -DOGRE_CONFIG_THREADS:INT=0 -DOGRE_CONFIG_THREAD_PROVIDER=none -DOGRE_CONFIG_ENABLE_LIBCPP_SUPPORT:BOOL=$NO_BOOST -DCMAKE_OSX_ARCHITECTURES=x86_64 -DOGRE_LIBRARY_OUTPUT=$OGRE_HOME/lib -DOGRE_ARCHIVE_OUTPUT=$OGRE_HOME/lib
    xcodebuild -configuration RelWithDebInfo

    mkdir -p $prefix/$what/{lib,include}
    cp -R $OGRE_HOME/lib/relwithdebinfo/* $prefix/$what/lib
    # cp $prefix/$what/lib/*.dylib $viewer/bin
    cp COPYING $prefix/$what
    # Replace the install name with more suitable one to avoid link problems
    if [ -f $viewer/bin/RenderSystem_GL.dylib ]; then
        install_name_tool -id $viewer/bin/RenderSystem_GL.dylib $viewer/bin/RenderSystem_GL.dylib
    fi

    export PKG_CONFIG_PATH=$build/$what/pkgconfig
fi

# Explicitly specify where the tundra deps boost resides, to allow cmake FindBoost pick it up.
if [ "$USE_BOOST" == "ON" ]; then
    export BOOST_ROOT=$prefix/boost/include
    export BOOST_INCLUDEDIR=$prefix/boost/include/boost
    export BOOST_LIBRARYDIR=$prefix/boost/lib
fi

what=assimp
baseurl=https://github.com/assimp/assimp.git
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    cd $build
    rm -rf $what
    echoInfo "Cloning $what repository, this may take a while..."
    git clone https://github.com/assimp/assimp.git $what
    cd $what
    git checkout e22bb03f807b345a9058352e5453b6491a235677
    
    # Patch assimp 
    patch -p0 -i $patches/assimp.patch
    if [ "$USE_BOOST" == "ON" ]; then
        $LC_CTYPE_OVERRIDE
        sed -e "s/-stdlib=libc++/ /" < CMakeLists.txt > x
        $LC_CTYPE_RESTORE
        mv x CMakeLists.txt
    fi       

    cmake . -DCMAKE_INSTALL_PREFIX=$prefix/$what
    make -j$NPROCS
    make install

    cp LICENSE $prefix/$what
    touch $tags/$what-done
fi

# HydraX, SkyX and PythonQT are build from the realxtend own dependencies.
# At least for the time being, until changes to those components flow into
# upstream..

cd $build
depdir=realxtend-tundra-deps
if [ ! -e $depdir ]
then
    echoInfo "Cloning source of HydraX/SkyX/PythonQT/NullRenderer..."
    git init $depdir
    cd $depdir
    git fetch https://github.com/realXtend/realxtend-tundra-deps.git sources:refs/remotes/origin/sources
    git remote add origin https://github.com/realXtend/realxtend-tundra-deps.git
    git checkout sources
else
    cd $depdir
    git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    if [ -z "`git merge sources origin/sources|grep "Already"`" ]; then
        echoInfo "Changes in GIT detected, rebuilding HydraX and SkyX"
        rm -f $tags/hydrax-done $tags/skyx-done $tags/pythonqt-done
    else
        echoInfo "No changes in realxtend deps git."
    fi
fi

echoInfo "Building SkyX and Hydrax with OGRE_HOME='$OGRE_HOME'"

what=hydrax
# HydraX build:
if test -f $tags/$what-done; then
    echoInfo "Hydrax is done"
else
    echoInfo "Building Hydrax:"
    cd $build/$depdir/hydrax

    cmake . -DCMAKE_INSTALL_PREFIX=$prefix/$what -DUSE_BOOST:BOOL=$USE_BOOST
    make -j$NPROCS
    make install

    cp License.txt $prefix/$what
    touch $tags/$what-done
fi

# SkyX build:
what=skyx
if test -f $tags/$what-done; then
    echoInfo "SkyX is done"
else
    echoInfo "Building SkyX:"
    cd $build/$depdir/skyx
    if test -f CMakeCache.txt; then
        rm CMakeCache.txt
    fi
    cmake . -DSKYX_DEPENDENCIES_DIR=$OGRE_HOME/Dependencies -DUSE_BOOST:BOOL=$USE_BOOST -DCMAKE_FRAMEWORK_PATH=$frameworkpath -DCMAKE_INSTALL_PREFIX=$prefix/$what
    make -j$NPROCS
    make install

    # Replace the install name with more suitable one to avoid link problems
    # TODO: Fix this in realxtend-tundra-deps:sources repo!
    if [ -f $prefix/$what/lib/libSkyX.0.dylib ]; then
        install_name_tool -id $prefix/$what/lib/libSkyX.0.dylib $prefix/$what/lib/libSkyX.0.dylib
    fi

    cp License.txt $prefix/$what
    touch $tags/$what-done
fi

what=vlc
VLCVERSION=1.1.12
baseurl=http://download.videolan.org/pub/videolan/vlc/$VLCVERSION/macosx
dmgname=vlc-$VLCVERSION-intel64.dmg
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    dmg=$tarballs/$dmgname
    tarballname=$what.tgz
    rm -f $dmg
    rm -rf $prefix/$what

    echoInfo "Fetching $what, this may take a while... "
    curl -L -o $dmg $baseurl/$dmgname
    hdiutil attach $dmg

    mountpoint=/Volumes/vlc-$VLCVERSION
    cd $mountpoint/VLC.app/Contents/MacOS
    echoInfo "Installing $what into $prefix/$what:"

    mkdir -p $prefix/$what
    cp -R * $prefix/$what
    rm $prefix/$what/VLC

    touch $tags/$what-done
    hdiutil detach $mountpoint
fi

cd $build

if [ $NO_BOOST == "ON" ]; then
    ZZIPLIBPREFIX=$OGRE_HOME/Dependencies
else
    ZZIPLIBPREFIX=$prefix/$what
fi

what=zziplib
pkgbase=zziplib-0.13.59
dlurl=http://sourceforge.net/projects/zziplib/files/zziplib13/0.13.59/$pkgbase.tar.bz2/download
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.bz2
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"

    ./configure --prefix=$ZZIPLIBPREFIX --enable-shared=NO --enable-static=YES
    make VERBOSE=1 -j$NPROCS
    make install
    cp ./docs/COPYING.ZLIB $prefix/$what
    touch $tags/$what-done
fi

export ZZIPLIB_ROOT=$ZZIPLIBPREFIX

what=websocketpp
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    cd $build
    rm -rf $what
    echoInfo "Fetching $what, this may take a while... "
    git clone https://github.com/realXtend/websocketpp.git $what
    cd $what
    mkdir -p $prefix/include/$what
    rsync -r $what/* $prefix/include/$what
    touch $tags/$what-done
fi

cd $build


# All deps are now fetched and built. Do the actual Tundra build.

echoInfo "Using Mac OS X $OSX_VERSION, Xcode version $XCODE_VERSION, build configuration: $BUILD_CONFIGURATION"

XCODE_SUFFIX=
if [ "$MAKE_XCODE" == "1" ]; then
    XCODE_SUFFIX="-G Xcode"
fi

cd $viewer
if [ "$RUN_CMAKE" == "1" ]; then
    TUNDRA_DEP_PATH=$prefix cmake . $XCODE_SUFFIX -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION -DTUNDRA_NO_BOOST:BOOL=$NO_BOOST -DTUNDRA_CPP11_ENABLED:BOOL=$NO_BOOST -DTUNDRA_VERSION_POSTFIX="$TUNDRA_VERSION_POSTFIX"
fi

if [ "$RUN_MAKE" == "1" ]; then
    if [ "$MAKE_XCODE" == "1" ]; then
        xcodebuild -configuration $BUILD_CONFIGURATION VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES
    else
        make -j$NPROCS VERBOSE=1
    fi
fi
