#include "test-mx.h"

static void
title_changed_cb (MxComboBox *box)
{
  printf ("title now: %s\n", mx_combo_box_get_title (box));
}

static void
index_changed_cb (MxComboBox *box)
{
  printf ("index now: %d\n", mx_combo_box_get_index (box));
}

static gboolean
stage_key_press_cb (ClutterActor *actor,
                    ClutterKeyEvent *event,
                    MxComboBox *box)
{
  if (event->keyval == 'r')
    {
      mx_combo_box_set_title (box, "London");
    }

  if (event->keyval >= '0' && event->keyval <= '9')
    {
      mx_combo_box_set_index (box, event->keyval - 48);
    }

  return FALSE;
}


void
combo_box_main (ClutterContainer *stage)
{
  ClutterActor *combo;

  combo = (ClutterActor*) mx_combo_box_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), combo);
  clutter_actor_set_position (combo, 10, 10);

  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Strand");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Fleet Street");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Trafalgar Square");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Leicester Square");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Coventry Street");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Piccadilly");
  mx_combo_box_set_title (MX_COMBO_BOX (combo), "London");

  g_signal_connect (combo, "notify::title", G_CALLBACK (title_changed_cb),
                    NULL);
  g_signal_connect (combo, "notify::index", G_CALLBACK (index_changed_cb),
                    NULL);

  g_signal_connect (stage, "key-press-event", G_CALLBACK (stage_key_press_cb),
                    combo);

}
