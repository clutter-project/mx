/*
 * mx-icon.c: icon widget
 *
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
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

/**
 * SECTION:mx-icon
 * @short_description: a simple styled icon actor
 *
 * #MxIcon is a simple styled texture actor that displays an image from
 * a stylesheet.
 */

#include "mx-icon.h"

G_DEFINE_TYPE (MxIcon, mx_icon, MX_TYPE_WIDGET)

static void
mx_icon_get_preferred_height (ClutterActor *actor,
                                gfloat        for_width,
                                gfloat       *min_height,
                                gfloat       *pref_height)
{
  ClutterActor *background;
  gfloat height;

  background = mx_widget_get_background_image (MX_WIDGET (actor));

  if (background)
    {
      clutter_actor_get_preferred_height (background, -1, NULL, &height);
    }
  else
    {
      height = 0;
    }

  if (min_height)
    *min_height = height;

  if (pref_height)
    *pref_height = height;
}

static void
mx_icon_get_preferred_width (ClutterActor *actor,
                               gfloat        for_height,
                               gfloat       *min_width,
                               gfloat       *pref_width)
{
  ClutterActor *background;
  gfloat width;

  background = mx_widget_get_background_image (MX_WIDGET (actor));

  if (background)
    {
      clutter_actor_get_preferred_width (background, -1, NULL, &width);
    }
  else
    {
      width = 0;
    }

  if (min_width)
    *min_width = width;

  if (pref_width)
    *pref_width = width;
}

static void
mx_icon_class_init (MxIconClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->get_preferred_height = mx_icon_get_preferred_height;
  actor_class->get_preferred_width = mx_icon_get_preferred_width;
}

static void
mx_icon_init (MxIcon *self)
{
  /* make sure we are not reactive */
  clutter_actor_set_reactive (CLUTTER_ACTOR (self), FALSE);
}

/**
 * mx_icon_new:
 *
 * Create a newly allocated #MxIcon
 *
 * Returns: A newly allocated #MxIcon
 */
MxIcon*
mx_icon_new (void)
{
  return g_object_new (MX_TYPE_ICON, NULL);
}

