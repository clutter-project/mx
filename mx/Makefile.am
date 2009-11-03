NULL =

# installed utilities
bin_PROGRAMS = mx-create-image-cache
mx_create_image_cache_SOURCES = mx-create-image-cache.c
mx_create_image_cache_LDADD = $(GTK_LIBS)
mx_create_image_cache_CFLAGS = $(GTK_CFLAGS) $(MX_MAINTAINER_CFLAGS)

BUILT_SOURCES = 		\
	mx-enum-types.h 	\
	mx-enum-types.c 	\
	mx-marshal.h 		\
	mx-marshal.c

# keep sorted alphabetically, please
source_h = \
	$(top_srcdir)/mx/mx-action.h		\
	$(top_srcdir)/mx/mx-adjustment.h		\
	$(top_srcdir)/mx/mx-bin.h			\
	$(top_srcdir)/mx/mx-box-layout.h		\
	$(top_srcdir)/mx/mx-box-layout-child.h	\
	$(top_srcdir)/mx/mx-combo-box.h 		\
	$(top_srcdir)/mx/mx-button.h 		\
	$(top_srcdir)/mx/mx-button-group.h 	\
	$(top_srcdir)/mx/mx-draggable.h 		\
	$(top_srcdir)/mx/mx-droppable.h 		\
	$(top_srcdir)/mx/mx-clipboard.h		\
	$(top_srcdir)/mx/mx-entry.h 		\
	$(top_srcdir)/mx/mx-expander.h 		\
	$(top_srcdir)/mx/mx-grid.h 			\
	$(top_srcdir)/mx/mx-item-factory.h		\
	$(top_srcdir)/mx/mx-item-view.h 		\
	$(top_srcdir)/mx/mx-list-view.h 		\
	$(top_srcdir)/mx/mx-icon.h 			\
	$(top_srcdir)/mx/mx-label.h 		\
	$(top_srcdir)/mx/mx-notebook.h 		\
	$(top_srcdir)/mx/mx-progress-bar.h		\
	$(top_srcdir)/mx/mx-popup.h 		\
	$(top_srcdir)/mx/mx-scroll-bar.h 		\
	$(top_srcdir)/mx/mx-scroll-view.h 		\
	$(top_srcdir)/mx/mx-scrollable.h 		\
	$(top_srcdir)/mx/mx-stylable.h 		\
	$(top_srcdir)/mx/mx-style.h 		\
	$(top_srcdir)/mx/mx-subtexture.h		\
	$(top_srcdir)/mx/mx-table-child.h 		\
	$(top_srcdir)/mx/mx-table.h 		\
	$(top_srcdir)/mx/mx-texture-cache.h 	\
	$(top_srcdir)/mx/mx-texture-frame.h 	\
	$(top_srcdir)/mx/mx-tooltip.h		\
	$(top_srcdir)/mx/mx-types.h 		\
	$(top_srcdir)/mx/mx-utils.h                 \
	$(top_srcdir)/mx/mx-viewport.h		\
	$(top_srcdir)/mx/mx-widget.h		\
	$(NULL)

source_gtk_h = \
	$(top_srcdir)/mx/mx-gtk-frame.h 		\
	$(top_srcdir)/mx/mx-gtk-expander.h 		\
	$(top_srcdir)/mx/mx-gtk-light-switch.h 	\
	$(NULL)

source_h_priv = \
	$(top_srcdir)/mx/mx-private.h

source_c = \
	$(top_srcdir)/mx/mx-adjustment.c 		\
	$(top_srcdir)/mx/mx-action.c 		\
	$(top_srcdir)/mx/mx-bin.c			\
	$(top_srcdir)/mx/mx-box-layout.c		\
	$(top_srcdir)/mx/mx-box-layout-child.c	\
	$(top_srcdir)/mx/mx-css.c		\
	$(top_srcdir)/mx/mx-combo-box.c		\
	$(top_srcdir)/mx/mx-button.c 		\
	$(top_srcdir)/mx/mx-button-group.c 	\
	$(top_srcdir)/mx/mx-draggable.c		\
	$(top_srcdir)/mx/mx-droppable.c		\
	$(top_srcdir)/mx/mx-clipboard.c 		\
	$(top_srcdir)/mx/mx-entry.c 		\
	$(top_srcdir)/mx/mx-expander.c 		\
	$(top_srcdir)/mx/mx-grid.c 			\
	$(top_srcdir)/mx/mx-icon.c 			\
	$(top_srcdir)/mx/mx-item-factory.c 		\
	$(top_srcdir)/mx/mx-item-view.c 		\
	$(top_srcdir)/mx/mx-list-view.c 		\
	$(top_srcdir)/mx/mx-label.c 		\
	$(top_srcdir)/mx/mx-notebook.c 		\
	$(top_srcdir)/mx/mx-progress-bar.c		\
	$(top_srcdir)/mx/mx-popup.c			\
	$(top_srcdir)/mx/mx-scroll-bar.c 		\
	$(top_srcdir)/mx/mx-scroll-view.c		\
	$(top_srcdir)/mx/mx-scrollable.c 		\
	$(top_srcdir)/mx/mx-stylable.c 		\
	$(top_srcdir)/mx/mx-style.c 		\
	$(top_srcdir)/mx/mx-subtexture.c		\
	$(top_srcdir)/mx/mx-table.c 		\
	$(top_srcdir)/mx/mx-table-child.c 		\
	$(top_srcdir)/mx/mx-texture-cache.c 	\
	$(top_srcdir)/mx/mx-texture-frame.c 	\
	$(top_srcdir)/mx/mx-tooltip.c 		\
	$(top_srcdir)/mx/mx-utils.c                 \
	$(top_srcdir)/mx/mx-viewport.c 		\
	$(top_srcdir)/mx/mx-widget.c		\
	$(NULL)

source_gtk_c = \
	$(top_srcdir)/mx/mx-gtk-frame.c 		\
	$(top_srcdir)/mx/mx-gtk-expander.c 		\
	$(top_srcdir)/mx/mx-gtk-light-switch.c 	\
	$(NULL)

EXTRA_DIST = \
	mx-marshal.list \
	mx-enum-types.h.in \
	mx-enum-types.c.in \
	mx-version.h.in

STAMP_FILES = stamp-mx-marshal.h stamp-mx-enum-types.h

CLEANFILES = $(STAMP_FILES) $(BUILT_SOURCES)

mx-marshal.h: stamp-mx-marshal.h
	@true
stamp-mx-marshal.h: Makefile mx-marshal.list
	$(GLIB_GENMARSHAL) \
		--prefix=_mx_marshal \
		--header \
	$(srcdir)/mx-marshal.list > xgen-tmh && \
	(cmp -s xgen-tmh mx-marshal.h || cp -f xgen-tmh mx-marshal.h) && \
	rm -f xgen-tmh && \
	echo timestamp > $(@F)

mx-marshal.c: Makefile mx-marshal.list
	(echo "#include \"mx-marshal.h\"" ; \
	 $(GLIB_GENMARSHAL) \
		--prefix=_mx_marshal \
		--body \
	 $(srcdir)/mx-marshal.list ) > xgen-tmc && \
	cp -f xgen-tmc mx-marshal.c && \
	rm -f xgen-tmc

mx-enum-types.h: stamp-mx-enum-types.h Makefile
	@true
stamp-mx-enum-types.h: $(source_h) mx-enum-types.h.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template mx-enum-types.h.in \
	  $(source_h) ) >> xgen-teth && \
	(cmp -s xgen-teth mx-enum-types.h || cp xgen-teth mx-enum-types.h) && \
	rm -f xgen-teth && \
	echo timestamp > $(@F)

mx-enum-types.c: stamp-mx-enum-types.h mx-enum-types.c.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template mx-enum-types.c.in \
	  $(source_h) ) >> xgen-tetc && \
	cp xgen-tetc mx-enum-types.c && \
	rm -f xgen-tetc


# libmx library

mxincludedir = $(includedir)/mx-$(MX_API_VERSION)/mx
mxinclude_DATA = 				\
	$(source_h) 				\
	$(source_gtk_h)				\
	$(top_srcdir)/mx/mx.h 		\
	$(top_srcdir)/mx/mx-gtk.h 		\
	$(top_builddir)/mx/mx-version.h     \
	$(top_builddir)/mx/mx-enum-types.h	\
	$(NULL)

lib_LTLIBRARIES = libmx-@MX_API_VERSION@.la \
		  libmx-gtk-@MX_API_VERSION@.la

common_includes = \
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" 				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DMX_COMPILATION						\
	-DG_DISABLE_DEPRECATED

common_ldflags = \
	-export-dynamic			\
	-export-symbols-regex "^mx.*"	\
	-rpath $(libdir)

libmx_@MX_API_VERSION@_la_CFLAGS =	\
	$(common_includes)		\
	-DG_LOG_DOMAIN=\"Mx\"		\
	-DCLUTTER_DISABLE_DEPRECATED	\
	$(MX_MAINTAINER_CFLAGS)	\
	$(MX_DEBUG_CFLAGS)		\
	$(MX_CFLAGS)			\
	$(NULL)

libmx_@MX_API_VERSION@_la_LDFLAGS = $(MX_LT_LDFLAGS) $(common_ldflags)

libmx_@MX_API_VERSION@_la_SOURCES =	\
	$(BUILT_SOURCES) 		\
	$(source_h)			\
	$(source_h_priv)		\
	$(source_c)			\
	mx-private.c			\
	mx.h 				\
	$(NULL)

libmx_@MX_API_VERSION@_la_LIBADD = $(MX_LIBS)

#
# Mx GTK+ Library
#
# This is a seperate library with GTK+ widgets, but it should always be
# included with the Mx package
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

if HAVE_INTROSPECTION

BUILT_GIRSOURCES =

Mx-@MX_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libmx-@MX_API_VERSION@.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace Mx --nsversion=@MX_API_VERSION@ \
		$(common_includes) \
		--c-include="mx/mx.h" \
		--include=GObject-2.0 \
		--include=Clutter-1.0 \
		--library=libmx-@MX_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
		--pkg gobject-2.0 \
		--pkg clutter-1.0 \
		--pkg libccss-1 \
		--output $@ \
		$(source_h) \
		$(top_srcdir)/mx/mx.h \
		$(top_builddir)/mx/mx-version.h \
		$(top_builddir)/mx/mx-enum-types.h \
		$(source_c)

BUILT_GIRSOURCES += Mx-@MX_API_VERSION@.gir

MxGtk-@MX_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libmx-gtk-@MX_API_VERSION@.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace MxGtk --nsversion=@MX_API_VERSION@ \
		$(common_includes) \
		--c-include="mx/mx-gtk.h" \
		--include=GObject-2.0 \
		--include=Gtk-2.0 \
		--library=libmx-gtk-@MX_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
		--pkg gobject-2.0 \
		--pkg gtk+-2.0 \
		--pkg libccss-1 \
		--output $@ \
		$(source_gtk_h) \
		$(top_srcdir)/mx/mx-gtk.h \
		$(source_gtk_c)

BUILT_GIRSOURCES += MxGtk-@MX_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN) \
	LD_LIBRARY_PATH=.libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=. \
		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)

endif # HAVE_INTROSPECTION

-include $(top_srcdir)/git.mk
