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

private_ogre=false
# Set build_valgrind true if you want zero optimizations build-options and valgrind installed.
# Also kNet messageConnection.cpp is modified so that server keepAliveTimeout is 3min instead of 15s.
build_valgrind=false

if [ x$private_ogre != xtrue ]; then
   more="$more libogre-dev"
fi

if lsb_release -c | egrep -q "lucid|maverick|natty|oneiric"; then
        which aptitude > /dev/null 2>&1 || sudo apt-get install aptitude
        if [ x$build_valgrind != xfalse ]; then
            more="$more libc6 libc6-dbg valgrind"
        fi
	sudo aptitude -y install scons python-dev libogg-dev libvorbis-dev \
	 libopenjpeg-dev libcurl4-gnutls-dev libexpat1-dev libphonon-dev \
	 build-essential g++ libboost-all-dev libpoco-dev \
	 ccache libqt4-dev python-dev \
	 freeglut3-dev \
	 libxmlrpc-epi-dev bison flex libxml2-dev cmake libalut-dev \
	 liboil0.3-dev mercurial unzip xsltproc libqtscript4-qtbindings \
	 nvidia-cg-toolkit libfreetype6-dev libxaw7-dev libois-dev doxygen libcppunit-dev \
     libzzip-dev libxrandr-dev libfreeimage-dev $more
fi
	 #python-gtk2-dev libdbus-glib-1-dev \
         #libtelepathy-farsight-dev libnice-dev libgstfarsight0.10-dev \
         #libtelepathy-qt4-dev python-gst0.10-dev \ 

function build-regular {
    urlbase=$1
    shift
    what=$1
    shift
    pkgbase=$what-$1
    dlurl=$urlbase/$pkgbase.tar.gz    

    cd $build
    
    if test -f $tags/$what-done; then
	echo $what is done
    else
	rm -rf $pkgbase
        zip=$tarballs/$pkgbase.tar.gz
        test -f $zip || wget -O $zip $dlurl
	tar xzf $zip
	cd $pkgbase
	./configure --disable-debug --disable-static --prefix=$prefix
	make -j $nprocs
	make install
	touch $tags/$what-done
    fi
}

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
    # If valgrind build: Change connection timeout to 3min.
    if [ x$build_valgrind != xtrue ]; then
        sed -e "s/180.f/30.f/" < src/MessageConnection.cpp > x
        mv x src/MessageConnection.cpp
    else
        sed -e "s/30.f/180.f/" < src/MessageConnection.cpp > x
        mv x src/MessageConnection.cpp
    fi
    cmake . -DCMAKE_BUILD_TYPE=Debug
    make -j $nprocs
    cp lib/libkNet.so $prefix/lib/
    rsync -r include/* $prefix/include/
    touch $tags/$what-done
fi


if [ x$private_ogre = xtrue ]; then
    what=ogre
    if test -f $tags/$what-done; then
        echo $what is done
    else
        cd $build
        rm -rf $what
        hg clone http://bitbucket.org/sinbad/$what/ -u v1-7-3
        cd $what
        mkdir -p $what-build
        cd $what-build
        cmake .. -DCMAKE_INSTALL_PREFIX=$prefix
        make -j $nprocs VERBOSE=1
        make install
        touch $tags/$what-done
    fi
fi

what=Caelum
if test -f $tags/$what-done; then
    echo $what is done
else
    cd $build
    pkgbase=$what-0.5.0
    rm -rf $pkgbase
    zip=../tarballs/$pkgbase.zip
    test -f $zip || wget -O $zip http://ovh.dl.sourceforge.net/project/caelum/caelum/0.5/$pkgbase.zip
    unzip $zip
    cd $pkgbase
    sed "s/depflags.has_key/False and depflags.has_key/g" < SConstruct > SConstruct.edit
    mv SConstruct.edit SConstruct
    scons extra_ccflags="-fPIC -DPIC"
    mkdir -p $prefix/etc/OGRE
    cp plugins.cfg $prefix/etc/OGRE/
    cp lib$what.a $prefix/lib/
    cp main/include/* $prefix/include/
    touch $tags/$what-done
fi


cd $build
what=PythonQt
ver=2.0.1
if test -f $tags/$what-done; then
    echo $what is done
else
    rm -rf $what$ver
    zip=../tarballs/$what$ver.zip
    test -f $zip || wget -O $zip http://downloads.sourceforge.net/project/pythonqt/pythonqt/$what-$ver/$what$ver.zip
    unzip $zip
    cd $what$ver
    pyver=$(python -c 'import sys; print sys.version[:3]')
    sed -i "s/PYTHON_VERSION=.*/PYTHON_VERSION=$pyver/" build/python.prf
    fn=generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
    sed 's/CocoaRequestModal = QEvent::CocoaRequestModal,//' < $fn > x
    mv x $fn
    qmake
    make -j$nprocs
    rm -f $prefix/lib/lib$what*
    cp -a lib/lib$what* $prefix/lib/
    cp src/PythonQt*.h $prefix/include/
    cp extensions/PythonQt_QtAll/PythonQt*.h $prefix/include/
    touch $tags/$what-done
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

ln -fvs /usr/include/xmlrpc-epi/*.h $prefix/include/

if lsb_release -c | grep -q lucid; then
    : # nothing
else
    : #build-regular http://nice.freedesktop.org/releases/ libnice 0.0.10
fi


if test "$1" = "--depsonly"; then
    exit 0
fi

if [ x$build_valgrind != xtrue ]; then
    options="-O -g"
else
    options="-O0 -fno-inline -Wall -g"
    cd $viewer/bin/
    cat > ./.valgrindrc <<EOF
--memcheck:leak-check=full
--memcheck:error-limit=no
--memcheck:track-origins=yes
--memcheck:suppressions=$viewer/bin/valgrind/supps/gtk_init.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/libgdk.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/libgobject.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/libPython.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/nVidia-libGL.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/qt47supp.supp
--memcheck:suppressions=$viewer/bin/valgrind/supps/qtjsc.supp
--memcheck:log-file=$viewer/bin/valgrind/logs/valgrindMemcheck.log
--massif:stacks=yes
--massif:depth=40
--massif:massif-out-file=valgrind/logs/massif.out
--smc-check=all
EOF
fi

cd $viewer
cat > ccache-g++-wrapper <<EOF
#!/bin/sh
exec ccache g++ $options \$@
EOF
chmod +x ccache-g++-wrapper
NAALI_DEP_PATH=$prefix cmake -DCMAKE_CXX_COMPILER="$viewer/ccache-g++-wrapper" .
make -j $nprocs VERBOSE=1

if [ x$private_ogre = xtrue ]; then
sed '/PluginFolder/c \PluginFolder=lib/OGRE' $viewer/bin/plugins-unix.cfg > tmpfile ; mv tmpfile /$viewer/bin/plugins-unix.cfg
fi
