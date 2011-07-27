/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-droppable.c: droppable interface
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

#ifndef __MX_DROPPABLE_H__
#define __MX_DROPPABLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>
#include <mx/mx-draggable.h>

G_BEGIN_DECLS

#define MX_TYPE_DROPPABLE             (mx_droppable_get_type ())
#define MX_DROPPABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_DROPPABLE, MxDroppable))
#define MX_IS_DROPPABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_DROPPABLE))
#define MX_DROPPABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), MX_TYPE_DROPPABLE, MxDroppableIface))

/**
 * MxDroppable:
 *
 * This is an opaque structure whose members cannot be directly accessed.
 */
typedef struct _MxDroppable           MxDroppable; /* dummy typedef */
typedef struct _MxDroppableIface      MxDroppableIface;

struct _MxDroppableIface
{
  /*< private >*/
  GTypeInterface g_iface;

  /*< public >*/
  /* vfuncs, not signals */
  void     (* enable)      (MxDroppable *droppable);
  void     (* disable)     (MxDroppable *droppable);

  gboolean (* accept_drop) (MxDroppable *droppable,
                            MxDraggable *draggable);

  /* signals */
  void (* over_in)  (MxDroppable         *droppable,
                     MxDraggable         *draggable);
  void (* over_out) (MxDroppable         *droppable,
                     MxDraggable         *draggable);
  void (* drop)     (MxDroppable         *droppable,
                     MxDraggable         *draggable,
                     gfloat               event_x,
                     gfloat               event_y,
                     gint                 button,
                     ClutterModifierType  modifiers);
};

GType mx_droppable_get_type (void) G_GNUC_CONST;

void     mx_droppable_enable      (MxDroppable *droppable);
void     mx_droppable_disable     (MxDroppable *droppable);
gboolean mx_droppable_is_enabled  (MxDroppable *droppable);

gboolean mx_droppable_accept_drop (MxDroppable *droppable,
                                   MxDraggable *draggable);

G_END_DECLS

#endif /* __MX_DROPPABLE_H__ */
