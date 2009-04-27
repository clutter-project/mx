/* nbtk-cell-renderer.h */

#ifndef _NBTK_CELL_RENDERER_H
#define _NBTK_CELL_RENDERER_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_CELL_RENDERER nbtk_cell_renderer_get_type()

#define NBTK_CELL_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_CELL_RENDERER, NbtkCellRenderer))

#define NBTK_CELL_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_CELL_RENDERER, NbtkCellRendererClass))

#define NBTK_IS_CELL_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_CELL_RENDERER))

#define NBTK_IS_CELL_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_CELL_RENDERER))

#define NBTK_CELL_RENDERER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_CELL_RENDERER, NbtkCellRendererClass))

typedef struct {
  GInitiallyUnowned parent;
} NbtkCellRenderer;

typedef struct {
  GInitiallyUnownedClass parent_class;

  ClutterActor *(*get_actor) (NbtkCellRenderer *renderer);
} NbtkCellRendererClass;

GType nbtk_cell_renderer_get_type (void);

ClutterActor* nbtk_cell_renderer_get_actor (NbtkCellRenderer *renderer);

G_END_DECLS

#endif /* _NBTK_CELL_RENDERER_H */
