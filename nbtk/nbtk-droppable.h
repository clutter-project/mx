#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly."
#endif

#ifndef __NBTK_DROPPABLE_H__
#define __NBTK_DROPPABLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>
#include <nbtk/nbtk-draggable.h>

G_BEGIN_DECLS

#define NBTK_TYPE_DROPPABLE             (nbtk_droppable_get_type ())
#define NBTK_DROPPABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_DROPPABLE, NbtkDroppable))
#define NBTK_IS_DROPPABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_DROPPABLE))
#define NBTK_DROPPABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), NBTK_TYPE_DROPPABLE, NbtkDroppableIface))

typedef struct _NbtkDroppable           NbtkDroppable; /* dummy typedef */
typedef struct _NbtkDroppableIface      NbtkDroppableIface;

struct _NbtkDroppableIface
{
  GTypeInterface g_iface;

  /* vfuncs, not signals */
  void     (* enable)      (NbtkDroppable *droppable);
  void     (* disable)     (NbtkDroppable *droppable);

  gboolean (* accept_drop) (NbtkDroppable *droppable,
                            NbtkDraggable *draggable);

  /* signals */
  void (* over_in)  (NbtkDroppable       *droppable,
                     NbtkDraggable       *draggable);
  void (* over_out) (NbtkDroppable       *droppable,
                     NbtkDraggable       *draggable);
  void (* drop)     (NbtkDroppable       *droppable,
                     NbtkDraggable       *draggable,
                     gfloat               event_x,
                     gfloat               event_y,
                     gint                 button,
                     ClutterModifierType  modifiers);
};

GType nbtk_droppable_get_type (void) G_GNUC_CONST;

void     nbtk_droppable_enable      (NbtkDroppable *droppable);
void     nbtk_droppable_disable     (NbtkDroppable *droppable);
gboolean nbtk_droppable_is_enabled  (NbtkDroppable *droppable);

gboolean nbtk_droppable_accept_drop (NbtkDroppable *droppable,
                                     NbtkDraggable *draggable);

G_END_DECLS

#endif /* __NBTK_DROPPABLE_H__ */
