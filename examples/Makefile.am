AM_CFLAGS = $(MX_CFLAGS) $(MX_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/mx/libmx-$(MX_API_VERSION).la
INCLUDES = -I$(top_srcdir)

noinst_PROGRAMS = moblin-sample-app
moblin_sample_app_SOURCES = moblin-sample-app.c

EXTRA_DIST = custom.css

-include $(top_srcdir)/git.mk

