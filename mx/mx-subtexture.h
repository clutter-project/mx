/*
 * mx-subtexture.h: Class to wrap a texture and "subframe" it.
 *
 * Based on
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

#ifndef __MX_SUBTEXTURE_H__
#define __MX_SUBTEXTURE_H__

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_SUBTEXTURE                 (mx_subtexture_get_type ())
#define MX_SUBTEXTURE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SUBTEXTURE, MxSubtexture))
#define MX_SUBTEXTURE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_SUBTEXTURE, MxSubtextureClass))
#define MX_IS_SUBTEXTURE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SUBTEXTURE))
#define MX_IS_SUBTEXTURE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_SUBTEXTURE))
#define MX_SUBTEXTURE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_SUBTEXTURE, MxSubtextureClass))

typedef struct _MxSubtexture                MxSubtexture;
typedef struct _MxSubtexturePrivate         MxSubtexturePrivate;
typedef struct _MxSubtextureClass           MxSubtextureClass;

/**
 * MxSubtexture:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxSubtexture
{
  /*< private >*/
  ClutterActor parent_instance;
  
  MxSubtexturePrivate    *priv;
};

struct _MxSubtextureClass 
{
  ClutterActorClass parent_class;

  /* padding for future expansion */
  void (*_mx_box_1) (void);
  void (*_mx_box_2) (void);
  void (*_mx_box_3) (void);
  void (*_mx_box_4) (void);
}; 

GType mx_subtexture_get_type (void) G_GNUC_CONST;

ClutterActor *  mx_subtexture_new                (ClutterTexture *texture,
                                                  gint            top,
                                                  gint            left,
                                                  gint            width,
                                                  gint            height);
void            mx_subtexture_set_parent_texture (MxSubtexture   *frame,
                                                  ClutterTexture *texture);
ClutterTexture *mx_subtexture_get_parent_texture (MxSubtexture   *frame);
void            mx_subtexture_set_frame          (MxSubtexture   *frame,
                                                  gint            top,
                                                  gint            left,
                                                  gint            width,
                                                  gint            height);
void            mx_subtexture_get_frame          (MxSubtexture   *frame,
                                                  gint           *top,
                                                  gint           *left,
                                                  gint           *width,
                                                  gint           *height);

G_END_DECLS

#endif /* __MX_SUBTEXTURE_H__ */
