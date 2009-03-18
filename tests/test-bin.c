#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  ClutterActor *stage;
  ClutterActor *bin, *button;
  NbtkPadding   padding = { 10, 20, 30, 40 };

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  bin = (ClutterActor *) nbtk_bin_new ();
  nbtk_bin_set_padding (NBTK_BIN (bin), &padding);
  clutter_actor_set_position (CLUTTER_ACTOR (bin), 50, 50);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (bin), NULL);

  button = (ClutterActor *) nbtk_button_new_with_label ("Hello World!");
  nbtk_bin_set_child (NBTK_BIN (bin), button);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
