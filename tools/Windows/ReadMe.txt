This folder contains master scripts that should be invoked from slave scripts.
For build scripts specific to certain Visual Studio versions, see the VS<XXXX> folders.

BuildAll.cmd - Master script for building both dependencies and Tundra from command-line.
BuildDeps.cmd - Master script for building dependencies from command-line.
BuildTundraDebug.cmd - Script for building Debug Tundra from command-line.
BuildTundraRelWithdDebInfo.cmd - Script for building RelWithDebInfo Tundra from command-line.
DeployDeps.cmd - Script for deploying Tundra dependencies.
FetchPrebuiltDeps.cmd - Script for fetching prebuilt Tundra dependencies.
RunCMake.cmd - Master script for running CMake for Tundra.
VSConfig.cmd - Masdter script containing various utility variables used by build scripts.
CleanBuild.bat - Cleans up after possible previous Tundra build in order to guarantee a fresh build next time.
