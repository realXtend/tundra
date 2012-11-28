#!/bin/bash
set -e

# This script is based on instructions from
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools

# Work in Tundra repository root directory.
cd ..
rootdir=$(pwd)
echo "Assuming $rootdir is the Tundra root directory."

title=Tundra

rm -f pack.temp.dmg
echo "Creating temporary disk image."
hdiutil create -srcfolder "build/" -volname "${title}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 1000000k pack.temp.dmg
device=$(hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $1}')
echo "Mounted temp disk image to $device"

echo "Adding background for installer."
backgroundPictureName="splash.png"
mkdir -p /Volumes/${title}/.background
cp tools/installers/mac/Resources/splash.png /Volumes/${title}/.background/

applicationName="Tundra"

echo "Automating installer window."
echo '
tell application "Finder"
tell disk "'${title}'"
open
set current view of container window to icon view
set toolbar visible of container window to false
set statusbar visible of container window to false
set the bounds of container window to {400, 100, 995, 450}
set theViewOptions to the icon view options of container window
set arrangement of theViewOptions to not arranged
set icon size of theViewOptions to 72
set background picture of theViewOptions to file ".background:'${backgroundPictureName}'"
make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
set position of item "'${applicationName}'" of container window to {124, 190}
set position of item "Applications" of container window to {480, 190}

set position of item ".background" of container window to {10000, 0}
set position of item ".DS_Store" of container window to {10000, 0}
set position of item ".fseventsd" of container window to {10000, 0}
set position of item ".Trashes" of container window to {10000, 0}
close
open
update without registering applications
delay 5
end tell
end tell
' | osascript

echo "Making installer files read-only."
FILES=/Volumes/"${title}"/*
for f in $FILES
do
# TODO Also process files starting with '.', but ignore any errors. (now aborts on .ds_store file) 
   if [[ $f != .* ]];
   then
      echo "chmodding $f"
      chmod -Rf go-w $f
   fi
done

sync
sync

echo "Detaching temp disk image."
hdiutil detach ${device}
finalDMGName=tundra.dmg
rm -f ${finalDMGName}
echo "Generating final disk image."
hdiutil convert "pack.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
rm -r pack.temp.dmg

echo "Done."
