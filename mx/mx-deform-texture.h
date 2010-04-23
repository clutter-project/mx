/*
 * mx-deform-texture.h: A texture deformation actor
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

#ifndef _MX_DEFORM_TEXTURE_H
#define _MX_DEFORM_TEXTURE_H

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_DEFORM_TEXTURE mx_deform_texture_get_type()

#define MX_DEFORM_TEXTURE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DEFORM_TEXTURE, MxDeformTexture))

#define MX_DEFORM_TEXTURE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DEFORM_TEXTURE, MxDeformTextureClass))

#define MX_IS_DEFORM_TEXTURE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DEFORM_TEXTURE))

#define MX_IS_DEFORM_TEXTURE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DEFORM_TEXTURE))

#define MX_DEFORM_TEXTURE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DEFORM_TEXTURE, MxDeformTextureClass))

typedef struct _MxDeformTexture MxDeformTexture;
typedef struct _MxDeformTextureClass MxDeformTextureClass;
typedef struct _MxDeformTexturePrivate MxDeformTexturePrivate;

/**
 * MxDeformTexture:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxDeformTexture
{
  MxWidget parent;

  MxDeformTexturePrivate *priv;
};

struct _MxDeformTextureClass
{
  MxWidgetClass parent_class;

  /* vfuncs */
  void (*deform) (MxDeformTexture   *texture,
                  CoglTextureVertex *vertex,
                  gfloat             width,
                  gfloat             height);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_deform_texture_get_type (void) G_GNUC_CONST;

void mx_deform_texture_get_resolution (MxDeformTexture *texture,
                                       gint            *tiles_x,
                                       gint            *tiles_y);

void mx_deform_texture_set_resolution (MxDeformTexture *texture,
                                       gint             tiles_x,
                                       gint             tiles_y);

void mx_deform_texture_set_textures (MxDeformTexture *texture,
                                     ClutterTexture  *front,
                                     ClutterTexture  *back);

void mx_deform_texture_get_textures (MxDeformTexture  *texture,
                                     ClutterTexture  **front,
                                     ClutterTexture  **back);

void mx_deform_texture_invalidate (MxDeformTexture *texture);

G_END_DECLS

#endif /* _MX_DEFORM_TEXTURE_H */
