@rem Builds and deploys Tundra x86 using the INSTALL project, so that you can run the NSIS install script.
@cd ..
@call MakeInstall.cmd "Visual Studio 9 2008" -DINSTALL_BINARIES_ONLY:BOOL=ON
