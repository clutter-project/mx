/*
 * mx-offscreen.h: An offscreen texture container
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_OFFSCREEN_H
#define _MX_OFFSCREEN_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_OFFSCREEN mx_offscreen_get_type()

#define MX_OFFSCREEN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_OFFSCREEN, MxOffscreen))

#define MX_OFFSCREEN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_OFFSCREEN, MxOffscreenClass))

#define MX_IS_OFFSCREEN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_OFFSCREEN))

#define MX_IS_OFFSCREEN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_OFFSCREEN))

#define MX_OFFSCREEN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_OFFSCREEN, MxOffscreenClass))

/**
 * MxOffscreen:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct _MxOffscreen MxOffscreen;
typedef struct _MxOffscreenClass MxOffscreenClass;
typedef struct _MxOffscreenPrivate MxOffscreenPrivate;

struct _MxOffscreen
{
  ClutterTexture parent;

  MxOffscreenPrivate *priv;
};

struct _MxOffscreenClass
{
  ClutterTextureClass parent_class;

  /* vfuncs */
  void (* paint_child) (MxOffscreen *self);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_offscreen_get_type (void) G_GNUC_CONST;

ClutterActor *mx_offscreen_new (void);

void mx_offscreen_set_child (MxOffscreen *offscreen, ClutterActor *actor);
ClutterActor *mx_offscreen_get_child (MxOffscreen *offscreen);

void mx_offscreen_set_pick_child (MxOffscreen *offscreen, gboolean pick);
gboolean mx_offscreen_get_pick_child (MxOffscreen *offscreen);

void mx_offscreen_set_auto_update (MxOffscreen *offscreen,
                                   gboolean auto_update);
gboolean mx_offscreen_get_auto_update (MxOffscreen *offscreen);

void     mx_offscreen_set_redirect_enabled (MxOffscreen *offscreen,
                                            gboolean     enabled);
gboolean mx_offscreen_get_redirect_enabled (MxOffscreen *offscreen);

CoglHandle mx_offscreen_get_buffer (MxOffscreen *offscreen);

void mx_offscreen_update (MxOffscreen *offscreen);

void mx_offscreen_set_accumulation_enabled (MxOffscreen *offscreen,
                                           gboolean     enable);
gboolean mx_offscreen_get_accumulation_enabled (MxOffscreen *offscreen);

CoglHandle mx_offscreen_get_accumulation_material (MxOffscreen *offscreen);

G_END_DECLS

#endif /* _MX_OFFSCREEN_H */
