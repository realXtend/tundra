#!/bin/bash
# script to build naali and most deps.

set -e
set -x

# script to build naali and most deps.
#
# note: you need to enable the universe and multiverse software sources
# as this script attempts to get part of the deps using apt-get

viewer=$(dirname $(readlink -f $0))/..
deps=$viewer/../naali-deps
mkdir -p $deps
deps=$(cd $deps && pwd)
viewer=$(cd $viewer && pwd)

prefix=$deps/install
build=$deps/build
tarballs=$deps/tarballs
tags=$deps/tags



# -j<n> param for make, for how many processes to run concurrently

nprocs=`grep -c "^processor" /proc/cpuinfo`

mkdir -p $tarballs $build $prefix/{lib,share,etc,include} $tags

export OGRE_HOME=$prefix
export PATH=$prefix/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export NAALI_DEP_PATH=$prefix
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include
export CC="ccache gcc"
export CXX="ccache g++"
export CCACHE_DIR=$deps/ccache
export TUNDRA_PYTHON_ENABLED=TRUE

if lsb_release -c | egrep -q "lucid|maverick|natty|oneiric|precise|maya|lisa|katya|julia|isadora|quantal|nadia" && tty >/dev/null; then
        which aptitude > /dev/null 2>&1 || sudo apt-get install aptitude
	sudo aptitude -y install git-core python-dev libogg-dev libvorbis-dev \
	 build-essential g++ libboost-all-dev libois-dev \
	 ccache libqt4-dev python-dev freeglut3-dev \
	 libxml2-dev cmake libalut-dev libtheora-dev ed \
	 liboil0.3-dev mercurial unzip xsltproc libois-dev libxrandr-dev \
	 libspeex-dev nvidia-cg-toolkit subversion \
	 libfreetype6-dev libfreeimage-dev libzzip-dev \
	 libxaw7-dev libgl1-mesa-dev libglu1-mesa-dev \
	 libvlc-dev libspeexdsp-dev libprotobuf-dev \
	 libprotobuf-c0 libprotobuf-c0-dev \
	 protobuf-c-compiler protobuf-compiler \
     libqt4-opengl-dev libqtwebkit-dev \
     libspeexdsp-dev libprotobuf-dev \
     libvlc-dev
fi

what=bullet-2.79-rev2440
whatdir=bullet-2.79
if test -f $tags/$what-done; then
    echo $what is done
else
    cd $build
    rm -rf $whatdir
    test -f $tarballs/$what.tgz || wget -P $tarballs http://bullet.googlecode.com/files/$what.tgz
    tar zxf $tarballs/$what.tgz
    cd $whatdir
    cmake -DCMAKE_INSTALL_PREFIX=$prefix -DBUILD_DEMOS=OFF -DBUILD_{NVIDIA,AMD,MINICL}_OPENCL_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DINSTALL_EXTRA_LIBS=ON -DCMAKE_CXX_FLAGS_RELEASE="-O2 -g -fPIC" .
    make -j $nprocs
    make install
    touch $tags/$what-done
fi

what=celt
if test -f $tags/$what-done; then
    echo $what id done
else
    urlbase=http://downloads.xiph.org/releases/celt
    pkgbase=celt-0.11.1
    dlurl=$urlbase/$pkgbase.tar.gz
    cd $build
    rm -rf $pkgbase
    zip=$tarballs/$pkgbase.tar.gz
    test -f $zip || wget -P $tarballs $dlurl
    tar xzf $zip
    cd $pkgbase
    ./configure --prefix=$prefix
    make VERBOSE=1 -j$NPROCS
    make install
    touch $tags/$what-done
fi

what=qtscriptgenerator
if test -f $tags/$what-done; then 
   echo $what is done
else
    cd $build
    rm -rf $what
    git clone https://git.gitorious.org/qt-labs/$what.git
    cd $what
    patch -l -p1 <<EOF
Description: Include QtWebkit and Phonon unconditionally.
 This is necessary as both aren't built by the Qt source.
Author: Felix Geyer <debfx-pkg@fobos.de>
Acked-By: Modestas Vainius <modax@debian.org>
Last-Update: 2011-03-20

--- a/generator/qtscript_masterinclude.h
+++ b/generator/qtscript_masterinclude.h
@@ -53,13 +53,9 @@
 #  include <QtXmlPatterns/QtXmlPatterns>
 #endif
 
-#ifndef QT_NO_WEBKIT
 #  include <QtWebKit/QtWebKit>
-#endif
 
-#ifndef QT_NO_PHONON
 #  include <phonon/phonon>
-#endif
 
 #include "../qtbindings/qtscript_core/qtscriptconcurrent.h"
 
EOF
    cd generator
    qmake
    make -j $nprocs
    ./generator --include-paths=`qmake -query QT_INSTALL_HEADERS`
    cd ..

    cd qtbindings
    sed -i 's/qtscript_phonon //' qtbindings.pro
    qmake
    make -j $nprocs
    cd ..
    cd ..
    touch $tags/$what-done
fi
mkdir -p $viewer/bin/qtplugins/script
cp -lf $build/$what/plugins/script/* $viewer/bin/qtplugins/script/

what=assimp--3.0.1270-source-only
if test -f $tags/$what-done; then
	echo $what is done
else
	cd $build
	rm -rf $what
    test -f $tarballs/$what.zip || wget -P $tarballs http://downloads.sourceforge.net/project/assimp/assimp-3.0/assimp--3.0.1270-source-only.zip
    unzip $tarballs/$what.zip
	#git clone git://github.com/assimp/assimp.git $what
	cd $what
	#sed -e "s/string_type::size_type/typename string_type::size_type/" < code/ObjTools.h > x
	#mv x code/ObjTools.h
	cmake -DCMAKE_INSTALL_PREFIX=$prefix .
	make -j $nprocs
	make install
	touch $tags/$what-done
fi

what=kNet
if test -f $tags/$what-done; then 
   echo $what is done
else
    cd $build
    rm -rf kNet
    git clone https://github.com/juj/kNet
    cd kNet
    git checkout stable
    sed -e "s/USE_TINYXML TRUE/USE_TINYXML FALSE/" -e "s/kNet STATIC/kNet SHARED/" < CMakeLists.txt > x
    mv x CMakeLists.txt
    cmake . -DCMAKE_BUILD_TYPE=Debug
    make -j $nprocs
    cp lib/libkNet.so $prefix/lib/
    rsync -r include/* $prefix/include/
    touch $tags/$what-done
fi

what=ogre-safe-nocrashes
if test -f $tags/$what-done; then 
   echo "Testing whether there are new changes in $what"
   cd $build/$what
   res=`hg pull -u|grep "no changes found"`
   if [ -z "$res" ]; then
       echo "Changes detected in $what. Removing $what-done tag and forcing a rebuild."
       rm -f $tags/$what-done
   fi
fi
if test -f $tags/$what-done; then
   echo $what is done
else
    cd $build
    if [ ! -e "$what" ]; then
        echo "$what does not exist. Cloning a new copy..."
        hg clone https://bitbucket.org/clb/ogre-safe-nocrashes
    fi


    if lsb_release -c | egrep -q "lucid|maverick|natty|oneiric|precise|quantal" && tty >/dev/null; then
    sudo apt-get build-dep libogre-dev
    fi
    cd $what
    hg checkout v1-8 # Make sure we are in the right branch
    # Fix linking with recent boost libs
    sed -i -s 's/OGRE_BOOST_COMPONENTS thread/OGRE_BOOST_COMPONENTS system thread/' CMake/Dependencies.cmake
    mkdir -p $what-build
    cd $what-build  
    cmake .. -DCMAKE_INSTALL_PREFIX=$prefix -DOGRE_BUILD_PLUGIN_BSP:BOOL=OFF -DOGRE_BUILD_PLUGIN_PCZ:BOOL=OFF -DOGRE_BUILD_SAMPLES:BOOL=OFF
    make -j $nprocs VERBOSE=1
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
    echo "Cloning source of HydraX/SkyX/PythonQT/NullRenderer..."
    git init $depdir
    cd $depdir
    git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    git remote add origin https://code.google.com/p/realxtend-tundra-deps/
    git checkout sources
else
    cd $depdir
    git fetch https://code.google.com/p/realxtend-tundra-deps/ sources:refs/remotes/origin/sources
    if [ -z "`git merge sources origin/sources|grep "Already"`" ]; then
        echo "Changes in GIT detected, rebuilding HydraX, SkyX and PythonQT"
        rm -f $tags/hydrax-done $tags/skyx-done $tags/pythonqt-done
    else
        echo "No changes in realxtend deps git."
    fi
fi

# HydraX build:
if test -f $tags/hydrax-done; then
    echo "Hydrax-done"
else
    cd $build/$depdir/hydrax
    sed -i 's!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(shell pkg-config OGRE --cflags)!' makefile
    sed -i 's!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(shell pkg-config OGRE --libs)!' makefile
    make -j $nprocs PREFIX=$prefix
    make PREFIX=$prefix install
    touch $tags/hydrax-done
fi
# SkyX build

if test -f $tags/skyx-done; then
   echo "SkyX-done"
else
   cd $build/$depdir/skyx
   my_ogre_home=$OGRE_HOME
   if [ -z "$my_ogre_home" ]; then
	    my_ogre_home=`pkg-config --variable=prefix OGRE`
       if [ -z "$my_ogre_home" ]; then
           echo "OGRE_HOME not defined, check your pkg-config or set OGRE_HOME manually.";
           exit 0;
       fi
   fi
   echo "Using OGRE_HOME = $OGRE_HOME"
   SKYX_SOURCE_DIR=`pwd`
   env OGRE_HOME=$my_ogre_home/lib/OGRE cmake -DCMAKE_INSTALL_PREFIX=$prefix .
   make -j $nprocs install
   touch $tags/skyx-done
fi

# PythonQT build
if test -f $tags/pythonqt-done; then
    echo "PythonQt-done"
else
    cd $build/$depdir/PythonQt
    pyver=$(python -c 'import sys; print sys.version[:3]')
    sed -i "s/PYTHON_VERSION=.*/PYTHON_VERSION=$pyver/" build/python.prf
    fn=generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
    sed 's/CocoaRequestModal = QEvent::CocoaRequestModal,//' < $fn > x
    mv x $fn
    qmake
    if ! make -j $nprocs; then
    # work around PythonQt vs Qt 4.8 incompatibility
	cd src
	rm -f moc_PythonQtStdDecorators.cpp
	make moc_PythonQtStdDecorators.cpp
	sed -i -e 's/void PythonQtStdDecorators::qt_static_metacall/#undef emit\nvoid PythonQtStdDecorators::qt_static_metacall/'  moc_PythonQtStdDecorators.cpp
	cd ..
	make -j $nprocs
    fi
    rm -f $prefix/lib/libPythonQt*
    cp -a lib/libPythonQt* $prefix/lib/
    cp src/PythonQt*.h $prefix/include/
    cp extensions/PythonQt_QtAll/PythonQt*.h $prefix/include/
    touch $tags/pythonqt-done
fi

cd $build
what=qtpropertybrowser
if test -f $tags/$what-done; then
    echo $what is done
else
    pkgbase=${what}-2.5_1-opensource
    rm -rf $pkgbase
    zip=../tarballs/$pkgbase.tar.gz
    test -f $zip || wget -O $zip http://ftp.heanet.ie/mirrors/ftp.trolltech.com/pub/qt/solutions/lgpl/$pkgbase.tar.gz
    tar zxf $zip
    cd $pkgbase
    echo yes | ./configure -library
    qmake
    make -j$nprocs
    cp lib/lib* $prefix/lib/
    # luckily only extensionless headers under src match Qt*:
    cp src/qt*.h src/Qt* $prefix/include/
    touch $tags/$what-done
fi

cd $build
what=qxmpp
rev=r1671
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $what
    svn checkout http://qxmpp.googlecode.com/svn/trunk@$rev $what
    cd $what
    sed 's/# DEFINES += QXMPP_USE_SPEEX/DEFINES += QXMPP_USE_SPEEX/g' src/src.pro > src/temp
    sed 's/# LIBS += -lspeex/LIBS += -lspeex/g' src/temp > src/src.pro
    sed 's/LIBS += $$QXMPP_LIBS/LIBS += $$QXMPP_LIBS -lspeex/g' tests/tests.pro > tests/temp && mv tests/temp tests/tests.pro
    rm src/temp
    qmake
    make -j $nprocs
    mkdir -p $prefix/include/$what
    cp src/*.h $prefix/include/$what
    cp lib/libqxmpp.a $prefix/lib/
    touch $tags/$what-done
fi

if test "$1" = "--depsonly"; then
    exit 0
fi

cd $viewer
cat > ccache-g++-wrapper <<EOF
#!/bin/sh
exec ccache g++ -O -g \$@
EOF
chmod +x ccache-g++-wrapper
TUNDRA_DEP_PATH=$prefix cmake -DCMAKE_CXX_COMPILER="$viewer/ccache-g++-wrapper" . -DCMAKE_MODULE_PATH=$prefix/lib/SKYX/cmake
make -j $nprocs VERBOSE=1
