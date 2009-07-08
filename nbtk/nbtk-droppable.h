/*
 * nbtk-droppable.c: droppable interface
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
