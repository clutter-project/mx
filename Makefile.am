SUBDIRS = nbtk po tests

pcfiles = nbtk-1.0.pc

nbtk-1.0.pc: nbtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

nbtkdocdir = ${prefix}/doc/nbtk
nbtkdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

EXTRA_DIST = $(nbtkdoc_DATA) nbtk.pc.in

CLEANFILES = $(pcfiles)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = \
        aclocal.m4 \
        compile \
        config.guess \
        config.h.in \
        config.sub \
        configure \
        depcomp \
        gtk-doc.make \
        install-sh \
        ltmain.sh \
        Makefile.in \
        missing \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	INSTALL \
	mkinstalldirs

