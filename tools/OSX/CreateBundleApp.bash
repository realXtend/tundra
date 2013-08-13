#!/bin/bash

# Reconfigure thèse paths if necessary!
startdir=$(pwd)
cd ../../deps
depsdir=$(pwd)
cd build/ogre-safe-nocrashes
ogredir=$(pwd)
cd $depsdir/osx-64/qt
qtdir=$(pwd)
cd $startdir

echo "Assuming Qt is found at $qtdir."
if [ ! -d $qtdir ]; then
	echo "ERROR: qtdir=$qtdir does not seem to exist! Please configure this script by hand to point to where you installed QtSDK."
fi
echo "Assuming Ogre is found at $ogredir."

qtlibdir=$qtdir/lib
qtpluginsdir=$qtdir/plugins

# Establish directories we're working in.
cd ../..
startdir=$(pwd)
echo "Assuming $startdir is the Tundra repository root directory."
mkdir -p build/Tundra.app/ # Hide errors if path doesn't exist.
cd build/Tundra.app/
bundledir=$(pwd)
frameworksdir=$bundledir/Contents/Frameworks
cd $startdir

echo "Deleting old Tundra.app bundle."
rm -rf build/Tundra.app/

echo "Creating new Tundra.app bundle structure."
mkdir -p $bundledir/Contents/{Components,Frameworks,MacOS,Plugins,Resources}
mkdir -p $bundledir/Contents/Resources/Scripts

echo "Deploying Tundra files to app bundle in $bundledir."
cp -R bin/* $bundledir/Contents/MacOS
cp -R tools/OSX/Installer/mac/* $bundledir/Contents
cp tools/OSX/TundraLauncher.app/Contents/MacOS/applet $bundledir/Contents/MacOS
cp tools/OSX/TundraLauncher.app/Contents/Resources/Scripts/main.scpt $bundledir/Contents/Resources/Scripts

echo "Deploying Qt frameworks from $qtlibdir to app bundle."
cp -R $qtlibdir/QtCore.framework $frameworksdir
cp -R $qtlibdir/QtGui.framework $frameworksdir
cp -R $qtlibdir/QtNetwork.framework $frameworksdir
cp -R $qtlibdir/QtXml.framework $frameworksdir
cp -R $qtlibdir/QtXmlPatterns.framework $frameworksdir
cp -R $qtlibdir/QtScript.framework $frameworksdir
cp -R $qtlibdir/QtScriptTools.framework $frameworksdir
cp -R $qtlibdir/QtWebKit.framework $frameworksdir
cp -R $qtlibdir/QtSvg.framework $frameworksdir
cp -R $qtlibdir/QtSql.framework $frameworksdir

#TODO Remove this.
cp -R $qtlibdir/QtDeclarative.framework $frameworksdir

echo "Deploying Qt plugins from $qtpluginsdir to app bundle."
cp -R $qtpluginsdir/* $bundledir/Contents/Plugins
mkdir $bundledir/Contents/MacOS/qtplugins
mkdir $bundledir/Contents/MacOS/qtplugins/script
cp -R $depsdir/build/qtscriptgenerator/plugins/script/*.dylib $bundledir/Contents/MacOS/qtplugins/script 
debugplugins=`find $bundledir/Contents/MacOS/qtplugins/script -name "*_debug*.dylib"`
rm $debugplugins

echo "Deploying Ogre plugins from $ogredir to app bundle."
cp $ogredir/lib/relwithdebinfo/*.dylib $bundledir/Contents/Plugins
cp $ogredir/lib* $bundledir/Contents/Components
ogresamples=`find $bundledir/Contents/Plugins -name "Sample_*.dylib"`
rm $ogresamples

echo "Deploying Ogre frameworks from $ogredir to app bundle."
cp -R $ogredir/lib/relwithdebinfo/Ogre.framework $frameworksdir
cp -R $ogredir/lib/relwithdebinfo/OgreOverlay.framework $frameworksdir
cp -R $ogredir/lib/relwithdebinfo/Plugin_CgProgramManager.framework $frameworksdir
cp -R $ogredir/lib/relwithdebinfo/Plugin_OctreeSceneManager.framework $frameworksdir
cp -R $ogredir/lib/relwithdebinfo/Plugin_ParticleFX.framework $frameworksdir
cp -R $ogredir/lib/relwithdebinfo/RenderSystem_GL.framework $frameworksdir

# When running in an app bundle, Ogre plugins are loaded from the Plugins/ folder of the app bundle.
mv $bundledir/Contents/MacOS/*.dylib $bundledir/Contents/Plugins

# Deploy Tundra deps dylibs to Plugins
cp $depsdir/lib/*.dylib $bundledir/Contents/Plugins

echo "Deploying system Cg.framework to app bundle."
cp -R $depsdir/Frameworks/Cg.framework $frameworksdir

chmod -R u+w $frameworksdir/Cg.framework

echo "Cleaning redundant files from bundle (*_debug*.dylib etc.)"
debugdylibs=`find $bundledir -name "*_debug*.dylib"`
rm $debugdylibs
