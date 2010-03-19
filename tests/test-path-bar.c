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
add_cb (MxPathBar *bar)
{
  gchar *string = g_strdup_printf ("Crumb %d", mx_path_bar_get_level (bar) + 1);
  mx_path_bar_push (bar, string);
  g_free (string);
}

static void
remove_cb (MxPathBar *bar)
{
  mx_path_bar_pop (bar);
}

static void
toggle_editable_cb (MxPathBar *bar)
{
  ClutterActor *parent = clutter_actor_get_parent (CLUTTER_ACTOR (bar));
  gboolean editable = !mx_path_bar_get_editable (bar);
  mx_path_bar_set_editable (bar, editable);
  if (MX_IS_BIN (parent))
    mx_bin_set_fill (MX_BIN (parent), editable, FALSE);
  else if (MX_IS_BOX_LAYOUT (parent))
    clutter_container_child_set (CLUTTER_CONTAINER (parent),
                                 CLUTTER_ACTOR (bar),
                                 "x-fill", editable,
                                 NULL);
}

static void
relabel_cb (MxPathBar *bar)
{
  const gchar *string = mx_path_bar_get_text (bar);
  mx_path_bar_set_label (bar, 1, string ? string : "");
}

int
main (int argc, char **argv)
{
  MxWindow *window;
  MxApplication *app;
  ClutterActor *stage, *bar, *button, *hbox;

  app = mx_application_new (&argc, &argv, "Test PathBar", 0);

  window = mx_application_create_window (app);
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);

  bar = mx_path_bar_new ();
  mx_path_bar_set_clear_on_change (MX_PATH_BAR (bar), TRUE);

  hbox = mx_box_layout_new ();

  button = mx_button_new_with_label ("Add crumb");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (add_cb), bar);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  button = mx_button_new_with_label ("Remove crumb");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (remove_cb), bar);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  button = mx_button_new_with_label ("Toggle editable");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (toggle_editable_cb), bar);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  button = mx_button_new_with_label ("Re-label first crumb");
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (relabel_cb), bar);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  /* -a for 'alternative packing'... */
  if (argc > 1 && g_str_equal (argv[1], "-a"))
    {
      ClutterActor *vbox = mx_box_layout_new ();
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (vbox), MX_ORIENTATION_VERTICAL);
      clutter_container_add_actor (CLUTTER_CONTAINER (vbox), bar);
      clutter_container_add_actor (CLUTTER_CONTAINER (vbox), hbox);
      mx_window_set_child (window, vbox);
      mx_path_bar_set_editable (MX_PATH_BAR (bar), TRUE);
    }
  else
    {
      MxToolbar *toolbar = mx_window_get_toolbar (window);
      clutter_container_add_actor (CLUTTER_CONTAINER (toolbar), bar);
      mx_window_set_child (window, hbox);
    }

  clutter_actor_show (stage);

  mx_application_run (app);

  return 0;
}
