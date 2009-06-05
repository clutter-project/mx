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

#include <nbtk/nbtk.h>

static void
changed_cb (NbtkAdjustment *adjustment,
            gpointer        data)
{
  printf ("%s() %.2f\n", __FUNCTION__,
                         nbtk_adjustment_get_value (adjustment));
}

int
main (int argc, char *argv[])
{
  ClutterActor    *stage;
  NbtkWidget      *scroll;
  NbtkAdjustment  *adjustment;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 200);

  adjustment = nbtk_adjustment_new (0., 0., 100., 1., 10., 10.);
  g_signal_connect (adjustment, "notify::value", 
                    G_CALLBACK (changed_cb), NULL);  

  scroll = nbtk_scroll_bar_new (adjustment);
  clutter_actor_set_position (CLUTTER_ACTOR (scroll), 50, 100);
  clutter_actor_set_size (CLUTTER_ACTOR (scroll), 200, 30);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (scroll), NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
