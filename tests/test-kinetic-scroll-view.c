/*
 * Copyright 2013 Collabora Ltd.
 *
 * Authors: Rodrigo Moya <rodrigo.moya@collabora.co.uk>
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
startup_cb (MxApplication *application)
{
  MxWindow *window;
  ClutterActor *scroll, *child;
  gint i;

  window = mx_application_create_window (application, "Test Widgets");

  /* Create scroll view */
  scroll = mx_kinetic_scroll_view_new ();
  mx_kinetic_scroll_view_set_use_grab (MX_KINETIC_SCROLL_VIEW (scroll), TRUE);
  mx_kinetic_scroll_view_set_mouse_button (MX_KINETIC_SCROLL_VIEW (scroll), 3);
  clutter_actor_set_clip_to_allocation (scroll, TRUE);

  child = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);

  for (i = 0; i < 10000; i++) {
    ClutterActor *layout, *label, *icon;
    gchar *s = g_strdup_printf ("Row %d", i);

    layout = mx_box_layout_new_with_orientation (MX_ORIENTATION_HORIZONTAL);
    label = mx_label_new_with_text (s);
    clutter_actor_add_child (layout, label);
    g_free (s);

    icon = mx_icon_new ();
    mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-left");
    mx_icon_set_icon_size (MX_ICON (icon), 32);
    clutter_actor_add_child (layout, icon);

    clutter_actor_add_child (child, layout);
  }

  clutter_actor_add_child (scroll, child);

  mx_window_set_child (window, scroll);

  /* show the window */
  mx_window_set_has_toolbar (window, FALSE);
  mx_window_show (window);
}

int
main (int argc, char *argv[])
{
  MxApplication *application;

  application = mx_application_new ("org.clutter-project.Mx.TestKineticScrollView", 0);
  g_signal_connect_after (application, "startup", G_CALLBACK (startup_cb), NULL);

  /* run the application */
  g_application_run (G_APPLICATION (application), argc, argv);

  return 0;
}
