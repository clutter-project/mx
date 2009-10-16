SUBDIRS = mx data tests docs build po

GITIGNOREFILES= $(top_srcdir)/build/autotools\
		$(top_srcdir)/docs/reference/libmx/tmpl

ACLOCAL_AMFLAGS = -I build/autotools
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

pcfiles = mx-$(MX_API_VERSION).pc mx-gtk-$(MX_API_VERSION).pc

mx-$(MX_API_VERSION).pc: mx.pc
	@cp -f $< $@

mx-gtk-$(MX_API_VERSION).pc: mx-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = $(mxdoc_DATA) mx.pc.in mx-gtk.pc.in

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

