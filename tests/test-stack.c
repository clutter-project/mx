/*
 * Copyright 2010 Intel Corporation.
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
animate_rotation (ClutterContainer *stack)
{
  gfloat angle = 0;
  GList *c, *children = clutter_container_get_children (stack);

  children = g_list_reverse (children);
  for (c = children; c; c = c->next, angle += 5)
    clutter_actor_animate (CLUTTER_ACTOR (c->data), CLUTTER_EASE_OUT_QUAD, 250,
                           "opacity", 0xff,
                           "rotation-angle-z", angle,
                           "fixed::rotation-center-z-gravity",
                           CLUTTER_GRAVITY_SOUTH_EAST,
                           NULL);
  g_list_free (children);
}

static void
lower_bottom (ClutterAnimation *animation,
              ClutterActor *actor)
{
  ClutterContainer *stack =
    CLUTTER_CONTAINER (clutter_actor_get_parent (actor));
  clutter_container_lower_child (stack, actor, NULL);
}

static void
button_clicked_cb (ClutterActor     *button,
                   ClutterContainer *stack)
{
  clutter_actor_animate (CLUTTER_ACTOR (button), CLUTTER_EASE_IN_OUT_QUAD, 400,
                         "opacity", 0x00,
                         "signal::completed",
                           lower_bottom,
                           button,
                         "signal-swapped-after::completed",
                           animate_rotation,
                           stack,
                         NULL);
}

int
main (int argc, char **argv)
{
  gint i;
  MxWindow *window;
  MxApplication *app;
  MxTextureCache *cache;
  ClutterActor *stage, *stack1, *stack2, *label;

  app = mx_application_new (&argc, &argv, "Test Stack", 0);

  window = mx_application_create_window (app);
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);

  stack1 = mx_stack_new ();
  stack2 = mx_stack_new ();
  label = mx_label_new_with_text ("This is a test of the stack");

  clutter_container_add (CLUTTER_CONTAINER (stack1), stack2, label, NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (stack1), label,
                               "x-fill", FALSE,
                               "y-fill", FALSE,
                               "x-align", MX_ALIGN_START,
                               "y-align", MX_ALIGN_END,
                               NULL);

  cache = mx_texture_cache_get_default ();
  for (i = 0; i < 10; i++)
    {
      ClutterActor *button = mx_button_new ();
      ClutterActor *texture = mx_texture_cache_get_actor (cache,
                                                          "redhand.png");
      mx_bin_set_child (MX_BIN (button), texture);
      clutter_container_add_actor (CLUTTER_CONTAINER (stack2), button);
      clutter_container_child_set (CLUTTER_CONTAINER (stack2), button,
                                   "x-fill", FALSE,
                                   "y-fill", FALSE,
                                   NULL);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (button_clicked_cb), stack2);
    }
  animate_rotation (CLUTTER_CONTAINER (stack2));

  mx_window_set_child (window, stack1);

  clutter_actor_show (stage);

  mx_application_run (app);

  return 0;
}
