/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2009, 2010 Intel Corporation.
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
#include <stdlib.h>

static void
swap_orientation (ClutterActor *button,
                  MxGrid     *grid)
{
  mx_grid_set_orientation (grid, !mx_grid_get_orientation (grid));
}

static void
ensure_visible (ClutterText *text,
                MxGrid      *grid)
{
  GList *children;
  ClutterActor *child;

  const gchar *string = clutter_text_get_text (text);
  gint number = atoi (string) - 1;

  children = clutter_container_get_children (CLUTTER_CONTAINER (grid));
  child = g_list_nth_data (children, number);
  if (child)
    {
      ClutterGeometry geom;
      MxScrollView *scroll = (MxScrollView *)
        clutter_actor_get_parent (CLUTTER_ACTOR (grid));
      clutter_actor_get_allocation_geometry (child, &geom);
      mx_scroll_view_ensure_visible (scroll, &geom);
      printf ("Making child %d visible\n", number);
    }
  else
    printf ("Couldn't make child %d visible\n", number);
  g_list_free (children);
}

static void
set_max_stride (ClutterText *text,
                MxGrid      *grid)
{
  const gchar *string = clutter_text_get_text (text);
  mx_grid_set_max_stride (grid, atoi (string));
  printf ("Max Stride: %d\n", atoi (string));
}

void
scroll_grid_main (ClutterContainer *stage)
{
  ClutterActor *scroll, *grid, *table, *label, *visible_entry, *stride_entry;
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
      gchar *text;

      text = g_strdup_printf ("Button %d", i);

      button = mx_button_new_with_label (text);
      clutter_container_add_actor (CLUTTER_CONTAINER (grid), button);
      mx_widget_set_tooltip_text (MX_WIDGET (button), "test");
      if (i == 1)
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (swap_orientation),
                          grid);

      g_free (text);
    }

  table = mx_table_new ();

  label = mx_label_new_with_text ("Make button visible:");
  visible_entry = mx_entry_new ();
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      0, 0,
                                      "x-expand", FALSE,
                                      NULL);
  mx_table_add_actor (MX_TABLE (table), visible_entry, 0, 1);

  label = mx_label_new_with_text ("Set max-stride:");
  stride_entry = mx_entry_new ();
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      1, 0,
                                      "x-expand", FALSE,
                                      NULL);
  mx_table_add_actor (MX_TABLE (table), stride_entry, 1, 1);

  clutter_actor_set_position (table, 10, 420);
  clutter_actor_set_width (table, 400);
  clutter_container_add_actor (stage, table);

  g_signal_connect (mx_entry_get_clutter_text (MX_ENTRY (visible_entry)),
                    "activate",
                    G_CALLBACK (ensure_visible),
                    grid);

  g_signal_connect (mx_entry_get_clutter_text (MX_ENTRY (stride_entry)),
                    "activate",
                    G_CALLBACK (set_max_stride), grid);
}
