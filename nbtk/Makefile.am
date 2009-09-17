NULL =

# installed utilities
bin_PROGRAMS = nbtk-create-image-cache
nbtk_create_image_cache_SOURCES = nbtk-create-image-cache.c
nbtk_create_image_cache_LDADD = $(GTK_LIBS)
nbtk_create_image_cache_CFLAGS = $(GTK_CFLAGS) $(NBTK_MAINTAINER_CFLAGS)

BUILT_SOURCES = 		\
	nbtk-enum-types.h 	\
	nbtk-enum-types.c 	\
	nbtk-marshal.h 		\
	nbtk-marshal.c

# keep sorted alphabetically, please
source_h = \
	$(top_srcdir)/nbtk/nbtk-action.h		\
	$(top_srcdir)/nbtk/nbtk-adjustment.h		\
	$(top_srcdir)/nbtk/nbtk-bin.h			\
	$(top_srcdir)/nbtk/nbtk-box-layout.h		\
	$(top_srcdir)/nbtk/nbtk-box-layout-child.h	\
	$(top_srcdir)/nbtk/nbtk-combo-box.h 		\
	$(top_srcdir)/nbtk/nbtk-button.h 		\
	$(top_srcdir)/nbtk/nbtk-draggable.h 		\
	$(top_srcdir)/nbtk/nbtk-droppable.h 		\
	$(top_srcdir)/nbtk/nbtk-clipboard.h		\
	$(top_srcdir)/nbtk/nbtk-entry.h 		\
	$(top_srcdir)/nbtk/nbtk-expander.h 		\
	$(top_srcdir)/nbtk/nbtk-grid.h 			\
	$(top_srcdir)/nbtk/nbtk-item-factory.h		\
	$(top_srcdir)/nbtk/nbtk-item-view.h 		\
	$(top_srcdir)/nbtk/nbtk-list-view.h 		\
	$(top_srcdir)/nbtk/nbtk-icon.h 			\
	$(top_srcdir)/nbtk/nbtk-label.h 		\
	$(top_srcdir)/nbtk/nbtk-progress-bar.h		\
	$(top_srcdir)/nbtk/nbtk-popup.h 		\
	$(top_srcdir)/nbtk/nbtk-scroll-bar.h 		\
	$(top_srcdir)/nbtk/nbtk-scroll-view.h 		\
	$(top_srcdir)/nbtk/nbtk-scrollable.h 		\
	$(top_srcdir)/nbtk/nbtk-stylable.h 		\
	$(top_srcdir)/nbtk/nbtk-style.h 		\
	$(top_srcdir)/nbtk/nbtk-subtexture.h		\
	$(top_srcdir)/nbtk/nbtk-table-child.h 		\
	$(top_srcdir)/nbtk/nbtk-table.h 		\
	$(top_srcdir)/nbtk/nbtk-texture-cache.h 	\
	$(top_srcdir)/nbtk/nbtk-texture-frame.h 	\
	$(top_srcdir)/nbtk/nbtk-tooltip.h		\
	$(top_srcdir)/nbtk/nbtk-types.h 		\
	$(top_srcdir)/nbtk/nbtk-utils.h                 \
	$(top_srcdir)/nbtk/nbtk-viewport.h		\
	$(top_srcdir)/nbtk/nbtk-widget.h		\
	$(NULL)

source_gtk_h = \
	$(top_srcdir)/nbtk/nbtk-gtk-frame.h 		\
	$(top_srcdir)/nbtk/nbtk-gtk-expander.h 		\
	$(top_srcdir)/nbtk/nbtk-gtk-light-switch.h 	\
	$(NULL)

source_h_priv = \
	$(top_srcdir)/nbtk/nbtk-private.h

source_c = \
	$(top_srcdir)/nbtk/nbtk-adjustment.c 		\
	$(top_srcdir)/nbtk/nbtk-action.c 		\
	$(top_srcdir)/nbtk/nbtk-bin.c			\
	$(top_srcdir)/nbtk/nbtk-box-layout.c		\
	$(top_srcdir)/nbtk/nbtk-box-layout-child.c	\
	$(top_srcdir)/nbtk/nbtk-combo-box.c		\
	$(top_srcdir)/nbtk/nbtk-button.c 		\
	$(top_srcdir)/nbtk/nbtk-draggable.c		\
	$(top_srcdir)/nbtk/nbtk-droppable.c		\
	$(top_srcdir)/nbtk/nbtk-clipboard.c 		\
	$(top_srcdir)/nbtk/nbtk-entry.c 		\
	$(top_srcdir)/nbtk/nbtk-expander.c 		\
	$(top_srcdir)/nbtk/nbtk-grid.c 			\
	$(top_srcdir)/nbtk/nbtk-icon.c 			\
	$(top_srcdir)/nbtk/nbtk-item-factory.c 		\
	$(top_srcdir)/nbtk/nbtk-item-view.c 		\
	$(top_srcdir)/nbtk/nbtk-list-view.c 		\
	$(top_srcdir)/nbtk/nbtk-label.c 		\
	$(top_srcdir)/nbtk/nbtk-progress-bar.c		\
	$(top_srcdir)/nbtk/nbtk-popup.c			\
	$(top_srcdir)/nbtk/nbtk-scroll-bar.c 		\
	$(top_srcdir)/nbtk/nbtk-scroll-view.c		\
	$(top_srcdir)/nbtk/nbtk-scrollable.c 		\
	$(top_srcdir)/nbtk/nbtk-stylable.c 		\
	$(top_srcdir)/nbtk/nbtk-style.c 		\
	$(top_srcdir)/nbtk/nbtk-subtexture.c		\
	$(top_srcdir)/nbtk/nbtk-table.c 		\
	$(top_srcdir)/nbtk/nbtk-table-child.c 		\
	$(top_srcdir)/nbtk/nbtk-texture-cache.c 	\
	$(top_srcdir)/nbtk/nbtk-texture-frame.c 	\
	$(top_srcdir)/nbtk/nbtk-tooltip.c 		\
	$(top_srcdir)/nbtk/nbtk-utils.c                 \
	$(top_srcdir)/nbtk/nbtk-viewport.c 		\
	$(top_srcdir)/nbtk/nbtk-widget.c		\
	$(NULL)

source_gtk_c = \
	$(top_srcdir)/nbtk/nbtk-gtk-frame.c 		\
	$(top_srcdir)/nbtk/nbtk-gtk-expander.c 		\
	$(top_srcdir)/nbtk/nbtk-gtk-light-switch.c 	\
	$(NULL)

EXTRA_DIST = \
	nbtk-marshal.list \
	nbtk-enum-types.h.in \
	nbtk-enum-types.c.in \
	nbtk-version.h.in

STAMP_FILES = stamp-nbtk-marshal.h stamp-nbtk-enum-types.h

CLEANFILES = $(STAMP_FILES) $(BUILT_SOURCES)

nbtk-marshal.h: stamp-nbtk-marshal.h
	@true
stamp-nbtk-marshal.h: Makefile nbtk-marshal.list
	$(GLIB_GENMARSHAL) \
		--prefix=_nbtk_marshal \
		--header \
	$(srcdir)/nbtk-marshal.list > xgen-tmh && \
	(cmp -s xgen-tmh nbtk-marshal.h || cp -f xgen-tmh nbtk-marshal.h) && \
	rm -f xgen-tmh && \
	echo timestamp > $(@F)

nbtk-marshal.c: Makefile nbtk-marshal.list
	(echo "#include \"nbtk-marshal.h\"" ; \
	 $(GLIB_GENMARSHAL) \
		--prefix=_nbtk_marshal \
		--body \
	 $(srcdir)/nbtk-marshal.list ) > xgen-tmc && \
	cp -f xgen-tmc nbtk-marshal.c && \
	rm -f xgen-tmc

nbtk-enum-types.h: stamp-nbtk-enum-types.h Makefile
	@true
stamp-nbtk-enum-types.h: $(source_h) nbtk-enum-types.h.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template nbtk-enum-types.h.in \
	  $(source_h) ) >> xgen-teth && \
	(cmp -s xgen-teth nbtk-enum-types.h || cp xgen-teth nbtk-enum-types.h) && \
	rm -f xgen-teth && \
	echo timestamp > $(@F)

nbtk-enum-types.c: stamp-nbtk-enum-types.h nbtk-enum-types.c.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template nbtk-enum-types.c.in \
	  $(source_h) ) >> xgen-tetc && \
	cp xgen-tetc nbtk-enum-types.c && \
	rm -f xgen-tetc


# libnbtk library

nbtkincludedir = $(includedir)/nbtk-$(NBTK_API_VERSION)/nbtk
nbtkinclude_DATA = 				\
	$(source_h) 				\
	$(source_gtk_h)				\
	$(top_srcdir)/nbtk/nbtk.h 		\
	$(top_srcdir)/nbtk/nbtk-gtk.h 		\
	$(top_builddir)/nbtk/nbtk-version.h     \
	$(top_builddir)/nbtk/nbtk-enum-types.h	\
	$(NULL)

lib_LTLIBRARIES = libnbtk-@NBTK_API_VERSION@.la \
		  libnbtk-gtk-@NBTK_API_VERSION@.la

common_includes = \
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" 				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DNBTK_COMPILATION						\
	-DG_DISABLE_DEPRECATED

common_ldflags = \
	-export-dynamic			\
	-export-symbols-regex "^nbtk.*"	\
	-rpath $(libdir)

libnbtk_@NBTK_API_VERSION@_la_CFLAGS =	\
	$(common_includes)		\
	-DG_LOG_DOMAIN=\"Nbtk\"		\
	-DCLUTTER_DISABLE_DEPRECATED	\
	$(NBTK_MAINTAINER_CFLAGS)	\
	$(NBTK_DEBUG_CFLAGS)		\
	$(NBTK_CFLAGS)			\
	$(NULL)

libnbtk_@NBTK_API_VERSION@_la_LDFLAGS = $(NBTK_LT_LDFLAGS) $(common_ldflags)

libnbtk_@NBTK_API_VERSION@_la_SOURCES =	\
	$(BUILT_SOURCES) 		\
	$(source_h)			\
	$(source_h_priv)		\
	$(source_c)			\
	nbtk-private.c			\
	nbtk.h 				\
	$(NULL)

libnbtk_@NBTK_API_VERSION@_la_LIBADD = $(NBTK_LIBS)

#
# Nbtk GTK+ Library
#
# This is a seperate library with GTK+ widgets, but it should always be
# included with the Nbtk package
#

# libnbtk-gtk library

libnbtk_gtk_@NBTK_API_VERSION@_la_CFLAGS =	\
	$(common_includes)			\
	-DG_LOG_DOMAIN=\"NbtkGtk\"		\
	-DGTK_DISABLE_DEPRECATED		\
	$(NBTK_MAINTAINER_CFLAGS)		\
	$(NBTK_DEBUG_CFLAGS)			\
	$(GTK_CFLAGS)				\
	$(NULL)

libnbtk_gtk_@NBTK_API_VERSION@_la_LDFLAGS = $(GTK_LT_LDFLAGS) $(common_ldflags)

libnbtk_gtk_@NBTK_API_VERSION@_la_SOURCES =	\
	$(source_gtk_h)			\
	$(source_h_priv)		\
	$(source_gtk_c)			\
	nbtk-gtk.h 			\
	$(NULL)

libnbtk_gtk_@NBTK_API_VERSION@_la_LIBADD = $(GTK_LIBS)

if HAVE_INTROSPECTION

BUILT_GIRSOURCES =

Nbtk-@NBTK_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libnbtk-@NBTK_API_VERSION@.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace Nbtk --nsversion=@NBTK_API_VERSION@ \
		$(common_includes) \
		--c-include="nbtk/nbtk.h" \
		--include=GObject-2.0 \
		--include=Clutter-1.0 \
		--library=libnbtk-@NBTK_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
		--pkg gobject-2.0 \
		--pkg clutter-1.0 \
		--pkg libccss-1 \
		--output $@ \
		$(source_h) \
		$(top_srcdir)/nbtk/nbtk.h \
		$(top_builddir)/nbtk/nbtk-version.h \
		$(top_builddir)/nbtk/nbtk-enum-types.h \
		$(source_c)

BUILT_GIRSOURCES += Nbtk-@NBTK_API_VERSION@.gir

NbtkGtk-@NBTK_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libnbtk-gtk-@NBTK_API_VERSION@.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace NbtkGtk --nsversion=@NBTK_API_VERSION@ \
		$(common_includes) \
		--c-include="nbtk/nbtk-gtk.h" \
		--include=GObject-2.0 \
		--include=Gtk-2.0 \
		--library=libnbtk-gtk-@NBTK_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
		--pkg gobject-2.0 \
		--pkg gtk+-2.0 \
		--pkg libccss-1 \
		--output $@ \
		$(source_gtk_h) \
		$(top_srcdir)/nbtk/nbtk-gtk.h \
		$(source_gtk_c)

BUILT_GIRSOURCES += NbtkGtk-@NBTK_API_VERSION@.gir

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
