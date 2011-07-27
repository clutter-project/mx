/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scrollable.h: Scrollable interface
 *
 * Copyright 2008 OpenedHand
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
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_SCROLLABLE_H__
#define __MX_SCROLLABLE_H__

#include <glib-object.h>
#include <mx/mx-adjustment.h>

G_BEGIN_DECLS

#define MX_TYPE_SCROLLABLE            (mx_scrollable_get_type ())
#define MX_SCROLLABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SCROLLABLE, MxScrollable))
#define MX_IS_SCROLLABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SCROLLABLE))
#define MX_SCROLLABLE_GET_IFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MX_TYPE_SCROLLABLE, MxScrollableIface))

/**
 * MxScrollable:
 *
 * This is an opaque structure whose members cannot be directly accessed.
 */
typedef struct _MxScrollable MxScrollable; /* Dummy object */
typedef struct _MxScrollableIface MxScrollableIface;

struct _MxScrollableIface
{
  /*< private >*/
  GTypeInterface parent;

  /*< public >*/
  void (* set_adjustments) (MxScrollable  *scrollable,
                            MxAdjustment  *hadjustment,
                            MxAdjustment  *vadjustment);
  void (* get_adjustments) (MxScrollable  *scrollable,
                            MxAdjustment **hadjustment,
                            MxAdjustment **vadjustment);
};

GType mx_scrollable_get_type (void) G_GNUC_CONST;

void mx_scrollable_set_adjustments (MxScrollable  *scrollable,
                                    MxAdjustment  *hadjustment,
                                    MxAdjustment  *vadjustment);
void mx_scrollable_get_adjustments (MxScrollable  *scrollable,
                                    MxAdjustment **hadjustment,
                                    MxAdjustment **vadjustment);

G_END_DECLS

#endif /* __MX_SCROLLABLE_H__ */
