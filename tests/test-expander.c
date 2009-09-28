/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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
expand_complete_cb (MxExpander  *expander,
                    gpointer       user_data)
{
  gboolean expanded;

  expanded = mx_expander_get_expanded (expander);
  printf ("expand complete (%s)\n",
          (expanded) ? "open": "closed");
}

static void
set_expanded (ClutterActor    *actor,
              ClutterKeyEvent *event,
              MxExpander    *expander)
{
  gboolean expand;

  if (event->keyval != 32)
    return;

  expand = mx_expander_get_expanded (expander);

  mx_expander_set_expanded (expander, !expand);
}

static void
stage_size_notify_cb (ClutterActor *stage,
                      GParamSpec *pspec,
                      ClutterActor *table)
{
  gfloat width, height;

  clutter_actor_get_size (stage, &width, &height);
}

void
expander_main (ClutterContainer *stage)
{
  ClutterActor *expander, *scroll, *grid;
  int i;

  expander = mx_expander_new ();
  mx_expander_set_label (MX_EXPANDER (expander), "Expander");
  clutter_container_add_actor (stage,
                               expander);
  clutter_actor_set_position (expander, 10, 10);

  g_signal_connect (expander, "expand-complete",
                    G_CALLBACK (expand_complete_cb), NULL);

  scroll = mx_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (expander), scroll);
  clutter_actor_set_size (scroll, 320, 240);

  grid = mx_grid_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), grid);

  for (i = 1; i <= 50; i++)
    {
      ClutterActor *button;
      gchar *label;

      label = g_strdup_printf ("Button %d", i);

      button = mx_button_new_with_label (label);
      clutter_container_add_actor (CLUTTER_CONTAINER (grid), button);

      g_free (label);
    }

  g_signal_connect (stage, "notify::width",
                    G_CALLBACK (stage_size_notify_cb), expander);
  g_signal_connect (stage, "notify::height",
                    G_CALLBACK (stage_size_notify_cb), expander);
  g_signal_connect (stage, "key-release-event",
                    G_CALLBACK (set_expanded), expander);

}
