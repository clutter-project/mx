/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2010 Intel Corporation.
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
modal_frame_main (ClutterContainer *group)
{
  ClutterActor *stage, *launch, *dialog, *frame, *close;

  launch = mx_button_new_with_label ("Launch dialog");

  dialog = mx_modal_frame_new ();

  frame = mx_frame_new ();
  close = mx_button_new_with_label ("Close dialog");
  mx_bin_set_child (MX_BIN (frame), close);

  mx_bin_set_child (MX_BIN (dialog), frame);

  clutter_actor_set_position (launch, 50, 50);

  clutter_container_add (group, launch, NULL);

  g_signal_connect_swapped (launch, "clicked",
                            G_CALLBACK (clutter_actor_show), dialog);
  g_signal_connect_swapped (close, "clicked",
                            G_CALLBACK (clutter_actor_hide), dialog);

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (group));
  mx_modal_frame_set_transient_parent (MX_MODAL_FRAME (dialog),
                                       stage ? stage : CLUTTER_ACTOR (group));
}
