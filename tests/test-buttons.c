#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk.h>

int
main (int argc, char *argv[])
{
  NbtkWidget *button;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  NbtkPadding padding = { 0, };
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = nbtk_button_new_with_label ("Hello");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 100, 100);
  clutter_actor_set_size (CLUTTER_ACTOR (button), 100, 100);

  button = nbtk_button_new ();
  nbtk_button_set_icon_from_file (NBTK_BUTTON (button),
                                  "redhand.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 300, 100);
  padding.left = CLUTTER_UNITS_FROM_DEVICE (20);
  padding.right = CLUTTER_UNITS_FROM_DEVICE (20);
  padding.top = CLUTTER_UNITS_FROM_DEVICE (20);
  padding.bottom = CLUTTER_UNITS_FROM_DEVICE (20);
  nbtk_widget_set_padding (NBTK_WIDGET (button), &padding);
  clutter_actor_set_size (CLUTTER_ACTOR (button), 100, 100);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
