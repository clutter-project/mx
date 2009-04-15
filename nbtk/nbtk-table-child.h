
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

#ifndef __NBTK_TABLE_CHILD_H__
#define __NBTK_TABLE_CHILD_H__

#include <nbtk/nbtk-types.h>
#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_TABLE_CHILD          (nbtk_table_child_get_type ())
#define NBTK_TABLE_CHILD(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChild))
#define NBTK_IS_TABLE_CHILD(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))
#define NBTK_IS_TABLE_CHILD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))

typedef struct _NbtkTableChild         NbtkTableChild;
typedef struct _NbtkTableChildClass    NbtkTableChildClass;

struct _NbtkTableChild
{
  NbtkWidgetChild parent_instance;

  gint col;
  gint row;
  gint col_span;
  gint row_span;
  gboolean keep_ratio : 1;
  gboolean x_expand : 1;
  gboolean y_expand : 1;
  gboolean x_fill : 1;
  gboolean y_fill : 1;
  gdouble x_align;
  gdouble y_align;
};

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

struct _NbtkTableChildClass
{
  NbtkWidgetChildClass parent_class;
};

GType nbtk_table_child_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __NBTK_TABLE_H__ */
