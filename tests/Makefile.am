NULL =

AM_CFLAGS = $(MX_CFLAGS) $(GTK_CFLAGS)  $(MX_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/mx/libmx-$(MX_API_VERSION).la \
        $(top_builddir)/mx/libmx-gtk-$(MX_API_VERSION).la


INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 		\
	test-draggable		\
	test-droppable		\
	test-grid \
	test-gtk		\
	test-table   		\
	test-table-2            \
	test-texture-frame 	\
	test-views 		\
	test-box-layout		\
	test-mx			\
	$(NULL)

test_table_SOURCES = test-table.c
test_table_2_SOURCES = test-table-2.c
test_texture_frame_SOURCES = test-texture-frame.c
test_grid_SOURCES = test-grid.c
test_gtk_SOURCES = test-gtk.c
test_views_SOURCES = test-views.c

test_mx_SOURCES = test-mx.c \
		  test-mx.h \
		  test-buttons.c \
		  test-combo-box.c \
		  test-entry.c \
		  test-expander.c \
		  test-label.c \
		  test-progress-bar.c \
		  test-scroll-bar.c \
		  test-scroll-grid.c \
		  test-scroll-view.c \
		  test-slider.c \
		  test-styles.c \
		  test-tooltips.c \
		  test-toggle.c \
		  test-button-group.c \
		  $(NULL)

test_draggable_SOURCES = test-draggable.c
test_droppable_SOURCES = test-droppable.c

test_box_layout_SOURCES = test-box-layout.c

EXTRA_DIST = \
	     style/border-image-dark.png \
	     style/border-image-light.png \
	     style/border-image.png \
	     style/cyan-button.png \
	     style/default.css \
	     style/green-button.png \
	     style/lime-button.png \
	     style/red-button.png \
	     style/redhand.png \
	     style/rounded-corner.png \
	     test-texture-frame.png

-include $(top_srcdir)/git.mk
