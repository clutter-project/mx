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

int
main (int argc, char *argv[])
{
  NbtkWidget *scroll, *view, *label, *grid;
  ClutterActor *stage;
  gdouble column_size;
  int i;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 200, 200);

  scroll = (NbtkWidget *) nbtk_scroll_view_new ();
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (scroll), NULL);
  clutter_actor_set_size (CLUTTER_ACTOR (scroll), 150, 150);

  view = (NbtkWidget *) nbtk_viewport_new ();
  clutter_container_add (CLUTTER_CONTAINER (scroll), CLUTTER_ACTOR (view), NULL);

  grid = nbtk_grid_new ();
  clutter_container_add (CLUTTER_CONTAINER (view), CLUTTER_ACTOR (grid), NULL);

  column_size = 0;
  for (i = 0; i < 15; i++)
    {
      char *text = g_strdup_printf ("label %d", i);
      label = nbtk_label_new (text);
      clutter_container_add (CLUTTER_CONTAINER (grid),
                              CLUTTER_ACTOR (label), NULL);
      g_free (text);

      if (column_size < clutter_actor_get_width (CLUTTER_ACTOR (label)))
        column_size = clutter_actor_get_width (CLUTTER_ACTOR (label));
    }

  nbtk_scroll_view_set_column_size (NBTK_SCROLL_VIEW (scroll), column_size);
  printf ("Column size: %.2f\n", nbtk_scroll_view_get_column_size (NBTK_SCROLL_VIEW (scroll)));

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
