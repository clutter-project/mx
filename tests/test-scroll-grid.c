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
#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <mx/mx.h>

static void
stage_size_notify_cb (ClutterActor *stage,
                      GParamSpec *pspec,
                      ClutterActor *table)
{
  gfloat width, height;

  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_size (table, width - 100, height - 100);
}

static void
swap_orientation (ClutterActor *button,
                  MxGrid     *grid)
{
  mx_grid_set_column_major (grid,
                              !mx_grid_get_column_major (grid));
}

static void
set_max_stride (ClutterActor    *actor,
                ClutterKeyEvent *event,
                MxGrid        *grid)
{
  mx_grid_set_max_stride (grid, event->keyval - 48);
  printf ("Max Stride: %d\n", event->keyval - 48);
}

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *scroll, *grid;
  int i;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  scroll = mx_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);
  clutter_actor_set_position (scroll, 50, 50);

  grid = mx_grid_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), grid);

  for (i = 1; i <= 200; i++)
    {
      ClutterActor *button;
      gchar *label;

      label = g_strdup_printf ("Button %d", i);

      button = mx_button_new_with_label (label);
      clutter_container_add_actor (CLUTTER_CONTAINER (grid), button);
      mx_widget_set_tooltip_text (MX_WIDGET (button), "test");
      if (i == 1)
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (swap_orientation),
                          grid);

      g_free (label);
    }

  g_signal_connect (stage, "notify::width",
                    G_CALLBACK (stage_size_notify_cb), scroll);
  g_signal_connect (stage, "notify::height",
                    G_CALLBACK (stage_size_notify_cb), scroll);
  g_signal_connect (stage, "key-release-event",
                    G_CALLBACK (set_max_stride), grid);
  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
