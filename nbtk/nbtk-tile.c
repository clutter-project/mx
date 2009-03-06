/* nbtk-tile.c: Plain instatiable widget actor.
 *
 * Copyright (C) 2009 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Robert Staudinger <robsta@openedhand.com>
 */

/**
 * SECTION:nbtk-tile
 * @short_description: Plain instatiable widget actor.
 *
 * An instantiable widget actor that, mainly meant for CSS-stylable widget
 * parts.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-tile.h"

#define NBTK_TILE_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_TILE, NbtkTilePrivate))

struct _NbtkTilePrivate
{
  guint is_hover : 1;
};

G_DEFINE_TYPE (NbtkTile, nbtk_tile, NBTK_TYPE_WIDGET)

static gboolean
nbtk_tile_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  NbtkTile *self = NBTK_TILE (actor);

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (self), "hover");
  self->priv->is_hover = 1;

  return FALSE;
}

static gboolean
nbtk_tile_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  NbtkTile *self = NBTK_TILE (actor);

  self->priv->is_hover = 0;
  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (self), NULL);

  return FALSE;
}

static void
nbtk_tile_class_init (NbtkTileClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkTilePrivate));

  actor_class->enter_event = nbtk_tile_enter;
  actor_class->leave_event = nbtk_tile_leave;
}

static void
nbtk_tile_init (NbtkTile *tile)
{
  tile->priv = NBTK_TILE_GET_PRIVATE (tile);
}

/**
 * nbtk_tile_new:
 *
 * Create a new tile.
 *
 * Returns: a new #NbtkTile
 */
NbtkWidget *
nbtk_tile_new (void)
{
  return g_object_new (NBTK_TYPE_TILE, NULL);
}
