NULL =

AM_CFLAGS = $(NBTK_CFLAGS) $(NBTK_MAINTAINER_CFLAGS)
LDADD = $(top_builddir)/nbtk/libnbtk-$(NBTK_API_VERSION).la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 		\
	test-buttons 		\
	test-label 		\
	test-entry 		\
	test-expander 		\
	test-grid \
	test-script  		\
	test-styles  		\
	test-table   		\
	test-table-2            \
	test-texture-frame 	\
	test-tooltips 		\
	test-padding 		\
	test-icon-view 		\
	test-border-image	\
	test-scroll-bar		\
	test-scroll-grid	\
	test-scroll-view-2	\
	test-scroll-view-3	\
	$(NULL)

test_buttons_SOURCES = test-buttons.c
test_script_SOURCES = test-script.c
test_styles_SOURCES = test-styles.c
test_table_SOURCES = test-table.c
test_table_2_SOURCES = test-table-2.c
test_tooltips_SOURCES = test-tooltips.c
test_texture_frame_SOURCES = test-texture-frame.c
test_label_SOURCES = test-label.c
test_entry_SOURCES = test-entry.c
test_expander_SOURCES = test-expander.c
test_grid_SOURCES = test-grid.c
test_icon_view_SOURCES = test-icon-view.c
test_padding_SOURCES = test-padding.c
test_border_image_SOURCES = test-border-image.c

test_scroll_bar_SOURCES = test-scroll-bar.c
test_scroll_grid_SOURCES = test-scroll-grid.c
test_scroll_view_2_SOURCES = test-scroll-view-2.c
test_scroll_view_3_SOURCES = test-scroll-view-3.c

EXTRA_DIST = \
        redhand.png rounded-corner.png \
        style/border-image-dark.png \
        style/border-image-light.png \
        style/border-image.png \
        style/cyan-button.png \
        style/default.css \
        style/green-button.png \
        style/lime-button.png \
        style/red-button.png \
        style/redhand.png \
        style/rounded-corner.png
