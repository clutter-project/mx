/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_table_get_type (void) G_GNUC_CONST;

ClutterActor *mx_table_new (void);


void mx_table_set_column_spacing (MxTable      *table,
                                  gint          spacing);
gint mx_table_get_column_spacing (MxTable      *table);

void mx_table_set_row_spacing    (MxTable      *table,
                                  gint          spacing);
gint mx_table_get_row_spacing    (MxTable      *table);

void mx_table_add_actor          (MxTable      *table,
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
