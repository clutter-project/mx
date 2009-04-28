/* nbtk-icon-view.h */

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

typedef struct {
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



G_END_DECLS

#endif /* _NBTK_ICON_VIEW_H */
