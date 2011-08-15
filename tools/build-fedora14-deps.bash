#!/bin/bash
set -e
set -x

# script to build naali and most deps. first manually add the
# following ppa sources using add-apt-repository or the software
# sources gui tool: ppa:mapopa/qt4.6 ppa:andrewfenn/ogredev

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

export PATH=$prefix/bin:/usr/lib64:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include
export CC="ccache gcc"
export CXX="ccache g++"
export CCACHE_DIR=$deps/ccache

private_ogre=false

if [ x$private_ogre != xtrue ]; then
            more="$more ogre-devel"
fi

yum localinstall -y --nogpgcheck http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-stable.noarch.rpm

yum groupinstall -y "Development Tools"
yum install -y scons libogg-devel python-devel libvorbis-devel openjpeg-devel \
libcurl-devel expat-devel phonon-devel boost-devel poco-devel \
pygtk2-devel dbus-devel ccache qt-devel telepathy-farsight-devel libnice-devel \
bison flex libxml2-devel ois-devel cmake freealut-devel liboil-devel pango-devel \
wget qt qt4 mercurial unzip libxslt qtscriptbindings freeglut-devel xmlrpc-epi-devel \
qt-webkit-devel Cg libXaw-devel freetype-devel \
ois-devel doxygen cppunit-devel zziplib-devel libXrandr-devel freeimage-devel $more\

if test -f /usr/bin/qmake; then
	echo qmake exists
else
	ln -s /usr/bin/qmake-qt4 /usr/bin/qmake
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
if test -f $tags/$what-done; then 
   echo $what is done
else
    cd $build
    rm -rf knet
    hg clone -r stable http://bitbucket.org/clb/knet
    cd knet
    sed -e "s/USE_TINYXML TRUE/USE_TINYXML FALSE/" -e "s/kNet STATIC/kNet SHARED/"  < CMakeLists.txt > x
#-e "s/#set(USE_QT/set(USE_QT/"
    mv x CMakeLists.txt
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

	if [ ! -f /usr/bin/python2.6-config ]; then
		ln -s /usr/bin/python2.7-config /usr/bin/python2.6-config
	fi

    qmake
	sed -i 's/CocoaRequestModal = QEvent::CocoaRequestModal,//g' generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
    make -j2
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
    # luckily only extensionless headers under src match Qt*:
    cp src/qt*.h src/Qt* $prefix/include/
    touch $tags/$what-done
fi



ln -fvs /usr/include/xmlrpc-epi/*.h $prefix/include/

    build-regular http://nice.freedesktop.org/releases/ libnice 0.0.10
#    build-regular http://gstreamer.freedesktop.org/src/gstreamer/ gstreamer 0.10.33
#    build-regular http://gstreamer.freedesktop.org/src/gst-plugins-base/ gst-plugins-base 0.10.25
#    build-regular http://gstreamer.freedesktop.org/src/gst-python/ gst-python 0.10.17
#    build-regular http://farsight.freedesktop.org/releases/farsight2/ farsight2 0.0.17
#    build-regular http://farsight.freedesktop.org/releases/obsolete/gst-plugins-farsight/ gst-plugins-farsight 0.12.11
#    build-regular http://telepathy.freedesktop.org/releases/telepathy-glib/ telepathy-glib 0.13.0
#    build-regular http://telepathy.freedesktop.org/releases/telepathy-farsight/ telepathy-farsight 0.0.13
#    build-regular http://telepathy.freedesktop.org/releases/telepathy-qt4/ telepathy-qt4 0.2.1
    build-regular http://downloads.sourceforge.net/project/poco/sources/poco-1.3.6/ poco 1.3.6p1

if test "$1" = "--depsonly"; then
    exit 0
fi

cd $viewer
cat > ccache-g++-wrapper <<EOF
#!/bin/sh
exec ccache g++ -O \$@
EOF
chmod +x ccache-g++-wrapper
NAALI_DEP_PATH=$prefix cmake -DCMAKE_CXX_FLAGS:STRING="-lrt -lboost_filesystem -lboost_thread-mt" -DCMAKE_CXX_COMPILER="$viewer/ccache-g++-wrapper" .
make -j $nprocs VERBOSE=1

sed '/PluginFolder/c \PluginFolder=lib/OGRE' $viewer/bin/plugins-unix.cfg > tmpfile ; mv tmpfile /$viewer/bin/plugins-unix.cfg


