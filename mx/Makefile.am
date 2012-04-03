NULL =

# installed utilities
bin_PROGRAMS = mx-create-image-cache
mx_create_image_cache_SOURCES = mx-create-image-cache.c
mx_create_image_cache_LDADD = $(MX_IMAGE_CACHE_LIBS)
mx_create_image_cache_CFLAGS = $(MX_IMAGE_CACHE_CFLAGS) $(MX_MAINTAINER_CFLAGS)

BUILT_SOURCES = 		\
	mx-enum-types.h 	\
	mx-enum-types.c 	\
	mx-marshal.h 		\
	mx-marshal.c

if HAVE_X11
WINSYS_SRC = \
	$(top_srcdir)/mx/x11/mx-clipboard-x11.c	\
	$(top_srcdir)/mx/x11/mx-settings-x11.c	\
	$(top_srcdir)/mx/x11/mx-settings-x11.h	\
	$(top_srcdir)/mx/x11/mx-window-x11.c	\
	$(top_srcdir)/mx/x11/mx-window-x11.h	\
	$(top_srcdir)/mx/x11/xsettings-client.c	\
	$(top_srcdir)/mx/x11/xsettings-client.h	\
	$(top_srcdir)/mx/x11/xsettings-common.c	\
	$(top_srcdir)/mx/x11/xsettings-common.h	\
	$(NULL)
else
WINSYS_SRC = \
	$(top_srcdir)/mx/mx-clipboard-default.c	\
	$(NULL)
endif

# keep sorted alphabetically, please
source_h = \
	$(top_srcdir)/mx/mx-action.h		\
	$(top_srcdir)/mx/mx-actor-manager.h	\
	$(top_srcdir)/mx/mx-adjustment.h		\
	$(top_srcdir)/mx/mx-application.h	\
	$(top_srcdir)/mx/mx-bin.h			\
	$(top_srcdir)/mx/mx-box-layout.h		\
	$(top_srcdir)/mx/mx-box-layout-child.h	\
	$(top_srcdir)/mx/mx-combo-box.h 		\
	$(top_srcdir)/mx/mx-button.h 		\
	$(top_srcdir)/mx/mx-button-group.h 	\
	$(top_srcdir)/mx/mx-deform-texture.h 	\
	$(top_srcdir)/mx/mx-deform-bow-tie.h 	\
	$(top_srcdir)/mx/mx-deform-page-turn.h 	\
	$(top_srcdir)/mx/mx-deform-waves.h 	\
	$(top_srcdir)/mx/mx-dialog.h 		\
	$(top_srcdir)/mx/mx-draggable.h 		\
	$(top_srcdir)/mx/mx-droppable.h 		\
	$(top_srcdir)/mx/mx-clipboard.h		\
	$(top_srcdir)/mx/mx-entry.h 		\
	$(top_srcdir)/mx/mx-expander.h 		\
	$(top_srcdir)/mx/mx-fade-effect.h 	\
	$(top_srcdir)/mx/mx-focus-manager.h 	\
	$(top_srcdir)/mx/mx-focusable.h 	\
	$(top_srcdir)/mx/mx-frame.h		\
	$(top_srcdir)/mx/mx-grid.h 			\
	$(top_srcdir)/mx/mx-item-factory.h		\
	$(top_srcdir)/mx/mx-item-view.h 		\
	$(top_srcdir)/mx/mx-list-view.h 		\
	$(top_srcdir)/mx/mx-offscreen.h 		\
	$(top_srcdir)/mx/mx-icon.h 			\
	$(top_srcdir)/mx/mx-image.h 		\
	$(top_srcdir)/mx/mx-icon-theme.h 	\
	$(top_srcdir)/mx/mx-label.h 		\
	$(top_srcdir)/mx/mx-notebook.h 		\
	$(top_srcdir)/mx/mx-path-bar.h 		\
	$(top_srcdir)/mx/mx-progress-bar.h		\
	$(top_srcdir)/mx/mx-menu.h 		\
	$(top_srcdir)/mx/mx-scroll-bar.h 		\
	$(top_srcdir)/mx/mx-scroll-view.h 		\
	$(top_srcdir)/mx/mx-scrollable.h 		\
	$(top_srcdir)/mx/mx-settings.h		\
	$(top_srcdir)/mx/mx-settings-provider.h	\
	$(top_srcdir)/mx/mx-slider.h 		\
	$(top_srcdir)/mx/mx-spinner.h 		\
	$(top_srcdir)/mx/mx-stack.h 		\
	$(top_srcdir)/mx/mx-stack-child.h 	\
	$(top_srcdir)/mx/mx-stylable.h 		\
	$(top_srcdir)/mx/mx-style.h 		\
	$(top_srcdir)/mx/mx-table-child.h 		\
	$(top_srcdir)/mx/mx-table.h 		\
	$(top_srcdir)/mx/mx-texture-cache.h 	\
	$(top_srcdir)/mx/mx-texture-frame.h 	\
	$(top_srcdir)/mx/mx-toggle.h 		\
	$(top_srcdir)/mx/mx-toolbar.h		\
	$(top_srcdir)/mx/mx-tooltip.h		\
	$(top_srcdir)/mx/mx-types.h 		\
	$(top_srcdir)/mx/mx-utils.h                 \
	$(top_srcdir)/mx/mx-viewport.h		\
	$(top_srcdir)/mx/mx-widget.h		\
	$(top_srcdir)/mx/mx-window.h		\
	$(top_srcdir)/mx/mx-floating-widget.h \
	$(top_srcdir)/mx/mx-kinetic-scroll-view.h \
	$(NULL)

source_h_priv = \
	$(top_srcdir)/mx/mx-css.h		\
	$(top_srcdir)/mx/mx-native-window.h	\
	$(top_srcdir)/mx/mx-path-bar-button.h	\
	$(top_srcdir)/mx/mx-progress-bar-fill.h	\
	$(top_srcdir)/mx/mx-private.h		\
	$(top_srcdir)/mx/mx-settings-provider.h	\
	$(NULL)

source_c = \
	$(top_srcdir)/mx/mx-adjustment.c 		\
	$(top_srcdir)/mx/mx-action.c 		\
	$(top_srcdir)/mx/mx-actor-manager.c 	\
	$(top_srcdir)/mx/mx-application.c	\
	$(top_srcdir)/mx/mx-bin.c			\
	$(top_srcdir)/mx/mx-box-layout.c		\
	$(top_srcdir)/mx/mx-box-layout-child.c	\
	$(top_srcdir)/mx/mx-css.c		\
	$(top_srcdir)/mx/mx-combo-box.c		\
	$(top_srcdir)/mx/mx-button.c 		\
	$(top_srcdir)/mx/mx-button-group.c 	\
	$(top_srcdir)/mx/mx-deform-texture.c 	\
	$(top_srcdir)/mx/mx-deform-bow-tie.c 	\
	$(top_srcdir)/mx/mx-deform-page-turn.c 	\
	$(top_srcdir)/mx/mx-deform-waves.c 	\
	$(top_srcdir)/mx/mx-dialog.c 		\
	$(top_srcdir)/mx/mx-draggable.c		\
	$(top_srcdir)/mx/mx-droppable.c		\
	$(top_srcdir)/mx/mx-entry.c 		\
	$(top_srcdir)/mx/mx-expander.c 		\
	$(top_srcdir)/mx/mx-fade-effect.c 	\
	$(top_srcdir)/mx/mx-focus-manager.c 	\
	$(top_srcdir)/mx/mx-focusable.c 	\
	$(top_srcdir)/mx/mx-frame.c		\
	$(top_srcdir)/mx/mx-grid.c 			\
	$(top_srcdir)/mx/mx-icon-theme.c 	\
	$(top_srcdir)/mx/mx-icon.c 			\
	$(top_srcdir)/mx/mx-image.c 		\
	$(top_srcdir)/mx/mx-item-factory.c 		\
	$(top_srcdir)/mx/mx-item-view.c 		\
	$(top_srcdir)/mx/mx-list-view.c 		\
	$(top_srcdir)/mx/mx-label.c 		\
	$(top_srcdir)/mx/mx-notebook.c 		\
	$(top_srcdir)/mx/mx-offscreen.c 	\
	$(top_srcdir)/mx/mx-path-bar.c 		\
	$(top_srcdir)/mx/mx-path-bar-button.c 	\
	$(top_srcdir)/mx/mx-progress-bar.c		\
	$(top_srcdir)/mx/mx-progress-bar-fill.c	\
	$(top_srcdir)/mx/mx-menu.c			\
	$(top_srcdir)/mx/mx-scroll-bar.c 		\
	$(top_srcdir)/mx/mx-scroll-view.c		\
	$(top_srcdir)/mx/mx-scrollable.c 		\
	$(top_srcdir)/mx/mx-settings.c	\
	$(top_srcdir)/mx/mx-slider.c 		\
	$(top_srcdir)/mx/mx-spinner.c 		\
	$(top_srcdir)/mx/mx-stack.c 		\
	$(top_srcdir)/mx/mx-stack-child.c 	\
	$(top_srcdir)/mx/mx-stylable.c 		\
	$(top_srcdir)/mx/mx-style.c 		\
	$(top_srcdir)/mx/mx-table.c 		\
	$(top_srcdir)/mx/mx-table-child.c 		\
	$(top_srcdir)/mx/mx-texture-cache.c 	\
	$(top_srcdir)/mx/mx-texture-frame.c 	\
	$(top_srcdir)/mx/mx-toggle.c		\
	$(top_srcdir)/mx/mx-toolbar.c 		\
	$(top_srcdir)/mx/mx-tooltip.c 		\
	$(top_srcdir)/mx/mx-types.c 		\
	$(top_srcdir)/mx/mx-utils.c 		\
	$(top_srcdir)/mx/mx-viewport.c 		\
	$(top_srcdir)/mx/mx-widget.c		\
	$(top_srcdir)/mx/mx-window.c		\
	$(top_srcdir)/mx/mx-floating-widget.c	\
	$(top_srcdir)/mx/mx-kinetic-scroll-view.c \
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
	$(AM_V_GEN)$(GLIB_GENMARSHAL) \
		--prefix=_mx_marshal \
		--header \
	$(srcdir)/mx-marshal.list > xgen-tmh && \
	(cmp -s xgen-tmh mx-marshal.h || cp -f xgen-tmh mx-marshal.h) && \
	rm -f xgen-tmh && \
	echo timestamp > $(@F)

mx-marshal.c: Makefile mx-marshal.list
	$(AM_V_GEN)(echo "#include \"mx-marshal.h\"" ; \
	 $(GLIB_GENMARSHAL) \
		--prefix=_mx_marshal \
		--body \
	 $(srcdir)/mx-marshal.list ) > xgen-tmc && \
	cp -f xgen-tmc mx-marshal.c && \
	rm -f xgen-tmc

mx-enum-types.h: stamp-mx-enum-types.h Makefile
	@true
stamp-mx-enum-types.h: $(source_h) mx-enum-types.h.in
	$(AM_V_GEN)( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template mx-enum-types.h.in \
	  $(source_h) ) >> xgen-teth && \
	(cmp -s xgen-teth mx-enum-types.h || cp xgen-teth mx-enum-types.h) && \
	rm -f xgen-teth && \
	echo timestamp > $(@F)

mx-enum-types.c: stamp-mx-enum-types.h mx-enum-types.c.in
	$(AM_V_GEN)( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template mx-enum-types.c.in \
	  $(source_h) ) >> xgen-tetc && \
	cp xgen-tetc mx-enum-types.c && \
	rm -f xgen-tetc


# libmx library

mxincludedir = $(includedir)/mx-$(MX_API_VERSION)/mx
mxinclude_DATA = 				\
	$(source_h) 				\
	$(top_srcdir)/mx/mx.h 		\
	$(top_builddir)/mx/mx-version.h     \
	$(top_builddir)/mx/mx-enum-types.h	\
	$(NULL)

lib_LTLIBRARIES = libmx-@MX_API_VERSION@.la

common_includes = \
	-I$(top_srcdir)							\
	-I$(top_builddir)						\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" 				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DMX_COMPILATION

common_ldflags = \
	-export-dynamic			\
	-export-symbols-regex "^mx.*"	\
	-rpath $(libdir)

libmx_@MX_API_VERSION@_la_CFLAGS =	\
	$(common_includes)		\
	-DG_LOG_DOMAIN=\"Mx\"		\
	$(MX_MAINTAINER_CFLAGS)	\
	$(MX_DEBUG_CFLAGS)		\
	$(MX_CFLAGS)			\
	$(NULL)

libmx_@MX_API_VERSION@_la_LDFLAGS = $(MX_LT_LDFLAGS) $(common_ldflags)

libmx_@MX_API_VERSION@_la_SOURCES =	\
	$(BUILT_SOURCES) 		\
	$(WINSYS_SRC)			\
	$(source_h)			\
	$(source_h_priv)		\
	$(source_c)			\
	$(top_srcdir)/mx/mx-native-window.c	\
	$(top_srcdir)/mx/mx-private.c	\
	$(top_srcdir)/mx/mx-settings-provider.c	\
	$(top_srcdir)/mx/mx.h 		\
	$(NULL)

libmx_@MX_API_VERSION@_la_LIBADD = $(MX_LIBS) -lm

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS = Mx-@MX_API_VERSION@.gir

Mx-@MX_API_VERSION@.gir: libmx-@MX_API_VERSION@.la Makefile $(INTROSPECTION_SCANNER)

Mx_@MX_API_VERSION_AM@_gir_NAMESPACE = Mx
Mx_@MX_API_VERSION_AM@_gir_VERSION   = @MX_API_VERSION@
Mx_@MX_API_VERSION_AM@_gir_LIBS      = libmx-@MX_API_VERSION@.la
Mx_@MX_API_VERSION_AM@_gir_FILES     = \
	$(source_h) \
	$(top_srcdir)/mx/mx.h \
	$(top_builddir)/mx/mx-version.h \
	$(top_builddir)/mx/mx-enum-types.h \
	$(source_c)
Mx_@MX_API_VERSION_AM@_gir_CFLAGS = $(common_includes) $(MX_CFLAGS)
Mx_@MX_API_VERSION_AM@_gir_INCLUDES = Clutter-1.0
Mx_@MX_API_VERSION_AM@_gir_SCANNERFLAGS = \
	--warn-all \
	--c-include='mx/mx.h' \
	--pkg-export=mx-@MX_API_VERSION@

girdir = $(datadir)/gir-1.0/
gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelibs_DATA)
endif # HAVE_INTROSPECTION

-include $(top_srcdir)/git.mk
