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

static void
close_dialog_cb (gpointer      unused,
                 ClutterActor *dialog)
{
  clutter_actor_hide (dialog);
}

void
dialog_main (ClutterContainer *group)
{
  ClutterActor *stage, *launch, *dialog, *label;
  MxAction *action;

  launch = mx_button_new_with_label ("Launch dialog");
  dialog = mx_dialog_new ();

  clutter_actor_set_position (launch, 50, 50);
  clutter_container_add (group, launch, NULL);
  g_signal_connect_swapped (launch, "clicked",
                            G_CALLBACK (clutter_actor_show), dialog);

  label = mx_label_new_with_text ("This is a dialog");
  mx_bin_set_child (MX_BIN (dialog), label);

  action = mx_action_new_full ("nothing", "Do nothing", NULL, dialog);
  mx_dialog_add_action (MX_DIALOG (dialog), action);

  action = mx_action_new_full ("close", "Close",
                               G_CALLBACK (close_dialog_cb), dialog);
  mx_dialog_add_action (MX_DIALOG (dialog), action);

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (group));
  mx_dialog_set_transient_parent (MX_DIALOG (dialog),
                                  stage ? stage : CLUTTER_ACTOR (group));
}
