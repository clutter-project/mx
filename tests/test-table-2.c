#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int     argc,
      char  **argv)
{
  NbtkWidget *table, *label;
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  table = nbtk_table_new ();
  nbtk_table_set_col_spacing (NBTK_TABLE (table), 10);
  clutter_actor_set_position (CLUTTER_ACTOR (table), 50, 50);
  clutter_actor_set_size (CLUTTER_ACTOR (table), 50, 50);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (table), NULL);

  label = nbtk_label_new ("Short");
  nbtk_table_add_widget (NBTK_TABLE (table), label, 0, 0);

  label = nbtk_label_new ("Loooooooooooooooong text");
  nbtk_table_add_widget (NBTK_TABLE (table), label, 0, 1);

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
