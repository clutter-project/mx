AM_CFLAGS = $(NBTK_CFLAGS) $(NBTK_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/nbtk/libnbtk-$(NBTK_API_VERSION).la
INCLUDES = -I$(top_srcdir)

noinst_PROGRAMS = moblin-sample-app
moblin_sample_app_SOURCES = moblin-sample-app.c

EXTRA_DIST = \
		custom.css
		theme/button-active.png \
		theme/default.css \
		theme/button-hover.png \
		theme/scroll-button-down.png \
		theme/scroll-button-down-hover.png \
		theme/scroll-handle.png \
		theme/scroll-button-up-hover.png \
		theme/scroll-handle-hover.png \
		theme/button.png \
		theme/scroll-button-up.png \
		theme/text-entry.png

