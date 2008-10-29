/* nbtk-texture-frame.h: Expandible texture actor
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _HAVE_NBTK_TEXTURE_FRAME_H
#define _HAVE_NBTK_TEXTURE_FRAME_H

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_TEXTURE_FRAME (nbtk_texture_frame_get_type ())

#define NBTK_TEXTURE_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_TEXTURE_FRAME, NbtkTextureFrame))

#define NBTK_TEXTURE_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_TEXTURE_FRAME, NbtkTextureFrameClass))

#define NBTK_IS_TEXTURE_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_TEXTURE_FRAME))

#define NBTK_IS_TEXTURE_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_TEXTURE_FRAME))

#define NBTK_TEXTURE_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_TEXTURE_FRAME, NbtkTextureFrameClass))

typedef struct _NbtkTextureFrame        NbtkTextureFrame;
typedef struct _NbtkTextureFramePrivate NbtkTextureFramePrivate;
typedef struct _NbtkTextureFrameClass   NbtkTextureFrameClass;

struct _NbtkTextureFrame
{
  ClutterCloneTexture              parent;
  
  /*< priv >*/
  NbtkTextureFramePrivate    *priv;
};

struct _NbtkTextureFrameClass 
{
  ClutterCloneTextureClass parent_class;

  /* padding for future expansion */
  void (*_clutter_box_1) (void);
  void (*_clutter_box_2) (void);
  void (*_clutter_box_3) (void);
  void (*_clutter_box_4) (void);
}; 

GType         nbtk_texture_frame_get_type (void) G_GNUC_CONST;
ClutterActor *nbtk_texture_frame_new      (ClutterTexture *texture,
                                           gint            left,
                                           gint            top,
                                           gint            right,
                                           gint            bottom);

G_END_DECLS

#endif /* _HAVE_NBTK_TEXTURE_FRAME_H */
