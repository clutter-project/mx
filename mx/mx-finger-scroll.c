/* mx-finger-scroll.c: Finger scrolling container actor
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
 */

#include "mx-finger-scroll.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"
#include "mx-scroll-bar.h"
#include "mx-scrollable.h"
#include "mx-scroll-view.h"
#include <clutter/clutter.h>
#include <math.h>

G_DEFINE_TYPE (MxFingerScroll, mx_finger_scroll, MX_TYPE_SCROLL_VIEW)

#define FINGER_SCROLL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  MX_TYPE_FINGER_SCROLL, \
                                  MxFingerScrollPrivate))

typedef struct {
  /* Units to store the origin of a click when scrolling */
  gfloat x;
  gfloat y;
  GTimeVal    time;
} MxFingerScrollMotion;

struct _MxFingerScrollPrivate
{
  /* Scroll mode */
  MxFingerScrollMode mode;

  GArray                *motion_buffer;
  guint                  last_motion;

  /* Variables for storing acceleration information for kinetic mode */
  ClutterTimeline       *deceleration_timeline;
  gfloat            dx;
  gfloat            dy;
  gdouble           decel_rate;
  gdouble           accumulated_delta;

  /* Variables to fade in/out scroll-bars */
  //ClutterEffectTemplate *template;
  ClutterTimeline       *hscroll_timeline;
  ClutterTimeline       *vscroll_timeline;
};

enum {
  PROP_MODE = 1,
  PROP_DECEL_RATE,
  PROP_BUFFER,
};

static void
mx_finger_scroll_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (object)->priv;

  switch (property_id)
    {
    case PROP_MODE :
      g_value_set_enum (value, priv->mode);
      break;
    case PROP_DECEL_RATE :
      g_value_set_double (value, priv->decel_rate);
      break;
    case PROP_BUFFER :
      g_value_set_uint (value, priv->motion_buffer->len);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_finger_scroll_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (object)->priv;

  switch (property_id)
    {
    case PROP_MODE :
      priv->mode = g_value_get_enum (value);
      g_object_notify (object, "mode");
      break;
    case PROP_DECEL_RATE :
      priv->decel_rate = g_value_get_double (value);
      g_object_notify (object, "decel-rate");
      break;
    case PROP_BUFFER :
      g_array_set_size (priv->motion_buffer, g_value_get_uint (value));
      g_object_notify (object, "motion-buffer");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_finger_scroll_dispose (GObject *object)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (object)->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }

  if (priv->hscroll_timeline)
    {
      clutter_timeline_stop (priv->hscroll_timeline);
      g_object_unref (priv->hscroll_timeline);
      priv->hscroll_timeline = NULL;
    }

  if (priv->vscroll_timeline)
    {
      clutter_timeline_stop (priv->vscroll_timeline);
      g_object_unref (priv->vscroll_timeline);
      priv->vscroll_timeline = NULL;
    }
  /*
  if (priv->template)
    {
      g_object_unref (priv->template);
      priv->template = NULL;
    }
  */
  G_OBJECT_CLASS (mx_finger_scroll_parent_class)->dispose (object);
}

static void
mx_finger_scroll_finalize (GObject *object)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (object)->priv;

  g_array_free (priv->motion_buffer, TRUE);

  G_OBJECT_CLASS (mx_finger_scroll_parent_class)->finalize (object);
}


static void
mx_finger_scroll_class_init (MxFingerScrollClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFingerScrollPrivate));

  object_class->get_property = mx_finger_scroll_get_property;
  object_class->set_property = mx_finger_scroll_set_property;
  object_class->dispose = mx_finger_scroll_dispose;
  object_class->finalize = mx_finger_scroll_finalize;

  g_object_class_install_property (object_class,
                                   PROP_MODE,
                                   g_param_spec_enum ("mode",
                                                      "MxFingerScrollMode",
                                                      "Scrolling mode",
                                                      MX_TYPE_FINGER_SCROLL_MODE,
                                                      MX_FINGER_SCROLL_MODE_PUSH,
                                                      G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_DECEL_RATE,
                                   g_param_spec_double ("decel-rate",
                                                        "Deceleration rate",
                                                        "Rate at which the view "
                                                        "will decelerate in "
                                                        "kinetic mode.",
                                                        1.1,
                                                        G_MAXDOUBLE,
                                                        1.1,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_BUFFER,
                                   g_param_spec_uint ("motion-buffer",
                                                      "Motion buffer",
                                                      "Amount of motion "
                                                      "events to buffer",
                                                      1, G_MAXUINT, 3,
                                                      G_PARAM_READWRITE));
}

static gboolean
motion_event_cb (ClutterActor *actor,
                 ClutterMotionEvent *event,
                 MxFingerScroll *scroll)
{
  gfloat x, y;

  MxFingerScrollPrivate *priv = scroll->priv;

  if (clutter_actor_transform_stage_point (actor,
                                           event->x,
                                           event->y,
                                           &x, &y))
    {
      MxFingerScrollMotion *motion;
      ClutterActor *child =
        mx_bin_get_child (MX_BIN (scroll));

      if (child)
        {
          gdouble dx, dy;
          MxAdjustment *hadjust, *vadjust;

          mx_scrollable_get_adjustments (MX_SCROLLABLE (child),
                                           &hadjust,
                                           &vadjust);

          motion = &g_array_index (priv->motion_buffer,
                                   MxFingerScrollMotion, priv->last_motion);
          dx = (motion->x - x) + mx_adjustment_get_value (hadjust);
          dy = (motion->y - y) + mx_adjustment_get_value (vadjust);

          mx_adjustment_set_value (hadjust, dx);
          mx_adjustment_set_value (vadjust, dy);
        }

      priv->last_motion ++;
      if (priv->last_motion == priv->motion_buffer->len)
        {
          priv->motion_buffer = g_array_remove_index (priv->motion_buffer, 0);
          g_array_set_size (priv->motion_buffer, priv->last_motion);
          priv->last_motion --;
        }

      motion = &g_array_index (priv->motion_buffer,
                               MxFingerScrollMotion, priv->last_motion);
      motion->x = x;
      motion->y = y;
      g_get_current_time (&motion->time);
    }

  return TRUE;
}

static void
hfade_complete_cb (ClutterActor *scrollbar, MxFingerScroll *scroll)
{
  scroll->priv->hscroll_timeline = NULL;
}

static void
vfade_complete_cb (ClutterActor *scrollbar, MxFingerScroll *scroll)
{
  scroll->priv->vscroll_timeline = NULL;
}

static void
show_scrollbars (MxFingerScroll *scroll, gboolean show)
{
  //ClutterActor *hscroll, *vscroll;
  MxFingerScrollPrivate *priv = scroll->priv;

  /* Stop current timelines */
  if (priv->hscroll_timeline)
    {
      clutter_timeline_stop (priv->hscroll_timeline);
      g_object_unref (priv->hscroll_timeline);
    }

  if (priv->vscroll_timeline)
    {
      clutter_timeline_stop (priv->vscroll_timeline);
      g_object_unref (priv->vscroll_timeline);
    }

  //hscroll = mx_scroll_view_get_hscroll_bar (MX_SCROLL_VIEW (scroll));
  //vscroll = mx_scroll_view_get_vscroll_bar (MX_SCROLL_VIEW (scroll));

  /* Create new ones */
  /*
  if (!CLUTTER_ACTOR_IS_REACTIVE (hscroll))
    priv->hscroll_timeline = clutter_effect_fade (
                               priv->template,
                               hscroll,
                               show ? 0xFF : 0x00,
                               (ClutterEffectCompleteFunc)hfade_complete_cb,
                               scroll);

  if (!CLUTTER_ACTOR_IS_REACTIVE (vscroll))
    priv->vscroll_timeline = clutter_effect_fade (
                               priv->template,
                               vscroll,
                               show ? 0xFF : 0x00,
                               (ClutterEffectCompleteFunc)vfade_complete_cb,
                               scroll);
  */
}

static void
clamp_adjustments (MxFingerScroll *scroll)
{
  ClutterActor *child = mx_bin_get_child (MX_BIN (scroll));

  if (child)
    {
      gdouble d, value, lower, step_increment;
      MxAdjustment *hadj, *vadj;

      mx_scrollable_get_adjustments (MX_SCROLLABLE (child),
                                     &hadj, &vadj);

      /* Snap to the nearest step increment on hadjustment */
      mx_adjustment_get_values (hadj, &value, &lower, NULL,
                                &step_increment, NULL, NULL);
      d = (rint ((value - lower) / step_increment) *
          step_increment) + lower;
      mx_adjustment_set_value (hadj, d);

      /* Snap to the nearest step increment on vadjustment */
      mx_adjustment_get_values (vadj, &value, &lower, NULL,
                                &step_increment, NULL, NULL);
      d = (rint ((value - lower) / step_increment) *
          step_increment) + lower;
      mx_adjustment_set_value (vadj, d);
    }
}

static void
deceleration_completed_cb (ClutterTimeline *timeline,
                           MxFingerScroll *scroll)
{
  show_scrollbars (scroll, FALSE);
  clamp_adjustments (scroll);
  g_object_unref (timeline);
  scroll->priv->deceleration_timeline = NULL;
}

static void
deceleration_new_frame_cb (ClutterTimeline *timeline,
                           gint frame_num,
                           MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *child = mx_bin_get_child (MX_BIN (scroll));

  if (child)
    {
      gdouble value, lower, upper, page_size;
      MxAdjustment *hadjust, *vadjust;

      gboolean stop = TRUE;

      mx_scrollable_get_adjustments (MX_SCROLLABLE (child),
                                       &hadjust,
                                       &vadjust);

      priv->accumulated_delta += clutter_timeline_get_delta (timeline);

      while (priv->accumulated_delta > 1000.0/60.0)
        {
          mx_adjustment_set_value (hadjust,
                                   priv->dx +
                                   mx_adjustment_get_value (hadjust));
          mx_adjustment_set_value (vadjust,
                                   priv->dy +
                                   mx_adjustment_get_value (vadjust));
          priv->dx = priv->dx / priv->decel_rate;
          priv->dy = priv->dy / priv->decel_rate;

          priv->accumulated_delta -= 1000.0/60.0;
        }

      /* Check if we've hit the upper or lower bounds and stop the timeline */
      mx_adjustment_get_values (hadjust, &value, &lower, &upper,
                                NULL, NULL, &page_size);
      if (((priv->dx > 0) && (value < upper - page_size)) ||
          ((priv->dx < 0) && (value > lower)))
        stop = FALSE;

      if (stop)
        {
          mx_adjustment_get_values (vadjust, &value, &lower, &upper,
                                    NULL, NULL, &page_size);
          if (((priv->dy > 0) && (value < upper - page_size)) ||
              ((priv->dy < 0) && (value > lower)))
            stop = FALSE;
        }

      if (stop)
        {
          clutter_timeline_stop (timeline);
          deceleration_completed_cb (timeline, scroll);
        }
    }
}

static gboolean
button_release_event_cb (ClutterActor *actor,
                         ClutterButtonEvent *event,
                         MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *child = mx_bin_get_child (MX_BIN (scroll));
  gboolean decelerating = FALSE;

  if (event->button != 1)
    return FALSE;

  g_signal_handlers_disconnect_by_func (actor,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (actor,
                                        button_release_event_cb,
                                        scroll);

  clutter_ungrab_pointer ();

  if ((priv->mode == MX_FINGER_SCROLL_MODE_KINETIC) && (child))
    {
      gfloat event_x, event_y;

      if (clutter_actor_transform_stage_point (actor, event->x, event->y,
                                               &event_x, &event_y))
        {
          gdouble value, lower, step_increment, d, a, x, y, n;
          gfloat frac, x_origin, y_origin;
          GTimeVal release_time, motion_time;
          MxAdjustment *hadjust, *vadjust;
          glong time_diff;
          gint i;

          /* Get time delta */
          g_get_current_time (&release_time);

          /* Get average position/time of last x mouse events */
          priv->last_motion ++;
          x_origin = y_origin = 0;
          motion_time = (GTimeVal){ 0, 0 };
          for (i = 0; i < priv->last_motion; i++)
            {
              MxFingerScrollMotion *motion =
                &g_array_index (priv->motion_buffer, MxFingerScrollMotion, i);

              /* FIXME: This doesn't guard against overflows - Should
               *        either fix that, or calculate the correct maximum
               *        value for the buffer size
               */
              x_origin += motion->x;
              y_origin += motion->y;
              motion_time.tv_sec += motion->time.tv_sec;
              motion_time.tv_usec += motion->time.tv_usec;
            }
          x_origin = x_origin / priv->last_motion;
          y_origin = y_origin / priv->last_motion;
          motion_time.tv_sec /= priv->last_motion;
          motion_time.tv_usec /= priv->last_motion;

          if (motion_time.tv_sec == release_time.tv_sec)
            time_diff = release_time.tv_usec - motion_time.tv_usec;
          else
            time_diff = release_time.tv_usec +
                        (G_USEC_PER_SEC - motion_time.tv_usec);

          /* Work out the fraction of 1/60th of a second that has elapsed */
          frac = (time_diff/1000.0) / (1000.0/60.0);

          /* See how many units to move in 1/60th of a second */
          priv->dx = (x_origin - event_x) / frac;
          priv->dy = (y_origin - event_y) / frac;

          /* If the delta is too low for the equations to work,
           * bump the values up a bit.
           */
          if (ABS (priv->dx) < 2)
            priv->dx = (priv->dx > 0) ? 2 : -2;
          if (ABS (priv->dy) < 2)
            priv->dy = (priv->dy > 0) ? 2 : -2;

          /* Get adjustments to do step-increment snapping */
          mx_scrollable_get_adjustments (MX_SCROLLABLE (child),
                                         &hadjust,
                                         &vadjust);

          /* We want n, where x / y^n < z,
           * x = Distance to move per frame
           * y = Deceleration rate
           * z = maximum distance from target
           *
           * Rearrange to n = log (x / z) / log (y)
           * To simplify, z = 1, so n = log (x) / log (y)
           *
           * As z = 1, this will cause stops to be slightly abrupt -
           * add a constant 15 frames to compensate.
           */
          x = MAX (ABS (priv->dx), ABS (priv->dy));
          y = priv->decel_rate;
          n = logf (x) / logf (y) + 15.0;

          /* Now we have n, adjust dx/dy so that we finish on a step
           * boundary.
           *
           * Distance moved, using the above variable names:
           *
           * d = x + x/y + x/y^2 + ... + x/y^n
           *
           * Using geometric series,
           *
           * d = (1 - 1/y^(n+1))/(1 - 1/y)*x
           *
           * Let a = (1 - 1/y^(n+1))/(1 - 1/y),
           *
           * d = a * x
           *
           * Find d and find its nearest page boundary, then solve for x
           *
           * x = d / a
           */

          /* Get adjustments, work out y^n */
          a = (1.0 - 1.0 / pow (y, n + 1)) / (1.0 - 1.0 / y);

          /* Solving for dx */
          mx_adjustment_get_values (hadjust, &value, &lower, NULL,
                                    &step_increment, NULL, NULL);

          /* Make sure we pick the next nearest step increment in the
           * same direction as the push.
           */
          if (priv->dx > 0)
            d = ceil ((value - lower) / step_increment);
          else
            d = floor ((value - lower) / step_increment);

          d = ((d * step_increment) + lower) - value;
          priv->dx = d / a;

          /* Solving for dy */
          mx_adjustment_get_values (vadjust, &value, &lower, NULL,
                                    &step_increment, NULL, NULL);

          if (priv->dy > 0)
            d = ceil ((value - lower) / step_increment);
          else
            d = floor ((value - lower) / step_increment);

          d = ((d * step_increment) + lower) - value;
          priv->dy = d / a;

          priv->deceleration_timeline = clutter_timeline_new ((gint)n * 60);

          g_signal_connect (priv->deceleration_timeline, "new_frame",
                            G_CALLBACK (deceleration_new_frame_cb), scroll);
          g_signal_connect (priv->deceleration_timeline, "completed",
                            G_CALLBACK (deceleration_completed_cb), scroll);
          priv->accumulated_delta = 0;
          clutter_timeline_start (priv->deceleration_timeline);
          decelerating = TRUE;
        }
    }

  /* Reset motion event buffer */
  priv->last_motion = 0;

  if (!decelerating)
    {
      show_scrollbars (scroll, FALSE);
      clamp_adjustments (scroll);
    }

  /* Pass through events to children.
   * FIXME: this probably breaks click-count.
   */
  clutter_event_put ((ClutterEvent *)event);

  return TRUE;
}

static gboolean
after_event_cb (MxFingerScroll *scroll)
{
  /* Check the pointer grab - if something else has grabbed it - for example,
   * a scroll-bar or some such, don't do our funky stuff.
   */
  if (clutter_get_pointer_grab () != CLUTTER_ACTOR (scroll))
    {
      g_signal_handlers_disconnect_by_func (scroll,
                                            motion_event_cb,
                                            scroll);
      g_signal_handlers_disconnect_by_func (scroll,
                                            button_release_event_cb,
                                            scroll);
    }

  return FALSE;
}

static gboolean
captured_event_cb (ClutterActor     *actor,
                   ClutterEvent     *event,
                   MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv = scroll->priv;

  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      MxFingerScrollMotion *motion;
      ClutterButtonEvent *bevent = (ClutterButtonEvent *)event;

      /* Reset motion buffer */
      priv->last_motion = 0;
      motion = &g_array_index (priv->motion_buffer, MxFingerScrollMotion, 0);

      if ((bevent->button == 1) &&
          (clutter_actor_transform_stage_point (actor, bevent->x, bevent->y,
                                                &motion->x, &motion->y)))
        {
          g_get_current_time (&motion->time);

          if (priv->deceleration_timeline)
            {
              clutter_timeline_stop (priv->deceleration_timeline);
              g_object_unref (priv->deceleration_timeline);
              priv->deceleration_timeline = NULL;
            }

          /* Fade in scroll-bars */
          show_scrollbars (scroll, TRUE);

          clutter_grab_pointer (actor);

          /* Add a high priority idle to check the grab after the event
           * emission is finished.
           */
          g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                           (GSourceFunc)after_event_cb,
                           scroll,
                           NULL);

          g_signal_connect (actor,
                            "motion-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
          g_signal_connect (actor,
                            "button-release-event",
                            G_CALLBACK (button_release_event_cb),
                            scroll);
        }
    }

  return FALSE;
}

static void
hscroll_notify_reactive_cb (ClutterActor     *bar,
                            GParamSpec       *pspec,
                            MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv;

  priv = scroll->priv;
  if (CLUTTER_ACTOR_IS_REACTIVE (bar))
    {
      if (priv->hscroll_timeline)
        {
          clutter_timeline_stop (priv->hscroll_timeline);
          g_object_unref (priv->hscroll_timeline);
          priv->hscroll_timeline = NULL;
        }
      clutter_actor_set_opacity (bar, 0xFF);
    }
}

static void
vscroll_notify_reactive_cb (ClutterActor     *bar,
                            GParamSpec       *pspec,
                            MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv;

  priv = scroll->priv;
  if (CLUTTER_ACTOR_IS_REACTIVE (bar))
    {
      if (priv->vscroll_timeline)
        {
          clutter_timeline_stop (priv->vscroll_timeline);
          g_object_unref (priv->vscroll_timeline);
          priv->vscroll_timeline = NULL;
        }
      clutter_actor_set_opacity (bar, 0xFF);
    }
}

static void
mx_finger_scroll_init (MxFingerScroll *self)
{
  //ClutterActor *scrollbar;
  MxFingerScrollPrivate *priv = self->priv = FINGER_SCROLL_PRIVATE (self);

  priv->motion_buffer = g_array_sized_new (FALSE, TRUE,
                                           sizeof (MxFingerScrollMotion), 3);
  g_array_set_size (priv->motion_buffer, 3);
  priv->decel_rate = 1.1f;

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
  g_signal_connect (CLUTTER_ACTOR (self),
                    "captured-event",
                    G_CALLBACK (captured_event_cb),
                    self);

  /* Make the scroll-bars unreactive and set their opacity - we'll fade them 
   * in/out when we scroll.
   * Also, hook onto notify::reactive and don't fade in/out when the bars are 
   * set reactive (which you might want to do if you want finger-scrolling 
   * *and* a scroll bar.
   */
  /*
  scrollbar = mx_scroll_view_get_hscroll_bar (MX_SCROLL_VIEW (self));
  clutter_actor_set_reactive (scrollbar, FALSE);
  clutter_actor_set_opacity (scrollbar, 0x00);
  g_signal_connect (scrollbar, "notify::reactive",
                    G_CALLBACK (hscroll_notify_reactive_cb), self);

  scrollbar = mx_scroll_view_get_vscroll_bar (MX_SCROLL_VIEW (self));
  clutter_actor_set_reactive (scrollbar, FALSE);
  clutter_actor_set_opacity (scrollbar, 0x00);
  g_signal_connect (scrollbar, "notify::reactive",
                    G_CALLBACK (vscroll_notify_reactive_cb), self);
  */
  //priv->template = clutter_effect_template_new_for_duration (250,
  //                                              CLUTTER_ALPHA_RAMP_INC);
}

ClutterActor *
mx_finger_scroll_new (MxFingerScrollMode mode)
{
  return CLUTTER_ACTOR (g_object_new (MX_TYPE_FINGER_SCROLL,
                                      "mode", mode, NULL));
}

void
mx_finger_scroll_stop (MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv;

  g_return_if_fail (MX_IS_FINGER_SCROLL (scroll));

  priv = scroll->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }
}
