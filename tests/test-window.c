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
small_screen_cb (MxToggle   *toggle,
                 GParamSpec *pspec,
                 MxWindow   *window)
{
  mx_window_set_small_screen (window, mx_toggle_get_active (toggle));
}

static void
fullscreen_cb (MxToggle     *toggle,
               GParamSpec   *pspec,
               ClutterStage *stage)
{
  clutter_stage_set_fullscreen (stage, mx_toggle_get_active (toggle));
}

int
main (int argc, char **argv)
{
  MxApplication *app;
  ClutterActor *stage, *toggle, *label, *table;

  app = mx_application_new (&argc, &argv, "Test PathBar", 0);

  stage = (ClutterActor *)mx_application_create_window (app);
  mx_window_set_icon_name (MX_WINDOW (stage), "window-new");

  clutter_actor_set_size (stage, 480, 320);

  table = mx_table_new ();
  mx_table_set_column_spacing (MX_TABLE (table), 8);
  mx_table_set_row_spacing (MX_TABLE (table), 12);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), table);

  toggle = mx_toggle_new ();
  label = mx_label_new ("Toggle small-screen mode");
  g_signal_connect (toggle, "notify::active",
                    G_CALLBACK (small_screen_cb), stage);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      toggle,
                                      0, 0,
                                      "x-expand", TRUE,
                                      "x-align", 1.0,
                                      "x-fill", FALSE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      0, 1,
                                      "x-expand", TRUE,
                                      "x-align", 0.0,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);

  toggle = mx_toggle_new ();
  label = mx_label_new ("Toggle full-screen mode");
  g_signal_connect (toggle, "notify::active",
                    G_CALLBACK (fullscreen_cb), stage);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      toggle,
                                      1, 0,
                                      "x-expand", TRUE,
                                      "x-align", 1.0,
                                      "x-fill", FALSE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      1, 1,
                                      "x-expand", TRUE,
                                      "x-align", 0.0,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);

  clutter_actor_show (stage);

  mx_application_run (app);

  return 0;
}
