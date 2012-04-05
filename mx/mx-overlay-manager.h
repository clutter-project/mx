/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-overlay-manager.h: A layer mecanism for tooltips and menus.
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

#ifndef _MX_OVERLAY_MANAGER_H
#define _MX_OVERLAY_MANAGER_H

#include <glib-object.h>

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_OVERLAY_MANAGER mx_overlay_manager_get_type()

#define MX_OVERLAY_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_OVERLAY_MANAGER, MxOverlayManager))

#define MX_OVERLAY_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_OVERLAY_MANAGER, MxOverlayManagerClass))

#define MX_IS_OVERLAY_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_OVERLAY_MANAGER))

#define MX_IS_OVERLAY_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_OVERLAY_MANAGER))

#define MX_OVERLAY_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_OVERLAY_MANAGER, MxOverlayManagerClass))

/**
 * MxOverlayLayer:
 * @MX_OVERLAY_LAYER_TOOLTIP: Layer for tooltips
 * @MX_OVERLAY_LAYER_MENU: Layer for menus
 *
 * Support values layer
 */
typedef enum /*< prefix=MX_OVERLAY_LAYER >*/
{
  MX_OVERLAY_LAYER_TOOLTIP,
  MX_OVERLAY_LAYER_MENU,

  MX_OVERLAY_LAST
} MxOverlayLayer;

typedef struct _MxOverlayManager MxOverlayManager;
typedef struct _MxOverlayManagerClass MxOverlayManagerClass;
typedef struct _MxOverlayManagerPrivate MxOverlayManagerPrivate;

struct _MxOverlayManager
{
  GObject parent;

  MxOverlayManagerPrivate *priv;
};

struct _MxOverlayManagerClass
{
  GObjectClass parent_class;
};

GType mx_overlay_manager_get_type (void) G_GNUC_CONST;

ClutterActor *mx_overlay_manager_get_overlay (MxOverlayManager *manager,
                                              MxOverlayLayer    overlay);

MxOverlayManager *mx_overlay_manager_get_for_stage (ClutterActor *stage);

G_END_DECLS

#endif /* _MX_OVERLAY_MANAGER_H */
