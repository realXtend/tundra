#!/bin/bash
set -e
set -x

# script to build naali and most deps.
#
# if you want to use caelum, install ogre and nvidia cg from
# ppa:andrewfenn/ogredev, change the caelum setting to 1 in
# top-level CMakeBuildConfig.txt and enable Cg module in bin/plugins-unix.cfg


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

export PATH=$prefix/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include
export CC="ccache gcc"
export CXX="ccache g++"
export CCACHE_DIR=$deps/ccache

if lsb_release -c | egrep -q "lucid|maverick|natty"; then
        which aptitude > /dev/null 2>&1 || sudo apt-get install aptitude
	sudo aptitude -y install python-dev libogg-dev libvorbis-dev \
	 build-essential g++ libogre-dev libboost-all-dev \
	 ccache libqt4-dev python-dev freeglut3-dev \
	 libxml2-dev cmake libalut-dev libtheora-dev \
	 liboil0.3-dev mercurial unzip xsltproc libqtscript4-qtbindings
fi

what=bullet-2.77
if test -f $tags/$what-done; then
    echo $what is done
else
    cd $build
    rm -rf $what
    test -f $tarballs/$what.tgz || wget -P $tarballs http://bullet.googlecode.com/files/$what.tgz
    tar zxf $tarballs/$what.tgz
    cd $what
    cmake -DCMAKE_INSTALL_PREFIX=$prefix -DBUILD_DEMOS=OFF -DINSTALL_EXTRA_LIBS=ON -DCMAKE_CXX_FLAGS_RELEASE="-O2 -fPIC -DNDEBUG -DBT_NO_PROFILE" .
    make -j $nprocs
    make install
    touch $tags/$what-done
fi


#what=hydrax
#if test -f $tags/$what-done; then
#    echo $what is done
#else
#    cd $build
#    rm -rf $what
#    tarballname=libhydrax_0.5.4-5.tar.gz
#    url=https://launchpad.net/~sonsilentsea-team/+archive/sonsilentsea/+files/$tarballname
#    test -f $tarballs/$tarballname || wget -P $tarballs $url
#    tar zxf $tarballs/$tarballname
#    cd libhydrax-0.5.4
#    sed -i "s!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(pkg-config OGRE --cflags)!" makefile
#    sed -i "s!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(pkg-config OGRE --libs)!" makefile
#    make -j $nprocs PREFIX=$prefix
#    make PREFIX=$prefix install
#    touch $tags/$what-done
#fi


#what=skyx
#if test -f $tags/$what-done; then
#    echo $what is done
#else
#    cd $build
#    rm -rf $what
#    tarballname=libskyx_0.1.1.orig.tar.gz
#    url=https://launchpad.net/~sonsilentsea-team/+archive/sonsilentsea/+files/$tarballname
#    test -f $tarballs/$tarballname || wget -P $tarballs $url
#    tar zxf $tarballs/$tarballname
#    cd skyx-0.1.1.orig
#    sed -i "s!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(pkg-config OGRE --cflags)!" makefile
#    sed -i "s!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(pkg-config OGRE --libs)!" makefile
#    make -j $nprocs PREFIX=$prefix
#    make PREFIX=$prefix install
#    touch $tags/$what-done
#fi

what=qtscriptgenerator
if test -f $tags/$what-done; then 
   echo $what is done
else
    cd $build
    rm -rf $what
    git clone git://gitorious.org/qt-labs/$what.git
    cd $what

    cd generator
    qmake
    make -j $nprocs
    ./generator --include-paths=/usr/include/qt4
    cd ..

    cd qtbindings
    sed -i 's/qtscript_phonon //' qtbindings.pro 
    sed -i 's/qtscript_webkit //' qtbindings.pro 
    qmake
    make -j $nprocs
    cd ..
    cd ..
    touch $tags/$what-done
fi
mkdir -p $viewer/bin/qtscript-plugins/script
cp -lf $build/$what/plugins/script/* $viewer/bin/qtscript-plugins/script/


what=knet
if false && test -f $tags/$what-done; then 
   echo $what is done
else
    cd $build
    rm -rf knet
    hg clone -r stable http://bitbucket.org/clb/knet
    cd knet
    sed -e "s/USE_TINYXML TRUE/USE_TINYXML FALSE/" -e "s/kNet STATIC/kNet SHARED/" < CMakeLists.txt > x
    mv x CMakeLists.txt
    cmake . -DCMAKE_BUILD_TYPE=Debug
    make -j $nprocs
    cp lib/libkNet.so $prefix/lib/
    rsync -r include/* $prefix/include/
    touch $tags/$what-done
fi

#cd $build
#what=PythonQt
#ver=2.0.1
#if test -f $tags/$what-done; then
#    echo $what is done
#else
#    rm -rf $what$ver
#    zip=../tarballs/$what$ver.zip
#    test -f $zip || wget -O $zip http://downloads.sourceforge.net/project/pythonqt/pythonqt/$what-$ver/$what$ver.zip
#    unzip $zip
#    cd $what$ver
#    fn=generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
#    sed 's/CocoaRequestModal = QEvent::CocoaRequestModal,//' < $fn > x
#    mv x $fn
#    qmake
#    make -j$nprocs
#    rm -f $prefix/lib/lib$what*
#    cp -a lib/lib$what* $prefix/lib/
#    cp src/PythonQt*.h $prefix/include/
#    cp extensions/PythonQt_QtAll/PythonQt*.h $prefix/include/
#    touch $tags/$what-done
#fi

# HydraX, SkyX and PythonQT are build from the realxtend own dependencies.
# At least for the time being, until changes to those components flow into
# upstream..

cd $build
depdir=realxtend-tundra-deps
if [ ! -e $depdir ]
then
    echo "Cloning source of HydraX/SkyX/PythonQT/NullRenderer..."
    git clone https://code.google.com/p/realxtend-tundra-deps
fi
cd $depdir
git pull
git checkout sources
# HydraX build:
if test -f $tags/hydrax-done; then
    echo "Hydrax-done"
else
    cd $build/$depdir/hydrax
    sed -i "s!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(pkg-config OGRE --cflags)!" makefile
    sed -i "s!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(pkg-config OGRE --libs)!" makefile
    make -j $nprocs PREFIX=$prefix
    make PREFIX=$prefix install
    touch $tags/hydrax-done
fi
# SkyX build
if test -f $tags/skyx-done; then
    echo "SkyX-done"
else
    cd $build/$depdir/skyx
    sed -i "s!^OGRE_CFLAGS.*!OGRE_CFLAGS = $(pkg-config OGRE --cflags)!" makefile
    sed -i "s!^OGRE_LDFLAGS.*!OGRE_LDFLAGS = $(pkg-config OGRE --libs)!" makefile
    make -j $nprocs PREFIX=$prefix
    # Media should be media, linux files case sensitive..
    sed -i "s/Media/media/" makefile
    make PREFIX=$prefix install
    touch $tags/skyx-done
fi
# PythonQT build
if test -f $tags/pythonqt-done; then
    echo "PythonQt-done"
else
    cd $build/$depdir/PythonQt
    qmake
    make -j $nprocs
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
    test -f $zip || wget -O $zip http://get.qt.nokia.com/qt/solutions/lgpl/$pkgbase.tar.gz
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

if test "$1" = "--depsonly"; then
    exit 0
fi

cd $viewer
cat > ccache-g++-wrapper <<EOF
#!/bin/sh
exec ccache g++ -O -g \$@
EOF
chmod +x ccache-g++-wrapper
TUNDRA_DEP_PATH=$prefix cmake -DCMAKE_CXX_COMPILER="$viewer/ccache-g++-wrapper" .
make -j $nprocs VERBOSE=1
