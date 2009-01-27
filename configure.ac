AC_INIT(nbtk, 0.1)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
# GETTEXT_PACKAGE=nbtk
# AC_SUBST(GETTEXT_PACKAGE)
# AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
# AM_GLIB_GNU_GETTEXT
# IT_PROG_INTLTOOL([0.35.0])

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(NBTK, [clutter-0.9 libccss-1])

# check for gtk-doc
GTK_DOC_CHECK(1.9)


# Make libtool respect the make -s switch (kudos to jacob berman)
changequote(,)dnl
LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl


AC_OUTPUT([
Makefile
nbtk.pc
docs/Makefile
docs/reference/Makefile
docs/reference/libnbtk/Makefile
nbtk/Makefile
tests/Makefile
])
