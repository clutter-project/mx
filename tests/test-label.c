#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk.h>

int
main (int argc, char *argv[])
{
  NbtkWidget *button, *label;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *stage;
  GError *err = NULL;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  label = nbtk_label_new ("Hello World!");
  clutter_actor_set_position (CLUTTER_ACTOR (label), 50, 50);

  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (label), NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
