This folder contains master scripts that should be invoked from slave scripts.
For build scripts specific to certain Visual Studio versions, see the VS<XXXX> folders.

Installer\                      Tools for creating an NSIS installer for Tundra.
Mods\                           File modifications required by the build scripts.
Utils\                          Utility functionality needed by the build scripts.

BuildAll.cmd                    Master script for building both dependencies and Tundra from command-line.
BuildDeps.cmd                   Master script for building dependencies from command-line.
BuildTundraDebug.cmd            Script for building Debug Tundra from command-line.
BuildTundraRelWithdDebInfo.cmd  Script for building RelWithDebInfo Tundra from command-line.
RunCMake.cmd                    Master script for running CMake for Tundra.
                                Should be called with a valid CMage generator string, f.ex.: 'RunCMake "Visual Studio 10 Win64"'.
VSConfig.cmd                    Master script containing various utility variables used by the build scripts.
                                Should be called from the build scripts with a valid CMage generator string, f.ex. 'VSConfig "Visual Studio 10 Win64"'.
CleanBuild.bat                  Cleans up after possible previous Tundra build in order to guarantee a fresh build next time.

DEPRECATED:
DeployDeps.cmd                  Script for deploying Tundra dependencies.
FetchPrebuiltDeps.cmd           Script for fetching prebuilt Tundra dependencies.
