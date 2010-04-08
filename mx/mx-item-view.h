/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-item-view.h: MxGrid powered by a model
 *
 * Copyright 2009 Intel Corporation.
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_ITEM_VIEW_H
#define _MX_ITEM_VIEW_H

#include <glib-object.h>

#include "mx-grid.h"
#include "mx-item-factory.h"

G_BEGIN_DECLS

#define MX_TYPE_ITEM_VIEW mx_item_view_get_type()

#define MX_ITEM_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_ITEM_VIEW, MxItemView))

#define MX_ITEM_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_ITEM_VIEW, MxItemViewClass))

#define MX_IS_ITEM_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_ITEM_VIEW))

#define MX_IS_ITEM_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_ITEM_VIEW))

#define MX_ITEM_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_ITEM_VIEW, MxItemViewClass))

typedef struct _MxItemViewPrivate MxItemViewPrivate;

/**
 * MxItemView:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  MxGrid parent;

  MxItemViewPrivate *priv;
} MxItemView;

typedef struct {
  MxGridClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
} MxItemViewClass;

GType mx_item_view_get_type (void);

ClutterActor *mx_item_view_new (void);

void          mx_item_view_set_model     (MxItemView    *item_view,
                                          ClutterModel  *model);
ClutterModel* mx_item_view_get_model     (MxItemView    *item_view);

void          mx_item_view_set_item_type (MxItemView    *item_view,
                                          GType          item_type);
GType         mx_item_view_get_item_type (MxItemView    *item_view);

void          mx_item_view_add_attribute (MxItemView    *item_view,
                                          const gchar   *attribute,
                                          gint           column);

void          mx_item_view_freeze        (MxItemView    *item_view);
void          mx_item_view_thaw          (MxItemView    *item_view);
void          mx_item_view_set_factory   (MxItemView    *item_view,
                                          MxItemFactory *factory);
MxItemFactory* mx_item_view_get_factory  (MxItemView    *item_view);

G_END_DECLS

#endif /* _MX_ITEM_VIEW_H */
