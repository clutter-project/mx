/*
 * nbtk-item-view.h: NbtkGrid powered by a model
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


#ifndef _NBTK_ITEM_VIEW_H
#define _NBTK_ITEM_VIEW_H

#include <glib-object.h>
#include "nbtk-grid.h"

G_BEGIN_DECLS

#define NBTK_TYPE_ITEM_VIEW nbtk_item_view_get_type()

#define NBTK_ITEM_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_ITEM_VIEW, NbtkItemView))

#define NBTK_ITEM_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_ITEM_VIEW, NbtkItemViewClass))

#define NBTK_IS_ITEM_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_ITEM_VIEW))

#define NBTK_IS_ITEM_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_ITEM_VIEW))

#define NBTK_ITEM_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_ITEM_VIEW, NbtkItemViewClass))

typedef struct _NbtkItemViewPrivate NbtkItemViewPrivate;

/**
 * NbtkItemView:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  NbtkGrid parent;

  NbtkItemViewPrivate *priv;
} NbtkItemView;

typedef struct {
  NbtkGridClass parent_class;
} NbtkItemViewClass;

GType nbtk_item_view_get_type (void);

NbtkWidget* nbtk_item_view_new (void);

void         nbtk_item_view_set_model (NbtkItemView *item_view,
                                       ClutterModel *model);
ClutterModel* nbtk_item_view_get_model (NbtkItemView *item_view);

void          nbtk_item_view_set_item_type (NbtkItemView *item_view,
                                            GType         item_type);
GType         nbtk_item_view_get_item_type (NbtkItemView *item_view);

void
nbtk_item_view_add_attribute (NbtkItemView *item_view,
                              const gchar *attribute,
                              gint column);

void nbtk_item_view_freeze (NbtkItemView *item_view);
void nbtk_item_view_thaw (NbtkItemView *item_view);


G_END_DECLS

#endif /* _NBTK_ITEM_VIEW_H */
