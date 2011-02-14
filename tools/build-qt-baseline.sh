#!/bin/bash
export QTBUILDDIR=$HOME/qt
export QTTARGETDIR=/usr/local/qt-releases/v4.7.1
export QTINSTALLDIR=/usr/local/qt-releases/qt
mkdir -p $QTBUILDDIR
cd $QTBUILDDIR
#---- Build QT
git clone git://gitorious.org/qt/qt.git
cd qt
git checkout v4.7.1
./configure --prefix=$QTTARGETDIR --disable-qt3support -dbus -phonon-backend | tee configure.log
make -j 8
sudo make install
sudo ln -s $QTTARGETDIR $QTINSTALLDIR
#---- Build QT3D
git clone git://gitorious.org/qt-labs/qt3d.git
cd qt3d
sudo ln -s $QTBUILDDIR/qt/include/QtGui/private $QTTARGETDIR/include/QtGui/private
sudo ln -s $QTBUILDDIR/qt/include/QtCore/private $QTTARGETDIR/include/QtCore/private
sudo ln -s $QTBUILDDIR/qt/include/QtDeclarative/private $QTTARGETDIR/include/QtDeclarative/private
sudo ln -s $QTBUILDDIR/qt/include/QtOpenGL/private $QTTARGETDIR/include/QtOpenGL/private
$QTTARGETDIR/bin/qmake opengl.pro
make -j 8
sudo make install
cd ..
#----- Building QT mobility
wget http://get.qt.nokia.com/qt/add-ons/qt-mobility-opensource-src-1.2.0-tp1.tar.gz
tar xvf qt-mobility-opensource-src-1.2.0-tp1.tar.gz
cd qt-mobility-opensource-src-1.2.0-tp1
sudo aptitude install libxv-dev libbluetooth-dev
./configure --prefix $QTINSTALLDIR -qmake-exec $QTINSTALLDIR/bin/qmake -examples
make -j 8
sudo make install
exit 0
