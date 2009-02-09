/* nbtk-scroll-bar.h: Scroll bar actor
 *
 * Copyright (C) 2008 OpenedHand
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
 *
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Nbtk by: Robert Staudinger <robsta@openedhand.com>
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_SCROLL_BAR_H__
#define __NBTK_SCROLL_BAR_H__

#include <nbtk/nbtk-adjustment.h>
#include <nbtk/nbtk-bin.h>

G_BEGIN_DECLS

#define NBTK_TYPE_SCROLL_BAR            (nbtk_scroll_bar_get_type())
#define NBTK_SCROLL_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_SCROLL_BAR, NbtkScrollBar))
#define NBTK_IS_SCROLL_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_SCROLL_BAR))
#define NBTK_SCROLL_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_SCROLL_BAR, NbtkScrollBarClass))
#define NBTK_IS_SCROLL_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_SCROLL_BAR))
#define NBTK_SCROLL_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_SCROLL_BAR, NbtkScrollBarClass))

typedef struct _NbtkScrollBar          NbtkScrollBar;
typedef struct _NbtkScrollBarPrivate   NbtkScrollBarPrivate;
typedef struct _NbtkScrollBarClass     NbtkScrollBarClass;

struct _NbtkScrollBar
{
  /*< private >*/
  NbtkBin parent_instance;

  NbtkScrollBarPrivate *priv;
};

struct _NbtkScrollBarClass
{
  NbtkBinClass parent_class;
};

GType nbtk_scroll_bar_get_type (void) G_GNUC_CONST;

ClutterActor *  nbtk_scroll_bar_new             (NbtkAdjustment *adjustment);
ClutterActor *  nbtk_scroll_bar_new_with_handle (NbtkAdjustment *adjustment,
                                                 ClutterActor   *handle);
void            nbtk_scroll_bar_set_adjustment  (NbtkScrollBar  *bar,
                                                 NbtkAdjustment *adjustment);
NbtkAdjustment *nbtk_scroll_bar_get_adjustment  (NbtkScrollBar  *bar);
void            nbtk_scroll_bar_set_handle      (NbtkScrollBar  *bar,
                                                 ClutterActor   *handle);
ClutterActor *  nbtk_scroll_bar_get_handle      (NbtkScrollBar  *bar);
void            nbtk_scroll_bar_set_texture     (NbtkScrollBar  *bar,
                                                 ClutterActor   *texture);
ClutterActor *  nbtk_scroll_bar_get_texture     (NbtkScrollBar  *bar);

G_END_DECLS

#endif /* __NBTK_SCROLL_BAR_H__ */
