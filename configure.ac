m4_define([mx_major], [1])
m4_define([mx_minor], [1])
m4_define([mx_micro], [4])

m4_define([mx_version], [mx_major.mx_minor.mx_micro])
m4_define([mx_api_version], [1.2])

# increase the interface age of 2 for each release
# set to 0 if the API changes
m4_define([mx_interface_age], [0])
m4_define([mx_binary_age], [m4_eval(100 * mx_minor + mx_micro)])

AC_INIT([mx], [mx_version])
AC_CONFIG_MACRO_DIR([build/autotools])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_HEADERS([config.h])

AC_ISC_POSIX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_PATH_GLIB_2_0
AC_CHECK_FUNCS([localtime_r])

MX_MAJOR_VERSION=mx_major
MX_MINOR_VERSION=mx_minor
MX_MICRO_VERSION=mx_micro
MX_VERSION=mx_version
MX_API_VERSION=mx_api_version
AC_SUBST(MX_MAJOR_VERSION)
AC_SUBST(MX_MINOR_VERSION)
AC_SUBST(MX_MICRO_VERSION)
AC_SUBST(MX_VERSION)
AC_SUBST(MX_API_VERSION)

m4_define([lt_current], [m4_eval(100 * mx_minor + mx_micro - mx_interface_age)])
m4_define([lt_revision], [mx_interface_age])
m4_define([lt_age], [m4_eval(mx_binary_age - mx_interface_age)])
MX_LT_CURRENT=lt_current
MX_LT_REV=lt_revision
MX_LT_AGE=lt_age
MX_LT_VERSION="$MX_LT_CURRENT:$MX_LT_REV:$MX_LT_AGE"
MX_LT_LDFLAGS="-version-info $MX_LT_VERSION"

AC_SUBST(MX_LT_VERSION)
AC_SUBST(MX_LT_LDFLAGS)

dnl programs
dnl AM_PATH_GLIB_2_0 will find these for us
#AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
#AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

dnl = Enable debug level ===================================================

m4_define([debug_default],
          [m4_if(m4_eval(mx_minor % 2), [1], [yes], [minimum])])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                              [Enables on debugging @<:@default=debug_default@:>@])],
              [],
              [enable_debug=debug_default])

AS_CASE([$enable_debug],
        [yes],
        [
          test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
          MX_DEBUG_CFLAGS="-DMX_ENABLE_DEBUG"
        ],

        [no],
        [
          MX_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
        ],

        [minimum],
        [
          MX_DEBUG_CFLAGS="-DMX_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
        ],

        [AC_MSG_ERROR([Invalid value for --enable-debug])]
)

AC_SUBST(MX_DEBUG_CFLAGS)

dnl = Enable strict compiler flags =========================================
AC_ARG_ENABLE([maintainer-flags],
              [AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                              [Use strict compiler flags @<:@default=maintainer_flags_default@:>@])],
              [],
              [enable_maintainer_flags=no])

AS_IF([test "x$enable_maintainer_flags" = "xyes"],
      [
        AS_COMPILER_FLAGS([MX_MAINTAINER_CFLAGS],
                          ["-Werror -Wall -Wshadow -Wcast-align
                            -Wno-uninitialized -Wempty-body -Wformat-security
                            -Winit-self -Wmissing-declarations
                            -Wredundant-decls"])
      ],
      [
        AS_COMPILER_FLAGS([MX_MAINTAINER_CFLAGS], ["-Wall"])
      ]
)

AC_SUBST(MX_MAINTAINER_CFLAGS)

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
GETTEXT_PACKAGE="mx-$MX_API_VERSION"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [GETTEXT package name])
ALL_LINGUAS="`grep -v '^#' "$srcdir/po/LINGUAS" | tr '\n' ' '`"
AM_GLIB_GNU_GETTEXT
GLIB_DEFINE_LOCALEDIR(LOCALEDIR)
# IT_PROG_INTLTOOL([0.35.0])

AM_PROG_LIBTOOL

MX_REQUIRES="clutter-1.0 clutter-imcontext-0.1 libccss-1 = 0.3.1"
PKG_CHECK_MODULES(MX, [$MX_REQUIRES])
PKG_CHECK_MODULES(GTK, [gtk+-2.0])

# check for gtk-doc
GTK_DOC_CHECK([1.9])

GOBJECT_INTROSPECTION_CHECK([0.6.4])

SHAVE_INIT([build/autotools], [enable])

AC_CONFIG_FILES([
        Makefile
        mx.pc
        mx-gtk.pc
        build/Makefile
        build/autotools/Makefile
        build/autotools/shave
        build/autotools/shave-libtool
        data/Makefile
        data/style/Makefile
        docs/Makefile
        docs/reference/Makefile
        docs/reference/libmx/Makefile
        docs/reference/libmx/version.xml
        examples/Makefile
        mx/Makefile
        mx/mx-version.h
        po/Makefile.in
        tests/Makefile
])

AC_OUTPUT
