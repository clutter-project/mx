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

#include <mx/mx.h>

static void
small_screen_cb (MxWindow *window)
{
  mx_window_set_small_screen (window, !mx_window_get_small_screen (window));
}

static void
fullscreen_cb (ClutterStage *stage)
{
  clutter_stage_set_fullscreen (stage, !clutter_stage_get_fullscreen (stage));
}

int
main (int argc, char **argv)
{
  MxApplication *app;
  ClutterActor *stage, *button, *hbox;

  app = mx_application_new (&argc, &argv, "Test PathBar", 0);

  stage = (ClutterActor *)mx_application_create_window (app);

  hbox = mx_box_layout_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), hbox);

  button = mx_button_new_with_label ("Toggle small-screen mode");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (small_screen_cb), stage);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);
  clutter_container_child_set (CLUTTER_CONTAINER (hbox),
                               button,
                               "x-fill", FALSE,
                               "y-fill", FALSE,
                               NULL);

  button = mx_button_new_with_label ("Toggle fullscreen mode");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (fullscreen_cb), stage);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);
  clutter_container_child_set (CLUTTER_CONTAINER (hbox),
                               button,
                               "x-fill", FALSE,
                               "y-fill", FALSE,
                               NULL);

  clutter_actor_show (stage);

  mx_application_run (app);

  return 0;
}
