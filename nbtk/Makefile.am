AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(NBTK_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

# auto-generated header and C files

GLIB_GENMARSHAL = `pkg-config --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS = `pkg-config --variable=glib_mkenums glib-2.0`


BUILT_SOURCES = \
	nbtk-enum-types.h 	\
	nbtk-enum-types.c 	\
	nbtk-marshal.h 		\
	nbtk-marshal.c

source_h = \
	nbtk-button.h \
	nbtk-entry.h \
	nbtk-enum-types.h 	\
	nbtk-label.h \
	nbtk-stylable.h \
	nbtk-style.h \
	nbtk-table.h \
	nbtk-texture-cache.h \
	nbtk-texture-frame.h \
	nbtk-tooltip.h \
	nbtk-types.h \
	nbtk-widget.h \
	nbtk.h

EXTRA_DIST = nbtk-marshal.list nbtk-enum-types.h.in nbtk-enum-types.c.in

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
tidyinclude_DATA = \
  $(source_h)

lib_LTLIBRARIES = libnbtk.la

libnbtk_la_SOURCES = \
		$(BUILT_SOURCES) \
		$(source_h) \
		nbtk-button.c \
		nbtk-entry.c \
		nbtk-label.c \
		nbtk-private.h \
		nbtk-stylable.c \
		nbtk-style.c \
		nbtk-table.c \
		nbtk-texture-cache.c \
		nbtk-texture-frame.c \
		nbtk-tooltip.c \
		nbtk-widget.c


libnbtk_la_LIBADD = $(NBTK_LIBS)
