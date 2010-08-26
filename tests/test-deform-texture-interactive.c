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

typedef struct {
  ClutterActor *texture;
  const char *name;
  double factor;
  ClutterActor *value_label;
} PropertyInfo;

static PropertyInfo properties_info [3] =
{
  { NULL, "period", 1.0     , NULL },
  { NULL, "angle" , 2 * G_PI, NULL },
  { NULL, "radius", 100     , NULL }
};

static void
on_value_changed (MxSlider     *slider,
                     GParamSpec   *pspec,
                     PropertyInfo *info)
{
  gdouble value;
  char *label;

  value = mx_slider_get_value (slider);
  value *= info->factor;
  g_object_set (info->texture, info->name, value, NULL);
  label = g_strdup_printf ("%.2f", value);
  mx_label_set_text (MX_LABEL (info->value_label), label);
  g_free (label);
}

int
main (int argc, char *argv[])
{
  MxWindow *window;
  MxApplication *app;
  gfloat width, height;
  ClutterActor *stage, *table, *slider, *label, *texture, *front, *back;
  ClutterColor stage_color = { 0xcc, 0xcc, 0xcc, 0xb0 };

#if defined (HAVE_X11) && CLUTTER_CHECK_VERSION(1,2,0)
  /* Enable argb visuals for coolness with compositors */
  clutter_x11_set_use_argb_visual (TRUE);
#endif

  app = mx_application_new (&argc, &argv, "Pimp My Page Turn", 0);

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

  properties_info[0].texture = texture;
  properties_info[1].texture = texture;
  properties_info[2].texture = texture;

  /* Make the subdivision size a bit higher than default so it looks nicer */
  clutter_actor_get_preferred_size (texture, NULL, NULL, &width, &height);
  mx_deform_texture_set_resolution (MX_DEFORM_TEXTURE (texture), 64, 64);

  /* Create a table that will be our top level container */
  table = mx_table_new ();
  mx_table_set_row_spacing (MX_TABLE (table), 12);

  /* Add the texture first */
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      texture,
                                      0, 0,
                                      "column-span", 3,
                                      NULL);

  label = mx_label_new_with_text ("Period");
  slider = mx_slider_new ();
  properties_info[0].value_label = mx_label_new_with_text ("0.0");
  clutter_actor_set_width (slider, 200);
  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (on_value_changed), &properties_info[0]);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      1, 0,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_END,
                                      "y-fill", FALSE,
                                      "x-fill", TRUE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      slider,
                                      1, 1,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      properties_info[0].value_label,
                                      1, 2,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);

  label = mx_label_new_with_text ("Angle");
  slider = mx_slider_new ();
  properties_info[1].value_label = mx_label_new_with_text ("0.00");
  clutter_actor_set_width (slider, 200);
  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (on_value_changed), &properties_info[1]);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      2, 0,
                                      "x-expand", TRUE,
                                      "y-expand", TRUE,
                                      "x-align", MX_ALIGN_END,
                                      "y-expand", FALSE,
                                      "y-fill", FALSE,
                                      "x-fill", TRUE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      slider,
                                      2, 1,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      properties_info[1].value_label,
                                      2, 2,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);

  label = mx_label_new_with_text ("Radius");
  slider = mx_slider_new ();
  properties_info[2].value_label = mx_label_new_with_text ("24.00");
  clutter_actor_set_width (slider, 200);
  mx_slider_set_value (MX_SLIDER (slider), 0.24);
  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (on_value_changed), &properties_info[2]);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      label,
                                      3, 0,
                                      "x-expand", TRUE,
                                      "y-expand", TRUE,
                                      "x-align", MX_ALIGN_END,
                                      "y-expand", FALSE,
                                      "y-fill", FALSE,
                                      "x-fill", TRUE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      slider,
                                      3, 1,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);
  mx_table_add_actor_with_properties (MX_TABLE (table),
                                      properties_info[2].value_label,
                                      3, 2,
                                      "x-expand", TRUE,
                                      "y-expand", FALSE,
                                      "x-align", MX_ALIGN_MIDDLE,
                                      "y-fill", FALSE,
                                      "x-fill", FALSE,
                                      NULL);
  /* Add the table to the window */
  mx_window_set_child (window, table);

  /* Begin */
  clutter_actor_show (stage);
  mx_application_run (app);

  return 0;
}

