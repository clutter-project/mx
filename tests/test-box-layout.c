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


void
find_last_child (ClutterActor *actor, ClutterActor **child)
{
  *child = actor;
}

void
enter_event (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  ClutterColor color = { 0xff, 0xff, 0xff, 0xff };
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (actor), 2);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (actor), &color);
}

void
leave_event (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (actor), 0);
}


void
add_actor (ClutterContainer *container)
{
  ClutterActor *rect;
  ClutterColor color = { 0xff, 0xff, 0xff, 255 };

  clutter_color_from_hls (&color,
                          g_random_double_range (0.0, 360.0), 0.5, 0.5);

  rect = clutter_rectangle_new_with_color (&color);
  clutter_actor_set_size (rect, 32, 64);
  clutter_container_add_actor (container, rect);
  clutter_actor_set_reactive (rect, TRUE);
  g_signal_connect (rect, "enter-event", G_CALLBACK (enter_event), NULL);
  g_signal_connect (rect, "leave-event", G_CALLBACK (leave_event), NULL);
}

gboolean
key_release_cb (ClutterActor    *actor,
                ClutterKeyEvent *event,
                NbtkBoxLayout   *box)
{

  if (event->keyval == 'v')
    {
      nbtk_box_layout_set_vertical (box,
                                    !nbtk_box_layout_get_vertical (box));
    }

  if (event->keyval == 'p')
    {
      nbtk_box_layout_set_pack_start (box,
                                      !nbtk_box_layout_get_pack_start (box));
    }

  if (event->keyval == '=')
    {
      add_actor ((ClutterContainer*) box);
    }

  if (event->keyval == '-')
    {
      ClutterActor *child = NULL;

      clutter_container_foreach (CLUTTER_CONTAINER (box), (ClutterCallback) find_last_child, &child);

      if (child)
        clutter_container_remove_actor (CLUTTER_CONTAINER (box), child);
    }

  return FALSE;
}

int
main (int argc, char *argv[])
{
  NbtkWidget *box, *scrollview;
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 220, 220);

  scrollview = nbtk_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), (ClutterActor*) scrollview);
  clutter_actor_set_position ((ClutterActor*) scrollview, 10, 10);
  clutter_actor_set_size ((ClutterActor*) scrollview, 200, 200);

  box = nbtk_box_layout_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scrollview), (ClutterActor*) box);

  add_actor ((ClutterContainer*) box);
  add_actor ((ClutterContainer*) box);
  add_actor ((ClutterContainer*) box);


  g_signal_connect (stage, "key-release-event", G_CALLBACK (key_release_cb),
                    box);
  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
