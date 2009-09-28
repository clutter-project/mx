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
progress_bar_main (ClutterContainer *stage)
{
  ClutterActor *progress_bar;
  ClutterAnimation *animation;

  progress_bar = mx_progress_bar_new ();
  animation = clutter_actor_animate (progress_bar,
                                     CLUTTER_LINEAR,
                                     5000,
                                     "progress", 1.0,
                                     NULL);
  clutter_animation_set_loop (animation, TRUE);

  clutter_container_add_actor (stage, progress_bar);
  clutter_actor_set_width (progress_bar, 280);
  clutter_actor_set_position (progress_bar, 20, 20);
}

