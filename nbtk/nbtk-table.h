/*
 * nbtk-table.h: Table layout widget
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

#ifndef __NBTK_TABLE_H__
#define __NBTK_TABLE_H__

#include <nbtk/nbtk-types.h>
#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

/**
 * NbtkTableChildOptions:
 * @NBTK_KEEP_ASPECT_RATIO: whether to respect the widget's aspect ratio
 * @NBTK_X_EXPAND: whether to allocate extra space on the widget's x-axis
 * @NBTK_Y_EXPAND: whether to allocate extra space on the widget's y-axis
 * @NBTK_X_FILL: whether to stretch the child to fill the cell horizontally
 * @NBTK_Y_FILL: whether to stretch the child to fill the cell vertically
 *
 * Denotes the child properties an NbtkTable child will have.
 */
typedef enum
{
  NBTK_KEEP_ASPECT_RATIO = 1 << 0,
  NBTK_X_EXPAND          = 1 << 1,
  NBTK_Y_EXPAND          = 1 << 2,
  NBTK_X_FILL            = 1 << 3,
  NBTK_Y_FILL            = 1 << 4
} NbtkTableChildOptions;

#define NBTK_TYPE_TABLE                (nbtk_table_get_type ())
#define NBTK_TABLE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TABLE, NbtkTable))
#define NBTK_IS_TABLE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TABLE))
#define NBTK_TABLE_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TABLE, NbtkTableClass))
#define NBTK_IS_TABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TABLE))
#define NBTK_TABLE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TABLE, NbtkTableClass))

typedef struct _NbtkTable              NbtkTable;
typedef struct _NbtkTablePrivate       NbtkTablePrivate;
typedef struct _NbtkTableClass         NbtkTableClass;

/**
 * NbtkTable:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _NbtkTable
{
  /*< private >*/
  NbtkWidget parent_instance;

  NbtkTablePrivate *priv;
};

struct _NbtkTableClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_table_get_type (void) G_GNUC_CONST;

NbtkWidget* nbtk_table_new                (void);
void        nbtk_table_set_col_spacing    (NbtkTable *table, gint spacing);
void        nbtk_table_set_row_spacing    (NbtkTable *table, gint spacing);
gint        nbtk_table_get_col_spacing    (NbtkTable *table);
gint        nbtk_table_get_row_spacing    (NbtkTable *table);
void        nbtk_table_set_padding        (NbtkTable *table, const NbtkPadding *padding);
void        nbtk_table_get_padding        (NbtkTable *table, NbtkPadding *padding);
void        nbtk_table_add_widget         (NbtkTable *table, NbtkWidget *widget, gint row, gint column);
void        nbtk_table_add_widget_full    (NbtkTable *table, NbtkWidget *widget, gint row, gint column, gint rowspan, gint colspan, NbtkTableChildOptions options, gdouble xalign, gdouble yalign);
void        nbtk_table_add_actor          (NbtkTable *table, ClutterActor *actor, gint row, gint column);
void        nbtk_table_add_actor_full     (NbtkTable *table, ClutterActor *actor, gint row, gint column, gint rowspan, gint colspan, NbtkTableChildOptions options, gdouble xalign, gdouble yalign);
void        nbtk_table_add_actor_with_properties (NbtkTable    *table,
                                                  ClutterActor *actor,
                                                  gint          row,
                                                  gint          column,
                                                  const gchar  *first_property_name,
                                                  ...);
void        nbtk_table_set_widget_colspan (NbtkTable *table, NbtkWidget *widget, gint colspan);
void        nbtk_table_set_widget_rowspan (NbtkTable *table, NbtkWidget *widget, gint rowspan);
void        nbtk_table_insert_actor_at_position (NbtkTable *table, ClutterActor *actor, gint x, gint y);

G_END_DECLS

#endif /* __NBTK_TABLE_H__ */
