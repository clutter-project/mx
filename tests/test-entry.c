#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

gboolean
set_focus (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  clutter_actor_grab_key_focus (actor);

  return FALSE;
}

void
btn_clicked_cb (ClutterActor *button, NbtkEntry *entry)
{
  nbtk_entry_set_text (entry, "Here is some text");
}

void
clear_btn_clicked_cb (ClutterActor *button, NbtkEntry *entry)
{
  nbtk_entry_set_text (entry, "");
}

int
main (int argc, char *argv[])
{
  NbtkWidget *entry;
  ClutterActor *stage;
  NbtkWidget *button, *clear_button;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 300);

  entry = nbtk_entry_new ("Hello World!");
  clutter_actor_set_position (CLUTTER_ACTOR (entry), 20, 20);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         CLUTTER_ACTOR (entry), NULL);
  g_signal_connect (entry, "button-press-event",
                    G_CALLBACK (set_focus), NULL);

  clutter_stage_set_key_focus (CLUTTER_STAGE (stage),
                               CLUTTER_ACTOR (nbtk_entry_get_clutter_text (NBTK_ENTRY (entry))));

  entry = nbtk_entry_new ("");
  clutter_actor_set_position (CLUTTER_ACTOR (entry), 20, 70);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         CLUTTER_ACTOR (entry), NULL);
  nbtk_entry_set_hint_text (NBTK_ENTRY (entry), "hint hint...");
  g_signal_connect (entry, "button-press-event",
                    G_CALLBACK (set_focus), NULL);

  button = nbtk_button_new_with_label ("Set");
  clutter_actor_set_position (CLUTTER_ACTOR (button), 20, 120);
  g_signal_connect (button, "clicked", G_CALLBACK (btn_clicked_cb), entry);

  clear_button = nbtk_button_new_with_label ("clear");
  clutter_actor_set_position (CLUTTER_ACTOR (clear_button), 70, 120);
  g_signal_connect (clear_button, "clicked",
                    G_CALLBACK (clear_btn_clicked_cb), entry);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         CLUTTER_ACTOR (button),
                         CLUTTER_ACTOR (clear_button),
                         NULL);


  entry = nbtk_entry_new ("");
  clutter_actor_set_position (CLUTTER_ACTOR (entry), 20, 170);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         CLUTTER_ACTOR (entry), NULL);
  nbtk_entry_set_hint_text (NBTK_ENTRY (entry), "Search...");
  nbtk_entry_set_primary_icon_from_file (NBTK_ENTRY (entry),
                                         "edit-find.png");
  nbtk_entry_set_secondary_icon_from_file (NBTK_ENTRY (entry),
                                           "edit-clear.png");
  g_signal_connect_swapped (entry, "primary-icon-clicked",
                            G_CALLBACK (g_print), "primary icon clicked\n");
  g_signal_connect_swapped (entry, "secondary-icon-clicked",
                            G_CALLBACK (g_print), "secondary icon clicked\n");
  g_signal_connect (entry, "button-press-event",
                    G_CALLBACK (set_focus), NULL);


  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
