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
scroll_view_main (ClutterContainer *stage)
{
  gint width, height;
  MxAdjustment *hadjust, *vadjust;
  ClutterActor *scroll, *kinetic, *view, *texture;
/*
  scroll = mx_scroll_view_new ();
  mx_scroll_view_set_enable_gestures (MX_SCROLL_VIEW (scroll), TRUE);

  clutter_container_add_actor (stage, scroll);
  clutter_actor_set_position (scroll, 10, 10);
  clutter_actor_set_size (scroll, 300, 300);

  view = (ClutterActor *) mx_viewport_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), view);


  texture = clutter_texture_new_from_file ("redhand.png", NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), texture);
  g_object_set (texture, "repeat-x", TRUE, "repeat-y", TRUE, NULL);
  clutter_actor_set_size (texture, 800, 639);
*/



  scroll = mx_scroll_view_new ();
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
}
