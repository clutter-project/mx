/*
 * mx-frame.c: frame actor
 *
 * Copyright 2009 Intel Corporation
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
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_FRAME_H
#define _MX_FRAME_H

#include "mx-bin.h"

G_BEGIN_DECLS

#define MX_TYPE_FRAME mx_frame_get_type()

#define MX_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_FRAME, MxFrame))

#define MX_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_FRAME, MxFrameClass))

#define MX_IS_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_FRAME))

#define MX_IS_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_FRAME))

#define MX_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_FRAME, MxFrameClass))

typedef struct _MxFrame MxFrame;
typedef struct _MxFrameClass MxFrameClass;
typedef struct _MxFramePrivate MxFramePrivate;

/**
 * MxFrame:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxFrame
{
  MxBin parent;

  MxFramePrivate *priv;
};

struct _MxFrameClass
{
  MxBinClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_frame_get_type (void) G_GNUC_CONST;

ClutterActor *mx_frame_new (void);

G_END_DECLS

#endif /* _MX_FRAME_H */
