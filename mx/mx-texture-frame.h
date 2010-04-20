/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-texture-frame.h: Expandible texture actor
 *
 * Copyright 2007, 2008 OpenedHand Ltd
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
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_TEXTURE_FRAME_H__
#define __MX_TEXTURE_FRAME_H__

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_TEXTURE_FRAME                 (mx_texture_frame_get_type ())
#define MX_TEXTURE_FRAME(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_TEXTURE_FRAME, MxTextureFrame))
#define MX_TEXTURE_FRAME_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_TEXTURE_FRAME, MxTextureFrameClass))
#define MX_IS_TEXTURE_FRAME(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_TEXTURE_FRAME))
#define MX_IS_TEXTURE_FRAME_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_TEXTURE_FRAME))
#define MX_TEXTURE_FRAME_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_TEXTURE_FRAME, MxTextureFrameClass))

typedef struct _MxTextureFrame                MxTextureFrame;
typedef struct _MxTextureFramePrivate         MxTextureFramePrivate;
typedef struct _MxTextureFrameClass           MxTextureFrameClass;

/**
 * MxTextureFrame:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxTextureFrame
{
  /*< private >*/
  ClutterActor parent_instance;
  
  MxTextureFramePrivate    *priv;
};

struct _MxTextureFrameClass 
{
  ClutterActorClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_texture_frame_get_type (void) G_GNUC_CONST;

ClutterActor   *mx_texture_frame_new                (ClutterTexture *texture,
                                                     gfloat          top,
                                                     gfloat          right,
                                                     gfloat          bottom,
                                                     gfloat          left);
void            mx_texture_frame_set_parent_texture (MxTextureFrame *frame,
                                                     ClutterTexture *texture);
ClutterTexture *mx_texture_frame_get_parent_texture (MxTextureFrame *frame);
void            mx_texture_frame_set_border_values  (MxTextureFrame *frame,
                                                     gfloat          top,
                                                     gfloat          right,
                                                     gfloat          bottom,
                                                     gfloat          left);
void            mx_texture_frame_get_border_values  (MxTextureFrame *frame,
                                                     gfloat         *top,
                                                     gfloat         *right,
                                                     gfloat         *bottom,
                                                     gfloat         *left);
G_END_DECLS

#endif /* __MX_TEXTURE_FRAME_H__ */
