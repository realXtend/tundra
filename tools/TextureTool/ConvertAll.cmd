@echo off

echo This script processes all .Texture, .jpg, .png and .bmp files in the current directory
echo and converts them to BC1 (no alpha) or BC3 (alpha) -compressed textures.
echo It is best that you add the directory where ConvertAll.cmd resides to your PATH so
echo you can invoke it for any directory.
echo The outputted textures are placed into the subdirectory \dds.
echo The nVidia Texture Tools 2.08 is required.
echo See http://developer.nvidia.com/gpu-accelerated-texture-compression
echo Press any key to start, or Ctrl-C to abort.
pause

echo Processing files with alpha channels.

mkdir dds

for %%i IN (*.Texture) DO TextureTool.exe %%i --hasalpha
for %%i IN (*.jpg) DO TextureTool.exe %%i --hasalpha
for %%i IN (*.png) DO TextureTool.exe %%i --hasalpha
for %%i IN (*.bmp) DO TextureTool.exe %%i --hasalpha

echo Converting files with alpha to compressed DDS.
for %%i IN (*.dds) DO nvcompress -bc3 "%%i"
move *.dds dds

echo Processing files without alpha channels.
for %%i IN (*.Texture) DO TextureTool.exe %%i
for %%i IN (*.jpg) DO TextureTool.exe %%i
for %%i IN (*.png) DO TextureTool.exe %%i
for %%i IN (*.bmp) DO TextureTool.exe %%i

echo Converting files without alpha to compressed DDS.
for %%i IN (*.dds) DO nvcompress -bc1 "%%i"
move *.dds dds
