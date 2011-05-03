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

if lsb_release -c | egrep -q "lucid|maverick"; then
        which aptitude > /dev/null 2>&1 || sudo apt-get install aptitude
	sudo aptitude -y install scons python-dev libogg-dev libvorbis-dev \
	 libopenjpeg-dev libcurl4-gnutls-dev libexpat1-dev libphonon-dev \
	 build-essential g++ libogre-dev libboost-all-dev libpoco-dev \
	 python-gtk2-dev libdbus-glib-1-dev ccache libqt4-dev python-dev \
         libtelepathy-farsight-dev libnice-dev libgstfarsight0.10-dev \
         libtelepathy-qt4-dev python-gst0.10-dev freeglut3-dev \
	 libxmlrpc-epi-dev bison flex libxml2-dev cmake libalut-dev \
	 liboil0.3-dev mercurial unzip xsltproc libqtscript4-qtbindings
fi

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
    build-regular http://nice.freedesktop.org/releases/ libnice 0.0.10
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
NAALI_DEP_PATH=$prefix cmake -DCMAKE_CXX_COMPILER="$viewer/ccache-g++-wrapper" .
make -j $nprocs VERBOSE=1
