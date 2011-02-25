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
 * Written by: Damien Lespiau <damien.lespiau@intel.com>
 *
 */
#include "test-mx.h"

static void
on_value_changed (MxSlider         *slider,
                  GParamSpec       *pspec,
                  ClutterRectangle *rectangle)
{
  ClutterColor color = {0xff, 0, 0, 0xff};
  gdouble value;

  value = mx_slider_get_value (slider);
  color.alpha = (guint8) (value * 0xff);
  clutter_rectangle_set_color (rectangle, &color);

  mx_slider_set_buffer_value (slider, MIN (0.1 + value * 1.5, 1.0));
}

void
slider_main (ClutterContainer *stage)
{
  ClutterActor *rectangle, *slider;
  ClutterColor color = {0xff, 0, 0, 0};

  slider = mx_slider_new ();
  rectangle = clutter_rectangle_new_with_color (&color);

  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (on_value_changed), rectangle);

  clutter_container_add_actor (stage, slider);
  clutter_actor_set_width (slider, 280);
  clutter_actor_set_position (slider, 20, 20);

  clutter_container_add_actor (stage, rectangle);
  clutter_actor_set_size (rectangle, 64, 64);
  clutter_actor_set_position (rectangle, 108, 52);

  mx_slider_set_value (MX_SLIDER (slider), 0.5);
}
