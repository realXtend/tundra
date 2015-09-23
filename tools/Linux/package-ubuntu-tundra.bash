#!/bin/bash

viewer=$(dirname $(readlink -f $0))/../..
deps=$viewer/../naali-deps
deps=$(cd $deps && pwd)
viewer=$(cd $viewer && pwd)

prefix=$deps/install/lib
dest=deb/opt/realxtend-tundra

version="2.5.4"
package="realxtend-tundra-$version-ubuntu-14.04-amd64.deb"

echo "Creating $package"
echo "from  $prefix"
echo "to    $dest"
echo ""

# Prepare
rm -r -f deb/opt
mkdir -p $dest

mkdir -p $prefix/qtplugins/script
cp -f -L $deps/build/qtscriptgenerator/plugins/script/*.so $prefix/qtplugins/script

# List
libraries=( 
    libkNet.so
    libOgre*.so.1.9.0
    OGRE/Plugin_*.so.1.9.0
    OGRE/RenderSystem_*.so.1.9.0
    libSkyX.so.0
    libhydrax.so
    libassimp.so.3
    libcelt0.so.2
    libQtSolutions_PropertyBrowser-head.so.1
    libqjson.so.0
    qtplugins/script/*.so
)

destinations=(
    .
    .
    .
    .
    .
    .
    .
    .
    .
    .
    qtplugins/script
)

echo "Copying and stripping Tundra third-party libraries..."
for (( i = 0 ; i < ${#libraries[@]} ; i++ )) do
    lib=${libraries[$i]}
    libname=$(basename $lib)
    libsrc=$prefix/$lib
    libdest=${destinations[$i]}

    echo "$lib "
    echo "  cp..."
    mkdir -p $dest/$libdest
    cp -f -L $libsrc $dest/$libdest
    echo "  strip..."
    strip --strip-debug $dest/$libdest/$libname
done

# Copy Tundra build and data
echo "Copying and stripping Tundra executable, plugins and data..."
cp $viewer/bin/** $dest
strip --strip-debug $dest/Tundra
strip --strip-debug $dest/*.so

tundradirs=(
    data
    dox
    jsmodules
    media
    plugins
    scenes
)
for (( i = 0 ; i < ${#tundradirs[@]} ; i++ )) do
    dir=${tundradirs[$i]}
    echo "$dir "
    echo "  cp..."
    mkdir -p $dest/$dir
    cp -f -r $viewer/bin/$dir/** $dest/$dir
    echo "  strip..."
    strip --strip-debug $dest/$dir/*.so
done

# Copy Tundra config file with fixed Ogre library paths
rm -f $dest/tundra-rendering-ogre.json
cp tundra-rendering-ogre-deb.json $dest/tundra-rendering-ogre.json

# Create Tundra executable script
cat > $dest/RunTundra <<EOF
#!/bin/sh

tundradir=\$(dirname \$(readlink -f \$0))
cd \$tundradir
LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH ./Tundra --config tundra-client.json
EOF
chmod +x $dest/RunTundra

echo "Building .deb package..."
dpkg --build deb $package

echo ""
echo "---- ALL DONE ----"
echo ""
