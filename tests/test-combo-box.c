/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2009 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 */
#include "test-mx.h"

static void
title_changed_cb (MxComboBox *box)
{
  printf ("title now: %s\n", mx_combo_box_get_active_text (box));
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
      mx_combo_box_set_active_text (box, "London");
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
  mx_combo_box_set_active_text (MX_COMBO_BOX (combo), "London");

  g_signal_connect (combo, "notify::title", G_CALLBACK (title_changed_cb),
                    NULL);
  g_signal_connect (combo, "notify::index", G_CALLBACK (index_changed_cb),
                    NULL);

  g_signal_connect (stage, "key-press-event", G_CALLBACK (stage_key_press_cb),
                    combo);

  combo = mx_combo_box_new ();
  clutter_actor_set_position (combo, 20, 40);

  mx_combo_box_insert_text_with_icon (MX_COMBO_BOX (combo), 0, "Cloudy",
                                      "weather-few-clouds");
  mx_combo_box_insert_text_with_icon (MX_COMBO_BOX (combo), 0, "Clear",
                                      "weather-clear");
  mx_combo_box_insert_text_with_icon (MX_COMBO_BOX (combo), 0, "Snow",
                                      "weather-snow");
  mx_combo_box_insert_text_with_icon (MX_COMBO_BOX (combo), 0, "Overcast",
                                      "weather-overcast");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), combo);
}
