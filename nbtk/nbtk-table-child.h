/*
 * nbtk-table-child.h: Table child implementation
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

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_TABLE_CHILD_H__
#define __NBTK_TABLE_CHILD_H__

#include <nbtk/nbtk-types.h>
#include <nbtk/nbtk-widget.h>
#include <nbtk/nbtk-table.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_TABLE_CHILD          (nbtk_table_child_get_type ())
#define NBTK_TABLE_CHILD(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChild))
#define NBTK_IS_TABLE_CHILD(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))
#define NBTK_IS_TABLE_CHILD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))

typedef struct _NbtkTableChild         NbtkTableChild;
typedef struct _NbtkTableChildClass    NbtkTableChildClass;

/**
 * NbtkTableChild:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
struct _NbtkTableChild
{
  /*< private >*/
  NbtkWidgetChild parent_instance;

  gint col;
  gint row;
  gint col_span;
  gint row_span;
  gdouble x_align;
  gdouble y_align;
  guint allocate_hidden : 1;
  guint x_expand : 1;
  guint y_expand : 1;
  guint x_fill : 1;
  guint y_fill : 1;
};


struct _NbtkTableChildClass
{
  NbtkWidgetChildClass parent_class;
};

GType nbtk_table_child_get_type (void) G_GNUC_CONST;

gint nbtk_table_child_get_col_span (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_col_span (NbtkTable *table, ClutterActor *child, gint span);

gint nbtk_table_child_get_row_span (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_row_span (NbtkTable *table, ClutterActor *child, gint span);

gboolean nbtk_table_child_get_x_fill (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_x_fill (NbtkTable *table, ClutterActor *child, gboolean fill);

gboolean nbtk_table_child_get_y_fill (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_y_fill (NbtkTable *table, ClutterActor *child, gboolean fill);

gboolean nbtk_table_child_get_col_expand (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_col_expand (NbtkTable *table, ClutterActor *child, gboolean expand);

gboolean nbtk_table_child_get_row_expand (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_row_expand (NbtkTable *table, ClutterActor *child, gboolean expand);

NbtkAlign nbtk_table_child_get_x_align (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_x_align (NbtkTable *table, ClutterActor *child, NbtkAlign align);

NbtkAlign nbtk_table_child_get_y_align (NbtkTable *table, ClutterActor *child);
void nbtk_table_child_set_y_align (NbtkTable *table, ClutterActor *child, NbtkAlign align);
void nbtk_table_child_set_allocate_hidden (NbtkTable *table, ClutterActor *child, gboolean value);
gboolean nbtk_table_child_get_allocate_hidden (NbtkTable *table, ClutterActor *child);

G_END_DECLS

#endif /* __NBTK_TABLE_H__ */
