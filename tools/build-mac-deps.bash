#!/bin/bash
set -e
set -x

# script to build most deps. omitted at least (= install sdk manually)
# * Ogre SDK 1.6.5+
#
# * Ogre binary dependencies package (OgreDependencies_OSX_Eihort_20080115)
#
# * QT 4.6.1+ for Carbon 32-bit (to match Ogre)

export CMAKE_OSX_ARCHITECTURES=i386
export CFLAGS="-arch i386"
export CXXFLAGS="-arch i386"
export CMAKE_C_FLAGS="-arch i386"
export CMAKE_CXX_FLAGS="-arch i386"

deps=$HOME/src/deps
viewer=$deps/../realxtend-naali
prefix=$deps
build=$deps/build
tarballs=$deps/tarballs
tags=$deps/tags

# -j<n> param for make, for how many processes to run concurrently

nprocs=1

mkdir -p $tarballs $build $prefix/{lib,share,etc,include} $tags

export PATH=$prefix/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include

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
        test -f $zip || curl -L -o $zip $dlurl
	tar xzf $zip

	if test $what = xmlrpc-epi; then
	    cd xmlrpc # well, almost regular
	else
	    cd $pkgbase
	fi
	./configure --disable-debug --disable-shared --prefix=$prefix
	make -j $nprocs
	make install
	touch $tags/$what-done
    fi
}

cd $build
what=boost    
urlbase=http://downloads.sourceforge.net/project/boost/boost/1.41.0/
pkgbase=boost_1_41_0
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
	./bjam address-model=32 architecture=x86 install
	touch $tags/$what-done
    fi


viewerdeps_svnroot=http://realxtend-naali-deps.googlecode.com/svn


cd $build
what=PythonQt
if test -f $tags/$what-done; then
    echo $what is done
else
    if test -d $what; then
	svn update $what
    else
	svn co $viewerdeps_svnroot/trunk/$what $what
    fi
    cd $what
    sed 's/^#CONFIG += release/CONFIG += release/' < build/common.prf > x &&
      mv x build/common.prf

    qmake -spec macx-g++
    make -j $nprocs
    rm -f $prefix/lib/lib$what*
    cp -a lib/lib$what* $prefix/lib/
    cp src/PythonQt*.h $prefix/include/
    cp extensions/PythonQt_QtAll/PythonQt*.h $prefix/include/
    touch $tags/$what-done
fi

cd $build
what=OpenJPEG
if test -f $tags/$what-done; then
    echo $what is done
else

    if test -d $what; then
	svn update $what
    else
	svn co $viewerdeps_svnroot/trunk/$what $what
    fi
    cd $what
    cmake .
    make
    cp -a bin/libopenjpeg.a $prefix/lib/
    cp libopenjpeg/openjpeg.h $prefix/include
    touch $tags/$what-done
fi


build-regular http://downloads.sourceforge.net/project/poco/sources/poco-1.3.6/ poco 1.3.6p1

ln -sf xmlrpc $build/xmlrpc-epi-0.54.1
build-regular http://downloads.sourceforge.net/project/xmlrpc-epi/xmlrpc-epi-base/0.54.1/ xmlrpc-epi 0.54.1

# build-regular http://nice.freedesktop.org/releases/ libnice 0.0.10
# build-regular http://gstreamer.freedesktop.org/src/gstreamer/ gstreamer 0.10.25
# build-regular http://gstreamer.freedesktop.org/src/gst-plugins-base/ gst-plugins-base 0.10.25
# build-regular http://gstreamer.freedesktop.org/src/gst-python/ gst-python 0.10.17
# build-regular http://farsight.freedesktop.org/releases/farsight2/ farsight2 0.0.16
# build-regular http://farsight.freedesktop.org/releases/gst-plugins-farsight/ gst-plugins-farsight 0.12.11
# build-regular http://telepathy.freedesktop.org/releases/telepathy-glib/ telepathy-glib 0.9.1
# build-regular http://telepathy.freedesktop.org/releases/telepathy-farsight/ telepathy-farsight 0.0.12
# build-regular http://telepathy.freedesktop.org/releases/telepathy-qt4/ telepathy-qt4 0.2.0
cd $viewer
NAALI_DEP_PATH=$prefix CMAKE_FRAMEWORK_PATH=$deps/OgreSDK/Dependencies cmake .
make -j $nprocs VERBOSE=1

