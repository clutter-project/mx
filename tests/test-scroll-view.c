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
notify_overshoot_cb (MxToggle            *toggle,
                     GParamSpec          *pspec,
                     MxKineticScrollView *view)
{
  gboolean on = mx_toggle_get_active (toggle);
  mx_kinetic_scroll_view_set_overshoot (view, on ? 0.2 : 0.0);
}

static void
notify_elastic_cb (MxToggle     *toggle,
                   GParamSpec   *pspec,
                   MxScrollable *view)
{
  MxAdjustment *hadjust, *vadjust;
  gboolean on = mx_toggle_get_active (toggle);

  mx_scrollable_get_adjustments (view, &hadjust, &vadjust);
  mx_adjustment_set_elastic (hadjust, on);
  mx_adjustment_set_elastic (vadjust, on);
}

static gboolean
true_cb ()
{
  return TRUE;
}

void
scroll_view_main (ClutterContainer *stage)
{
  gint width, height;
  MxAdjustment *hadjust, *vadjust;
  ClutterActor *label, *elastic, *overshoot, *scroll, *kinetic, *view, *texture;

  scroll = mx_scroll_view_new ();

  /* Make sure something underneath the kinetic scroll view swallows events
   * so that we don't end up moving the window.
   */
  g_signal_connect (scroll, "button-press-event", G_CALLBACK (true_cb), NULL);

  kinetic = mx_kinetic_scroll_view_new ();

  clutter_container_add_actor (stage, scroll);
  clutter_actor_set_position (scroll, 10, 10);
  clutter_actor_set_size (scroll, 300, 300);

  view = mx_viewport_new ();
  mx_viewport_set_sync_adjustments (MX_VIEWPORT (view), FALSE);
  clutter_container_add_actor (CLUTTER_CONTAINER (kinetic), view);
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), kinetic);


  texture = clutter_texture_new_from_file ("redhand.png", NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), texture);
  g_object_set (texture, "repeat-x", TRUE, "repeat-y", TRUE, NULL);
  clutter_actor_set_size (texture, 1280, 1280);

  clutter_texture_get_base_size (CLUTTER_TEXTURE (texture),
                                 &width, &height);
  mx_scrollable_get_adjustments (MX_SCROLLABLE (view),
                                 &hadjust, &vadjust);
  mx_adjustment_set_values (hadjust,
                            0,
                            0,
                            1280,
                            width,
                            width * 3,
                            300);
  mx_adjustment_set_values (vadjust,
                            0,
                            0,
                            1280,
                            height,
                            height * 3,
                            300);

  label = mx_label_new_with_text ("Toggle over-shooting:");
  overshoot = mx_toggle_new ();
  clutter_actor_set_position (label, 320, 10);
  clutter_actor_set_position (overshoot, 330 + clutter_actor_get_width (label),
                              10);
  clutter_container_add (stage, label, overshoot, NULL);

  g_signal_connect (overshoot, "notify::active",
                    G_CALLBACK (notify_overshoot_cb), kinetic);

  label = mx_label_new_with_text ("Toggle elasticity:");
  elastic = mx_toggle_new ();
  clutter_actor_set_position (label, 320,
                              20 + clutter_actor_get_height (overshoot));
  clutter_actor_set_position (elastic, clutter_actor_get_x (overshoot),
                              clutter_actor_get_y (label));
  clutter_container_add (stage, label, elastic, NULL);

  g_signal_connect (elastic, "notify::active",
                    G_CALLBACK (notify_elastic_cb), kinetic);
}
