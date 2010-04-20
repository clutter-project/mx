/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scroll-view.h: Container with scroll-bars
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

#ifndef __MX_SCROLL_VIEW_H__
#define __MX_SCROLL_VIEW_H__

#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_SCROLL_VIEW            (mx_scroll_view_get_type())
#define MX_SCROLL_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SCROLL_VIEW, MxScrollView))
#define MX_IS_SCROLL_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SCROLL_VIEW))
#define MX_SCROLL_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_SCROLL_VIEW, MxScrollViewClass))
#define MX_IS_SCROLL_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_SCROLL_VIEW))
#define MX_SCROLL_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_SCROLL_VIEW, MxScrollViewClass))

typedef struct _MxScrollView          MxScrollView;
typedef struct _MxScrollViewPrivate   MxScrollViewPrivate;
typedef struct _MxScrollViewClass     MxScrollViewClass;

/**
 * MxScrollView:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxScrollView
{
  /*< private >*/
  MxBin parent_instance;

  MxScrollViewPrivate *priv;
};

struct _MxScrollViewClass
{
  MxBinClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_scroll_view_get_type (void) G_GNUC_CONST;

ClutterActor *mx_scroll_view_new (void);


void           mx_scroll_view_set_enable_mouse_scrolling (MxScrollView  *scroll,
                                                          gboolean       enabled);
gboolean       mx_scroll_view_get_enable_mouse_scrolling (MxScrollView  *scroll);

void           mx_scroll_view_set_enable_gestures (MxScrollView  *scroll,
                                                   gboolean       enabled);
gboolean       mx_scroll_view_get_enable_gestures (MxScrollView  *scroll);
void           mx_scroll_view_set_scroll_policy   (MxScrollView  *scroll,
                                                   MxScrollPolicy policy);
MxScrollPolicy mx_scroll_view_get_scroll_policy   (MxScrollView  *scroll);
void           mx_scroll_view_ensure_visible (MxScrollView          *scroll,
                                              const ClutterGeometry *geometry);

G_END_DECLS

#endif /* __MX_SCROLL_VIEW_H__ */
