NULL =

source_gtk_h = \
	$(top_srcdir)/mx-gtk/mx-gtk-frame.h 		\
	$(top_srcdir)/mx-gtk/mx-gtk-light-switch.h 	\
	$(NULL)

source_gtk_c = \
	$(top_srcdir)/mx-gtk/mx-gtk-frame.c 		\
	$(top_srcdir)/mx-gtk/mx-gtk-light-switch.c 	\
	$(NULL)

mxincludedir = $(includedir)/mx-$(MX_API_VERSION)/mx-gtk
mxinclude_DATA = 				\
	$(source_gtk_h)				\
	$(top_srcdir)/mx-gtk/mx-gtk.h 		\
	$(NULL)

lib_LTLIBRARIES = libmx-gtk-@MX_API_VERSION@.la

#
# Mx GTK+ Library
#
# This is a seperate library with GTK+ widgets.
#

# libmx-gtk library

libmx_gtk_@MX_API_VERSION@_la_CFLAGS =	\
	$(common_includes)			\
	-DG_LOG_DOMAIN=\"MxGtk\"		\
	-DGTK_DISABLE_DEPRECATED		\
	$(MX_MAINTAINER_CFLAGS)		\
	$(MX_DEBUG_CFLAGS)			\
	$(GTK_CFLAGS)				\
	$(NULL)

libmx_gtk_@MX_API_VERSION@_la_LDFLAGS = $(GTK_LT_LDFLAGS) $(common_ldflags)

libmx_gtk_@MX_API_VERSION@_la_SOURCES =	\
	$(source_gtk_h)			\
	$(source_h_priv)		\
	$(source_gtk_c)			\
	mx-gtk.h 			\
	$(NULL)

libmx_gtk_@MX_API_VERSION@_la_LIBADD = $(GTK_LIBS)

CLEANFILES=

#
# Introspection Data
#

if HAVE_INTROSPECTION

BUILT_GIRSOURCES =

MxGtk-@MX_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libmx-gtk-@MX_API_VERSION@.la
	$(AM_V_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace MxGtk --nsversion=@MX_API_VERSION@ \
		$(common_includes) \
		--c-include="mx-gtk/mx-gtk.h" \
		--include=GObject-2.0 \
		--include=Gtk-2.0 \
		--library=libmx-gtk-@MX_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
		--pkg gobject-2.0 \
		--pkg gtk+-2.0 \
		--pkg-export mx-gtk-1.0 \
		--output $@ \
		$(source_gtk_h) \
		$(top_srcdir)/mx-gtk/mx-gtk.h \
		$(source_gtk_c)

BUILT_GIRSOURCES += MxGtk-@MX_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(AM_V_GEN) \
	LD_LIBRARY_PATH=.libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=. \
		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)

endif # HAVE_INTROSPECTION

-include $(top_srcdir)/git.mk
