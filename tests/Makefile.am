NULL =

AM_CFLAGS = $(NBTK_CFLAGS)
LDADD = $(top_builddir)/nbtk/libnbtk-$(NBTK_API_VERSION).la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 		\
	test-buttons 		\
	test-container-matching	\
	test-draggable		\
	test-droppable		\
	test-label 		\
	test-entry 		\
	test-script  		\
	test-styles  		\
	test-table   		\
	test-table-2 \
	test-texture-frame 	\
	test-padding 		\
	test-border-image	\
	test-scroll-bar		\
	test-scroll-view	\
	test-viewport
	$(NULL)

test_buttons_SOURCES = test-buttons.c
test_script_SOURCES = test-script.c
test_styles_SOURCES = test-styles.c
test_table_SOURCES = test-table.c
test_table_2_SOURCES = test-table-2.c
test_texture_frame_SOURCES = test-texture-frame.c
test_label_SOURCES = test-label.c
test_entry_SOURCES = test-entry.c
test_padding_SOURCES = test-padding.c
test_border_image_SOURCES = test-border-image.c
test_container_matching_SOURCES = test-container-matching.c

test_scroll_bar_SOURCES = test-scroll-bar.c
test_scroll_view_SOURCES = test-scroll-view.c
test_viewport_SOURCES = test-viewport.c

test_draggable_SOURCES = test-draggable.c
test_droppable_SOURCES = test-droppable.c
