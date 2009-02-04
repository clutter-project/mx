NULL =

BUILT_SOURCES = 		\
	nbtk-enum-types.h 	\
	nbtk-enum-types.c 	\
	nbtk-marshal.h 		\
	nbtk-marshal.c

# keep sorted alphabetically, please
source_h = \
	$(srcdir)/nbtk-behaviour-bounce.h 	\
	$(srcdir)/nbtk-button.h 		\
	$(srcdir)/nbtk-entry.h 			\
	$(srcdir)/nbtk-label.h 			\
	$(srcdir)/nbtk-stylable.h 		\
	$(srcdir)/nbtk-style.h 			\
	$(srcdir)/nbtk-table.h 			\
	$(srcdir)/nbtk-texture-cache.h 		\
	$(srcdir)/nbtk-texture-frame.h 		\
	$(srcdir)/nbtk-tooltip.h		\
	$(srcdir)/nbtk-types.h 			\
	$(srcdir)/nbtk-widget.h			\
	$(NULL)

source_h_priv = \
	nbtk-private.h

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
	(cmp xgen-teth nbtk-enum-types.h || cp xgen-teth nbtk-enum-types.h) && \
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

tidyincludedir = $(includedir)/nbtk
tidyinclude_DATA = 				\
	$(source_h) 				\
	$(top_srcdir)/nbtk/nbtk.h 		\
        $(top_builddir)/nbtk/nbtk-version.h     \
	$(top_builddir)/nbtk/nbtk-enum-types.h	\
	$(NULL)

lib_LTLIBRARIES = libnbtk.la

libnbtk_la_CFLAGS = 							\
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

libnbtk_la_LDFLAGS = 			\
	$(NBTK_LT_LDFLAGS)		\
	-export-dynamic			\
	-export-symbols-regex "^nbtk.*"	\
	-rpath $(libdir) 		\
	$(NULL)

libnbtk_la_SOURCES = 			\
	$(BUILT_SOURCES) 		\
	$(source_h) 			\
	$(source_h_priv) 		\
	nbtk-behaviour-bounce.c 	\
	nbtk-button.c 			\
	nbtk-entry.c 			\
	nbtk-label.c 			\
	nbtk-stylable.c 		\
	nbtk-style.c 			\
	nbtk-table.c 			\
	nbtk-texture-cache.c 		\
	nbtk-texture-frame.c 		\
	nbtk-tooltip.c 			\
	nbtk-widget.c			\
	$(NULL)

libnbtk_la_LIBADD = $(NBTK_LIBS)
