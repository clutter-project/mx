m4_define([mx_major], [1])
m4_define([mx_minor], [1])
m4_define([mx_micro], [0])

m4_define([mx_version], [mx_major.mx_minor.mx_micro])
m4_define([mx_api_version], [1.0])

# increase the interface age of 2 for each release
# set to 0 if the API changes
m4_define([mx_interface_age], [0])
# binary compatibility changed at interface 2, so the binary age needs to
# reflect this
m4_define([mx_abi_offset], [2])
m4_define([mx_binary_age], [m4_eval(100 * mx_minor + mx_micro - mx_abi_offset)])

AC_INIT([mx], [mx_version])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

# enable quiet ("silent") builds, if available
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

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
        AS_COMPILER_FLAGS([MX_MAINTAINER_CFLAGS],
                          ["-Wall"])
      ]
)

AC_SUBST(MX_MAINTAINER_CFLAGS)


dnl = Required Packages ====================================================

MX_REQUIRES="clutter-x11-1.0 >= 1.2.0 xrandr >= 1.2.0"

dnl = Optional Packages ====================================================

AC_ARG_WITH([clutter-imcontext],
            [AC_HELP_STRING([--without-clutter-imcontext],
                            [disable input method support])],
            [],
            [with_clutter_imcontext=auto])

AS_IF([test "x$with_clutter_imcontext" != xno],
      [PKG_CHECK_EXISTS([clutter-imcontext-0.1],
                        [AC_DEFINE([HAVE_CLUTTER_IMCONTEXT], [1],
                                   [Define you have clutter-imcontext])
                         MX_REQUIRES="$MX_REQUIRES clutter-imcontext-0.1"
                         with_clutter_imcontext=yes],
                        [AS_IF([test "x$with_clutter_imcontext" = xyes],AC_MSG_FAILURE([Could not find clutter-imcontext. Use --without-clutter-imcontext to disable input method support.]))
                        with_clutter_imcontext=no]]
      ))


AC_ARG_WITH([clutter-gesture],
            [AC_HELP_STRING([--with-clutter-gesture],
                            [enable gesture support])],
            [],
            [with_clutter_gesture=auto])

AS_IF([test "x$with_clutter_gesture" != xno],
      [PKG_CHECK_EXISTS([clutter-gesture],
                        [AC_DEFINE([HAVE_CLUTTER_GESTURE], [1],
                                   [Define if clutter-gesture is available])
                         MX_REQUIRES="$MX_REQUIRES clutter-gesture"
                         with_clutter_gesture=yes],
                        [AS_IF([test "x$with_clutter_gesture" = xyes],
                                AC_MSG_FAILURE([Could not find clutter-gesture. Use --without-clutter-gesture to disable gesture support.]))
                        with_clutter_gesture=no]]
       ))

AC_ARG_WITH([startup-notification],
            [AC_HELP_STRING([--without-startup-notification],
                            [disable startup notification])],
            [],
            [with_startup_notification=auto])

AS_IF([test "x$with_startup_notification" != xno],
      [PKG_CHECK_EXISTS([libstartup-notification-1.0 >= 0.9],
                        [AC_DEFINE([HAVE_STARTUP_NOTIFICATION], [1],
                                   [Define if startup notification is enabled])
                        MX_REQUIRES="$MX_REQUIRES libstartup-notification-1.0 >= 0.9"
                        with_startup_notification=yes],
                        [AS_IF([test "x$with_startup_notification" = xyes],
                               AC_MSG_FAILURE([Could not find libstartup-notification. Use --without-startup-notification to disable startup notification support.]))
                        with_startup_notification=no]]
      ))

AC_ARG_WITH([dbus],
            [AC_HELP_STRING([--without-dbus],
                            [disable DBus support])],
            [],
            [with_dbus=auto])

AS_IF([test "x$with_dbus" != xno],
      [PKG_CHECK_EXISTS([dbus-glib-1 >= 0.82],
                        [AC_DEFINE([HAVE_DBUS], [1],
                                   [Define if DBus support is enabled])
                        MX_REQUIRES="$MX_REQUIRES dbus-glib-1 >= 0.82"
                        with_dbus=yes],
                        [AS_IF([test "x$with_debug" = xyes],
                               AC_MSG_FAILURE([Could not find dbus-glib. Use --without-dbus to disable DBus support.]))
                        with_dbus=no]]
      ))

AC_ARG_WITH([glade],
            [AC_HELP_STRING([--with-glade],
                            [enable glade3 support])],
            [with_glade=yes],
            [])

AS_IF([test "x$with_glade" = xyes],
      [PKG_CHECK_EXISTS([gladeui-1.0 >= 3.4.5],
                        [PKG_PROG_PKG_CONFIG()
                         catalogdir=`${PKG_CONFIG} --variable=catalogdir gladeui-1.0`],
                        [AC_MSG_FAILURE([Could not find gladeui-1.0 >= 3.4.5])]]
      ))

AM_CONDITIONAL(WITH_GLADE, test "x$with_glade" = xyes)
AC_SUBST([catalogdir])


#
# Gtk+ widgets library
#

AC_ARG_ENABLE([gtk-widgets],
              [AC_HELP_STRING([--disable-gtk-widgets],
                              [disable building the gtk+ widgets library])],
              [],
              [enable_gtk_widgets=yes])

AM_CONDITIONAL(ENABLE_GTK_WIDGETS, test "x$enable_gtk_widgets" = xyes)

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
AS_ALL_LINGUAS
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE="mx-$MX_API_VERSION"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["$GETTEXT_PACKAGE"],
                   [Gettext domain name])
GLIB_DEFINE_LOCALEDIR(LOCALEDIR)
AM_GLIB_GNU_GETTEXT


AM_PROG_LIBTOOL

PKG_CHECK_MODULES(MX, [$MX_REQUIRES])
PKG_CHECK_MODULES(GTK, [gtk+-2.0])

# check for gtk-doc
GTK_DOC_CHECK([1.14])

GOBJECT_INTROSPECTION_CHECK([0.6.4])

AC_CONFIG_FILES([
        Makefile
        mx.pc
        mx-gtk.pc
        data/Makefile
        data/glade/Makefile
        data/style/Makefile
        docs/Makefile
        docs/reference/Makefile
        docs/reference/libmx/Makefile
        docs/reference/libmx/version.xml
        mx/Makefile
        mx-gtk/Makefile
        mx/mx-version.h
        po/Makefile.in
        tests/Makefile
])

AC_OUTPUT

dnl === Summary ===

echo ""
echo " Mx Toolkit - $VERSION"
echo ""
echo " Features:"
echo "   Clutter-Imcontext:    $with_clutter_imcontext"
echo "   Clutter-Gesture:      $with_clutter_gesture"
echo "   Startup Notification: $with_startup_notification"
echo "   Dbus:                 $with_dbus"
echo ""
