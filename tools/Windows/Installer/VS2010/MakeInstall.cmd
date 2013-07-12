@rem Deploys Tundra x86 using the INSTALL project, so that you can run the NSIS install script.
@cd ..
@call MakeInstall.cmd "Visual Studio 10" -DINSTALL_BINARIES_ONLY:BOOL=ON
