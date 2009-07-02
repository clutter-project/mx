m4_define([nbtk_major], [0])
m4_define([nbtk_minor], [15])
m4_define([nbtk_micro], [0])

m4_define([nbtk_version], [nbtk_major.nbtk_minor.nbtk_micro])
m4_define([nbtk_api_version], [1.0])

# increase the interface age of 2 for each release
# set to 0 if the API changes
m4_define([nbtk_interface_age], [0])
m4_define([nbtk_binary_age], [m4_eval(100 * nbtk_minor + nbtk_micro)])

AC_INIT([nbtk], [nbtk_version])
AC_CONFIG_MACRO_DIR([build/autotools])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_HEADERS([config.h])

GETTEXT_PACKAGE=nbtk
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["$GETTEXT_PACKAGE"],
                   [Gettext domain name])
AM_GLIB_GNU_GETTEXT

AC_ISC_POSIX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_PATH_GLIB_2_0

NBTK_MAJOR_VERSION=nbtk_major
NBTK_MINOR_VERSION=nbtk_minor
NBTK_MICRO_VERSION=nbtk_micro
NBTK_VERSION=nbtk_version
NBTK_API_VERSION=nbtk_api_version
AC_SUBST(NBTK_MAJOR_VERSION)
AC_SUBST(NBTK_MINOR_VERSION)
AC_SUBST(NBTK_MICRO_VERSION)
AC_SUBST(NBTK_VERSION)
AC_SUBST(NBTK_API_VERSION)

m4_define([lt_current], [m4_eval(100 * nbtk_minor + nbtk_micro - nbtk_interface_age)])
m4_define([lt_revision], [nbtk_interface_age])
m4_define([lt_age], [m4_eval(nbtk_binary_age - nbtk_interface_age)])
NBTK_LT_CURRENT=lt_current
NBTK_LT_REV=lt_revision
NBTKLT_AGE=lt_age
NBTK_LT_VERSION="$NBTK_LT_CURRENT:$NBTK_LT_REV:$NBTK_LT_AGE"
NBTK_LT_LDFLAGS="-version-info $NBTK_LT_VERSION"

AC_SUBST(NBTK_LT_VERSION)
AC_SUBST(NBTK_LT_LDFLAGS)

dnl programs
dnl AM_PATH_GLIB_2_0 will find these for us
#AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
#AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

dnl = Enable debug level ===================================================

m4_define([debug_default],
          [m4_if(m4_eval(nbtk_minor % 2), [1], [yes], [minimum])])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                              [Enables on debugging @<:@default=debug_default@:>@])],
              [],
              [enable_debug=debug_default])

AS_CASE([$enable_debug],
        [yes],
        [
          test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
          NBTK_DEBUG_CFLAGS="-DNBTK_ENABLE_DEBUG"
        ],

        [no],
        [
          NBTK_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
        ],

        [minimum],
        [
          NBTK_DEBUG_CFLAGS="-DNBTK_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
        ],

        [AC_MSG_ERROR([Invalid value for --enable-debug])]
)

AC_SUBST(NBTK_DEBUG_CFLAGS)

dnl = Enable strict compiler flags =========================================
AC_ARG_ENABLE([maintainer-flags],
              [AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                              [Use strict compiler flags @<:@default=maintainer_flags_default@:>@])],
              [],
              [enable_maintainer_flags=no])

AS_IF([test "x$enable_maintainer_flags" = "xyes"],
      [
        AS_COMPILER_FLAGS([NBTK_MAINTAINER_CFLAGS],
                          ["-Werror -Wall -Wshadow -Wcast-align
                            -Wno-uninitialized -Wempty-body -Wformat-security
                            -Winit-self -Wmissing-declarations
                            -Wredundant-decls"])
      ],
      [
        AS_COMPILER_FLAGS([NBTK_MAINTAINER_CFLAGS], ["-Wall"])
      ]
)

AC_SUBST(NBTK_MAINTAINER_CFLAGS)

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
# GETTEXT_PACKAGE=nbtk-$(NBTK_API_VERSION)
# AC_SUBST(GETTEXT_PACKAGE)
# AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [GETTEXT package name])
# ALL_LINGUAS=""
# AM_GLIB_GNU_GETTEXT
# GLIB_DEFINE_LOCALEDIR(LOCALEDIR)
# IT_PROG_INTLTOOL([0.35.0])

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(NBTK, [clutter-0.9 >= 0.9.6 clutter-imcontext-0.1 libccss-1 gio-2.0])
PKG_CHECK_MODULES(GTK, [gtk+-2.0])

# check for gtk-doc
GTK_DOC_CHECK([1.9])

SHAVE_INIT([build/autotools], [enable])

AC_CONFIG_FILES([
        Makefile
        nbtk.pc
        nbtk-gtk.pc
        build/Makefile
        build/autotools/Makefile
        build/autotools/shave
        build/autotools/shave-libtool
        data/Makefile
        data/style/Makefile
        docs/Makefile
        docs/reference/Makefile
        docs/reference/libnbtk/Makefile
        docs/reference/libnbtk/version.xml
        examples/Makefile
        nbtk/Makefile
        nbtk/nbtk-version.h
        tests/Makefile
])

AC_OUTPUT
