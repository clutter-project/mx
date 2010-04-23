/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-draggable.h: draggable interface
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
 *
 * Written by: Emmanuele Bassi <ebassi@linux.intel.com>
 *
 */


#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly."
#endif

#ifndef __MX_DRAGGABLE_H__
#define __MX_DRAGGABLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_DRAGGABLE             (mx_draggable_get_type ())
#define MX_DRAGGABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_DRAGGABLE, MxDraggable))
#define MX_IS_DRAGGABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_DRAGGABLE))
#define MX_DRAGGABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), MX_TYPE_DRAGGABLE, MxDraggableIface))

/**
 * MxDraggable:
 *
 * This is an opaque structure whose members cannot be directly accessed.
 */
typedef struct _MxDraggable           MxDraggable; /* dummy typedef */
typedef struct _MxDraggableIface      MxDraggableIface;

typedef enum {
  MX_DRAG_AXIS_NONE,
  MX_DRAG_AXIS_X,
  MX_DRAG_AXIS_Y
} MxDragAxis;

#if 0
/*
typedef enum {
  MX_DISABLE_CONTAINMENT,
  MX_CONTAIN_IN_STAGE,
  MX_CONTAIN_IN_PARENT,
  MX_CONTAIN_IN_AREA
} MxDragContainment;
*/
#endif

/**
 * MxDraggableIface:
 * @enable: virtual function called when enabling a #MxDraggable; MX
 *    already provides a default implementation
 * @disable: virtual function called when disabling a #MxDraggable; MX
 *    already provides a default implementation
 * @drag_begin: class handler for the #MxDraggable::drag-begin signal
 * @drag_motion: class handler for the #MxDraggable::drag-motion signal
 * @drag_end: class handler for the #MxDraggable::drag-end signal
 *
 * Interface for draggable #ClutterActor<!-- -->s.
 */
struct _MxDraggableIface
{
  /*< private >*/
  GTypeInterface g_iface;

  /*< public >*/
  /* vfuncs, not signals */
  void (* enable)  (MxDraggable *draggable);
  void (* disable) (MxDraggable *draggable);

  /* signals */
  void (* drag_begin)  (MxDraggable         *draggable,
                        gfloat               event_x,
                        gfloat               event_y,
                        gint                 event_button,
                        ClutterModifierType  modifiers);
  void (* drag_motion) (MxDraggable         *draggable,
                        gfloat               delta_x,
                        gfloat               delta_y);
  void (* drag_end)    (MxDraggable         *draggable,
                        gfloat               event_x,
                        gfloat               event_y);
};

GType mx_draggable_get_type (void) G_GNUC_CONST;

void              mx_draggable_set_axis             (MxDraggable       *draggable,
                                                     MxDragAxis         axis);
MxDragAxis        mx_draggable_get_axis             (MxDraggable       *draggable);

void              mx_draggable_set_drag_threshold   (MxDraggable       *draggable,
                                                     guint              threshold);
guint             mx_draggable_get_drag_threshold   (MxDraggable       *draggable);
#if 0
void              mx_draggable_set_containment_type (MxDraggable       *draggable,
                                                     MxDragContainment  containment);
MxDragContainment mx_draggable_get_containment_type (MxDraggable       *draggable);
void              mx_draggable_set_containment_area (MxDraggable       *draggable,
                                                     gfloat             x_1,
                                                     gfloat             y_1,
                                                     gfloat             x_2,
                                                     gfloat             y_2);
void              mx_draggable_get_containment_area (MxDraggable       *draggable,
                                                     gfloat            *x_1,
                                                     gfloat            *y_1,
                                                     gfloat            *x_2,
                                                     gfloat            *y_2);
#endif
void              mx_draggable_set_drag_actor       (MxDraggable       *draggable,
                                                     ClutterActor      *actor);
ClutterActor *    mx_draggable_get_drag_actor       (MxDraggable       *draggable);

void              mx_draggable_disable              (MxDraggable       *draggable);
void              mx_draggable_enable               (MxDraggable       *draggable);
gboolean          mx_draggable_is_enabled           (MxDraggable       *draggable);

G_END_DECLS

#endif /* __MX_DRAGGABLE_H__ */
