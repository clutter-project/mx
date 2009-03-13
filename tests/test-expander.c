#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  NbtkWidget *expander, *label;
  ClutterActor *stage;
  GError *err = NULL;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  expander = nbtk_expander_new ("Test");
  clutter_actor_set_width (CLUTTER_ACTOR (expander), 100);
  clutter_actor_set_position (CLUTTER_ACTOR (expander), 50, 50);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (expander), NULL);

  label = nbtk_label_new ("Payload");
  clutter_container_add (CLUTTER_CONTAINER (expander),
                         CLUTTER_ACTOR (label), NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
