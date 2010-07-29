/* mx-kinetic-scroll-view.h: Kinetic scrolling container actor
 *
 * Copyright (C) 2008 OpenedHand
 * Copyright (C) 2010 Intel Corporation.
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@linux.intel.com>
 */

#ifndef __MX_KINETIC_SCROLL_VIEW_H__
#define __MX_KINETIC_SCROLL_VIEW_H__

#include <glib-object.h>
#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_KINETIC_SCROLL_VIEW            (mx_kinetic_scroll_view_get_type())
#define MX_KINETIC_SCROLL_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_KINETIC_SCROLL_VIEW, MxKineticScrollView))
#define MX_IS_KINETIC_SCROLL_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_KINETIC_SCROLL_VIEW))
#define MX_KINETIC_SCROLL_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_KINETIC_SCROLL_VIEW, MxKineticScrollViewClass))
#define MX_IS_KINETIC_SCROLL_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_KINETIC_SCROLL_VIEW))
#define MX_KINETIC_SCROLL_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_KINETIC_SCROLL_VIEW, MxKineticScrollViewClass))

typedef struct _MxKineticScrollView          MxKineticScrollView;
typedef struct _MxKineticScrollViewPrivate   MxKineticScrollViewPrivate;
typedef struct _MxKineticScrollViewClass     MxKineticScrollViewClass;

struct _MxKineticScrollView
{
  /*< private >*/
  MxBin                  parent_instance;

  MxKineticScrollViewPrivate *priv;
};

struct _MxKineticScrollViewClass
{
  MxBinClass parent_class;
};

GType mx_kinetic_scroll_view_get_type (void) G_GNUC_CONST;

ClutterActor *mx_kinetic_scroll_view_new  (void);

void mx_kinetic_scroll_view_stop (MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_decel_rate (MxKineticScrollView *scroll,
                                            gdouble              rate);
gdouble mx_kinetic_scroll_view_get_decel_rate (MxKineticScrollView *scroll);

/*
void mx_kinetic_scroll_view_set_buffer_size (MxKineticScrollView *scroll,
                                             guint                size);
guint mx_kinetic_scroll_view_get_buffer_size (MxKineticScrollView *scroll);
*/

void mx_kinetic_scroll_view_set_use_captured (MxKineticScrollView *scroll,
                                              gboolean        use_captured);
gboolean mx_kinetic_scroll_view_get_use_captured (MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_mouse_button (MxKineticScrollView *scroll,
                                              guint32         button);
guint32 mx_kinetic_scroll_view_get_mouse_button (MxKineticScrollView *scroll);

G_END_DECLS

#endif /* __MX_KINETIC_SCROLL_VIEW_H__ */
