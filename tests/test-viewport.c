#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  ClutterActor  *stage;
  NbtkWidget    *view, *table, *label;
  int i;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 200, 200);

  view = (NbtkWidget *) nbtk_viewport_new ();
  clutter_actor_set_position (CLUTTER_ACTOR (view), 30, 30);
  clutter_actor_set_height (CLUTTER_ACTOR (view), 10);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (view), NULL);

  table = nbtk_table_new ();
  nbtk_bin_set_child (NBTK_BIN (view), CLUTTER_ACTOR (table));

  for (i = 0; i < 3; i++)
    {
      char *text = g_strdup_printf ("label %d", i);
      label = nbtk_label_new (text);
      nbtk_table_add_widget (NBTK_TABLE (table), label, i, 0);
      g_free (text);
    }

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
