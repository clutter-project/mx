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
#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <mx/mx.h>

static ClutterActor *hover_actor;

static void
find_last_child (ClutterActor *actor, ClutterActor **child)
{
  *child = actor;
}

static void
enter_event (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  ClutterColor color = { 0x00, 0x00, 0x00, 0xff };
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (actor), 2);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (actor), &color);

  hover_actor = actor;
}

static void
leave_event (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (actor), 0);

  hover_actor = NULL;
}

static void
button_release_event (ClutterActor *actor, ClutterButtonEvent *event,
                      ClutterContainer *box)
{
  gboolean xfill, yfill;
  gint xalign, yalign;

  if (event->button == 1)
    {
      clutter_container_child_get (box, actor,
                                   "x-fill", &xfill,
                                   "y-fill", &yfill,
                                   NULL);

      clutter_container_child_set (box, actor,
                                   "x-fill", !xfill,
                                   "y-fill", !yfill,
                                   NULL);
    }
  else if (event->button == 3)
    {
      clutter_container_child_get (box, actor,
                                   "x-align", &xalign,
                                   "y-align", &yalign,
                                   NULL);

      if (xalign < 2)
        xalign++;
      else
        xalign = 0;

      if (yalign < 2)
        yalign++;
      else
        yalign = 0;

      clutter_container_child_set (box, actor,
                                   "x-align", xalign,
                                   "y-align", yalign,
                                   NULL);
    }
  else if (event->button == 2)
    {
      gboolean expand;
      clutter_container_child_get (box, actor, "expand", &expand, NULL);

      clutter_container_child_set (box, actor, "expand", !expand, NULL);
    }
}

static void
add_actor (ClutterContainer *container)
{
  ClutterActor *rect;
  ClutterColor color = { 0xff, 0xff, 0xff, 255 };
  static gboolean expand = TRUE;

  clutter_color_from_hls (&color,
                          g_random_double_range (0.0, 360.0), 0.5, 0.5);

  rect = clutter_rectangle_new_with_color (&color);
  clutter_actor_set_size (rect, 32, 64);
  clutter_container_add_actor (container, rect);
  clutter_actor_set_reactive (rect, TRUE);
  g_signal_connect (rect, "enter-event", G_CALLBACK (enter_event), NULL);
  g_signal_connect (rect, "leave-event", G_CALLBACK (leave_event), NULL);
  g_signal_connect (rect, "button-release-event",
                    G_CALLBACK (button_release_event), container);

  clutter_container_child_set (container, rect, "expand", expand, NULL);
  expand = !expand;
}

static gboolean
key_release_cb (ClutterActor    *actor,
                ClutterKeyEvent *event,
                MxBoxLayout   *box)
{

  if (event->keyval == 'v')
    {
      mx_box_layout_set_orientation (box,
                                     !mx_box_layout_get_orientation (box));
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

  if (event->keyval == 'c')
    {
      gboolean clip;

      g_object_get (actor, "clip-to-allocation", &clip, NULL);
      g_object_set (actor, "clip-to-allocation", !clip, NULL);
    }

  if (event->keyval == 's')
    {
      guint spacing;

      spacing = mx_box_layout_get_spacing (box);

      if (spacing > 6)
        spacing = 0;
      else
        spacing++;

      mx_box_layout_set_spacing (box, spacing);
    }

  if (event->keyval == 'e')
    {
      if (hover_actor)
        {
          gboolean expand;
          clutter_container_child_get ((ClutterContainer*) box, hover_actor,
                                       "expand", &expand, NULL);
          clutter_container_child_set ((ClutterContainer*) box, hover_actor,
                                       "expand", !expand, NULL);
        }
    }

  if (event->keyval == 'a')
    {
      mx_box_layout_set_enable_animations (box,
                                           !mx_box_layout_get_enable_animations (box));
    }

  return FALSE;
}

static void
stage_size_changed_cb (ClutterActor *stage, GParamSpec *pspec, ClutterActor *scrollview)
{
  gfloat width, height;

  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_size (scrollview, width - 100, height - 100);
}

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *box, *scrollview;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  mx_style_load_from_file (mx_style_get_default(), "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 480);
  clutter_stage_set_user_resizable ((ClutterStage *) stage, TRUE);

  scrollview = mx_scroll_view_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scrollview);

  clutter_actor_set_position (scrollview, 50, 50);
  clutter_actor_set_size (scrollview, 500, 300);

  box = mx_box_layout_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (scrollview), box);

  add_actor ((ClutterContainer*) box);
  add_actor ((ClutterContainer*) box);
  add_actor ((ClutterContainer*) box);


  g_signal_connect (stage, "key-release-event", G_CALLBACK (key_release_cb),
                    box);
  g_signal_connect (stage, "notify::allocation",
                    G_CALLBACK (stage_size_changed_cb), scrollview);
  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
