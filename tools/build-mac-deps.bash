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
    echo " -o <PATH> | --ogre-path <PATH>       Specifies the path where a custom Ogre root directory is located. If this is "
    echo "                                      not specified, the default-installed Ogre framework will be used.            "
    echo " "
    echo " -rwdi | --release-with-debug-info    Enables debugging information to be included in compile-time"
    echo " "
    echo " -x | --xcode                         Make Xcode project instead of Makefiles                                      "
    echo " "
    echo " -nc | --no-run-cmake                 Do not run 'cmake .' after the dependencies are built. (The default is that  "
    echo "                                      'cmake .' is executed to generate Makefiles after all dependencies have been "
    echo "                                      built)."
    echo " "
    echo " -nm | --no-run-make                  Do not run 'make' after the dependencies are built. If --xcode is specified  "
    echo "                                      this option has no effect (The default is that 'make' is executed to start   "
    echo "                                      the compile process).                                                        "
    echo " "
    echo " -np | --number-of-processes <NUMBER> The number of processes to be run simultaneously, recommended for multi-core "
    echo "                                      or processors with hyper-threading technology for a faster compile process."
    echo "                                      The default value is set according to the reports by the operating system."
    echo " "
    echo " NOTE: The properties that are in middle brackets are optional if otherwise not specified."
    echo " "
    exit 1
}

echoInfo()
{
    COLOR="\033[32;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR INFO: $@ $ENDCOLOR | $logAndPrintThis
}

echoWarn()
{
    COLOR="\033[33;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR WARNING: $@ $ENDCOLOR | $logAndPrintThis
}

echoError()
{
    COLOR="\033[31;1m"
    ENDCOLOR="\033[0m"
    echo -e $COLOR ERROR: $@ $ENDCOLOR | $logAndPrintThis
}

logFile=build-mac.log
logAndPrintThis="tee -a $logFile"
echo "" | tee $logFile

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
echo "= Qt 4.8.0  http://qt.nokia.com/downloads/sdk-mac-os-cpp                                                    ="
echo "= MacPorts  http://www.macports.org/install.php                                                             ="
echo "= XQuartz   for Mac OS X 10.6.x-10.7.x:    NOT required                                                     ="
echo "=           for Mac OS X 10.8.x and newer: http://xquartz.macosforge.org/landing/                           ="
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
MAKE_XCODE="0"
NPROCS=`sysctl -n hw.ncpu`
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

        -rwdi | --release-with-debug-info ) RELWITHDEBINFO="1"
                                            ;;

        -d | --deps-path )                  shift
                                            if [ ! -d "$1" ]; then
                                                echoError "Bad directory for --deps-path: $1"
                                                ERRORS_OCCURED="1"
                                                shift
                                                continue
                                            fi
                                            REQUIRED_ARGS_COUNT=$((REQUIRED_ARGS_COUNT+1))
                                            DEPS=$1
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

         -x | --xcode )                     MAKE_XCODE="1"
                                            RUN_MAKE="0"
                                            ;;

        -nc | --no-run-cmake )              RUN_CMAKE="0"
                                            ;;

        -nm | --no-run-make )               RUN_MAKE="0"
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
    cwd=$(pwd)       # Temporarily save this path to the build script.
    viewer=$(pwd)/.. # Assume the build script lies at gittrunk/tools.
    cd $viewer
    viewer=$(pwd)
    cd $cwd        # Go back to not alter cwd.
fi

if [ ! -d "$QTDIR" ]; then
    #TODO This is very very prone to fail on anyone's system. (but at least we will correctly instruct to use --qt-path)
    if [ -d /usr/local/Trolltech/Qt-4.7.1 ]; then
        export QTDIR=/usr/local/Trolltech/Qt-4.7.1
    elif [ -d ~/QtSDK/Desktop/Qt/4.8.0/gcc ]; then
        export QTDIR=~/QtSDK/Desktop/Qt/4.8.0/gcc
    else
       echoError "Cannot find Qt. Please specify Qt directory with the --qt-path parameter."
    fi
fi

prefix=$DEPS
build=$DEPS/build
tarballs=$DEPS/tarballs
tags=$DEPS/tags
frameworkpath=$DEPS/Frameworks

mkdir -p $tarballs $build $prefix/{lib,share,etc,include} $tags $frameworkpath

if [ "$RELWITHDEBINFO" == "1" ]; then
    export CFLAGS="-gdwarf-2 -O2"
    export CXXFLAGS="-gdwarf-2 -O2"
    export CMAKE_C_FLAGS="-gdwarf-2 -O2"
    export CMAKE_CXX_FLAGS="-gdwarf-2 -O2"
else
    export CFLAGS="-O3"
    export CXXFLAGS="-O3"
    export CMAKE_C_FLAGS="-O3"
    export CMAKE_CXX_FLAGS="-O3"
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
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl >> $logFile
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what"
    ./bootstrap.sh --prefix=$prefix
    ./bjam toolset=darwin link=static threading=multi --with-thread --with-regex install
    touch $tags/$what-done
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
    cmake . -DCMAKE_INSTALL_PREFIX=$prefix -DCMAKE_DEBUG_POSTFIX= -DCMAKE_MINSIZEREL_POSTFIX= -DCMAKE_RELWITHDEBINFO_POSTFIX=
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
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
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
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.bz2
    tarball=$tarballs/$pkgbase.tar
    test -f $tarball || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl && bzip2 -d $zip
    tar xzf $tarballs/$pkgbase.tar

    echoInfo "Building $what:"
    cd $pkgbase
    ./configure --prefix=$prefix --with-ogg=$prefix --with-vorbis=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
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
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
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
    echoInfo "$what is done"
else
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || echoInfo "Fetching $what, this may take a while... " && curl -L -o $zip $dlurl
    tar xzf $zip

    cd $pkgbase
    echoInfo "Building $what:"
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
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
    ./configure --prefix=$prefix --enable-shared=NO
    make VERBOSE=1 -j$NPROCS
    make install
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
    echo "INCLUDEPATH += $prefix/include" >> src/src.pro
    echo "LIBS += -L$prefix/lib -lspeex" >> src/src.pro
    echo "CONFIG += debug_and_release" >> src/src.pro

    echoInfo "Building $what:"
    qmake
    make sub-src-all-ordered -j$NPROCS
    mkdir -p $prefix/include/qxmpp
    cp src/*.h $prefix/include/qxmpp
    cp lib/lib$what*.a $prefix/lib
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
    sed -e "s/qtscript_phonon //" < qtbindings.pro > x
    mv x qtbindings.pro  
    qmake
    make all -j$NPROCS
    cd ..
    cd ..
    mkdir -p $viewer/bin/qtplugins/script
    cp -f $build/$what/plugins/script/* $viewer/bin/qtplugins/script/
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
    sed -e "s/USE_TINYXML TRUE/USE_TINYXML FALSE/" -e "s/kNet STATIC/kNet SHARED/" -e "s/USE_BOOST TRUE/USE_BOOST FALSE/" < CMakeLists.txt > x
    mv x CMakeLists.txt
    echoInfo "Building $what:"
    cmake . -DCMAKE_BUILD_TYPE=Debug
    make -j$NPROCS
    cp lib/libkNet.dylib $prefix/lib/
    rsync -r include/* $prefix/include/
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
ogredepszip=OgreDependencies_OSX_20120525.zip
ogredepsurl=http://downloads.sourceforge.net/project/ogre/ogre-dependencies-mac/1.8/

if test -d $frameworkpath/Ogre.framework; then
    echoInfo "$what is done"
    if [ ! -d "$OGRE_HOME" ]; then      # If OGRE_HOME points to invalid location, force it to deps/build/ogre-safe-nocrashes
        export OGRE_HOME=$build/$what # If Ogre is built, then Hydrax and SkyX might be not and OGRE_HOME is needed still
        echoInfo "Setting up OGRE_HOME='$OGRE_HOME'"
    fi
else
    cd $build
    rm -rf $what

    echoInfo "Cloning $what repository, this may take a while..."
    hg clone $baseurl/$what
    cd $what
    hg checkout v1-8
    curl -L -o $ogredepszip $ogredepsurl$ogredepszip
    tar xzf $ogredepszip
    export OGRE_HOME=$build/$what
    echoInfo "Building $what:"
    cmake -G Xcode -DCMAKE_FRAMEWORK_PATH=$frameworkpath -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF -DOGRE_CONFIG_THREADS:INT=0 -DOGRE_CONFIG_THREAD_PROVIDER=none
    xcodebuild -configuration RelWithDebInfo

    cp -R $OGRE_HOME/lib/relwithdebinfo/Ogre.framework $frameworkpath
    cp $OGRE_HOME/lib/relwithdebinfo/*.dylib $viewer/bin
    export PKG_CONFIG_PATH=$build/$what/pkgconfig
fi

echoInfo "Building SkyX and Hydrax with OGRE_HOME='$OGRE_HOME'"

what=assimp
baseurl=https://assimp.svn.sourceforge.net/svnroot/assimp/trunk
if test -f $tags/$what-done; then
    echoInfo "$what is done"
else
    cd $build
    rm -rf $what
    echoInfo "Cloning $what repository, this may take a while..."
    svn checkout -r 1300 https://assimp.svn.sourceforge.net/svnroot/assimp/trunk $what
    cd $what
    # First sed statement: Apple's ld does not allow this version number, so override that
    # Second sed statement: Force add boost include path (the same as Ogre's dependencies include path)
    sed -e 's/(ASSIMP_SV_REVISION 1264)/(ASSIMP_SV_REVISION 1)/' -e 's/INCLUDE_DIRECTORIES( include )/INCLUDE_DIRECTORIES( include )\
    set (BOOST_INCLUDEDIR "${ENV_OGRE_HOME}\/Dependencies\/include")/' < CMakeLists.txt > temp
    mv temp CMakeLists.txt
    cmake . -DCMAKE_INSTALL_PREFIX=$prefix
    make -j4
    make install
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
    git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    git remote add origin https://code.google.com/p/realxtend-tundra-deps/
    git checkout sources
else
    cd $depdir
    git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    if [ -z "`git merge sources origin/sources|grep "Already"`" ]; then
        echoInfo "Changes in GIT detected, rebuilding HydraX, SkyX and PythonQT"
        rm -f $tags/hydrax-done $tags/skyx-done $tags/pythonqt-done
    else
        echoInfo "No changes in realxtend deps git."
    fi
fi

# HydraX build:
if test -f $tags/hydrax-done; then
    echoInfo "Hydrax is done"
else
    echoInfo "Building Hydrax:"
    cd $build/$depdir/hydrax

    OSXMAKE="-f makefile.macosx"
    make $OSXMAKE -j$NPROCS PREFIX=$prefix
    make $OSXMAKE PREFIX=$prefix install
    cp ./lib/Release/* $prefix/lib #for some reason, 'cp' that is invoked in the makefile does not copy the library. 
    touch $tags/hydrax-done
fi

# SkyX build:
if test -f $tags/skyx-done; then
    echoInfo "SkyX is done"
else
    echoInfo "Building SkyX:"
    cd $build/$depdir/skyx
    if test -f CMakeCache.txt; then
        rm CMakeCache.txt
    fi
    cmake . -DSKYX_DEPENDENCIES_DIR=$OGRE_HOME/Dependencies -DCMAKE_FRAMEWORK_PATH=$frameworkpath -DCMAKE_INSTALL_PREFIX=$prefix
    make -j$NPROCS
    make install
    touch $tags/skyx-done
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
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

# All deps are now fetched and built. Do the actual Tundra build.

# Explicitly specify where the tundra deps boost resides, to allow cmake FindBoost pick it up.
export BOOST_ROOT=$DEPS/include
export BOOST_INCLUDEDIR=$DEPS/include/boost
export BOOST_LIBRARYDIR=$DEPS/lib

XCODE_SUFFIX=
if [ "$MAKE_XCODE" == "1" ]; then
    XCODE_SUFFIX="-G Xcode"
fi

cd $viewer
if [ "$RUN_CMAKE" == "1" ]; then
    TUNDRA_DEP_PATH=$prefix cmake . $XCODE_SUFFIX
fi

if [ "$RUN_MAKE" == "1" ]; then
    make -j$NPROCS VERBOSE=1
fi
