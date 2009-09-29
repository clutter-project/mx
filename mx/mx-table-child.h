/*
 * mx-table-child.h: Table child implementation
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

#ifndef __MX_TABLE_CHILD_H__
#define __MX_TABLE_CHILD_H__

#include <mx/mx-types.h>
#include <mx/mx-widget.h>
#include <mx/mx-table.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_TABLE_CHILD          (mx_table_child_get_type ())
#define MX_TABLE_CHILD(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_TABLE_CHILD, MxTableChild))
#define MX_IS_TABLE_CHILD(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_TABLE_CHILD))
#define MX_TABLE_CHILD_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_TABLE_CHILD, MxTableChildClass))
#define MX_IS_TABLE_CHILD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_TABLE_CHILD))
#define MX_TABLE_CHILD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_TABLE_CHILD, MxTableChildClass))

typedef struct _MxTableChild         MxTableChild;
typedef struct _MxTableChildClass    MxTableChildClass;

/**
 * MxTableChild:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
struct _MxTableChild
{
  /*< private >*/
  ClutterChildMeta parent_instance;

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


struct _MxTableChildClass
{
  ClutterChildMetaClass parent_class;
};

GType mx_table_child_get_type (void) G_GNUC_CONST;

gint     mx_table_child_get_col_span        (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_col_span        (MxTable      *table,
                                             ClutterActor *child,
                                             gint          span);
gint     mx_table_child_get_row_span        (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_row_span        (MxTable      *table,
                                             ClutterActor *child,
                                             gint          span);
gboolean mx_table_child_get_x_fill          (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_x_fill          (MxTable      *table,
                                             ClutterActor *child,
                                             gboolean      fill);
gboolean mx_table_child_get_y_fill          (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_y_fill          (MxTable      *table,
                                             ClutterActor *child,
                                             gboolean      fill);
gboolean mx_table_child_get_x_expand        (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_x_expand        (MxTable      *table,
                                             ClutterActor *child,
                                             gboolean      expand);
gboolean mx_table_child_get_y_expand        (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_y_expand        (MxTable      *table,
                                             ClutterActor *child,
                                             gboolean      expand);
MxAlign  mx_table_child_get_x_align         (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_x_align         (MxTable      *table,
                                             ClutterActor *child,
                                             MxAlign       align);
MxAlign  mx_table_child_get_y_align         (MxTable      *table,
                                             ClutterActor *child);
void     mx_table_child_set_y_align         (MxTable      *table,
                                             ClutterActor *child,
                                             MxAlign       align);
void     mx_table_child_set_allocate_hidden (MxTable      *table,
                                             ClutterActor *child,
                                             gboolean      value);
gboolean mx_table_child_get_allocate_hidden (MxTable      *table,
                                             ClutterActor *child);

G_END_DECLS

#endif /* __MX_TABLE_H__ */
