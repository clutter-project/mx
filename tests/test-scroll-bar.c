
#include <nbtk/nbtk.h>
#include <nbtk/nbtk-scroll-button.h>

static void
clicked_cb (NbtkButton *button,
            gpointer    data)
{
  printf ("%s()\n", __FUNCTION__);
}

int
main (int argc, char *argv[])
{
  NbtkWidget *button;
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  button = nbtk_scroll_button_new ();
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 50);
  clutter_actor_set_size (CLUTTER_ACTOR (button), 24, 24);
  g_signal_connect (button, "clicked", 
                    G_CALLBACK (clicked_cb), NULL);

  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (button), NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
