#include <nbtk/nbtk.h>


static void
title_changed_cb (NbtkComboBox *box)
{
  printf ("title now: %s\n", nbtk_combo_box_get_title (box));
}

static void
index_changed_cb (NbtkComboBox *box)
{
  printf ("index now: %d\n", nbtk_combo_box_get_index (box));
}

static gboolean
stage_key_press_cb (ClutterActor *actor,
                    ClutterKeyEvent *event,
                    NbtkComboBox *box)
{
  if (event->keyval == 'r')
    {
      nbtk_combo_box_set_title (box, "London");
    }

  if (event->keyval >= '0' && event->keyval <= '9')
    {
      nbtk_combo_box_set_index (box, event->keyval - 48);
    }

  return FALSE;
}


int
main (int argc, char **argv)
{
  ClutterActor *stage;
  ClutterActor *combo;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  combo = (ClutterActor*) nbtk_combo_box_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), combo);
  clutter_actor_set_position (combo, 10, 10);

  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Strand");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Fleet Street");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Trafalgar Square");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Leicester Square");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Coventry Street");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Piccadilly");
  nbtk_combo_box_set_title (NBTK_COMBO_BOX (combo), "London");

  g_signal_connect (combo, "notify::title", G_CALLBACK (title_changed_cb),
                    NULL);
  g_signal_connect (combo, "notify::index", G_CALLBACK (index_changed_cb),
                    NULL);

  g_signal_connect (stage, "key-press-event", G_CALLBACK (stage_key_press_cb),
                    combo);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
