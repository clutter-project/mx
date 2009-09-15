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

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef _NBTK_LIST_VIEW_H
#define _NBTK_LIST_VIEW_H

#include <glib-object.h>
#include "nbtk-grid.h"
#include "nbtk-item-factory.h"

G_BEGIN_DECLS

#define NBTK_TYPE_LIST_VIEW nbtk_list_view_get_type()

#define NBTK_LIST_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_LIST_VIEW, NbtkListView))

#define NBTK_LIST_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_LIST_VIEW, NbtkListViewClass))

#define NBTK_IS_LIST_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_LIST_VIEW))

#define NBTK_IS_LIST_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_LIST_VIEW))

#define NBTK_LIST_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_LIST_VIEW, NbtkListViewClass))

typedef struct _NbtkListViewPrivate NbtkListViewPrivate;

/**
 * NbtkListView:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  NbtkGrid parent;

  NbtkListViewPrivate *priv;
} NbtkListView;

typedef struct {
  NbtkGridClass parent_class;
} NbtkListViewClass;

GType nbtk_list_view_get_type (void);

NbtkWidget* nbtk_list_view_new (void);

void         nbtk_list_view_set_model (NbtkListView *list_view,
                                       ClutterModel *model);
ClutterModel* nbtk_list_view_get_model (NbtkListView *list_view);

void          nbtk_list_view_set_item_type (NbtkListView *list_view,
                                            GType         item_type);
GType         nbtk_list_view_get_item_type (NbtkListView *list_view);

void
nbtk_list_view_add_attribute (NbtkListView *list_view,
                              const gchar *attribute,
                              gint column);

void nbtk_list_view_freeze (NbtkListView *list_view);
void nbtk_list_view_thaw (NbtkListView *list_view);
void nbtk_list_view_set_factory (NbtkListView    *list_view,
                                 NbtkItemFactory *factory);


G_END_DECLS

#endif /* _NBTK_LIST_VIEW_H */
