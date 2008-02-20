AC_INIT(firefox-apoc-adapter, 1.0)

AM_INIT_AUTOMAKE

AC_PROG_CXX
AC_PROG_LIBTOOL

#Checking and including XPCOM
PKG_CHECK_MODULES(XPCOM, firefox-xpcom,
[
        FIREFOX="yes"
	CXXFLAGS="$CXXFLAGS $XPCOM_CFLAGS -DMOZILLA_INTERNAL_API"
	LIBS="$LIBS $XPCOM_LIBS"
])

XPCOM_LIBDIR=`pkg-config --variable=libdir firefox-xpcom`
COMPONENT_DIR=$XPCOM_LIBDIR/components

#Checking NSPR headers
NSPR_INCLUDEDIR=""
AC_ARG_WITH(nspr-includes,
AC_HELP_STRING( [--with-nspr-includes=DIR],
		[specify the Netscape Portable Runtime include path]),
[
	if test "$withval" != "no"
	then
		CFLAGS="$CFLAGS -I$withval"
		CXXFLAGS="$CXXFLAGS -I$withval"
	fi
])

AC_CHECK_HEADERS([nspr.h], [],
AC_MSG_ERROR(Netscape Portable Runtime headers not found.
If you have it already installed please use --with-nspr-includes option to specify the correct include path.)
)
AC_CHECK_HEADERS([prtypes.h], [],
AC_MSG_ERROR(Netscape Portable Runtime headers not found.
If you have it already installed please use --with-nspr-includes option to specify the correct include path.)
)

AC_SUBST(XPCOM_LIBDIR)
AC_SUBST(COMPONENT_DIR)

AC_OUTPUT([
Makefile
src/Makefile
])
