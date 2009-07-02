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
	$(top_srcdir)/nbtk/nbtk-adjustment.h		\
	$(top_srcdir)/nbtk/nbtk-bin.h			\
	$(top_srcdir)/nbtk/nbtk-box-layout.h		\
	$(top_srcdir)/nbtk/nbtk-button.h 		\
	$(top_srcdir)/nbtk/nbtk-draggable.h 		\
	$(top_srcdir)/nbtk/nbtk-droppable.h 		\
	$(top_srcdir)/nbtk/nbtk-cell-renderer.h 	\
	$(top_srcdir)/nbtk/nbtk-clipboard.h		\
	$(top_srcdir)/nbtk/nbtk-entry.h 		\
	$(top_srcdir)/nbtk/nbtk-expander.h 		\
	$(top_srcdir)/nbtk/nbtk-fade-button.h 		\
	$(top_srcdir)/nbtk/nbtk-grid.h \
	$(top_srcdir)/nbtk/nbtk-gtk-frame.h 	\
	$(top_srcdir)/nbtk/nbtk-gtk-expander.h 	\
	$(top_srcdir)/nbtk/nbtk-gtk-light-switch.h 	\
	$(top_srcdir)/nbtk/nbtk-icon-view.h \
	$(top_srcdir)/nbtk/nbtk-icon.h \
	$(top_srcdir)/nbtk/nbtk-label.h 		\
	$(top_srcdir)/nbtk/nbtk-progress-bar.h		\
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
	$(top_srcdir)/nbtk/nbtk-viewport.h		\
	$(top_srcdir)/nbtk/nbtk-widget.h		\
	$(NULL)

source_h_priv = \
	$(top_srcdir)/nbtk/nbtk-private.h

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
	$(top_srcdir)/nbtk/nbtk.h 		\
	$(top_srcdir)/nbtk/nbtk-gtk.h 		\
	$(top_builddir)/nbtk/nbtk-version.h     \
	$(top_builddir)/nbtk/nbtk-enum-types.h	\
	$(NULL)

lib_LTLIBRARIES = libnbtk-@NBTK_API_VERSION@.la \
		  libnbtk-gtk-@NBTK_API_VERSION@.la

libnbtk_@NBTK_API_VERSION@_la_CFLAGS =					\
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" 				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DG_LOG_DOMAIN=\"Nbtk\"						\
	-DNBTK_COMPILATION						\
	-DG_DISABLE_DEPRECATED						\
	-DCLUTTER_DISABLE_DEPRECATED					\
	$(NBTK_MAINTAINER_CFLAGS)					\
	$(NBTK_DEBUG_CFLAGS)						\
	$(NBTK_CFLAGS)							\
	$(NULL)

libnbtk_@NBTK_API_VERSION@_la_LDFLAGS = \
	$(NBTK_LT_LDFLAGS)		\
	-export-dynamic			\
	-export-symbols-regex "^nbtk.*"	\
	-rpath $(libdir) 		\
	$(NULL)

libnbtk_@NBTK_API_VERSION@_la_SOURCES =	\
	$(BUILT_SOURCES) 		\
	$(source_h)			\
	$(source_h_priv)		\
	nbtk-adjustment.c 		\
	nbtk-bin.c			\
	nbtk-box-layout.c		\
	nbtk-button.c 			\
	nbtk-draggable.c		\
	nbtk-droppable.c		\
	nbtk-cell-renderer.c 		\
	nbtk-clipboard.c 		\
	nbtk-entry.c 			\
	nbtk-expander.c 		\
	nbtk-fade-button.c 		\
	nbtk-grid.c 			\
	nbtk-icon.c 			\
	nbtk-icon-view.c 		\
	nbtk-label.c 			\
	nbtk-progress-bar.c		\
	nbtk-scroll-bar.c 		\
	nbtk-scroll-button.c \
	nbtk-scroll-button.h \
	nbtk-scroll-view.c 		\
	nbtk-scrollable.c 		\
	nbtk-stylable.c 		\
	nbtk-style.c 			\
	nbtk-subtexture.c		\
	nbtk-table.c 			\
	nbtk-table-child.c 		\
	nbtk-texture-cache.c 		\
	nbtk-texture-frame.c 		\
	nbtk-tooltip.c 			\
	nbtk-viewport.c 		\
	nbtk-widget.c			\
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

libnbtk_gtk_@NBTK_API_VERSION@_la_CFLAGS =				\
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" 				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DG_LOG_DOMAIN=\"NbtkGtk\"					\
	-DNBTK_COMPILATION						\
	-DG_DISABLE_DEPRECATED						\
	-DGTK_DISABLE_DEPRECATED					\
	$(NBTK_MAINTAINER_CFLAGS)					\
	$(NBTK_DEBUG_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(NULL)

libnbtk_gtk_@NBTK_API_VERSION@_la_LDFLAGS = \
	$(GTK_LT_LDFLAGS)		\
	-export-dynamic			\
	-export-symbols-regex "^nbtk.*"	\
	-rpath $(libdir) 		\
	$(NULL)

libnbtk_gtk_@NBTK_API_VERSION@_la_SOURCES =	\
	$(source_h)			\
	nbtk-gtk-frame.c 		\
	nbtk-gtk-expander.c 		\
	nbtk-gtk-light-switch.c 	\
	nbtk-gtk.h 			\
	$(NULL)

libnbtk_gtk_@NBTK_API_VERSION@_la_LIBADD = $(GTK_LIBS)

-include $(top_srcdir)/git.mk
