/* nbtk-scroll-view.h: Container with scroll-bars
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

#ifndef __NBTK_SCROLL_VIEW_H__
#define __NBTK_SCROLL_VIEW_H__

#include <nbtk/nbtk-bin.h>

G_BEGIN_DECLS

#define NBTK_TYPE_SCROLL_VIEW            (nbtk_scroll_view_get_type())
#define NBTK_SCROLL_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_SCROLL_VIEW, NbtkScrollView))
#define NBTK_IS_SCROLL_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_SCROLL_VIEW))
#define NBTK_SCROLL_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_SCROLL_VIEW, NbtkScrollViewClass))
#define NBTK_IS_SCROLL_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_SCROLL_VIEW))
#define NBTK_SCROLL_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_SCROLL_VIEW, NbtkScrollViewClass))

typedef struct _NbtkScrollView          NbtkScrollView;
typedef struct _NbtkScrollViewPrivate   NbtkScrollViewPrivate;
typedef struct _NbtkScrollViewClass     NbtkScrollViewClass;

struct _NbtkScrollView
{
  /*< private >*/
  NbtkBin parent_instance;

  NbtkScrollViewPrivate *priv;
};

struct _NbtkScrollViewClass
{
  NbtkBinClass parent_class;
};

GType nbtk_scroll_view_get_type (void) G_GNUC_CONST;

NbtkWidget *  nbtk_scroll_view_new             (void);

ClutterActor *nbtk_scroll_view_get_hscroll_bar (NbtkScrollView *scroll);
ClutterActor *nbtk_scroll_view_get_vscroll_bar (NbtkScrollView *scroll);
ClutterActor *nbtk_scroll_view_get_child       (NbtkScrollView *scroll);

G_END_DECLS

#endif /* __NBTK_SCROLL_VIEW_H__ */
