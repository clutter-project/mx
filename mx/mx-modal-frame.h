/*
 * mx-modal-frame.h
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

#ifndef _MX_MODAL_FRAME_H
#define _MX_MODAL_FRAME_H

#include <glib-object.h>
#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_MODAL_FRAME mx_modal_frame_get_type()

#define MX_MODAL_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_MODAL_FRAME, MxModalFrame))

#define MX_MODAL_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_MODAL_FRAME, MxModalFrameClass))

#define MX_IS_MODAL_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_MODAL_FRAME))

#define MX_IS_MODAL_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_MODAL_FRAME))

#define MX_MODAL_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_MODAL_FRAME, MxModalFrameClass))

typedef struct _MxModalFrame MxModalFrame;
typedef struct _MxModalFrameClass MxModalFrameClass;
typedef struct _MxModalFramePrivate MxModalFramePrivate;

struct _MxModalFrame
{
  MxBin parent;

  MxModalFramePrivate *priv;
};

struct _MxModalFrameClass
{
  MxBinClass parent_class;
};

GType mx_modal_frame_get_type (void) G_GNUC_CONST;

ClutterActor *mx_modal_frame_new (void);

void mx_modal_frame_set_transient_parent (MxModalFrame *modal_frame,
                                          ClutterActor *actor);

void mx_modal_frame_show (MxModalFrame *modal_frame);
void mx_modal_frame_hide (MxModalFrame *modal_frame);

G_END_DECLS

#endif /* _MX_MODAL_FRAME_H */
