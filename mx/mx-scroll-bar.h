/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scroll-bar.h: Scroll bar actor
 *
 * Copyright 2008 OpenedHand
 * Copyright 2009, 2010 Intel Corporation.
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

#ifndef __MX_SCROLL_BAR_H__
#define __MX_SCROLL_BAR_H__

#include <mx/mx-adjustment.h>
#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_SCROLL_BAR            (mx_scroll_bar_get_type())
#define MX_SCROLL_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SCROLL_BAR, MxScrollBar))
#define MX_IS_SCROLL_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SCROLL_BAR))
#define MX_SCROLL_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_SCROLL_BAR, MxScrollBarClass))
#define MX_IS_SCROLL_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_SCROLL_BAR))
#define MX_SCROLL_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_SCROLL_BAR, MxScrollBarClass))

typedef struct _MxScrollBar          MxScrollBar;
typedef struct _MxScrollBarPrivate   MxScrollBarPrivate;
typedef struct _MxScrollBarClass     MxScrollBarClass;

/**
 * MxScrollBar:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxScrollBar
{
  /*< private >*/
  MxBin parent_instance;

  MxScrollBarPrivate *priv;
};

struct _MxScrollBarClass
{
  MxBinClass parent_class;

  /* signals */
  void (*scroll_start) (MxScrollBar *bar);
  void (*scroll_stop)  (MxScrollBar *bar);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_scroll_bar_get_type (void) G_GNUC_CONST;

ClutterActor *mx_scroll_bar_new (void);
ClutterActor *mx_scroll_bar_new_with_adjustment (MxAdjustment *adjustment);

void          mx_scroll_bar_set_adjustment (MxScrollBar  *bar,
                                            MxAdjustment *adjustment);
MxAdjustment *mx_scroll_bar_get_adjustment (MxScrollBar  *bar);

void          mx_scroll_bar_set_orientation (MxScrollBar   *bar,
                                             MxOrientation  orientation);
MxOrientation mx_scroll_bar_get_orientation (MxScrollBar   *bar);

G_END_DECLS

#endif /* __MX_SCROLL_BAR_H__ */
