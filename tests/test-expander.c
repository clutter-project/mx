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
#include <nbtk/nbtk.h>

static void
expand_complete_cb (NbtkExpander  *expander,
                    gpointer       user_data)
{
  gboolean expanded;

  expanded = nbtk_expander_get_expanded (expander);
  printf ("expand complete (%s)\n",
          (expanded) ? "open": "closed");
}

static void
set_expanded (ClutterActor    *actor,
              ClutterKeyEvent *event,
              NbtkExpander    *expander)
{
  gboolean expand;

  if (event->keyval != 32)
    return;

  expand = nbtk_expander_get_expanded (expander);

  nbtk_expander_set_expanded (expander, !expand);
}

static void
stage_size_notify_cb (ClutterActor *stage,
                      GParamSpec *pspec,
                      ClutterActor *table)
{
  gfloat width, height;

  clutter_actor_get_size (stage, &width, &height);
}

int
main (int argc, char *argv[])
{
  NbtkWidget *expander, *scroll, *grid;
  ClutterActor *stage;
  int i;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 480);
  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  expander = nbtk_expander_new ();
  nbtk_expander_set_label (NBTK_EXPANDER (expander), "Expander");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (expander));
  clutter_actor_set_position (CLUTTER_ACTOR (expander), 10, 10);

  g_signal_connect (expander, "expand-complete",
                    G_CALLBACK (expand_complete_cb), NULL);

  scroll = (NbtkWidget *) nbtk_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (expander),
                               CLUTTER_ACTOR (scroll));
  clutter_actor_set_size (CLUTTER_ACTOR (scroll), 320, 240);

  grid = nbtk_grid_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll),
                               CLUTTER_ACTOR (grid));

  for (i = 1; i <= 50; i++)
    {
      NbtkWidget *button;
      gchar *label;

      label = g_strdup_printf ("Button %d", i);

      button = nbtk_button_new_with_label (label);
      clutter_container_add_actor (CLUTTER_CONTAINER (grid),
                                   CLUTTER_ACTOR (button));

      g_free (label);
    }

  g_signal_connect (stage, "notify::width",
                    G_CALLBACK (stage_size_notify_cb), expander);
  g_signal_connect (stage, "notify::height",
                    G_CALLBACK (stage_size_notify_cb), expander);
  g_signal_connect (stage, "key-release-event",
                    G_CALLBACK (set_expanded), expander);
  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
