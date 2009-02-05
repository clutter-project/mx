NULL =

AM_CFLAGS = $(NBTK_CFLAGS)
LDADD = $(top_builddir)/nbtk/libnbtk-$(NBTK_API_VERSION).la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)

noinst_PROGRAMS = 		\
	test-buttons 		\
	test-label 		\
	test-entry 		\
	test-script  		\
	test-styles  		\
	test-table   		\
	test-texture-frame 	\
	test-padding 		\
	test-border-image	\
	$(NULL)

test_buttons_SOURCES = test-buttons.c
test_script_SOURCES = test-script.c
test_styles_SOURCES = test-styles.c
test_table_SOURCES = test-table.c
test_texture_frame_SOURCES = test-texture-frame.c
test_label_SOURCES = test-label.c
test_entry_SOURCES = test-entry.c
test_padding_SOURCES = test-padding.c
test_border_image_SOURCES = test-border-image.c
