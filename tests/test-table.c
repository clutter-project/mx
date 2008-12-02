#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  ClutterActor *stage;
  NbtkWidget *table;
  NbtkWidget *button1, *button2, *button3, *button4, *button5;
  NbtkPadding padding = {CLUTTER_UNITS_FROM_INT (10),
                         CLUTTER_UNITS_FROM_INT (30),
                         CLUTTER_UNITS_FROM_INT (10),
                         CLUTTER_UNITS_FROM_INT (10)};

  clutter_init (&argc, &argv);

  /* load the style sheet */
  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();

  table = nbtk_table_new ();
  clutter_actor_set_position (CLUTTER_ACTOR (table), 10, 10);
  nbtk_widget_set_padding (table, &padding);
  nbtk_table_set_col_spacing (NBTK_TABLE (table), 10);
  nbtk_table_set_row_spacing (NBTK_TABLE (table), 20);

  button1 = nbtk_button_new_with_label ("button1");
  button2 = nbtk_button_new_with_label ("button2");
  button3 = nbtk_button_new_with_label ("button3");
  button4 = nbtk_button_new_with_label ("button4");

  nbtk_table_add_widget (NBTK_TABLE (table), NBTK_WIDGET (button1), 0, 0);
  nbtk_table_add_widget (NBTK_TABLE (table), NBTK_WIDGET (button2), 0, 1);
  nbtk_table_add_widget (NBTK_TABLE (table), NBTK_WIDGET (button3), 1, 1);
  nbtk_table_add_widget (NBTK_TABLE (table), NBTK_WIDGET (button4), 2, 0);
  nbtk_table_set_widget_rowspan (NBTK_TABLE (table), NBTK_WIDGET (button1), 2);
  nbtk_table_set_widget_colspan (NBTK_TABLE (table), NBTK_WIDGET (button4), 2);

  clutter_actor_set_size (CLUTTER_ACTOR (button2), 10, 10);
  clutter_container_child_set (CLUTTER_CONTAINER (table), CLUTTER_ACTOR (button2),
                               "keep-aspect-ratio", TRUE, NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (table));
  nbtk_table_set_active_row (NBTK_TABLE (table), 1);
  nbtk_table_set_active_col (NBTK_TABLE (table), 1);
  clutter_actor_set_size (CLUTTER_ACTOR (table), 300, 300);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
