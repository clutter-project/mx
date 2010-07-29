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
#include "mx-private.h"
#include "mx-scrollable.h"
#include <math.h>

static void mx_scrollable_iface_init (MxScrollableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxFingerScroll, mx_finger_scroll, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                mx_scrollable_iface_init))

#define FINGER_SCROLL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  MX_TYPE_FINGER_SCROLL, \
                                  MxFingerScrollPrivate))

typedef struct {
  /* Units to store the origin of a click when scrolling */
  gfloat   x;
  gfloat   y;
  GTimeVal time;
} MxFingerScrollMotion;

struct _MxFingerScrollPrivate
{
  ClutterActor          *child;

  /* Scroll mode */
  MxFingerScrollMode     mode;
  gboolean               use_captured;
  guint32                button;

  /* Mouse motion event information */
  GArray                *motion_buffer;
  guint                  last_motion;

  /* Variables for storing acceleration information for kinetic mode */
  ClutterTimeline       *deceleration_timeline;
  gfloat                 dx;
  gfloat                 dy;
  gdouble                decel_rate;
  gdouble                accumulated_delta;
};

enum {
  PROP_0,

  PROP_MODE,
  PROP_DECEL_RATE,
  PROP_BUFFER,
  PROP_HADJUST,
  PROP_VADJUST,
  PROP_BUTTON,
  PROP_USE_CAPTURED
};

/* MxScrollableIface implementation */

static void
mx_finger_scroll_set_adjustments (MxScrollable *scrollable,
                                  MxAdjustment *hadjustment,
                                  MxAdjustment *vadjustment)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (scrollable)->priv;

  if (priv->child)
    mx_scrollable_set_adjustments (MX_SCROLLABLE (priv->child),
                                   hadjustment,
                                   vadjustment);
}

static void
mx_finger_scroll_get_adjustments (MxScrollable  *scrollable,
                                  MxAdjustment **hadjustment,
                                  MxAdjustment **vadjustment)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (scrollable)->priv;

  if (priv->child)
    {
      mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child),
                                     hadjustment,
                                     vadjustment);
    }
  else
    {
      if (hadjustment)
        *hadjustment = NULL;
      if (vadjustment)
        *vadjustment = NULL;
    }
}

static void
mx_scrollable_iface_init (MxScrollableIface *iface)
{
  iface->set_adjustments = mx_finger_scroll_set_adjustments;
  iface->get_adjustments = mx_finger_scroll_get_adjustments;
}

/* Object implementation */

static void
mx_finger_scroll_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  MxAdjustment *adjustment;
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

    case PROP_HADJUST:
      mx_finger_scroll_get_adjustments (MX_SCROLLABLE (object),
                                        &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST:
      mx_finger_scroll_get_adjustments (MX_SCROLLABLE (object),
                                        NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    case PROP_BUTTON:
      g_value_set_uint (value, priv->button);
      break;

    case PROP_USE_CAPTURED:
      g_value_set_boolean (value, priv->use_captured);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_finger_scroll_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  MxAdjustment *adjustment;
  MxScrollable *scrollable;
  MxFingerScroll *self = MX_FINGER_SCROLL (object);
  MxFingerScrollPrivate *priv = self->priv;

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

    case PROP_HADJUST:
      scrollable = MX_SCROLLABLE (object);
      mx_finger_scroll_get_adjustments (scrollable, NULL, &adjustment);
      mx_finger_scroll_set_adjustments (scrollable,
                                        g_value_get_object (value),
                                        adjustment);
      break;

    case PROP_VADJUST:
      scrollable = MX_SCROLLABLE (object);
      mx_finger_scroll_get_adjustments (scrollable, &adjustment, NULL);
      mx_finger_scroll_set_adjustments (scrollable,
                                        adjustment,
                                        g_value_get_object (value));
      break;

    case PROP_BUTTON:
      mx_finger_scroll_set_mouse_button (self, g_value_get_uint (value));
      break;

    case PROP_USE_CAPTURED:
      mx_finger_scroll_set_use_captured (self, g_value_get_boolean (value));
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
mx_finger_scroll_get_preferred_width (ClutterActor *actor,
                                      gfloat        for_height,
                                      gfloat       *min_width_p,
                                      gfloat       *nat_width_p)
{
  CLUTTER_ACTOR_CLASS (mx_finger_scroll_parent_class)->
    get_preferred_width (actor, for_height, NULL, nat_width_p);

  if (min_width_p)
    {
      MxPadding padding;

      mx_widget_get_padding (MX_WIDGET (actor), &padding);
      *min_width_p = padding.left + padding.right;
    }
}

static void
mx_finger_scroll_get_preferred_height (ClutterActor *actor,
                                       gfloat        for_width,
                                       gfloat       *min_height_p,
                                       gfloat       *nat_height_p)
{
  CLUTTER_ACTOR_CLASS (mx_finger_scroll_parent_class)->
    get_preferred_height (actor, for_width, NULL, nat_height_p);

  if (min_height_p)
    {
      MxPadding padding;

      mx_widget_get_padding (MX_WIDGET (actor), &padding);
      *min_height_p = padding.top + padding.bottom;
    }
}

static void
mx_finger_scroll_allocate (ClutterActor           *actor,
                           const ClutterActorBox  *box,
                           ClutterAllocationFlags  flags)
{
  CLUTTER_ACTOR_CLASS (mx_finger_scroll_parent_class)->
    allocate (actor, box, flags);

  mx_bin_allocate_child (MX_BIN (actor), box, flags);
}

static void
mx_finger_scroll_class_init (MxFingerScrollClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFingerScrollPrivate));

  object_class->get_property = mx_finger_scroll_get_property;
  object_class->set_property = mx_finger_scroll_set_property;
  object_class->dispose = mx_finger_scroll_dispose;
  object_class->finalize = mx_finger_scroll_finalize;

  actor_class->get_preferred_width = mx_finger_scroll_get_preferred_width;
  actor_class->get_preferred_height = mx_finger_scroll_get_preferred_height;
  actor_class->allocate = mx_finger_scroll_allocate;

  pspec = g_param_spec_enum ("mode",
                             "Mode",
                             "Scrolling mode",
                             MX_TYPE_FINGER_SCROLL_MODE,
                             MX_FINGER_SCROLL_MODE_PUSH,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_MODE, pspec);

  pspec = g_param_spec_double ("decel-rate",
                               "Deceleration rate",
                               "Rate at which the view will decelerate in "
                               "kinetic mode.",
                               1.1, G_MAXDOUBLE, 1.1,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_DECEL_RATE, pspec);

  pspec = g_param_spec_uint ("motion-buffer",
                             "Motion buffer",
                             "Amount of motion events to buffer",
                             1, G_MAXUINT, 3,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_BUFFER, pspec);

  pspec = g_param_spec_uint ("mouse-button",
                             "Mouse button",
                             "The mouse button used to control scrolling",
                             0, G_MAXUINT, 1,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_BUTTON, pspec);

  pspec = g_param_spec_boolean ("use-captured",
                                "Use captured",
                                "Use captured events to initiate scrolling",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_USE_CAPTURED, pspec);

  /* MxScrollable properties */
  g_object_class_override_property (object_class,
                                    PROP_HADJUST,
                                    "horizontal-adjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUST,
                                    "vertical-adjustment");
}

static gboolean
motion_event_cb (ClutterActor       *stage,
                 ClutterMotionEvent *event,
                 MxFingerScroll     *scroll)
{
  gfloat x, y;

  MxFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *actor = CLUTTER_ACTOR (scroll);

  if (event->type != CLUTTER_MOTION)
    return FALSE;

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
      else
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
button_release_event_cb (ClutterActor       *stage,
                         ClutterButtonEvent *event,
                         MxFingerScroll     *scroll)
{
  MxFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *actor = CLUTTER_ACTOR (scroll);
  ClutterActor *child = mx_bin_get_child (MX_BIN (scroll));
  gboolean decelerating = FALSE;

  if ((event->type != CLUTTER_BUTTON_RELEASE) ||
      (event->button != priv->button))
    return FALSE;

  g_signal_handlers_disconnect_by_func (stage,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (stage,
                                        button_release_event_cb,
                                        scroll);

  clutter_set_motion_events_enabled (TRUE);

  if ((priv->mode == MX_FINGER_SCROLL_MODE_KINETIC) && (child))
    {
      gfloat event_x, event_y;

      if (clutter_actor_transform_stage_point (actor, event->x, event->y,
                                               &event_x, &event_y))
        {
          gdouble value, lower, step_increment, d, ax, ay, y, nx, ny;
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
          y = priv->decel_rate;
          nx = logf (ABS (priv->dx)) / logf (y) + 15.0;
          ny = logf (ABS (priv->dy)) / logf (y) + 15.0;

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
          ax = (1.0 - 1.0 / pow (y, nx + 1)) / (1.0 - 1.0 / y);
          ay = (1.0 - 1.0 / pow (y, ny + 1)) / (1.0 - 1.0 / y);

          /* Solving for dx */
          mx_adjustment_get_values (hadjust, &value, &lower, NULL,
                                    &step_increment, NULL, NULL);

          /* Make sure we pick the next nearest step increment in the
           * same direction as the push.
           */
          priv->dx *= nx;
          if (ABS (priv->dx) < step_increment / 2)
            d = round ((value + priv->dx - lower) / step_increment);
          else if (priv->dx > 0)
            d = ceil ((value + priv->dx - lower) / step_increment);
          else
            d = floor ((value + priv->dx - lower) / step_increment);

          d = ((d * step_increment) + lower) - value;
          priv->dx = d / ax;

          /* Solving for dy */
          mx_adjustment_get_values (vadjust, &value, &lower, NULL,
                                    &step_increment, NULL, NULL);

          priv->dy *= ny;
          if (ABS (priv->dy) < step_increment / 2)
            d = round ((value + priv->dy - lower) / step_increment);
          else if (priv->dy > 0)
            d = ceil ((value + priv->dy - lower) / step_increment);
          else
            d = floor ((value + priv->dy - lower) / step_increment);

          d = ((d * step_increment) + lower) - value;
          priv->dy = d / ay;

          priv->deceleration_timeline =
            clutter_timeline_new ((gint)(MAX (nx, ny) * (1000/60.0)));

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
      clamp_adjustments (scroll);
    }

  return TRUE;
}

static gboolean
button_press_event_cb (ClutterActor     *actor,
                       ClutterEvent     *event,
                       MxFingerScroll *scroll)
{
  MxFingerScrollPrivate *priv = scroll->priv;
  ClutterButtonEvent *bevent = (ClutterButtonEvent *)event;
  ClutterActor *stage = clutter_actor_get_stage (actor);

  if ((event->type == CLUTTER_BUTTON_PRESS) &&
      (bevent->button == priv->button) &&
      stage)
    {
      MxFingerScrollMotion *motion;

      /* Reset motion buffer */
      priv->last_motion = 0;
      motion = &g_array_index (priv->motion_buffer, MxFingerScrollMotion, 0);

      if (clutter_actor_transform_stage_point (actor, bevent->x, bevent->y,
                                               &motion->x, &motion->y))
        {
          g_get_current_time (&motion->time);

          if (priv->deceleration_timeline)
            {
              clutter_timeline_stop (priv->deceleration_timeline);
              g_object_unref (priv->deceleration_timeline);
              priv->deceleration_timeline = NULL;
            }

          clutter_set_motion_events_enabled (FALSE);

          g_signal_connect (stage,
                            "captured-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
          g_signal_connect (stage,
                            "captured-event",
                            G_CALLBACK (button_release_event_cb),
                            scroll);
        }
    }

  return FALSE;
}

static void
mx_finger_scroll_actor_added_cb (ClutterContainer *container,
                                 ClutterActor     *actor)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (container)->priv;

  if (MX_IS_SCROLLABLE (actor))
    priv->child = actor;
  else
    g_warning ("Attempting to add an actor of type %s to "
               "a MxFingerScroll, but the actor does "
               "not implement MxScrollable.",
               g_type_name (G_OBJECT_TYPE (actor)));
}

static void
mx_finger_scroll_actor_removed_cb (ClutterContainer *container,
                                   ClutterActor     *actor)
{
  MxFingerScrollPrivate *priv = MX_FINGER_SCROLL (container)->priv;
  priv->child = NULL;
}

static void
mx_finger_scroll_init (MxFingerScroll *self)
{
  MxFingerScrollPrivate *priv = self->priv = FINGER_SCROLL_PRIVATE (self);

  priv->motion_buffer = g_array_sized_new (FALSE, TRUE,
                                           sizeof (MxFingerScrollMotion), 3);
  g_array_set_size (priv->motion_buffer, 3);
  priv->decel_rate = 1.1f;
  priv->button = 1;

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
  g_signal_connect (self, "button-press-event",
                    G_CALLBACK (button_press_event_cb), self);
  g_signal_connect (self, "actor-added",
                    G_CALLBACK (mx_finger_scroll_actor_added_cb), self);
  g_signal_connect (self, "actor-removed",
                    G_CALLBACK (mx_finger_scroll_actor_removed_cb), self);

  mx_bin_set_alignment (MX_BIN (self), MX_ALIGN_START, MX_ALIGN_START);
}

ClutterActor *
mx_finger_scroll_new (MxFingerScrollMode mode)
{
  return g_object_new (MX_TYPE_FINGER_SCROLL, "mode", mode, NULL);
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

void
mx_finger_scroll_set_mouse_button (MxFingerScroll *scroll,
                                   guint32         button)
{
  MxFingerScrollPrivate *priv;

  g_return_if_fail (MX_IS_FINGER_SCROLL (scroll));

  priv = scroll->priv;

  if (priv->button != button)
    {
      priv->button = button;
      g_object_notify (G_OBJECT (scroll), "mouse-button");
    }
}

guint32
mx_finger_scroll_get_mouse_button (MxFingerScroll *scroll)
{
  g_return_val_if_fail (MX_IS_FINGER_SCROLL (scroll), 0);
  return scroll->priv->button;
}

void
mx_finger_scroll_set_use_captured (MxFingerScroll *scroll,
                                   gboolean        use_captured)
{
  MxFingerScrollPrivate *priv;

  g_return_if_fail (MX_IS_FINGER_SCROLL (scroll));

  priv = scroll->priv;
  if (priv->use_captured != use_captured)
    {
      priv->use_captured = use_captured;

      g_signal_handlers_disconnect_by_func (scroll,
                                            button_press_event_cb,
                                            scroll);

      g_signal_connect (scroll,
                        use_captured ? "captured-event" : "button-press-event",
                        G_CALLBACK (button_press_event_cb),
                        scroll);

      g_object_notify (G_OBJECT (scroll), "use-captured");
    }
}

gboolean
mx_finger_scroll_get_use_captured (MxFingerScroll *scroll)
{
  g_return_val_if_fail (MX_IS_FINGER_SCROLL (scroll), FALSE);
  return scroll->priv->use_captured;
}
