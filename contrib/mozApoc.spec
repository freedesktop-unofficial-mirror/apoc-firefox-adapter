#
# Copyright (c) 2003 Sun Microsystems Inc.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
%define ADAPTER_VERSION 0.3
%define ADAPTER_RELEASE 2
%define MOZILLA_VERSION 1.4
%define MOZILLA_LOCATION /usr/lib
%define MOZILLA_ROOT %{MOZILLA_LOCATION}/mozilla-%{MOZILLA_VERSION}
%define _prefix %{MOZILLA_ROOT}
Summary: 	    APOC adapter for Mozilla
Name: 		    apoc-adapter-mozilla
Group: 		    Applications/Internet 
Version: 	    %{ADAPTER_VERSION}
Release: 	    %{ADAPTER_RELEASE}
Distribution: 	Sun Java Desktop System
Vendor: 	    Sun Microsystems Inc.
License: 	    Sun Microsystems Binary Code License (BCL) 
Autoreqprov:  	on
Requires: 	    mozilla = %{MOZILLA_VERSION}
Prefix:         %{_prefix}
BuildRoot:      %{_tmppath}/%{name}-%{version}-root

%description 
APOC adapter for Mozilla 1.4.

Provides mandatory and default preference values 
for mozilla from APOC policies.

Part of the Sun Java Desktop System

Property of Sun Microsystems Inc., 2003

%prep 

%build 
if test "$MOZ_OBJDIR" ; then
    cd $MOZ_OBJDIR/extensions/apoc
    make
fi

%install
if test "$MOZ_OBJDIR" ; then
    mkdir -p ${RPM_BUILD_ROOT}%{MOZILLA_ROOT}/components
    cp $MOZ_OBJDIR/dist/bin/components/libmozapoc.so ${RPM_BUILD_ROOT}%{MOZILLA_ROOT}/components/
fi

%clean
if test "$MOZ_OBJDIR" ; then
    rm -rf ${RPM_BUILD_ROOT}%MOZILLA_ROOT/components/libmozapoc.so
fi

%post
# Register our component for XPCOM on each install/update
MOZILLA_ROOT=$RPM_INSTALL_PREFIX
$MOZILLA_ROOT/regxpcom -x $MOZILLA_ROOT $MOZILLA_ROOT/components/libmozapoc.so

%postun
# Is this the last uninstall -> recreate registry from scratch
if [ $1 = 0 ] ; then
    MOZILLA_ROOT=$RPM_INSTALL_PREFIX
    $MOZILLA_ROOT/mozilla-rebuild-databases.pl
fi

%files 
%defattr (-, root, root)
%{_prefix}/components/libmozapoc.so

%changelog
