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

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *texture, *frame;
  GError *err = NULL;
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 500, 300);

  texture = clutter_texture_new_from_file ("test-texture-frame.png", &err);

  if (err)
    {
      g_critical ("%s", err->message);
      return 1;
    }

  frame = mx_texture_frame_new (CLUTTER_TEXTURE (texture), 10, 10, 10, 10);
  clutter_actor_set_position (frame, 50, 50);
  clutter_actor_set_size (frame, 400, 200);

  clutter_container_add (CLUTTER_CONTAINER (stage), frame, NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
