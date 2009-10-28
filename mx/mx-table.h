/*
 * mx-table.h: Table layout widget
 *
 * Copyright 2008, 2009 Intel Corporation.
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
 * Written by: Thomas Wood  <thomas@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_TABLE_H__
#define __MX_TABLE_H__

#include <mx/mx-types.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

/**
 * MxTableChildOptions:
 * @MX_KEEP_ASPECT_RATIO: whether to respect the widget's aspect ratio
 * @MX_X_EXPAND: whether to allocate extra space on the widget's x-axis
 * @MX_Y_EXPAND: whether to allocate extra space on the widget's y-axis
 * @MX_X_FILL: whether to stretch the child to fill the cell horizontally
 * @MX_Y_FILL: whether to stretch the child to fill the cell vertically
 *
 * Denotes the child properties an MxTable child will have.
 */
typedef enum
{
  MX_KEEP_ASPECT_RATIO = 1 << 0,
  MX_X_EXPAND          = 1 << 1,
  MX_Y_EXPAND          = 1 << 2,
  MX_X_FILL            = 1 << 3,
  MX_Y_FILL            = 1 << 4
} MxTableChildOptions;

#define MX_TYPE_TABLE                (mx_table_get_type ())
#define MX_TABLE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_TABLE, MxTable))
#define MX_IS_TABLE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_TABLE))
#define MX_TABLE_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_TABLE, MxTableClass))
#define MX_IS_TABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_TABLE))
#define MX_TABLE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_TABLE, MxTableClass))

typedef struct _MxTable              MxTable;
typedef struct _MxTablePrivate       MxTablePrivate;
typedef struct _MxTableClass         MxTableClass;

/**
 * MxTable:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxTable
{
  /*< private >*/
  MxWidget parent_instance;

  MxTablePrivate *priv;
};

struct _MxTableClass
{
  MxWidgetClass parent_class;
};

GType mx_table_get_type (void) G_GNUC_CONST;

ClutterActor *mx_table_new (void);

void mx_table_set_col_spacing (MxTable      *table,
                               gint          spacing);
void mx_table_set_row_spacing (MxTable      *table,
                               gint          spacing);
gint mx_table_get_col_spacing (MxTable      *table);
gint mx_table_get_row_spacing (MxTable      *table);
void mx_table_add_actor       (MxTable      *table,
                               ClutterActor *actor,
                               gint          row,
                               gint          column);

void mx_table_add_actor_with_properties (MxTable      *table,
                                         ClutterActor *actor,
                                         gint          row,
                                         gint          column,
                                         const gchar  *first_property_name,
                                         ...);

gint mx_table_get_row_count    (MxTable *table);
gint mx_table_get_column_count (MxTable *table);

G_END_DECLS

#endif /* __MX_TABLE_H__ */
