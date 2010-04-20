/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-grid.h: Reflowing grid layout container for mx.
 *
 * Copyright 2008, 2009, 2010 Intel Corporation.
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
 * Written by: Øyvind Kolås <pippin@linux.intel.com>
 * Ported to mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_GRID_H__
#define __MX_GRID_H__

#include <clutter/clutter.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_GRID               (mx_grid_get_type())
#define MX_GRID(obj)                                       \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                      \
                               MX_TYPE_GRID,               \
                               MxGrid))
#define MX_GRID_CLASS(klass)                               \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                       \
                            MX_TYPE_GRID,                  \
                            MxGridClass))
#define MX_IS_GRID(obj)                                    \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                      \
                               MX_TYPE_GRID))
#define MX_IS_GRID_CLASS(klass)                            \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                       \
                            MX_TYPE_GRID))
#define MX_GRID_GET_CLASS(obj)                             \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                       \
                              MX_TYPE_GRID,                \
                              MxGridClass))

typedef struct _MxGrid        MxGrid;
typedef struct _MxGridClass   MxGridClass;
typedef struct _MxGridPrivate MxGridPrivate;

struct _MxGridClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

/**
 * MxGrid:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
struct _MxGrid
{
  /*< private >*/
  MxWidget parent;

  MxGridPrivate *priv;
};

GType mx_grid_get_type (void) G_GNUC_CONST;

ClutterActor  *mx_grid_new                    (void);
void           mx_grid_set_line_alignment     (MxGrid   *self,
                                               MxAlign  value);
gboolean       mx_grid_get_line_alignment     (MxGrid   *self);
void           mx_grid_set_homogenous_rows    (MxGrid   *self,
                                               gboolean  value);
gboolean       mx_grid_get_homogenous_rows    (MxGrid   *self);
void           mx_grid_set_homogenous_columns (MxGrid   *self,
                                               gboolean  value);
gboolean       mx_grid_get_homogenous_columns (MxGrid   *self);

void           mx_grid_set_orientation        (MxGrid        *grid,
                                               MxOrientation  orientation);
MxOrientation  mx_grid_get_orientation        (MxGrid        *grid);

void           mx_grid_set_row_spacing        (MxGrid   *self,
                                               gfloat    value);
gfloat         mx_grid_get_row_spacing        (MxGrid   *self);
void           mx_grid_set_column_spacing     (MxGrid   *self,
                                               gfloat    value);
gfloat         mx_grid_get_column_spacing     (MxGrid   *self);
void           mx_grid_set_child_y_align      (MxGrid   *self,
                                               MxAlign   value);
MxAlign        mx_grid_get_child_y_align      (MxGrid   *self);
void           mx_grid_set_child_x_align      (MxGrid   *self,
                                               MxAlign   value);
MxAlign        mx_grid_get_child_x_align      (MxGrid   *self);

void mx_grid_set_max_stride (MxGrid *self,
                             gint    value);
gint mx_grid_get_max_stride (MxGrid *self);

G_END_DECLS

#endif /* __MX_GRID_H__ */
