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

void
scroll_grid_main (ClutterContainer *stage)
{
  ClutterActor *scroll, *grid;
  int i;

  scroll = mx_scroll_view_new ();
  clutter_container_add_actor (stage, scroll);
  clutter_actor_set_position (scroll, 10, 10);

  clutter_actor_set_size (scroll, 400, 400);

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

  g_signal_connect (stage, "key-release-event",
                    G_CALLBACK (set_max_stride), grid);
}
