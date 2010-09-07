NULL =

AM_CFLAGS = $(MX_CFLAGS) $(MX_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/mx/libmx-$(MX_API_VERSION).la

if ENABLE_GTK_WIDGETS
AM_CFLAGS += $(GTK_CFLAGS)
LDADD += $(top_builddir)/mx-gtk/libmx-gtk-$(MX_API_VERSION).la
endif

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 			\
	test-deform-texture		\
	test-deform-texture-interactive	\
	test-draggable			\
	test-droppable			\
	test-grid 			\
	test-path-bar			\
	test-shaders			\
	test-stack			\
	test-table   			\
	test-table-2            	\
	test-texture-frame 		\
	test-views 			\
	test-window 			\
	test-box-layout			\
	test-mx				\
	$(NULL)

if ENABLE_GTK_WIDGETS
noinst_PROGRAMS += test-gtk

test_gtk_SOURCES = test-gtk.c
endif

test_table_SOURCES = test-table.c
test_table_2_SOURCES = test-table-2.c
test_texture_frame_SOURCES = test-texture-frame.c
test_grid_SOURCES = test-grid.c
test_views_SOURCES = test-views.c

test_mx_SOURCES = test-mx.c \
		  test-mx.h \
		  test-buttons.c \
		  test-combo-box.c \
		  test-dialog.c \
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

test_deform_texture_SOURCES = test-deform-texture.c
test_deform_texture_interactive_SOURCES = test-deform-texture-interactive.c


test_draggable_SOURCES = test-draggable.c
test_droppable_SOURCES = test-droppable.c

test_box_layout_SOURCES = test-box-layout.c

test_path_bar_SOURCES = test-path-bar.c

test_shaders_SOURCES = test-shaders.c

test_stack_SOURCES = test-stack.c

test_window_SOURCES = test-window.c

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
