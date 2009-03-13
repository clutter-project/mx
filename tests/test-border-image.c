#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  NbtkWidget *button, *label;
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 300);

  button = nbtk_button_new_with_label ("Fancy Button");
  clutter_actor_set_name (CLUTTER_ACTOR (button), "test-border-image");
  clutter_actor_set_size (CLUTTER_ACTOR (button), -1, 50);
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 50);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (button), NULL);

  label = nbtk_label_new ("Fancy Label");
  clutter_actor_set_name (CLUTTER_ACTOR (label), "test-border-image");
  clutter_actor_set_position (CLUTTER_ACTOR (label), 50, 150);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (label), NULL);

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
