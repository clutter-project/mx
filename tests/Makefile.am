NULL =

AM_CFLAGS = $(MX_CFLAGS) $(MX_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/mx/libmx-$(MX_API_VERSION).la $(MX_LIBS)

if ENABLE_GTK_WIDGETS
AM_CFLAGS += $(GTK_CFLAGS)
LDADD += $(top_builddir)/mx-gtk/libmx-gtk-$(MX_API_VERSION).la $(GTK_LIBS)
endif

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 			\
	test-deform-texture		\
	test-draggable			\
	test-droppable			\
	test-window 			\
	test-widgets			\
	test-containers			\
	$(NULL)

if ENABLE_GTK_WIDGETS
noinst_PROGRAMS += test-gtk

test_gtk_SOURCES = test-gtk.c
endif

test_widgets_SOURCES = test-widgets.c
test_containers_SOURCES = test-containers.c

test_deform_texture_SOURCES = test-deform-texture.c

test_draggable_SOURCES = test-draggable.c
test_droppable_SOURCES = test-droppable.c

test_window_SOURCES = test-window.c

EXTRA_DIST = redhand.png

-include $(top_srcdir)/git.mk
