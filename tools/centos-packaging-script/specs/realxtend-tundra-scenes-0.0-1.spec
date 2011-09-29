Summary: Tundra viewer scenes
Name: realXtend-Tundra-Scenes
Version: 0.0
Release: 1
License: GPL
Group: Applications/Internet
Source: naali-latest.tar.gz
Patch: realXtend-Tundra.patch
BuildRoot: /opt/realXtend/

%description
Tundra is software to view and host interconnected 3D worlds.

%prep

%build

%install
mkdir -p $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-Scenes-0.0-1.x86_64/opt/realXtend/
cp -r $HOME/naali-build/naali/bin/scenes $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-Scenes-0.0-1.x86_64/opt/realXtend/

chmod 755 -R $HOME/rpmbuild/BUILDROOT/realXtend-Tundra-Scenes-0.0-1.x86_64/*

%clean
#rm -rf $RPM_BUILD_ROOT

%files
/opt/realXtend/*
%defattr(-,root,root)
%dir /opt/realXtend/

%doc
/opt/realXtend/*

%changelog


