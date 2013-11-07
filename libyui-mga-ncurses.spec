%define         git          5ded95a
%define         gitdate      20131106
%define         major        5
%define         libname      %mklibname yui %{major}-mga-ncurses
%define         develname    %mklibname -d yui-mga-ncurses

Name:           libyui-mga-ncurses
Version:        1.0.0
Release:        %mkrel -c git%{gitdate} 1
Summary:        UI abstraction library - Gtk plugin
License:        LGPLv2+
Group:          System/Libraries
Url:            git@bitbucket.org:_pmat_/libyui-mga-ncurses.git
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:    yui-devel
BuildRequires:    %{_lib}yui-ncurses-devel
BuildRequires:    %{_lib}yui-mga-devel
BuildRequires:    ncursesw-devel ncurses-devel
BuildRequires:    cmake
BuildRequires:    boost-devel
BuildRequires:    doxygen
BuildRequires:    texlive
BuildRequires:    ghostscript
Requires:         libyui
Requires:         libyui-mga
Requires:         libyui-ncurses

%description
%{summary}

#-----------------------------------------------------------------------

%package -n %libname
Summary:        %{summary}
Group:          System/Libraries
Requires:       libyui
Provides:       %{name} = %{version}-%{release}

%description -n %libname
This package contains the library needed to run programs
dynamically linked with libyui-mga-ncurses.

%files -n %libname
%doc COPYING*
%{_libdir}/yui/lib*.so.*


#-----------------------------------------------------------------------

%package -n %develname
Summary:        %{summary} header files
Group:          Development/Other
Requires:       libyui-devel
Requires:       %{name} = %{version}-%{release}


%description -n %develname
This package provides headers files for libyui-mga-ncurses development.

%files -n %develname
%{_includedir}/yui
%{_libdir}/yui/lib*.so
%{_libdir}/pkgconfig/libyui-mga-ncurses.pc
%{_libdir}/cmake/libyui-mga-ncurses
%doc %{_docdir}/libyui-mga-ncurses%{major}

#-----------------------------------------------------------------------

%prep
%setup -q -n %{name}-%{version}
%apply_patches

%build
./bootstrap.sh
%cmake -DPREFIX=%{_prefix}  \
       -DDOC_DIR=%{_docdir} \
       -DLIB_DIR=%{_lib}    \
       -DINSTALL_DOCS=yes


%make docs

%install
rm -rf %{buildroot}
%makeinstall_std -C build
find "%{buildroot}" -name "*.la" -delete
