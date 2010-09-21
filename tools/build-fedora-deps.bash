#!/bin/bash
set -e
set -x

# script to build naali and most deps. first manually add the
# following ppa sources using add-apt-repository or the software
# sources gui tool: ppa:mapopa/qt4.6 ppa:andrewfenn/ogredev

deps=$HOME/naali-build/naali-deps
viewer=$deps/../naali
viewerdeps_svn=http://realxtend-naali-deps.googlecode.com/svn/
prefix=$deps/install
build=$deps/build
tarballs=$deps/tarballs
tags=$deps/tags



# -j<n> param for make, for how many processes to run concurrently

nprocs=2

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
    scons extra_ccflags="-fPIC -DPIC"
    mkdir -p $prefix/etc/OGRE
    cp plugins.cfg $prefix/etc/OGRE/
    cp lib$what.a $prefix/lib/
    cp main/include/* $prefix/include/
    touch $tags/$what-done
fi

cd $build
what=PythonQt
if test -f $tags/$what-done; then
    echo $what is done
else
    if test -d $what; then
        svn update $what
    else
        svn co $viewerdeps_svn/trunk/$what $what
    fi
    cd $what
    /usr/lib/qt4/bin/qmake
    make
    rm -f $prefix/lib/lib$what*
    cp -a lib/lib$what* $prefix/lib/
    cp src/PythonQt*.h $prefix/include/
    cp extensions/PythonQt_QtAll/PythonQt*.h $prefix/include/
    touch $tags/$what-done
fi

ln -fvs /usr/include/xmlrpc-epi/*.h $prefix/include/

build-regular http://nice.freedesktop.org/releases/ libnice 0.0.10
build-regular http://gstreamer.freedesktop.org/src/gstreamer/ gstreamer 0.10.25
build-regular http://gstreamer.freedesktop.org/src/gst-plugins-base/ gst-plugins-base 0.10.25
build-regular http://gstreamer.freedesktop.org/src/gst-python/ gst-python 0.10.17
build-regular http://farsight.freedesktop.org/releases/farsight2/ farsight2 0.0.17
build-regular http://farsight.freedesktop.org/releases/gst-plugins-farsight/ gst-plugins-farsight 0.12.11
build-regular http://telepathy.freedesktop.org/releases/telepathy-glib/ telepathy-glib 0.9.1
build-regular http://telepathy.freedesktop.org/releases/telepathy-farsight/ telepathy-farsight 0.0.13
build-regular http://telepathy.freedesktop.org/releases/telepathy-qt4/ telepathy-qt4 0.2.1
build-regular http://downloads.sourceforge.net/project/poco/sources/poco-1.3.6/ poco 1.3.6p1

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

