/* nbtk-grid.h: Reflowing grid layout container for nbtk.
 *
 * Copyright (C) 2008-2009 Intel Corporation
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
 * Written by: Øyvind Kolås <pippin@linux.intel.com>
 * Ported to nbtk by: Robert Staudinger <robsta@openedhand.com>
 */

#ifndef __NBTK_GRID_H__
#define __NBTK_GRID_H__

#include <clutter/clutter.h>
#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_GRID               (nbtk_grid_get_type())
#define NBTK_GRID(obj)                                       \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                        \
                               NBTK_TYPE_GRID,               \
                               NbtkGrid))
#define NBTK_GRID_CLASS(klass)                               \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                         \
                            NBTK_TYPE_GRID,                  \
                            NbtkGridClass))
#define NBTK_IS_GRID(obj)                                    \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                        \
                               NBTK_TYPE_GRID))
#define NBTK_IS_GRID_CLASS(klass)                            \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                         \
                            NBTK_TYPE_GRID))
#define NBTK_GRID_GET_CLASS(obj)                             \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                         \
                              NBTK_TYPE_GRID,                \
                              NbtkGridClass))

typedef struct _NbtkGrid        NbtkGrid;
typedef struct _NbtkGridClass   NbtkGridClass;
typedef struct _NbtkGridPrivate NbtkGridPrivate;

struct _NbtkGridClass
{
  NbtkWidgetClass parent_class;
};

struct _NbtkGrid
{
  NbtkWidget parent;

  NbtkGridPrivate *priv;
};

GType nbtk_grid_get_type (void) G_GNUC_CONST;

NbtkWidget   *nbtk_grid_new                    (void);
void          nbtk_grid_set_end_align          (NbtkGrid    *self,
                                                gboolean     value);
gboolean      nbtk_grid_get_end_align          (NbtkGrid    *self);
void          nbtk_grid_set_homogenous_rows    (NbtkGrid    *self,
                                                gboolean     value);
gboolean      nbtk_grid_get_homogenous_rows    (NbtkGrid    *self);
void          nbtk_grid_set_homogenous_columns (NbtkGrid    *self,
                                                gboolean     value);
gboolean      nbtk_grid_get_homogenous_columns (NbtkGrid    *self);
void          nbtk_grid_set_column_major       (NbtkGrid    *self,
                                                gboolean     value);
gboolean      nbtk_grid_get_column_major       (NbtkGrid    *self);
void          nbtk_grid_set_row_gap            (NbtkGrid    *self,
                                                ClutterUnit  value);
ClutterUnit   nbtk_grid_get_row_gap            (NbtkGrid    *self);
void          nbtk_grid_set_column_gap         (NbtkGrid    *self,
                                                ClutterUnit  value);
ClutterUnit   nbtk_grid_get_column_gap         (NbtkGrid    *self);
void          nbtk_grid_set_valign             (NbtkGrid    *self,
                                                gdouble      value);
gdouble       nbtk_grid_get_valign             (NbtkGrid    *self);
void          nbtk_grid_set_halign             (NbtkGrid    *self,
                                                gdouble      value);
gdouble       nbtk_grid_get_halign             (NbtkGrid    *self);

G_END_DECLS

#endif /* __NBTK_GRID_H__ */
