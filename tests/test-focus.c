/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2011 Intel Corporation.
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

#include <stdlib.h>
#include "test-mx.h"

static void
button_clicked_cb (MxButton *button)
{
  clutter_actor_destroy (CLUTTER_ACTOR (button));
}

int
main (int argc, char *argv[])
{
  ClutterActor *button, *box, *stage;
  MxApplication *application;
  MxWindow *window;

  application = mx_application_new (&argc, &argv, "Test Mx focus handling",
                                    MX_APPLICATION_SINGLE_INSTANCE);

  window = mx_application_create_window (application);
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);

  box = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (box), MX_ORIENTATION_VERTICAL);
  mx_window_set_child (window, box);

  button = mx_button_new_with_label ("button #1 (activate to remove)");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb), NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), button, -1);

  mx_focus_manager_push_focus (mx_focus_manager_get_for_stage (CLUTTER_STAGE (stage)),
                               MX_FOCUSABLE (button));


  button = mx_button_new_with_label ("button #2 (activate to remove)");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb), NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), button, -1);

  button = mx_button_new_with_label ("button #3 (activate to remove)");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb), NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), button, -1);

  button = mx_button_new_with_label ("button #4 (activate to remove)");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_clicked_cb), NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), button, -1);

  clutter_actor_show (stage);

  mx_application_run (application);

  return EXIT_SUCCESS;
}
