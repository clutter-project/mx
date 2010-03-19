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
 * Boston, MA 02111-1307, USA.
 *
 */
#include "test-mx.h"

static void
button_clicked_cb (MxButton *button, gchar *name)
{
  printf ("%s button clicked!\n", name);
}

static gboolean
button_long_press_cb (MxButton *button, gfloat x, gfloat y,
                      MxLongPressAction action)
{
  if (action == MX_LONG_PRESS_ACTION)
    printf ("long press detected\n");
  else if (action == MX_LONG_PRESS_CANCEL)
    printf ("long press cancelled\n");

  return TRUE;
}

void
buttons_main (ClutterContainer *stage)
{
  ClutterActor *button;

  button = mx_button_new_with_label ("Normal Button");
  g_signal_connect (button, "long-press", G_CALLBACK (button_long_press_cb),
                    NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), button);
  clutter_actor_set_position (button, 100, 50);

  button = mx_button_new_with_label ("Toggle Button");
  g_signal_connect (button, "clicked", G_CALLBACK (button_clicked_cb),
                    "toggle");
  g_signal_connect (button, "long-press", G_CALLBACK (button_long_press_cb),
                    NULL);
  mx_button_set_is_toggle (MX_BUTTON (button), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), button);
  clutter_actor_set_position (button, 100, 100);
}
