/* nbtk-tile.h: Plain instatiable widget actor.
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

#ifndef __NBTK_TILE_H__
#define __NBTK_TILE_H__

#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_TILE                (nbtk_tile_get_type ())
#define NBTK_TILE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TILE, NbtkTile))
#define NBTK_IS_TILE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TILE))
#define NBTK_TILE_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TILE, NbtkTileClass))
#define NBTK_IS_TILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TILE))
#define NBTK_TILE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TILE, NbtkTileClass))

typedef struct _NbtkTile              NbtkTile;
typedef struct _NbtkTilePrivate       NbtkTilePrivate;
typedef struct _NbtkTileClass         NbtkTileClass;

struct _NbtkTile
{
  /*< private >*/
  NbtkWidget parent_instance;
};

struct _NbtkTileClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_tile_get_type (void) G_GNUC_CONST;

NbtkWidget *          nbtk_tile_new            (void);

G_END_DECLS

#endif /* __NBTK_TILE_H__ */
