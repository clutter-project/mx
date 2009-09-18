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
  ClutterActor *stage;
  MxWidget *progress_bar;
  ClutterAnimation *animation;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  progress_bar = mx_progress_bar_new ();
  animation = clutter_actor_animate (CLUTTER_ACTOR (progress_bar),
                                     CLUTTER_LINEAR,
                                     5000,
                                     "progress", 1.0,
                                     NULL);
  clutter_animation_set_loop (animation, TRUE);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (progress_bar));
  clutter_actor_set_size (CLUTTER_ACTOR (progress_bar), 280, 75);
  clutter_actor_set_position (CLUTTER_ACTOR (progress_bar), 20, 20);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}

