# This is the spec file for wxMathPlot
# Copyright (c) 2008, Davide Rondini
# RPM bulding reference:
# http://docs.fedoraproject.org/developers-guide/ch-rpm-building.html
# http://myy.helia.fi/~karte/linux/doc/rpm-build-as-user.html
# Thanks to Matthias Saou for his explanations on http://freshrpms.net/docs/fight.html

Name: wxMathPlot
Version:
Release: 2
Vendor: wxMathPlot team
License: wxWindows
Summary: 2D plot library for wxWidgets
Group: Applications/Development
Packager: Davide Rondini
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Source: %{name}-%{version}.tar.gz
# Since OpenSUSE has only wxGTK package, and wxGTK in Fedora depends on wxBase, wxBase is removed for higher compatibility
# Requires: wxBase wxGTK
Requires: wxGTK
BuildPrereq: cmake wxGTK-devel

%description
wxMathPlot is a library to add 2D scientific plot functionality to wxWidgets. It allows to embed inside your program a window for plotting scientific, statistical or mathematical data, with additions like legend or coordinate display in overlay.

%prep
%setup

%build

cmake -D CMAKE_INSTALL_PREFIX:STRING=%{buildroot}/usr -D GDB_DEBUG:BOOL=FALSE -D BUILD_NATIVE:BOOL=TRUE -D MATHPLOT_SHARED:STRING=TRUE -D WXMATHPLOT_BUILD_EXAMPLES:BOOL=FALSE .
make

%install
# rm -rf %{buildroot}
%makeinstall

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%package devel
Summary: 2D plot library for wxWidgets - Development files
Group: Applications/Development
# NOTE on arch: this will set architecture for only the devel package (which do not contain built files)
# This works on recent Fedora, but not on CentOS 5.x (And so Red Hat...), and the procedure is more complicated, explained here:
# http://lists.centos.org/pipermail/centos/2007-December/048261.html
BuildArch: noarch
%description devel
wxMathPlot is a library to add 2D scientific plot functionality to wxWidgets.
It allows to embed inside your program a window for plotting scientific, statistical or mathematical data, with additions like legend or coordinate display in overlay.
This is development package: header files and sample code.


%files
%defattr(-, root, root)
%{_libdir}/libmathplot.so

%files devel
%defattr(-, root, root)
%doc README
%doc Changelog
%{_includedir}/mathplot.h
%{_datadir}/wxMathPlot/samples/*
%{_datadir}/wxMathPlot/Doxyfile

%changelog


