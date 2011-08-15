Summary: Tundra viewer
Name: realXtend-Tundra
Version: 0.0
Release: fc13
License: GPL
Group: Applications/Internet
BuildRoot: /opt/realXtend/
Requires: boost-date-time, boost-filesystem, boost-thread, boost-regex , boost-program-options, poco-foundation, poco-net, poco-util, openal-soft, openjpeg-libs, boost-test, gnutls, qtscriptgenerator, qtscriptbindings, libogg, freetype, freeimage, zziplib, libXrandr, libXaw, freeglut
%description
Tundra is software to view and host interconnected 3D worlds.

%prep

%build

%install
mkdir -p $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/opt/realXtend/lib
mkdir -p $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/usr/
cp -r $HOME/naali-build/naali/bin/* $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/opt/realXtend/
rm -fr $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/opt/realXtend/scenes
cp -fr $HOME/naali-build/naali-deps/install/lib/* $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/opt/realXtend/lib
cp -fr $HOME/naali-build/usr_tundra/* $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/usr/

chmod 755 -R $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-0.0-fc13.x86_64/*

%clean
rm -rf $RPM_BUILD_ROOT

%files
/opt/realXtend/*
/usr/*
%defattr(-,root,root)
%dir /opt/realXtend/

%doc
/opt/realXtend/*

%changelog


%preun
/opt/realXtend/postunist
