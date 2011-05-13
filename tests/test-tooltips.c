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

void
tooltips_main (ClutterContainer *stage)
{
  ClutterActor *button, *bin;
  guint delay;

  button = mx_button_new_with_label ("Hide Me");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (clutter_actor_hide),
                    NULL);
  mx_widget_set_tooltip_text (MX_WIDGET (button), "Disappear");
  g_object_set (G_OBJECT (button), "tooltip-delay", 1000, NULL);
  g_object_get (G_OBJECT (button), "tooltip-delay", &delay, NULL);
  g_assert (delay == 1000);
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 50, 100);

  bin = mx_frame_new ();
  clutter_container_add_actor (stage, bin);
  clutter_actor_set_position (bin, 50, 200);

  button = mx_button_new_with_label ("Testing 123");
  mx_widget_set_tooltip_text (MX_WIDGET (button),
                              "Testing tooltips in a container...");
  mx_widget_set_tooltip_delay (MX_WIDGET (button), 300);
  delay = mx_widget_get_tooltip_delay (MX_WIDGET (button));
  g_assert (delay == 300);
  clutter_container_add_actor (CLUTTER_CONTAINER (bin), button);


  button = mx_button_new_with_label ("Testing Long Text on left");
  mx_widget_set_tooltip_text (MX_WIDGET (button), "Here is some really really"
                              " long text to test the handling in MxTooltip");
  mx_widget_set_tooltip_delay (MX_WIDGET (button), 0);
  delay = mx_widget_get_tooltip_delay (MX_WIDGET (button));
  g_assert (delay == 0);
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 0, 300);

  button = mx_button_new_with_label ("Testing Long Text on right");
  mx_widget_set_tooltip_text (MX_WIDGET (button), "Here is some really really"
                              " long text to test the handling in MxTooltip");
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 460, 300);

  button = mx_button_new_with_label ("Crazy");
  mx_widget_set_tooltip_text (MX_WIDGET (button),
                              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 250, 5);

  button = mx_button_new_with_label ("Bottom");
  mx_widget_set_tooltip_text (MX_WIDGET (button), "Hello Hello Hello");
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 250, 570);

  button = mx_button_new_with_label ("Bottom");
  mx_widget_set_tooltip_text (MX_WIDGET (button), "Hello Hello Hello");
  clutter_container_add_actor (stage, button);
  clutter_actor_set_position (button, 250, 700);
}
