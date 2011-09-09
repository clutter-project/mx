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

/**
 * MxKineticScrollView:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct _MxKineticScrollView          MxKineticScrollView;
typedef struct _MxKineticScrollViewPrivate   MxKineticScrollViewPrivate;
typedef struct _MxKineticScrollViewClass     MxKineticScrollViewClass;

typedef enum {
  MX_KINETIC_SCROLL_VIEW_STATE_IDLE,
  MX_KINETIC_SCROLL_VIEW_STATE_PANNING,
  MX_KINETIC_SCROLL_VIEW_STATE_SCROLLING,
  MX_KINETIC_SCROLL_VIEW_STATE_CLAMPING,
} MxKineticScrollViewState;

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

void mx_kinetic_scroll_view_set_deceleration (MxKineticScrollView *scroll,
                                              gdouble              rate);
gdouble mx_kinetic_scroll_view_get_deceleration (MxKineticScrollView *scroll);

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

void mx_kinetic_scroll_view_set_overshoot (MxKineticScrollView *scroll,
                                           gdouble              overshoot);
gdouble mx_kinetic_scroll_view_get_overshoot (MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_scroll_policy (MxKineticScrollView *scroll,
                                               MxScrollPolicy       policy);
MxScrollPolicy mx_kinetic_scroll_view_get_scroll_policy (
                                        MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_acceleration_factor (MxKineticScrollView *scroll,
    gdouble acceleration_factor);
gdouble mx_kinetic_scroll_view_get_acceleration_factor (
    MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_clamp_duration (MxKineticScrollView *scroll,
    guint clamp_duration);
guint mx_kinetic_scroll_view_get_clamp_duration (
    MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_clamp_mode (MxKineticScrollView *scroll,
    gulong clamp_mode);
gulong mx_kinetic_scroll_view_get_clamp_mode (
    MxKineticScrollView *scroll);

void mx_kinetic_scroll_view_set_clamp_to_center (MxKineticScrollView *scroll,
    gboolean clamp_to_center);
gboolean mx_kinetic_scroll_view_get_clamp_to_center (
    MxKineticScrollView *scroll);

G_END_DECLS

#endif /* __MX_KINETIC_SCROLL_VIEW_H__ */
