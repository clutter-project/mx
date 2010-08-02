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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mx/mx.h>

#ifdef HAVE_X11
#include <clutter/x11/clutter-x11.h>
#endif

static gint func = 0;

static ClutterActor *
replace_deformation (ClutterActor *texture, GType type)
{
  ClutterTexture *front, *back;
  MxWindow *window;
  gint x, y;

  window =
    mx_window_get_for_stage (CLUTTER_STAGE (clutter_actor_get_stage (texture)));

  mx_deform_texture_get_resolution (MX_DEFORM_TEXTURE (texture), &x, &y);
  mx_deform_texture_get_textures (MX_DEFORM_TEXTURE (texture),
                                  &front,
                                  &back);
  if (front)
    g_object_ref (front);
  if (back)
    g_object_ref (back);
  mx_deform_texture_set_textures (MX_DEFORM_TEXTURE (texture), NULL, NULL);

  texture = g_object_new (type, NULL);
  mx_deform_texture_set_resolution (MX_DEFORM_TEXTURE (texture), x, y);
  mx_deform_texture_set_textures (MX_DEFORM_TEXTURE (texture),
                                  front,
                                  back);
  if (front)
    g_object_unref (front);
  if (back)
    g_object_unref (back);

  mx_window_set_child (window, texture);

  return texture;
}

static void
completed_cb (ClutterAnimation *animation, ClutterActor *texture)
{
  switch (func)
    {
    case 0:
      /* Change direction of page-turn animation */
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 1:
      /* Replace page-turn deformation with bow-tie deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_BOW_TIE);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 2:
      /* Change direction of bow-tie animation */
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 3:
      /* Replace bow-tie deformation with waves deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_WAVES);
      g_object_set (G_OBJECT (texture), "amplitude", 0.0, NULL);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_QUAD, 5000,
                             "period", 2.0,
                             "amplitude", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 4:
      /* Reverse direction of waves deformation */
      clutter_actor_animate (texture, CLUTTER_EASE_OUT_QUAD, 5000,
                             "period", 4.0,
                             "amplitude", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 5:
      /* Replace waves deformation with page-turn deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_PAGE_TURN);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;
    }

  if (++func == 6)
    func = 0;
}

int
main (int argc, char *argv[])
{
  MxWindow *window;
  MxApplication *app;
  gfloat width, height;
  ClutterActor *stage, *texture, *front, *back;
  ClutterColor stage_color = { 0xcc, 0xcc, 0xcc, 0xb0 };

#if defined (HAVE_X11) && CLUTTER_CHECK_VERSION(1,2,0)
  /* Enable argb visuals for coolness with compositors */
  clutter_x11_set_use_argb_visual (TRUE);
#endif

  app = mx_application_new (&argc, &argv, "Test deformations", 0);

  window = mx_application_create_window (app);
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  /* Set a size so we don't just get our minimum size on map */
  clutter_actor_set_size (stage, 480, 480);

  /* Create a page-turn deformation */
  texture = mx_deform_page_turn_new ();
  if (argc > 1)
    {
      front = clutter_texture_new_from_file (argv[1], NULL);
    }
  else
    {
      front = mx_offscreen_new ();
      mx_offscreen_set_child (MX_OFFSCREEN (front),
                              mx_button_new_with_label ("Front face"));
    }

  if (argc > 2)
    {
      back = clutter_texture_new_from_file (argv[2], NULL);
    }
  else
    {
      back = mx_offscreen_new ();
      mx_offscreen_set_child (MX_OFFSCREEN (back),
                              mx_button_new_with_label ("Back face"));
    }

  mx_deform_texture_set_textures (MX_DEFORM_TEXTURE (texture),
                                  (ClutterTexture *)front,
                                  (ClutterTexture *)back);

  /* Make the subdivision size a bit higher than default so it looks nicer */
  clutter_actor_get_preferred_size (texture, NULL, NULL, &width, &height);
  mx_deform_texture_set_resolution (MX_DEFORM_TEXTURE (texture), 64, 64);

  /* Add it to the window */
  mx_window_set_child (window, texture);

  /* Start animation */
  clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                         "period", 1.0,
                         "signal-after::completed", completed_cb, texture,
                         NULL);

  /* Begin */
  clutter_actor_show (stage);
  mx_application_run (app);

  return 0;
}

