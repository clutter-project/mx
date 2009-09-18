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
#include <mx/mx.h>

static void
allocation_notify_cb (ClutterActor *stage,
                      GParamSpec   *pspec,
                      ClutterActor *scroll)
{
  gfloat width, height;

  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_size (scroll, width - 100, height - 100);
}



int
main (int argc, char *argv[])
{
  ClutterActor *scroll, *view, *texture;
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  scroll = (ClutterActor *) mx_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);
  clutter_actor_set_position (scroll, 50, 50);

  view = (ClutterActor *) mx_viewport_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), view);


  texture = clutter_texture_new_from_file ("redhand.png", NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), texture);
  g_object_set (texture, "repeat-x", TRUE, "repeat-y", TRUE, NULL);
  clutter_actor_set_size (texture, 800, 639);

  g_signal_connect (stage, "notify::allocation",
                    G_CALLBACK (allocation_notify_cb), scroll);

  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
