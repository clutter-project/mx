#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly."
#endif

#ifndef __NBTK_DRAGGABLE_H__
#define __NBTK_DRAGGABLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_DRAGGABLE             (nbtk_draggable_get_type ())
#define NBTK_DRAGGABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_DRAGGABLE, NbtkDraggable))
#define NBTK_IS_DRAGGABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_DRAGGABLE))
#define NBTK_DRAGGABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), NBTK_TYPE_DRAGGABLE, NbtkDraggableIface))

typedef struct _NbtkDraggable           NbtkDraggable; /* dummy typedef */
typedef struct _NbtkDraggableIface      NbtkDraggableIface;

typedef enum {
  NBTK_NO_AXIS,
  NBTK_X_AXIS,
  NBTK_Y_AXIS
} NbtkDragAxis;

typedef enum {
  NBTK_DISABLE_CONTAINMENT,
  NBTK_CONTAIN_IN_STAGE,
  NBTK_CONTAIN_IN_PARENT,
  NBTK_CONTAIN_IN_AREA
} NbtkDragContainment;

/**
 * NbtkDraggableIface:
 * @enable: virtual function called when enabling a #NbtkDraggable; NBTK
 *    already provides a default implementation
 * @disable: virtual function called when disabling a #NbtkDraggable; NBTK
 *    already provides a default implementation
 * @drag_begin: class handler for the #NbtkDraggable::drag-begin signal
 * @drag_motion: class handler for the #NbtkDraggable::drag-motion signal
 * @drag_end: class handler for the #NbtkDraggable::drag-end signal
 *
 * Interface for draggable #ClutterActor<!-- -->s.
 */
struct _NbtkDraggableIface
{
  GTypeInterface g_iface;

  /* vfuncs, not signals */
  void (* enable)  (NbtkDraggable *draggable);
  void (* disable) (NbtkDraggable *draggable);

  /* signals */
  void (* drag_begin)  (NbtkDraggable       *draggable,
                        gfloat               event_x,
                        gfloat               event_y,
                        gint                 event_button,
                        ClutterModifierType  modifiers);
  void (* drag_motion) (NbtkDraggable       *draggable,
                        gfloat               delta_x,
                        gfloat               delta_y);
  void (* drag_end)    (NbtkDraggable       *draggable,
                        gfloat               event_x,
                        gfloat               event_y);
};

GType nbtk_draggable_get_type (void) G_GNUC_CONST;

void                nbtk_draggable_set_axis             (NbtkDraggable       *draggable,
                                                         NbtkDragAxis         axis);
NbtkDragAxis        nbtk_draggable_get_axis             (NbtkDraggable       *draggable);

void                nbtk_draggable_set_drag_threshold   (NbtkDraggable       *draggable,
                                                         guint                threshold);
guint               nbtk_draggable_get_drag_threshold   (NbtkDraggable       *draggable);

void                nbtk_draggable_set_containment_type (NbtkDraggable       *draggable,
                                                         NbtkDragContainment  containment);
NbtkDragContainment nbtk_draggable_get_containment_type (NbtkDraggable       *draggable);
void                nbtk_draggable_set_containment_area (NbtkDraggable       *draggable,
                                                         gfloat               x_1,
                                                         gfloat               y_1,
                                                         gfloat               x_2,
                                                         gfloat               y_2);
void                nbtk_draggable_get_containment_area (NbtkDraggable       *draggable,
                                                         gfloat              *x_1,
                                                         gfloat              *y_1,
                                                         gfloat              *x_2,
                                                         gfloat              *y_2);

void                nbtk_draggable_disable              (NbtkDraggable       *draggable);
void                nbtk_draggable_enable               (NbtkDraggable       *draggable);
gboolean            nbtk_draggable_is_enabled           (NbtkDraggable       *draggable);

G_END_DECLS

#endif /* __NBTK_DRAGGABLE_H__ */
