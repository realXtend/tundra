#!/bin/bash
# Script to build naali and dependencies for Fedora 14

#IN CASE ERROR HAPPENS, $?-VARIABLE IS != 0
function errorCheck {
    if [ $? -ne 0 ];
    then
        echo $1
        exit $?
    fi
}

export PYTHON_PATH=/usr/bin/
export PYTHON_LIB=/usr/lib/python2.7/

if [ $(pwd) == / ]; then
    cd /builddir
fi

ARCH=$1
TIMESTAMP=$2
VER=$3
TAG=$4
USESTAMP=$5

rpmbuild=/rpmbuild
naalidir=$(pwd)/naali
doneflags=$rpmbuild/flags
packets=/packages/usr
workdir=$(pwd)

pcount=`grep -c "^processor" /proc/cpuinfo`

yum install -y git-core

cd $naalidir

if [ $TAG != "none" ]; then
	git show-ref $TAG
	if [ $? -ne 0 ]; then
		echo "Invalid tag" $TAG
		exit 1
	fi
	git checkout $TAG
	VER=$TAG	
else
	VER=`grep "Tundra" $naalidir/Viewer/main.cpp | cut -d 'v' -f2 -|cut -d '-' -f 1`
fi

cd ..

#Rename .spec files to match the name of package to be created
mv /builddir/specs/realxtend-tundra-0.0-fc14.spec /builddir/specs/realxtend-tundra-$VER-fc14.spec
mv /builddir/specs/realxtend-tundra-scenes-0.0-1.spec /builddir/specs/realxtend-tundra-scenes-$VER-1.spec

#Edit some lines to match the package to be creat
sed -e "s#/naali-build#$workdir#g" /builddir/specs/realxtend-tundra-$VER-fc14.spec > tmpfile ; mv tmpfile /builddir/specs/realxtend-tundra-$VER-fc14.spec
sed -e 's#$HOME##g' /builddir/specs/realxtend-tundra-$VER-fc14.spec > tmpfile ; mv tmpfile /builddir/specs/realxtend-tundra-$VER-fc14.spec
sed -e "s#/naali-build#$workdir#g" /builddir/specs/realxtend-tundra-scenes-$VER-1.spec > tmpfile ; mv tmpfile /builddir/specs/realxtend-tundra-scenes-$VER-1.spec
sed -e 's#$HOME##g' /builddir/specs/realxtend-tundra-scenes-$VER-1.spec > tmpfile ; mv tmpfile /builddir/specs/realxtend-tundra-scenes-$VER-1.spec
sed -e "s#0.0#$VER#g" /builddir/specs/realxtend-tundra-$VER-fc14.spec > tmpfile ; mv tmpfile /builddir/specs/realxtend-tundra-$VER-fc14.spec

#rpm build init
mkdir -p $packets/lib $packets/include $packets/../rpms
mkdir -p $rpmbuild/SPECS $rpmbuild/SOURCES $doneflags
cp /builddir/specs/* $rpmbuild/SPECS

cd $naalidir/tools

#remove debug data from builds
sed -i 's/ccache g++ -O -g /ccache g++ -O /' $naalidir/tools/build-fedora14-deps.bash

#enable OGRE build
sed -i "s#private_ogre=false#private_ogre=true#g" $naalidir/tools/build-fedora14-deps.bash

#Build
./build-fedora14-deps.bash
errorCheck "Check for error with build process"

#Create startup scripts
cat > $naalidir/bin/run-viewer.sh << EOF
#!/bin/bash
gconftool-2 --set /desktop/gnome/url-handlers/tundra/command '/opt/realXtend/run-viewer.sh' --type String
gconftool-2 --set /desktop/gnome/url-handlers/tundra/enabled --type Boolean true
cd /opt/realXtend
export LD_LIBRARY_PATH=.:./modules/core:./lib
./viewer "$@"
EOF

cat > $naalidir/bin/run-server.sh << EOF
#!/bin/bash
cd /opt/realXtend
export LD_LIBRARY_PATH=.:./modules/core:./lib
./server "$@"
EOF

cd $naalidir/bin/
#strip * */**

chmod 755 run-server.sh
chmod 755 run-viewer.sh

#Create rpm packages
rpmbuild -bb  -vv --target x86_64 --define '_topdir /rpmbuild' $rpmbuild/SPECS/realxtend-tundra-scenes-$VER-1.spec
rpmbuild -bb -vv --target x86_64 --define '_topdir /rpmbuild' $rpmbuild/SPECS/realxtend-tundra-$VER-fc14.spec

#Add timestamp if $USESTAMP is set
if [ x$USESTAMP == xtrue ]; then
	mv $rpmbuild/RPMS/x86_64/realXtend-Tundra-$VER-fc14.x86_64.rpm $rpmbuild/RPMS/x86_64/realXtend-Tundra-$VER-$TIMESTAMP-fc14.x86_64.rpm	
fi


