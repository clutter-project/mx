m4_define([mx_major], [1])
m4_define([mx_minor], [4])
m4_define([mx_micro], [7])

m4_define([mx_version], [mx_major.mx_minor.mx_micro])
m4_define([mx_api_version], [mx_major.0])
m4_define([mx_api_version_am], [mx_major\_0])

# increase the interface age of 2 for each release
# set to 0 if the API changes
m4_define([mx_interface_age], [0])
# binary compatibility changed at interface 2, so the binary age needs to
# reflect this
m4_define([mx_abi_offset], [2])
m4_define([mx_binary_age], [m4_eval(100 * mx_minor + mx_micro - mx_abi_offset)])

AC_INIT([mx], [mx_version])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([mx/mx.h])
AM_INIT_AUTOMAKE([1.11 foreign -Wno-portability no-define no-dist-gzip dist-xz])

# enable quiet ("silent") builds, if available
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])

AC_ISC_POSIX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_PATH_GLIB_2_0([2.28.0])
AC_CHECK_FUNCS([localtime_r])

MX_MAJOR_VERSION=mx_major
MX_MINOR_VERSION=mx_minor
MX_MICRO_VERSION=mx_micro
MX_VERSION=mx_version
MX_API_VERSION=mx_api_version
MX_API_VERSION_AM=mx_api_version_am
AC_SUBST(MX_MAJOR_VERSION)
AC_SUBST(MX_MINOR_VERSION)
AC_SUBST(MX_MICRO_VERSION)
AC_SUBST(MX_VERSION)
AC_SUBST(MX_API_VERSION)
AC_SUBST(MX_API_VERSION_AM)

m4_define([lt_current], [m4_eval(100 * mx_minor + mx_micro - mx_interface_age)])
m4_define([lt_revision], [mx_interface_age])
m4_define([lt_age], [m4_eval(mx_binary_age - mx_interface_age)])
MX_LT_CURRENT=lt_current
MX_LT_REV=lt_revision
MX_LT_AGE=lt_age
MX_LT_VERSION="$MX_LT_CURRENT:$MX_LT_REV:$MX_LT_AGE"
MX_LT_LDFLAGS="-no-undefined -version-info $MX_LT_VERSION"

AC_SUBST(MX_LT_VERSION)
AC_SUBST(MX_LT_LDFLAGS)

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
                          ["-Wall -Wshadow -Wcast-align
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

MX_REQUIRES="gdk-pixbuf-2.0"
CLUTTER_VERSION_REQUIRED="1.7.91"

dnl = Winsys ===============================================================

MX_WINSYS=x11
AC_ARG_WITH([winsys],
            [AC_HELP_STRING([--with-winsys=@<:@none/x11@:>@],
                            [Select the window system backend])],
            [MX_WINSYS=$with_winsys])

AS_CASE([$MX_WINSYS],
        [x11],
        [
          SUPPORT_X11=yes

          WINSYS_X11_REQUIRES="clutter-x11-1.0 >= $CLUTTER_VERSION_REQUIRED xrandr >= 1.2.0"
	  MX_REQUIRES="$MX_REQUIRES $WINSYS_X11_REQUIRES"
        ],
        [none],
        [
          WINSYS_NONE_REQUIRES="clutter-1.0 >= $CLUTTER_VERSION_REQUIRED"
	  MX_REQUIRES="$MX_REQUIRES $WINSYS_NONE_REQUIRES"
        ],
        [AC_MSG_ERROR([Invalid winsys: use 'x11' or 'none'])]
)

AS_IF([test "x$SUPPORT_X11" = "xyes"],
      [AC_DEFINE([HAVE_X11], [1], [Mx supports the X11 window system])])
AM_CONDITIONAL([HAVE_X11], [test "x$SUPPORT_X11" = "xyes"])
AC_SUBST(MX_WINSYS)

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
            [],
            [with_glade=no])

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

# If no parameter given default to yes otherwise use the enable value
# --enable-widgets == yes and --disable-widgets == no
AC_ARG_ENABLE([gtk-widgets],
              [AC_HELP_STRING([--disable-gtk-widgets],
                              [disable building the gtk+ widgets library])],
              [],
              [enable_gtk_widgets=yes])


AS_IF([test "x$enable_gtk_widgets" = "xyes"],
      [PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.20])
       GTK_CFLAGS="$GTK_CFLAGS -DGSEAL_ENABLE=1"],
      []
)

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
PKG_CHECK_MODULES(MX_IMAGE_CACHE, [gdk-pixbuf-2.0])

# check for gtk-doc

# gtkdocize greps for ^GTK_DOC_CHECK and parses it, so you need to have
# it on it's own line.
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14], [--flavour no-tmpl])
])

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
        docs/reference/libmx-gtk/Makefile
        docs/reference/libmx-gtk/version.xml
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
echo "   GTK+ widgets:         $enable_gtk_widgets"
echo "   Clutter-Imcontext:    $with_clutter_imcontext"
echo "   Clutter-Gesture:      $with_clutter_gesture"
echo "   Startup Notification: $with_startup_notification"
echo "   Dbus:                 $with_dbus"
echo "   Windowing system:     $MX_WINSYS"
echo ""
echo " Documentation:"
echo "   Build API Reference:  $enable_gtk_doc"
echo ""
echo " Extra:"
echo "   Build introspection data: $enable_introspection"
echo ""
