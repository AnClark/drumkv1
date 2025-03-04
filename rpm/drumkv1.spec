#
# spec file for package drumkv1
#
# Copyright (C) 2012-2023, rncbc aka Rui Nuno Capela. All rights reserved.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define name    drumkv1
%define version 0.9.28
%define release 69.2

%define _prefix	/usr

%if %{defined fedora}
%define debug_package %{nil}
%endif

%if 0%{?fedora_version} >= 34 || 0%{?suse_version} > 1500 || ( 0%{?sle_version} == 150200 && 0%{?is_opensuse} )
%define qt_major_version  6
%else
%define qt_major_version  5
%endif

Summary:	An old-school drum-kit sampler
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	GPL-2.0+
Group:		Productivity/Multimedia/Sound/Midi
Source0:	%{name}-%{version}.tar.gz
URL:		http://drumkv1.sourceforge.net
Packager:	rncbc.org

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires:	coreutils
BuildRequires:	pkgconfig
BuildRequires:	glibc-devel
BuildRequires:	cmake >= 3.15
%if 0%{?sle_version} >= 150200 && 0%{?is_opensuse}
BuildRequires:	gcc10 >= 10
BuildRequires:	gcc10-c++ >= 10
%define _GCC	/usr/bin/gcc-10
%define _GXX	/usr/bin/g++-10
%else
BuildRequires:	gcc >= 10
BuildRequires:	gcc-c++ >= 10
%define _GCC	/usr/bin/gcc
%define _GXX	/usr/bin/g++
%endif
%if 0%{qt_major_version} == 6
BuildRequires:	qtbase6-static >= 6.1
BuildRequires:	qttools6-static
BuildRequires:	qttranslations6-static
BuildRequires:	qtsvg6-static
BuildRequires:	qtwayland6-static
%else
BuildRequires:	pkgconfig(Qt5Core)
BuildRequires:	pkgconfig(Qt5Gui)
BuildRequires:	pkgconfig(Qt5Widgets)
BuildRequires:	pkgconfig(Qt5Xml)
BuildRequires:	pkgconfig(Qt5Svg)
%endif
%if %{defined fedora}
BuildRequires:	jack-audio-connection-kit-devel
BuildRequires:	alsa-lib-devel
%else
BuildRequires:	libjack-devel
BuildRequires:	alsa-devel
%endif
BuildRequires:	libsndfile-devel
BuildRequires:	liblo-devel
BuildRequires:	lv2-devel

BuildRequires:	pkgconfig(egl)

%description
  An old-school all-digital drum-kit sampler synthesizer with stereo fx.


%package -n %{name}-jack
Summary:	An old-school drum-kit sampler - JACK standalone
Provides:	%{name}_jack
Obsoletes:	%{name}-common <= %{version}, %{name} <= %{version}

%description -n %{name}-jack
  An old-school all-digital drum-kit sampler synthesizer with stereo fx.
  .
  This package provides the standalone JACK client application (drumkv1_jack)


%package -n %{name}-lv2
Summary:	An old-school drum-kit sampler - LV2 plugin
Provides:	%{name}_lv2, %{name}_lv2ui
Obsoletes:	%{name}-common <= %{version}

%description -n %{name}-lv2
  An old-school all-digital drum-kit sampler synthesizer with stereo fx.
  .
  This package provides the LV2 plugin (http://drumkv1.sourceforge.net/lv2)


%prep
%setup -q

%build
%if 0%{qt_major_version} == 6
source /opt/qt6.4-static/bin/qt6.4-static-env.sh
%endif
CXX=%{_GXX} CC=%{_GCC} \
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -Wno-dev -B build
cmake --build build %{?_smp_mflags}

%install
DESTDIR="%{buildroot}" \
cmake --install build

%clean
[ -d "%{buildroot}" -a "%{buildroot}" != "/" ] && %__rm -rf "%{buildroot}"


%files -n %{name}-jack
%defattr(-,root,root)
%doc README LICENSE ChangeLog
#dir %{_datadir}/applications
%dir %{_datadir}/metainfo
#dir %{_datadir}/mime
#dir %{_datadir}/mime/packages
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/hicolor/32x32
%dir %{_datadir}/icons/hicolor/32x32/apps
%dir %{_datadir}/icons/hicolor/32x32/mimetypes
%dir %{_datadir}/icons/hicolor/scalable
%dir %{_datadir}/icons/hicolor/scalable/apps
%dir %{_datadir}/icons/hicolor/scalable/mimetypes
#dir %{_datadir}/man
#dir %{_datadir}/man/man1
#dir %{_datadir}/man/fr
#dir %{_datadir}/man/fr/man1
%{_bindir}/%{name}_jack
%{_datadir}/metainfo/org.rncbc.%{name}.metainfo.xml
%{_datadir}/applications/org.rncbc.%{name}.desktop
%{_datadir}/mime/packages/org.rncbc.%{name}.xml
%{_datadir}/icons/hicolor/32x32/apps/org.rncbc.%{name}.png
%{_datadir}/icons/hicolor/scalable/apps/org.rncbc.%{name}.svg
%{_datadir}/icons/hicolor/32x32/mimetypes/org.rncbc.%{name}.application-x-%{name}*.png
%{_datadir}/icons/hicolor/scalable/mimetypes/org.rncbc.%{name}.application-x-%{name}*.svg
%{_datadir}/man/man1/%{name}.1.gz
%{_datadir}/man/fr/man1/%{name}.1.gz

%files -n %{name}-lv2
%defattr(-,root,root)
%dir %{_libdir}/lv2
%dir %{_libdir}/lv2/%{name}.lv2
%{_libdir}/lv2/%{name}.lv2/manifest.ttl
%{_libdir}/lv2/%{name}.lv2/%{name}.ttl
%{_libdir}/lv2/%{name}.lv2/%{name}.so
%{_libdir}/lv2/%{name}.lv2/%{name}_ui.ttl


%changelog
* Thu Dec 29 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.28
- An End-of-Year'22 Release.
* Tue Oct  4 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.27
- An Early-Autumn'22 Release.
* Tue Jun  7 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.26
- An End-of-Spring'22 Release.
* Thu Apr  7 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.25
- A Spring'22 Release.
* Sun Jan  9 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.24
- A Winter'22 Release.
* Wed Jul  7 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.23
- An Early-Summer'21 Release.
* Thu May 13 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.22
- A Spring'21 Release.
* Tue Mar 16 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.21
- An End-of-Winter'21 Release.
* Wed Feb 10 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.20
- A Winter'21 Release.
* Sat Dec 19 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.19
- A Winter'20 Release.
* Tue Oct 27 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.18
- A Fall'20 Release.
* Tue Sep  8 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.17
- A Late Summer'20 Release.
* Thu Aug  6 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.16
- A Summer'20 Release.
* Mon Jun 22 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.15
- An Early-Summer'20 Release.
* Tue May  5 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.14
- A Mid-Spring'20 Release.
* Thu Mar 26 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.13
- A Spring'20 Release.
* Thu Dec 26 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.12
- The Winter'19 Release.
* Thu Oct 31 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.11
- A Halloween'19 Release.
* Thu Oct  3 2019 Rui Nuno Capela <rncbc@rncbc.org>
- [xstatic] Prepared for qtbase5-static build.
* Tue Sep 24 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.10
- An Early-Fall'19 release.
* Thu Jul 18 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.9
- A Summer'19 release.
* Thu Jun  6 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.8
- A Spring'19 release.
* Sun Apr 14 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.7
- A Spring-Break'19 release.
* Mon Mar 18 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.6
- Pre-LAC2019 release frenzy.
* Mon Mar  4 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.9.5
- The End of Winter'19 release.
* Wed Dec 12 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.9.4
- A Late Autumn'18 release.
* Mon Oct 22 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.9.3
- An Autumn'18 release.
* Tue Jul 24 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.9.2
- A Summer'18 release.
* Tue Jun 26 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.9.1
- An Early Summer'18 release.
* Wed Mar  7 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.9.0
- The End of Winter'18 release.
* Wed Dec 20 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.6
- The End of Autumn'17 release.
* Sun Oct 29 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.5
- An Autumn'17 release.
* Tue Aug 22 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.4
- A Late-Summer'17 release.
* Wed Jun 21 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.3
- A Summer'17 release.
* Tue May  2 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.2
- Pre-LAC2017 release frenzy.
* Tue Mar 21 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.8.1
- A Spring'17 release.
* Thu Nov 17 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.8.0
- A Fall'16 release.
* Mon Sep 19 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.7.6
- The eleventh official beta.
* Thu Jun 16 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.7.5
- The tenth official beta.
* Wed Mar  2 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.7.4
- The ninth-bis official beta.
* Mon Feb 22 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.7.3
- The ninth official beta.
* Fri Dec  4 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.7.2
- The eighth official beta.
* Mon Aug 24 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.7.1
- A seventh-bis official beta.
* Fri Jul 24 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.7.0
- A seventh official beta.
* Sat May  9 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.6.3
- A sixth official beta.
* Thu Apr 30 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.6.2
- A fifth official beta.
* Mon Mar  9 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.6.1
- A fourth official beta.
* Fri Jan 23 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.6.0
- A third official beta.
* Fri Sep 12 2014 Rui Nuno Capela <rncbc@rncbc.org> 0.5.1
- One second official beta.
* Mon Jun 30 2014 Rui Nuno Capela <rncbc@rncbc.org> 0.5.0
- Officially beta now.
* Tue Apr 29 2014 Rui Nuno Capela <rncbc@rncbc.org> 0.4.2
- A pre-LAC frenzy beta.
* Mon Apr  7 2014 Rui Nuno Capela <rncbc@rncbc.org> 0.4.1
- A proto-beta bis.
* Thu Mar  6 2014 Rui Nuno Capela <rncbc@rncbc.org> 0.4.0
- A proto-beta party.
* Tue Dec 31 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.6
- A fifth of a Jubilee release.
* Mon Sep 30 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.5
- Fall greetings release.
* Tue Jul 16 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.4
- Brand new icon ready.
* Fri May 31 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.3
- Late spring blossoming.
* Fri Mar  1 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.2
- One third bug-fix release.
* Fri Feb  8 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.1
- One second bug-fix release.
* Fri Dec 21 2012 Rui Nuno Capela <rncbc@rncbc.org> 0.3.0
- Third coming release.
* Thu Nov 22 2012 Rui Nuno Capela <rncbc@rncbc.org> 0.2.0
- Second public release.
* Thu Oct 25 2012 Rui Nuno Capela <rncbc@rncbc.org> 0.1.0
- First public release.
* Wed Oct  3 2012 Rui Nuno Capela <rncbc@rncbc.org> 0.0.0
- Created initial drumkv1.spec
