/*
 * nbtk-icon.h: Icon actor
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */


#include "nbtk-icon.h"

G_DEFINE_TYPE (NbtkIcon, nbtk_icon, NBTK_TYPE_WIDGET)

static void
nbtk_icon_get_preferred_height (ClutterActor *actor,
                                ClutterUnit   for_width,
                                ClutterUnit  *min_height,
                                ClutterUnit  *pref_height)
{
  ClutterActor *background;
  ClutterUnit height;

  background = nbtk_widget_get_background_image (NBTK_WIDGET (actor));

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
nbtk_icon_get_preferred_width (ClutterActor *actor,
                               ClutterUnit   for_height,
                               ClutterUnit  *min_width,
                               ClutterUnit  *pref_width)
{
  ClutterActor *background;
  ClutterUnit width;

  background = nbtk_widget_get_background_image (NBTK_WIDGET (actor));

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
nbtk_icon_class_init (NbtkIconClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->get_preferred_height = nbtk_icon_get_preferred_height;
  actor_class->get_preferred_width = nbtk_icon_get_preferred_width;
}

static void
nbtk_icon_init (NbtkIcon *self)
{
  /* make sure we are not reactive */
  clutter_actor_set_reactive (CLUTTER_ACTOR (self), FALSE);
}

NbtkIcon*
nbtk_icon_new (void)
{
  return g_object_new (NBTK_TYPE_ICON, NULL);
}

