This folder contains 3rd party installers, tools and scripts for creating an NSIS Windows installer for Tundra.

VS2008\MakeBuild.bat        - Makes a build folder for VS 2008 SP1 x86 build of Tundra.
VS2008\MakeBuildX64.bat     - Makes a build folder for VS 2008 SP1 x64 build of Tundra.
VS2008\vcredist_x64.exe     - Installer for Microsoft Visual C++ 2008 SP1 Redistributable Package (x64).
VS2008\vcredist_x86.exe     - Installer for Microsoft Visual C++ 2008 SP1 Redistributable Package (x86).
VS2010\MakeBuild.bat        - Makes a build folder for VS 2010 SP1 x86 build of Tundra.
VS2010\MakeBuildX64.bat     - Makes a build folder for VS 2010 SP1 x64 build of Tundra.
VS2010\vcredist_x64.exe     - Installer for Microsoft Visual C++ 2010 SP1 Redistributable Package (x64).
VS2010\vcredist_x86.exe     - Installer for Microsoft Visual C++ 2010 SP1 Redistributable Package (x86).

dxwebsetup.exe              - Web installer for Microsoft DirectX SDK.
fileassoc.nsh               - Utility script used by the tundra-installer.nsi.
MakeClean.bat               - Cleans up the build folder from typically unneeded files (debug DLLs etc.).
oalinst.exe                 - Installer for OpenAL.
RunReinstall.bat            - Reinstalls Tundra, performs silent uninstall. Note
TundraExtHandler.reg        - Creates Windows context menu items for opening up Tundra scene files in Tundra release build.
TundraExtHandlerDebug.reg   - Creates Windows context menu items for opening up Tundra scene files in Tundra debug build.
tundra-installer.nsi        - NSIS script for making a Tundra installer. Make sure you have run the right MakeBuild script before running this.
