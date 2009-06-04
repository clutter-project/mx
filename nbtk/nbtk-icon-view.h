/*
 * nbtk-icon-view.h: NbtkGrid powered by a model
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


#ifndef _NBTK_ICON_VIEW_H
#define _NBTK_ICON_VIEW_H

#include <glib-object.h>
#include "nbtk-grid.h"
#include "nbtk-cell-renderer.h"

G_BEGIN_DECLS

#define NBTK_TYPE_ICON_VIEW nbtk_icon_view_get_type()

#define NBTK_ICON_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_ICON_VIEW, NbtkIconView))

#define NBTK_ICON_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_ICON_VIEW, NbtkIconViewClass))

#define NBTK_IS_ICON_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_ICON_VIEW))

#define NBTK_IS_ICON_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_ICON_VIEW))

#define NBTK_ICON_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_ICON_VIEW, NbtkIconViewClass))

typedef struct _NbtkIconViewPrivate NbtkIconViewPrivate;

/**
 * NbtkIconView:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  NbtkGrid parent;

  NbtkIconViewPrivate *priv;
} NbtkIconView;

typedef struct {
  NbtkGridClass parent_class;
} NbtkIconViewClass;

GType nbtk_icon_view_get_type (void);

NbtkWidget* nbtk_icon_view_new (void);

void         nbtk_icon_view_set_model (NbtkIconView *icon_view,
                                       ClutterModel *model);
ClutterModel* nbtk_icon_view_get_model (NbtkIconView *icon_view);

void              nbtk_icon_view_set_cell_renderer (NbtkIconView     *icon_view,
                                                    NbtkCellRenderer *renderer);
NbtkCellRenderer* nbtk_icon_view_get_cell_renderer (NbtkIconView *icon_view);

void
nbtk_icon_view_add_attribute (NbtkIconView *icon_view,
                              const gchar *attribute,
                              gint column);

void nbtk_icon_view_freeze (NbtkIconView *icon_view);
void nbtk_icon_view_thaw (NbtkIconView *icon_view);


G_END_DECLS

#endif /* _NBTK_ICON_VIEW_H */
