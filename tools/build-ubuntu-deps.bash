#!/bin/bash
set -e
set -x

# script to build naali and most deps. first manually add the
# following ppa sources using add-apt-repository or the software
# sources gui tool: ppa:mapopa/qt4.6

deps=$HOME/src/rex-deps
viewer=$deps/../rex-viewer
viewerdeps_svn=http://realxtend-naali-deps.googlecode.com/svn/
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

if lsb_release -c | grep -q lucid; then
	sudo aptitude -y install scons python-dev libogg-dev libvorbis-dev \
	 libopenjpeg-dev libcurl4-gnutls-dev libexpat1-dev libphonon-dev \
	 build-essential g++ libglib libogre-dev libboost-all-dev libpoco-dev \
	 python-gtk2-dev libdbus-glib-1-dev ccache libqt4-dev python-dev \
         libtelepathy-farsight-dev libnice-dev libgstfarsight0.10-dev \
         libtelepathy-qt4-dev \
	 libxmlrpc-epi-dev bison flex libxml2-dev liboil-dev cmake libalut-dev

	
fi

if lsb_release -c | grep -q karmic; then
	sudo aptitude -y install scons python-dev libogg-dev libvorbis-dev \
	 libopenjpeg-dev libcurl4-gnutls-dev libexpat1-dev libphonon-dev \
	 build-essential g++ libglib libogre-dev \
	 python-gtk2-dev libdbus-glib-1-dev ccache libqt4-dev python-dev \
	 libxmlrpc-epi-dev bison flex libxml2-dev liboil-dev cmake libalut-dev
	sudo apt-get install 'libboost1.38.*-dev' 
	
fi

if lsb_release -c | grep -q jaunty; then
	sudo aptitude -y install build-essential libboost1.37-dev \
	 libopenjpeg-dev libxmlrpc-epi-dev libcurl4-gnutls-dev libqt4-dev \
	 libexpat1-dev libphonon-dev python-dev scons g++ python-gtk2-dev \
	 libdbus-glib-1-dev ccache bison flex libxml2-dev liboil-dev \
	 libalut-dev libvorbis-dev libogg-dev
	sudo apt-get install 'libboost1.37.*-dev' cmake
	 
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
what=propertyeditor
if test -f $tags/$what-done; then
    echo $what is done
else

    if test -d $what; then
	svn update $what
    else
	svn co $viewerdeps_svn/trunk/$what $what
    fi
    cd $what
    cmake .
    make
    cp -a lib/libPropertyEditor.* $prefix/lib/
    cp lib/property*.h $prefix/include
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
    qmake
    make
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
    make
    cp lib/lib* $prefix/lib/
    cp src/qt*.h $prefix/include/
    touch $tags/$what-done
fi

ln -fvs /usr/include/xmlrpc-epi/*.h $prefix/include/

if lsb_release -c | grep -q lucid; then
    : # nothing
else
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

