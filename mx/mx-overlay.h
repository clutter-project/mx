/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-overlay.c: A layer mecanism for tooltips and menus.
 *
 * Copyright 2012 Intel Corporation.
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
 * Written by: Lionel Landwerlin <lionel.g.landwerlin@linux.intel.com>
 *
 */

#ifndef _MX_OVERLAY_H
#define _MX_OVERLAY_H

#include <glib-object.h>

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_OVERLAY mx_overlay_get_type()

#define MX_OVERLAY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_OVERLAY, MxOverlay))

#define MX_OVERLAY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_OVERLAY, MxOverlayClass))

#define MX_IS_OVERLAY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_OVERLAY))

#define MX_IS_OVERLAY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_OVERLAY))

#define MX_OVERLAY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_OVERLAY, MxOverlayClass))

typedef struct _MxOverlay MxOverlay;
typedef struct _MxOverlayClass MxOverlayClass;
typedef struct _MxOverlayPrivate MxOverlayPrivate;

struct _MxOverlay
{
  ClutterActor parent;

  MxOverlayPrivate *priv;
};

struct _MxOverlayClass
{
  ClutterActorClass parent_class;
};

GType mx_overlay_get_type (void) G_GNUC_CONST;

ClutterActor *mx_overlay_new (void);

G_END_DECLS

#endif /* _MX_OVERLAY_H */
