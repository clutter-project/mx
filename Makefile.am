SUBDIRS = nbtk data examples tests docs build po

ACLOCAL_AMFLAGS = -I build/autotools
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

pcfiles = nbtk-$(NBTK_API_VERSION).pc nbtk-gtk-$(NBTK_API_VERSION).pc

nbtk-$(NBTK_API_VERSION).pc: nbtk.pc
	@cp -f $< $@

nbtk-gtk-$(NBTK_API_VERSION).pc: nbtk-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = $(nbtkdoc_DATA) nbtk.pc.in nbtk-gtk.pc.in

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

-include $(top_srcdir)/git.mk

