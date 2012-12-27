#!/bin/bash

if [ ! -d "$QTDIR" ]; then
	echo "QTDIR was not set. Please export it to point to the path to Necessitas Qt."
fi

# for 'adb shell push', path cannot contain ~ , so resolve the path to absolute one.
curdir=$(pwd)
cd $QTDIR
QTDIR=$(pwd)
cd $curdir
echo "QTDIR='$QTDIR'"

echo "Testing the presence of adb in PATH.."
adb > /dev/null 2> /dev/null
echo "OK."

echo "Deploying Qt libraries to the device."
echo "Following Android devices found to be connected:"
adb devices

echo "Creating paths."
adb shell mkdir /data
adb shell mkdir /data/local
adb shell mkdir /data/local/qt
adb shell mkdir /data/local/qt/lib
echo "Deploying files."
adb push $QTDIR/lib/libQtCore.so /data/local/qt/lib
adb push $QTDIR/lib/libQtDeclarative.so /data/local/qt/lib
adb push $QTDIR/lib/libQtGui.so /data/local/qt/lib
adb push $QTDIR/lib/libQtNetwork.so /data/local/qt/lib
adb push $QTDIR/lib/libQtOpenGL.so /data/local/qt/lib
adb push $QTDIR/lib/libQtScript.so /data/local/qt/lib
adb push $QTDIR/lib/libQtScriptTools.so /data/local/qt/lib
adb push $QTDIR/lib/libQtSql.so /data/local/qt/lib
adb push $QTDIR/lib/libQtSvg.so /data/local/qt/lib
adb push $QTDIR/lib/libQtTest.so /data/local/qt/lib
adb push $QTDIR/lib/libQtWebKit.so /data/local/qt/lib
adb push $QTDIR/lib/libQtXmlPatterns.so /data/local/qt/lib
adb push $QTDIR/lib/libQtXml.so /data/local/qt/lib
adb push $QTDIR/plugins/platforms/android/libandroid-9.so /data/local/qt/lib

