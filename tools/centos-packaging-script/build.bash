#!/bin/bash
#This script builds realXtend tundra in chroot enviroment and packages it to rpm package.

WORKDIR=$(pwd)
BUILDDIR=fedorabuild
TIMESTAMP=`date '+%y%m%d'`
ARCH=amd64
CENTOS_RELEASE="centos-6"
VER=0.0
TAG=none
USESTAMP=xfalse
SERVER=xfalse
SIGNER="none"
BRANCH="tundra"

#IN CASE ERROR HAPPENS, $?-VARIABLE IS != 0
function errorCheck {
    if [ $? != 0 ];
    then
        echo "error" $1
	sudo umount $INSTALL_DIR/proc
        exit 1
    fi
}


USAGE="\nUsage: $0 [--help] [-i install directory] [-b branch] [-t tag] [-a architecture] [-f Fedora release] [-v version] [-d use timestamp] [-s server mode]
		\n	 Default settings 
      	\n   Install directory: $BUILDDIR
      	\n   Branch options: tundra
		\n   Tag options: no default
      	\n   Architecture: $ARCH (i386/x86_64)
      	\n   Linux release: $CENTOS_RELEASE (fedora-13/fedora-14)
      	\n   Version: $VER (0.0, 1.0 etc.)"

# Parse command line options.
while [ $# -gt 0 ]; do
    case "$1" in
    --help)
        echo -e $USAGE
        exit 0
        ;;
	-d)
		VER=$VER$TIMESTAMP
		USESTAMP=true;
        echo "Version: $VER"
		shift
		;;
	-s)
		SERVER=true
		shift
		;;
	-i)
	    shift
        if [ -z $1 ]; then
	        echo "-i option given, but no param for it"
		    exit 1
	    fi
	    BUILDDIR=$1
            echo $BUILDDIR
        echo "Branch option: $BUILDDIR"
	    shift;
        ;;
    -b)
	    shift
        if [ -z $1 ]; then
	        echo "-b option given, but no param for it"
		    exit 1
	    fi
	    BRANCH=$1
            echo $BRANCH
        echo "Branch option: $BRANCH"
	    shift;
        ;;
	-n)
	    shift
        if [ -z $1 ]; then
	        echo "-n option given, but no param for it"
		    exit 1
	    fi
	    BUILDNUMBER=$1
            echo $BRANCH
        echo "Buildnumber option: $BUILDNUMBER"
	    shift;
        ;;
	-t)
	    shift
        if [ -z $1 ]; then
	        echo "-t option given, but no param for it"
		    exit 1
	    fi
	    TAG=$1
            echo $BRANCH
        echo "Tag option: $TAG"
	    shift;
        ;;
	-a)
	    shift
        if [ -z $1 ]; then
		    echo "-a option given, but no param for it"
		    exit 1
	    fi
	    ARCH=$1
        echo "Architecture option: $ARCH"
	    shift;
        ;;
	-f)
	    shift
        if [ -z $1 ]; then
		    echo "-f option given, but no param for it"
		    exit 1
	    fi
	    CENTOS_RELEASE=$1
        echo "Linux option: $CENTOS_RELEASE"
	    shift;
        ;;
	-v)
	    shift
        if [ -z $1 ]; then
		    echo "-v option given, but no param for it"
		    exit 1
	    fi
	    VER=$1
        echo "Version: $VER"
	    shift;
        ;;
    *)
        # getopts issues an error message
        echo "Unknown param $1"
        exit 1
        ;;
    esac
done

BUILDDIR=$CENTOS_RELEASE-$ARCH

#Get rinse to be used for builds
if [ ! -d $WORKDIR/rinse ]; then
	git clone git://gitorious.org/rinse/rinse.git ./rinse
	cd rinse
	sudo make install
	cd $WORKDIR
fi

#Check if proc is mounted and umount if true

mount | grep "/proc on" 
if [ $? -eq 0 ]; then
    echo "unmounting /proc"
    TOUMOUNT=`mount | grep "/proc on" | awk '{ print $3 }'`
    sudo umount $TOUMOUNT
fi

#set -e
#set -x

sudo rm -fr $BUILDDIR
echo $BUILDDIR

#Get Fedora filesystem to be used for builds
sudo rinse --arch=$ARCH --directory=$BUILDDIR --distribution=$CENTOS_RELEASE

#Copy packages used in previous builds to save bandwith and time
if [ -d $WORKDIR/rpmcache-$CENTOS_RELEASE ]; then
	sudo cp -r $WORKDIR/rpmcache-$CENTOS_RELEASE/* $WORKDIR/$BUILDDIR/var/cache/
else
	sudo mkdir $WORKDIR/rpmcache-$CENTOS_RELEASE
fi

#Init directories
sudo mkdir $WORKDIR/$BUILDDIR/builddir
sudo mkdir $WORKDIR/$BUILDDIR/builddir/naali
sudo cp $CENTOS_RELEASE-packaging.bash $WORKDIR/$BUILDDIR/builddir/
sudo cp -r specs $WORKDIR/$BUILDDIR/builddir/
sudo cp -r usr_tundra $WORKDIR/$BUILDDIR/builddir/

#Get latest changes from git
if [ $BRANCH == "tundra" ]; then
#    sudo git stash
#    sudo git checkout $BRANCH
#    sudo git pull git://github.com/realXtend/naali.git $BRANCH
     sudo git clone ../../. $WORKDIR/$BUILDDIR/builddir/naali
fi

if [ $BRANCH == "master" ]; then
#    cd ../../../master
#    sudo git stash
#    sudo git checkout $BRANCH
#    sudo git pull git://github.com/realXtend/naali.git $BRANCH
     sudo git clone . $WORKDIR/$BUILDDIR/builddir/naali
#    cd -
fi

sudo cp qt4.6-scriptgenerator-0.0-el6.x86_64.rpm $WORKDIR/$BUILDDIR/builddir/naali/tools/
sudo cp Cg-3.0_February2011_x86_64.rpm $WORKDIR/$BUILDDIR/builddir/naali/tools/
sudo cp atrpms.repo $WORKDIR/$BUILDDIR/builddir/naali/tools/
sudo cp ../build-centos6-deps.bash $WORKDIR/$BUILDDIR/builddir/naali/tools/

if [ $BRANCH == "master" ];
then
	BRANCH=tundra
fi

VER=`grep "Tundra" ../../Viewer/main.cpp | cut -d 'v' -f2 -|cut -d '-' -f 1`

sudo mount --bind /proc $BUILDDIR/proc
sudo mount --bind /dev $BUILDDIR/dev

sudo chroot $BUILDDIR builddir/$CENTOS_RELEASE-packaging.bash $ARCH $TIMESTAMP $VER $TAG $USESTAMP | tee log/$TIMESTAMP.log

#Copy created packages from chrooted environment and backup used Fedora packages
sudo cp $WORKDIR/$BUILDDIR/rpmbuild/RPMS/x86_64/*.rpm $WORKDIR
sudo rm $WORKDIR/$BUILDDIR/rpmbuild/RPMS/x86_64/*.rpm
sudo cp -r $WORKDIR/$BUILDDIR/var/cache/yum/ $WORKDIR/rpmcache-$CENTOS_RELEASE

#If $SERVER is set, use upload.bash script to upload files to server
if [ x$SERVER == xtrue ]; then
	cd $WORKDIR
	sudo chmod 755 upload.bash
	sudo ./upload.bash $CENTOS_RELEASE $VER
	rm *.rpm
fi

sudo umount -f $BUILDDIR/proc
sudo umount $BUILDDIR/dev




