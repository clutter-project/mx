#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

static void
button_hide (NbtkButton *button)
{
  clutter_actor_hide (CLUTTER_ACTOR (button));
}

int
main (int argc, char *argv[])
{
  NbtkWidget *button;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  /* load the style sheet */
  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = nbtk_button_new_with_label ("Hide Me");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_hide),
                    "hello");
  nbtk_widget_set_tooltip_text (button, "Disappear");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 100);

  button = nbtk_button_new_with_label ("Testing 123");
  nbtk_widget_set_tooltip_text (button, "Testing the Tooltips...");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 200);


  button = nbtk_button_new_with_label ("Testing Long Text");
  nbtk_widget_set_tooltip_text (button,
                           "Here is some really"
                           " long text to test the handling in NbtkTooltip");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 300);

  button = nbtk_button_new_with_label ("Testing Long Text");
  nbtk_widget_set_tooltip_text (button,
                           "Here is some really"
                           " long text to test the handling in NbtkTooltip");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 500, 300);

  button = nbtk_button_new_with_label ("Crazy");
  nbtk_widget_set_tooltip_text (button,
                           "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 250, 5);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
