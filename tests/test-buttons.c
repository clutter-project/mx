#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

void
button_clicked_cb (NbtkButton *button, gchar *name)
{
  printf ("%s button clicked!\n", name);
}

int
main (int argc, char *argv[])
{
  NbtkWidget *button;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *stage, *icon;

  clutter_init (&argc, &argv);

  /* load the style sheet */
  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = nbtk_fade_button_new_with_label ("Fade World!");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb),
                    "fade");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 100, 100);

  button = nbtk_button_new ();
  clutter_actor_set_name (CLUTTER_ACTOR (button), "icon-button");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb),
                    "icon");
  nbtk_button_set_icon_from_file (NBTK_BUTTON (button),
                                  "redhand.png");
  nbtk_button_set_label (NBTK_BUTTON (button), "Red Hand");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 300, 100);

  button = nbtk_button_new_with_label ("icon actor");
  icon = (ClutterActor *) nbtk_bin_new ();
  clutter_actor_set_name (icon, "button-icon");
  clutter_actor_set_size (icon, 16, 16);
  nbtk_button_set_icon (NBTK_BUTTON (button), icon);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 300, 380);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb),
                    "actor icon");

  button = nbtk_button_new_with_label ("Toggle");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb),
                    "toggle");
  nbtk_button_set_toggle_mode (NBTK_BUTTON (button), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 100, 200);

  button = nbtk_button_new ();
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb),
                    "style");
  clutter_actor_set_name (CLUTTER_ACTOR (button), "style-button");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 100, 300);
  clutter_actor_set_size (CLUTTER_ACTOR (button), 100, 100);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
