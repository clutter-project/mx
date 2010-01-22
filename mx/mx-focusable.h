/*
 * mx-focusable: An interface for actors than can accept keyboard focus
 *
 * Copyright 2010 Intel Corporation
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_FOCUSABLE_H
#define _MX_FOCUSABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_FOCUSABLE mx_focusable_get_type()

#define MX_FOCUSABLE(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_FOCUSABLE, MxFocusable))

#define MX_IS_FOCUSABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_FOCUSABLE))

#define MX_FOCUSABLE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MX_TYPE_FOCUSABLE, MxFocusableIface))


typedef struct _MxFocusable      MxFocusable; /* dummy */
typedef struct _MxFocusableIface MxFocusableIface;

typedef enum
{
  MX_UP,
  MX_DOWN,
  MX_LEFT,
  MX_RIGHT,
  MX_NEXT,
  MX_PREVIOUS
} MxDirection;

struct _MxFocusableIface
{
  GObjectClass parent_class;

  MxFocusable* (*accept_focus) (MxFocusable *focusable);
  MxFocusable* (*move_focus)   (MxFocusable *focusable,
                                MxDirection  direction,
                                MxFocusable *from);
};

GType mx_focusable_get_type (void) G_GNUC_CONST;

MxFocusable* mx_focusable_move_focus   (MxFocusable *focusable,
                                        MxDirection  direction,
                                        MxFocusable *from);
MxFocusable* mx_focusable_accept_focus (MxFocusable *focusable);

G_END_DECLS

#endif /* _MX_FOCUSABLE_H */
