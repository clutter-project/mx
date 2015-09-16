/* mx-kinetic-scroll-view.c: Kinetic scrolling container actor
 *
 * Copyright (C) 2008 OpenedHand
 * Copyright (C) 2010, 2012 Intel Corporation.
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

/**
 * SECTION:mx-kinetic-scroll-view
 * @short_description: A kinetic scrolling container widget
 *
 * #MxKineticScrollView is a single child container for actors that implements
 * #MxScrollable. It allows the contained child to be dragged to scroll, and
 * maintains the momentum once the drag is complete. Deceleration after
 * dragging is configurable, and it will always snap to the
 * #MxAdjustment:step-increment boundary.
 *
 * #MxKineticScrollView also implements #MxScrollable itself, allowing it to
 * be embedded in an #MxScrollView to provide scroll-bars.
 *
 * Since: 1.2
 */

#include "mx-kinetic-scroll-view.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-scrollable.h"
#include "mx-focusable.h"
#include <math.h>

#define _KINETIC_DEBUG 0

static void mx_scrollable_iface_init (MxScrollableIface *iface);
static void mx_kinetic_scroll_view_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxKineticScrollView,
                         mx_kinetic_scroll_view, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                mx_scrollable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_kinetic_scroll_view_focusable_iface_init))

#define KINETIC_SCROLL_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                        MX_TYPE_KINETIC_SCROLL_VIEW, \
                                        MxKineticScrollViewPrivate))

typedef struct {
  /* Units to store the origin of a click when scrolling */
  gfloat   x;
  gfloat   y;
  GTimeVal time;
} MxKineticScrollViewMotion;

typedef enum {
  MX_AUTOMATIC_SCROLL_NONE,
  MX_AUTOMATIC_SCROLL_HORIZONTAL,
  MX_AUTOMATIC_SCROLL_VERTICAL
} MxAutomaticScroll;


struct _MxKineticScrollViewPrivate
{
  ClutterActor          *child;

  guint                  use_captured        : 1;
  guint                  use_grab            : 1;
  guint                  in_drag             : 1;
  guint                  hmoving             : 1;
  guint                  vmoving             : 1;
  guint                  align_tested        : 1;
  guint                  hclamping           : 1;
  guint                  vclamping           : 1;
  guint                  clamp_to_center     : 1;
  guint                  snap_on_page        : 1;

  guint32                button;
  ClutterInputDevice    *device;
  ClutterEventSequence  *sequence;
  ClutterActor          *source_press_actor;
  ClutterEvent          *cancel_event;

  MxAutomaticScroll        in_automatic_scroll;

  /* Mouse motion event information */
  GArray                *motion_buffer;
  guint                  last_motion;

  /* Variables for storing acceleration information */
  ClutterTimeline       *deceleration_timeline;
  gfloat                 dx;
  gfloat                 dy;
  gdouble                decel_rate;
  gdouble                overshoot;
  gdouble                accumulated_delta;
  gdouble                acceleration_factor;

  MxScrollPolicy         scroll_policy;

  guint                  clamp_duration;
  gulong                 clamp_mode;

  MxKineticScrollViewState state;
};

enum {
  PROP_0,

  PROP_DECELERATION,
/*  PROP_BUFFER_SIZE,*/
  PROP_HADJUST,
  PROP_VADJUST,
  PROP_BUTTON,
  PROP_USE_CAPTURED,
  PROP_USE_GRAB,
  PROP_OVERSHOOT,
  PROP_SCROLL_POLICY,
  PROP_ACCELERATION_FACTOR,
  PROP_CLAMP_DURATION,
  PROP_CLAMP_MODE,
  PROP_STATE,
  PROP_CLAMP_TO_CENTER,
  PROP_SNAP_ON_PAGE,
};

#if _KINETIC_DEBUG
# define LOG_DEBUG(args...) _log_debug(args)

static void
_log_debug (MxKineticScrollView *scroll, const gchar *fmt, ...)
{
  gchar *new_fmt;
  va_list args;

  va_start (args, fmt);

  new_fmt = g_strdup_printf ("%s(%p): %s",
                             (scroll->priv->scroll_policy == MX_SCROLL_POLICY_VERTICAL) ?
                             "vert" :
                             ((scroll->priv->scroll_policy == MX_SCROLL_POLICY_HORIZONTAL) ? "hori" : "both"),
                             scroll, fmt);

  g_logv ("Mx", G_LOG_LEVEL_MESSAGE, new_fmt, args);

  g_free (new_fmt);

  va_end (args);
}
#else
# define LOG_DEBUG(args...)
#endif /* _KINETIC_DEBUG */

static MxFocusable*
mx_kinetic_scroll_view_accept_focus (MxFocusable *focusable,
                                     MxFocusHint  hint)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (focusable)->priv;

  if (MX_IS_FOCUSABLE (priv->child))
    return mx_focusable_accept_focus (MX_FOCUSABLE (priv->child), hint);
  else
    return NULL;
}

static void
mx_kinetic_scroll_view_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_kinetic_scroll_view_accept_focus;
}


static void
emit_cursor_actor_cancel (MxKineticScrollView *scroll,
                          ClutterEvent *event)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  ClutterEvent *ev_cancel;
  ClutterActor *actor = clutter_event_get_source (event), *iactor;
  gfloat x, y;
  gint i;
  GPtrArray *event_tree = NULL;

  event_tree = g_ptr_array_sized_new (64);

  /* Build 'tree' of emitters for the event */
  iactor = priv->source_press_actor;
  while (iactor)
    {
      ClutterActor *parent;

      parent = clutter_actor_get_parent (iactor);

      if ((clutter_actor_get_reactive (iactor) &&
           (iactor != (ClutterActor *) scroll)) ||
          parent == NULL)
        {
          LOG_DEBUG (scroll, "\tcancel on %s(%p)",
                     G_OBJECT_TYPE_NAME (iactor), iactor);
          g_ptr_array_add (event_tree, g_object_ref (iactor));
        }

      iactor = parent;
    }

  clutter_event_get_coords (event, &x, &y);

  ev_cancel = clutter_event_new (CLUTTER_TOUCH_CANCEL);
  clutter_event_set_device (ev_cancel, priv->device);
  clutter_event_set_source (ev_cancel, actor);
  clutter_event_set_coords (ev_cancel, x, y);
  ev_cancel->touch.sequence = clutter_event_get_event_sequence (event);

  priv->cancel_event = ev_cancel;

  /* Capture */
  for (i = event_tree->len - 1; i >= 0; i--)
    clutter_actor_event (g_ptr_array_index (event_tree, i), ev_cancel, TRUE);

  /* Bubble */
  for (i = 0; i < event_tree->len; i++)
    clutter_actor_event (g_ptr_array_index (event_tree, i), ev_cancel, FALSE);

  priv->cancel_event = NULL;

  clutter_event_free (ev_cancel);
  g_ptr_array_free (event_tree, TRUE);
}

static gboolean release_event (MxKineticScrollView *scroll,
                               gint                 x,
                               gint                 y);

static gboolean mx_kinetic_scroll_view_event (ClutterActor *actor,
                                              ClutterEvent *event);

/* MxScrollableIface implementation */

static void
mx_kinetic_scroll_view_set_adjustments (MxScrollable *scrollable,
                                        MxAdjustment *hadjustment,
                                        MxAdjustment *vadjustment)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (scrollable)->priv;

  if (priv->child)
    mx_scrollable_set_adjustments (MX_SCROLLABLE (priv->child),
                                   hadjustment,
                                   vadjustment);
}

static void
mx_kinetic_scroll_view_get_adjustments (MxScrollable  *scrollable,
                                        MxAdjustment **hadjustment,
                                        MxAdjustment **vadjustment)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (scrollable)->priv;

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
  iface->set_adjustments = mx_kinetic_scroll_view_set_adjustments;
  iface->get_adjustments = mx_kinetic_scroll_view_get_adjustments;
}

/* Object implementation */

static void
mx_kinetic_scroll_view_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  MxAdjustment *adjustment;
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (object)->priv;

  switch (property_id)
    {
    case PROP_DECELERATION :
      g_value_set_double (value, priv->decel_rate);
      break;

/*
    case PROP_BUFFER_SIZE :
      g_value_set_uint (value, priv->motion_buffer->len);
      break;
*/

    case PROP_HADJUST:
      mx_kinetic_scroll_view_get_adjustments (MX_SCROLLABLE (object),
                                        &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST:
      mx_kinetic_scroll_view_get_adjustments (MX_SCROLLABLE (object),
                                        NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    case PROP_BUTTON:
      g_value_set_uint (value, priv->button);
      break;

    case PROP_USE_CAPTURED:
      g_value_set_boolean (value, priv->use_captured);
      break;

    case PROP_USE_GRAB:
      g_value_set_boolean (value, priv->use_grab);
      break;

    case PROP_OVERSHOOT:
      g_value_set_double (value, priv->overshoot);
      break;

    case PROP_SCROLL_POLICY:
      g_value_set_enum (value, priv->scroll_policy);
      break;

    case PROP_ACCELERATION_FACTOR :
      g_value_set_double (value, priv->acceleration_factor);
      break;

    case PROP_CLAMP_DURATION :
      g_value_set_uint (value, priv->clamp_duration);
      break;

    case PROP_CLAMP_MODE :
      g_value_set_ulong (value, priv->clamp_mode);
      break;

    case PROP_STATE :
      g_value_set_enum (value, priv->state);
      break;

    case PROP_CLAMP_TO_CENTER :
      g_value_set_boolean (value, priv->clamp_to_center);
      break;

    case PROP_SNAP_ON_PAGE :
      g_value_set_boolean (value, priv->snap_on_page);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_kinetic_scroll_view_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  MxAdjustment *adjustment;
  MxScrollable *scrollable;
  MxKineticScrollView *self = MX_KINETIC_SCROLL_VIEW (object);

  switch (property_id)
    {
    case PROP_DECELERATION :
      mx_kinetic_scroll_view_set_deceleration (self,
                                               g_value_get_double (value));
      break;

/*
    case PROP_BUFFER_SIZE :
      mx_kinetic_scroll_view_set_buffer_size (self, g_value_get_uint (value));
      break;
*/

    case PROP_HADJUST:
      scrollable = MX_SCROLLABLE (object);
      mx_kinetic_scroll_view_get_adjustments (scrollable, NULL, &adjustment);
      mx_kinetic_scroll_view_set_adjustments (scrollable,
                                        g_value_get_object (value),
                                        adjustment);
      break;

    case PROP_VADJUST:
      scrollable = MX_SCROLLABLE (object);
      mx_kinetic_scroll_view_get_adjustments (scrollable, &adjustment, NULL);
      mx_kinetic_scroll_view_set_adjustments (scrollable,
                                        adjustment,
                                        g_value_get_object (value));
      break;

    case PROP_BUTTON:
      mx_kinetic_scroll_view_set_mouse_button (self, g_value_get_uint (value));
      break;

    case PROP_USE_CAPTURED:
      mx_kinetic_scroll_view_set_use_captured (self,
                                               g_value_get_boolean (value));
      break;

    case PROP_USE_GRAB:
      mx_kinetic_scroll_view_set_use_grab (self,
                                           g_value_get_boolean (value));
      break;

    case PROP_OVERSHOOT:
      mx_kinetic_scroll_view_set_overshoot (self, g_value_get_double (value));
      break;

    case PROP_SCROLL_POLICY:
      mx_kinetic_scroll_view_set_scroll_policy (self, g_value_get_enum (value));
      break;

    case PROP_ACCELERATION_FACTOR :
      mx_kinetic_scroll_view_set_acceleration_factor (self,
          g_value_get_double (value));
      break;

    case PROP_CLAMP_DURATION :
      mx_kinetic_scroll_view_set_clamp_duration (self,
          g_value_get_uint (value));
      break;

    case PROP_CLAMP_MODE :
      mx_kinetic_scroll_view_set_clamp_mode (self,
          g_value_get_ulong (value));
      break;

    case PROP_CLAMP_TO_CENTER :
      mx_kinetic_scroll_view_set_clamp_to_center (self,
          g_value_get_boolean (value));
      break;

    case PROP_SNAP_ON_PAGE:
      mx_kinetic_scroll_view_set_snap_on_page (self,
          g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_kinetic_scroll_view_dispose (GObject *object)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (object)->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_clear_object (&priv->deceleration_timeline);
    }

  G_OBJECT_CLASS (mx_kinetic_scroll_view_parent_class)->dispose (object);
}

static void
mx_kinetic_scroll_view_finalize (GObject *object)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (object)->priv;

  g_array_free (priv->motion_buffer, TRUE);

  G_OBJECT_CLASS (mx_kinetic_scroll_view_parent_class)->finalize (object);
}

static void
mx_kinetic_scroll_view_get_preferred_width (ClutterActor *actor,
                                            gfloat        for_height,
                                            gfloat       *min_width_p,
                                            gfloat       *nat_width_p)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_kinetic_scroll_view_parent_class)->
    get_preferred_width (actor, for_height, NULL, nat_width_p);

  if (min_width_p && priv->scroll_policy != MX_SCROLL_POLICY_VERTICAL)
    {
      MxPadding padding;

      mx_widget_get_padding (MX_WIDGET (actor), &padding);
      *min_width_p = padding.left + padding.right;
    }
}

static void
mx_kinetic_scroll_view_get_preferred_height (ClutterActor *actor,
                                             gfloat        for_width,
                                             gfloat       *min_height_p,
                                             gfloat       *nat_height_p)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_kinetic_scroll_view_parent_class)->
    get_preferred_height (actor, for_width, NULL, nat_height_p);

  if (min_height_p && priv->scroll_policy != MX_SCROLL_POLICY_HORIZONTAL)
    {
      MxPadding padding;

      mx_widget_get_padding (MX_WIDGET (actor), &padding);
      *min_height_p = padding.top + padding.bottom;
    }
}

static void
mx_kinetic_scroll_view_allocate (ClutterActor           *actor,
                                 const ClutterActorBox  *box,
                                 ClutterAllocationFlags  flags)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (actor)->priv;
  ClutterActorBox childbox;

  CLUTTER_ACTOR_CLASS (mx_kinetic_scroll_view_parent_class)->
    allocate (actor, box, flags);


  if (priv->child)
    {
      mx_widget_get_available_area (MX_WIDGET (actor), box, &childbox);
      clutter_actor_allocate (priv->child, &childbox, flags);
    }
}

static void
mx_kinetic_scroll_view_paint (ClutterActor *actor)
{
  MxKineticScrollViewPrivate *priv = ((MxKineticScrollView *) actor)->priv;
  ClutterActorBox box;

  CLUTTER_ACTOR_CLASS (mx_kinetic_scroll_view_parent_class)->paint (actor);

  if (priv->child)
    {
      clutter_actor_get_allocation_box (priv->child, &box);
      cogl_clip_push_rectangle (box.x1, box.y1,
                                box.x2 - box.x1,
                                box.y2 - box.y1);
      clutter_actor_paint (priv->child);
      cogl_clip_pop ();
    }
}

static void
mx_kinetic_scroll_view_pick (ClutterActor *actor, const ClutterColor *color)
{
  MxKineticScrollViewPrivate *priv = ((MxKineticScrollView *) actor)->priv;
  ClutterActorBox box;

  CLUTTER_ACTOR_CLASS (mx_kinetic_scroll_view_parent_class)->pick (actor,
                                                                   color);

  if (priv->child)
    {
      clutter_actor_get_allocation_box (priv->child, &box);
      cogl_clip_push_rectangle (box.x1, box.y1,
                                box.x2 - box.x1,
                                box.y2 - box.y1);
      clutter_actor_paint (priv->child);
      cogl_clip_pop ();
    }
}

static gboolean
mx_kinetic_scroll_view_button_event (ClutterActor       *actor,
                                     ClutterButtonEvent *event)
{
  /* Avoid default implementation of MxWidget to set the 'active'
     state. */
  return FALSE;
}

static gboolean
mx_kinetic_scroll_view_touch_event (ClutterActor      *actor,
                                    ClutterTouchEvent *event)
{
  /* Avoid default implementation of MxWidget to set the 'active'
     state. */
  return FALSE;
}

static void
mx_kinetic_scroll_view_class_init (MxKineticScrollViewClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxKineticScrollViewPrivate));

  object_class->get_property = mx_kinetic_scroll_view_get_property;
  object_class->set_property = mx_kinetic_scroll_view_set_property;
  object_class->dispose = mx_kinetic_scroll_view_dispose;
  object_class->finalize = mx_kinetic_scroll_view_finalize;

  actor_class->get_preferred_width = mx_kinetic_scroll_view_get_preferred_width;
  actor_class->get_preferred_height = mx_kinetic_scroll_view_get_preferred_height;
  actor_class->allocate = mx_kinetic_scroll_view_allocate;
  actor_class->paint = mx_kinetic_scroll_view_paint;
  actor_class->pick = mx_kinetic_scroll_view_pick;

  actor_class->event = mx_kinetic_scroll_view_event;
  actor_class->button_press_event = mx_kinetic_scroll_view_button_event;
  actor_class->button_release_event = mx_kinetic_scroll_view_button_event;
  actor_class->touch_event = mx_kinetic_scroll_view_touch_event;

  pspec = g_param_spec_double ("deceleration",
                               "Deceleration",
                               "Rate at which the view will decelerate in.",
                               1.01, G_MAXDOUBLE, 1.1,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_DECELERATION, pspec);

  /*
  pspec = g_param_spec_uint ("buffer-size",
                             "Buffer size",
                             "Amount of motion events to buffer",
                             1, G_MAXUINT, 3,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_BUFFER_SIZE, pspec);
  */

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

  pspec = g_param_spec_boolean ("use-grab",
                                "Use grab",
                                "Use grab to initiate follow motion events",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_USE_GRAB, pspec);

  pspec = g_param_spec_double ("overshoot",
                               "Overshoot",
                               "The rate at which the view will decelerate "
                               "when scrolled beyond its boundaries.",
                               0.0, 1.0, 0.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_OVERSHOOT, pspec);

  pspec = g_param_spec_enum ("scroll-policy",
                             "Scroll Policy",
                             "The scroll policy",
                             MX_TYPE_SCROLL_POLICY,
                             MX_SCROLL_POLICY_BOTH,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SCROLL_POLICY, pspec);

  pspec = g_param_spec_double ("acceleration-factor",
                               "Initial acceleration factor",
                               "Factor applied to the initial acceleration.",
                               0.0, G_MAXDOUBLE, 1.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ACCELERATION_FACTOR, pspec);

  pspec = g_param_spec_uint ("clamp-duration",
                             "Clamp duration",
                             "Duration of the adjustment clamp animation.",
                             0, G_MAXUINT, 250,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLAMP_DURATION, pspec);

  pspec = g_param_spec_ulong ("clamp-mode",
                              "Clamp mode",
                              "Animation mode to use for the clamp animation.",
                              0, G_MAXULONG, CLUTTER_EASE_OUT_QUAD,
                              MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLAMP_MODE, pspec);

  pspec = g_param_spec_enum ("state",
                             "State",
                             "State of the scrolling",
                             MX_TYPE_KINETIC_SCROLL_VIEW_STATE,
                             MX_KINETIC_SCROLL_VIEW_STATE_IDLE,
                             MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_STATE, pspec);

  pspec = g_param_spec_boolean ("clamp-to-center",
                                "Clamp to center",
                                "Whether to clamp to step increments based on the center of the page.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLAMP_TO_CENTER, pspec);

  pspec = g_param_spec_boolean ("snap-on-page",
                                "Snap on page",
                                "Whether to stop animations on step increments.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SNAP_ON_PAGE, pspec);

  /* MxScrollable properties */
  g_object_class_override_property (object_class,
                                    PROP_HADJUST,
                                    "horizontal-adjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUST,
                                    "vertical-adjustment");
}

static void
set_state (MxKineticScrollView *scroll, MxKineticScrollViewState state)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;

  LOG_DEBUG (scroll, "%s: old state = %u, new state = %u",
             G_STRFUNC, priv->state, state);

  priv->state = state;
  g_object_notify (G_OBJECT (scroll), "state");

  LOG_DEBUG (scroll, "%s: finished setting state to %u", G_STRFUNC, state);
}

static gboolean
motion_event_cb (ClutterActor        *actor,
                 ClutterEvent        *event,
                 MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  ClutterEventType type = clutter_event_type (event);
  gfloat x, y;
  gboolean swallow = TRUE;

  if (priv->device != clutter_event_get_device (event))
    return FALSE;

  if (priv->cancel_event == event)
    {
      LOG_DEBUG (scroll, "%s: cancel", G_STRFUNC);
      return FALSE;
    }

  if (type == CLUTTER_MOTION)
    {
      ClutterModifierType modifier_state;
      /* For various reasons, it's possible we may not get the button release
       * event - in this case, check here if the button is still down and
       * handle it manually.
       *
       * We need to do this, or we can end up just stealing events forever.
       */
      clutter_event_get_coords (event, &x, &y);
      modifier_state = clutter_event_get_state (event);

      switch (priv->button)
        {
        default:
        case 1:
          if (!(modifier_state & CLUTTER_BUTTON1_MASK))
            return release_event (scroll, x, y);
          break;
        case 2:
          if (!(modifier_state & CLUTTER_BUTTON2_MASK))
            return release_event (scroll, x, y);
          break;
        case 3:
          if (!(modifier_state & CLUTTER_BUTTON3_MASK))
            return release_event (scroll, x, y);
          break;
        case 4:
          if (!(modifier_state & CLUTTER_BUTTON4_MASK))
            return release_event (scroll, x, y);
          break;
        case 5:
          if (!(modifier_state & CLUTTER_BUTTON5_MASK))
            return release_event (scroll, x, y);
          break;
        }
    }
  else if (type == CLUTTER_TOUCH_UPDATE)
    {
      if (clutter_event_get_event_sequence (event) != priv->sequence)
        return FALSE;
      clutter_event_get_coords (event, &x, &y);
    }
  else
    return FALSE;

  LOG_DEBUG (scroll, "motion point(%fx%f)", x, y);

  if (clutter_actor_transform_stage_point (CLUTTER_ACTOR (scroll),
                                           x, y, &x, &y))
    {
      MxKineticScrollViewMotion *motion;

      /* Check if we've passed the drag threshold */
      if (!priv->in_drag)
        {
          guint threshold;
          gboolean threshold_passed;
          MxSettings *settings = mx_settings_get_default ();
          gdouble dx, dy;

          g_object_get (G_OBJECT (settings),
                        "drag-threshold", &threshold, NULL);
          g_assert (priv->motion_buffer->len > 0);
          motion = &g_array_index (priv->motion_buffer,
                                   MxKineticScrollViewMotion, 0);

          dx = ABS (motion->x - x);
          dy = ABS (motion->y - y);

          if ((dy >= threshold) &&
              (priv->scroll_policy == MX_SCROLL_POLICY_VERTICAL ||
               priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
               priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC))
           threshold_passed = TRUE;
          else if ((dx >= threshold) &&
              (priv->scroll_policy == MX_SCROLL_POLICY_HORIZONTAL ||
               priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
               priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC))
           threshold_passed = TRUE;
          else
           threshold_passed = FALSE;

          switch (priv->scroll_policy)
            {
            case MX_SCROLL_POLICY_HORIZONTAL:
              if (dx < dy)
                threshold_passed = FALSE;
              break;

            case MX_SCROLL_POLICY_VERTICAL:
              if (dy < dx)
                threshold_passed = FALSE;
              break;

            default:
              break;
            }

          LOG_DEBUG (scroll, "motion dx=%f dy=%f passed=%i policy=%i threshold=%u",
                     dx, dy, threshold_passed, priv->scroll_policy, threshold);


          if (threshold_passed)
            {
              ClutterActor *stage;

              stage = clutter_actor_get_stage (actor);

              clutter_stage_set_motion_events_enabled (CLUTTER_STAGE (stage),
                                                       FALSE);

              priv->in_drag = TRUE;

              set_state (scroll, MX_KINETIC_SCROLL_VIEW_STATE_PANNING);

              if (!priv->use_captured)
                {
                  g_signal_handlers_disconnect_by_func (scroll,
                                                        motion_event_cb,
                                                        scroll);
                  g_signal_connect (stage,
                                    "captured-event",
                                    G_CALLBACK (motion_event_cb),
                                    scroll);
                }
              else
                {
                  /* swallow = FALSE; */
                  emit_cursor_actor_cancel (scroll, event);
                }

              if (priv->use_grab)
                {
                  if (priv->sequence)
                    clutter_input_device_sequence_grab (priv->device,
                                                        priv->sequence,
                                                        CLUTTER_ACTOR (scroll));
                  else
                    clutter_input_device_grab (priv->device,
                                               CLUTTER_ACTOR (scroll));
                }
            }
          else
            return FALSE;
        }

      g_assert (priv->motion_buffer->len > 0);
      LOG_DEBUG (scroll, "motion dx=%f dy=%f",
                 ABS (g_array_index (priv->motion_buffer, MxKineticScrollViewMotion, priv->motion_buffer->len - 1).x - x),
                 ABS (g_array_index (priv->motion_buffer, MxKineticScrollViewMotion, priv->motion_buffer->len - 1).y - y));

      if (priv->child)
        {
          gdouble dx, dy;
          MxAdjustment *hadjust, *vadjust;

          mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child),
                                         &hadjust, &vadjust);

          g_assert (priv->last_motion < priv->motion_buffer->len);
          motion = &g_array_index (priv->motion_buffer,
                                   MxKineticScrollViewMotion,
                                   priv->last_motion);

          if (!priv->align_tested)
            {
              priv->align_tested = TRUE;
              priv->in_automatic_scroll = MX_AUTOMATIC_SCROLL_NONE;
              if (priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC)
                {
                  gfloat scroll_threshold = M_PI_4/2;
                  gfloat drag_angle = atan((motion->y - y)/(x - motion->x));
                  if( (drag_angle > -scroll_threshold) && (drag_angle < scroll_threshold) )
                    priv->in_automatic_scroll = MX_AUTOMATIC_SCROLL_HORIZONTAL;
                  else if ( (drag_angle > (M_PI_2 - scroll_threshold)) ||
                            (drag_angle < -(M_PI_2 - scroll_threshold)) )
                    priv->in_automatic_scroll = MX_AUTOMATIC_SCROLL_VERTICAL;
                }
            }


          if (hadjust &&
              (priv->scroll_policy == MX_SCROLL_POLICY_HORIZONTAL ||
               priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
               priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC) &&
               (priv->in_automatic_scroll == MX_AUTOMATIC_SCROLL_HORIZONTAL ||
               priv->in_automatic_scroll == MX_AUTOMATIC_SCROLL_NONE))
            {
              dx = (motion->x - x) + mx_adjustment_get_value (hadjust);
              mx_adjustment_set_value (hadjust, dx);
            }

          if (vadjust &&
              (priv->scroll_policy == MX_SCROLL_POLICY_VERTICAL ||
               priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
               priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC) &&
               (priv->in_automatic_scroll == MX_AUTOMATIC_SCROLL_VERTICAL ||
               priv->in_automatic_scroll == MX_AUTOMATIC_SCROLL_NONE))
            {
              dy = (motion->y - y) + mx_adjustment_get_value (vadjust);
              mx_adjustment_set_value (vadjust, dy);
            }
        }

      priv->last_motion ++;
      if (priv->last_motion == priv->motion_buffer->len)
        {
          LOG_DEBUG (scroll, "increase buffer size to %u", priv->last_motion);
          g_array_set_size (priv->motion_buffer, priv->last_motion + 1);
        }

      motion = &g_array_index (priv->motion_buffer,
                               MxKineticScrollViewMotion, priv->last_motion);
      motion->x = x;
      motion->y = y;
      g_get_current_time (&motion->time);
    }

  return swallow;
}

static void
interpolation_completed_cb (MxAdjustment *adj, MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  MxAdjustment *hadj, *vadj;

  g_signal_handlers_disconnect_by_func (adj, interpolation_completed_cb,
                                        scroll);

  mx_scrollable_get_adjustments (MX_SCROLLABLE (scroll), &hadj, &vadj);
  if (adj == hadj)
    priv->hclamping = FALSE;
  else
    priv->vclamping = FALSE;

  if (!priv->hclamping && !priv->vclamping && \
      priv->state == MX_KINETIC_SCROLL_VIEW_STATE_CLAMPING)
    set_state (scroll, MX_KINETIC_SCROLL_VIEW_STATE_IDLE);
}

static void
clamp_adjustment (MxKineticScrollView *scroll,
                  MxAdjustment        *adj,
                  guint                duration)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  gdouble d, value, lower, upper, step_increment, page_size;

  /* Snap to the nearest step increment on hadjustment */
  mx_adjustment_get_values (adj, &value, &lower, &upper,
                            &step_increment, NULL, &page_size);

  if (priv->snap_on_page)
    d = (rint ((value - lower) / step_increment) *
         step_increment) + lower;
  else
    d = value;

  if (priv->clamp_to_center)
    {
      gdouble offset = page_size / 2;
      offset -= step_increment / 2;
      offset = step_increment - (int) offset % (int) step_increment;

      if (offset > step_increment / 2)
        offset = - (step_increment - offset);

      d += offset;
    }

  if (value != 0)
    {
      d = CLAMP (d, lower, upper - page_size);
      if (fabs ((upper - page_size) - value) < fabs (d - value))
        d = upper - page_size;
    }

  g_signal_connect (adj, "interpolation-completed",
                    G_CALLBACK (interpolation_completed_cb), scroll);
  mx_adjustment_interpolate (adj, d, duration, priv->clamp_mode);
}

static void
clamp_adjustments (MxKineticScrollView *scroll,
                   guint                duration,
                   gboolean             horizontal,
                   gboolean             vertical)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;

  if (priv->child)
    {
      MxAdjustment *hadj, *vadj;

      mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child), &hadj, &vadj);

      if (horizontal && hadj)
        {
          priv->hclamping = TRUE;
          clamp_adjustment (scroll, hadj, duration);
        }

      if (vertical && vadj)
        {
          priv->vclamping = TRUE;
          clamp_adjustment (scroll, vadj, duration);
        };
    }


  if (priv->hclamping || priv->vclamping)
    set_state (scroll, MX_KINETIC_SCROLL_VIEW_STATE_CLAMPING);
  else
    set_state (scroll, MX_KINETIC_SCROLL_VIEW_STATE_IDLE);
}

static void
deceleration_stopped_cb (ClutterTimeline     *timeline,
                         gboolean             is_finished,
                         MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  guint duration;

  LOG_DEBUG (scroll, "%s: priv->overshoot = %f, priv->clamp_duration = %u, "
             "priv->hmoving = %s, priv->vmoving = %s",
             G_STRFUNC, priv->overshoot, priv->clamp_duration,
             priv->hmoving ? "true" : "false",
             priv->vmoving ? "true" : "false");

  duration = (priv->overshoot > 0.0) ? priv->clamp_duration : 10;
  clamp_adjustments (scroll, duration, priv->hmoving, priv->vmoving);

  g_clear_object (&priv->deceleration_timeline);
}

static void
deceleration_new_frame_cb (ClutterTimeline     *timeline,
                           gint                 msecs,
                           MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;

  if (priv->child)
    {
      MxAdjustment *hadjust, *vadjust;

      gboolean stop = TRUE;

      mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child),
                                     &hadjust, &vadjust);

      LOG_DEBUG (scroll, "%s: initialising: msecs = %i, "
                 "priv->accumulated_delta = %f, delta = %u",
                 G_STRFUNC, msecs, priv->accumulated_delta,
                 clutter_timeline_get_delta (timeline));

      priv->accumulated_delta += clutter_timeline_get_delta (timeline);

      if (priv->accumulated_delta <= 1000.0/60.0)
        stop = FALSE;

      while (priv->accumulated_delta > 1000.0/60.0)
        {
          gdouble hvalue, vvalue;

          LOG_DEBUG (scroll, "%s: looping: stop = %s, "
                     "priv->accumulated_delta = %f, hadjust = %p, "
                     "vadjust = %p, priv->scroll_policy = %u, "
                     "priv->in_automatic_scroll = %u, priv->dx = %f, "
                     "priv->dy = %f, priv->overshoot = %f, "
                     "priv->hmoving = %s, priv->vmoving = %s",
                     G_STRFUNC, stop ? "true" : "false",
                     priv->accumulated_delta, hadjust, vadjust,
                     priv->scroll_policy, priv->in_automatic_scroll, priv->dx,
                     priv->dy, priv->overshoot,
                     priv->hmoving ? "true" : "false",
                     priv->vmoving ? "true" : "false");

          if (hadjust &&
              (priv->scroll_policy == MX_SCROLL_POLICY_HORIZONTAL ||
              priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
              priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC) &&
              priv->in_automatic_scroll != MX_AUTOMATIC_SCROLL_VERTICAL)
            {
              if (ABS (priv->dx) > 2)
                {
                  hvalue = priv->dx + mx_adjustment_get_value (hadjust);
                  mx_adjustment_set_value (hadjust, hvalue);

                  if (priv->overshoot > 0.0)
                    {
                      if ((hvalue > mx_adjustment_get_upper (hadjust) -
                           mx_adjustment_get_page_size (hadjust)) ||
                          (hvalue < mx_adjustment_get_lower (hadjust)))
                        priv->dx *= priv->overshoot;
                    }

                  priv->dx = priv->dx / priv->decel_rate;

                  stop = FALSE;
                }
              else if (priv->hmoving)
                {
                  guint duration;

                  priv->hmoving = FALSE;

                  duration = (priv->overshoot > 0.0) ?
                                priv->clamp_duration : 10;
                  clamp_adjustments (scroll, duration, TRUE, FALSE);
                }
            }

          if (vadjust &&
              (priv->scroll_policy == MX_SCROLL_POLICY_VERTICAL ||
              priv->scroll_policy == MX_SCROLL_POLICY_BOTH ||
              priv->scroll_policy == MX_SCROLL_POLICY_AUTOMATIC) &&
              priv->in_automatic_scroll != MX_AUTOMATIC_SCROLL_HORIZONTAL)
            {
              if (ABS (priv->dy) > 2)
                {
                  vvalue = priv->dy + mx_adjustment_get_value (vadjust);
                  mx_adjustment_set_value (vadjust, vvalue);

                  if (priv->overshoot > 0.0)
                    {
                      if ((vvalue > mx_adjustment_get_upper (vadjust) -
                           mx_adjustment_get_page_size (vadjust)) ||
                          (vvalue < mx_adjustment_get_lower (vadjust)))
                        priv->dy *= priv->overshoot;
                    }

                  priv->dy = priv->dy / priv->decel_rate;

                  stop = FALSE;
                }
              else if (priv->vmoving)
                {
                  guint duration;

                  priv->vmoving = FALSE;

                  duration = (priv->overshoot > 0.0) ?
                                priv->clamp_duration : 10;
                  clamp_adjustments (scroll, duration, FALSE, TRUE);
                }
            }

          priv->accumulated_delta -= 1000.0/60.0;
        }

      if (stop)
        {
          clutter_timeline_stop (timeline);
        }
    }
}

static gboolean
button_release_event_cb (ClutterActor        *stage,
                         ClutterEvent        *event,
                         MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  gfloat x, y;

  if (priv->device != clutter_event_get_device (event))
    return FALSE;

  if (priv->cancel_event == event)
    {
      LOG_DEBUG (scroll, "%s: cancel", G_STRFUNC);
      return FALSE;
    }

  switch (clutter_event_type (event))
    {
    case CLUTTER_BUTTON_RELEASE:
      if (clutter_event_get_button (event) == priv->button)
        {
          clutter_event_get_coords (event, &x, &y);
          return release_event (scroll, x, y);
        }
      break;

    case CLUTTER_TOUCH_CANCEL:
    case CLUTTER_TOUCH_END:
      if (clutter_event_get_event_sequence (event) == priv->sequence)
        {
          clutter_event_get_coords (event, &x, &y);
          return release_event (scroll, x, y);
        }
      break;

    default:
      break;
    }

  return FALSE;
}

static gboolean
release_event (MxKineticScrollView *scroll,
               gint                 x_pos,
               gint                 y_pos)
{
  ClutterActor *actor = CLUTTER_ACTOR (scroll);
  ClutterActor *stage = clutter_actor_get_stage (actor);
  MxKineticScrollViewPrivate *priv = scroll->priv;
  gboolean decelerating = FALSE;

  LOG_DEBUG (scroll, "RELEASE!");

  g_signal_handlers_disconnect_by_func (scroll,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (stage,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (stage,
                                        button_release_event_cb,
                                        scroll);

  if (!priv->in_drag)
    {
      LOG_DEBUG (scroll, "%s: priv->in_drag = false", G_STRFUNC);

      priv->device = NULL;
      priv->sequence = NULL;
      priv->last_motion = 0;
      return FALSE;
    }

  clutter_stage_set_motion_events_enabled (CLUTTER_STAGE (stage), TRUE);

  if (priv->child)
    {
      gfloat event_x = x_pos, event_y = y_pos;

      if (clutter_actor_transform_stage_point (actor, x_pos, y_pos,
                                               &event_x, &event_y))
        {
          gdouble value, lower, upper, step_increment, page_size,
                  d, ax, ay, y, nx, ny, n;
          gfloat frac, x_origin, y_origin;
          GTimeVal release_time, motion_time;
          MxAdjustment *hadjust, *vadjust;
          glong time_diff;
          guint duration;
          gint i;

          /* Get time delta */
          g_get_current_time (&release_time);

          /* Get average position/time of last x mouse events */
          priv->last_motion ++;
          if (priv->last_motion == priv->motion_buffer->len)
            {
              LOG_DEBUG (scroll, "increase buffer size to %u",
                         priv->last_motion);
              g_array_set_size (priv->motion_buffer, priv->last_motion + 1);
            }

          x_origin = y_origin = 0;
          motion_time = (GTimeVal){ 0, 0 };
          for (i = 0; i < priv->last_motion; i++)
            {
              MxKineticScrollViewMotion *motion =
                &g_array_index (priv->motion_buffer, MxKineticScrollViewMotion, i);

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
          priv->dx = (x_origin - event_x) / frac * priv->acceleration_factor;
          priv->dy = (y_origin - event_y) / frac * priv->acceleration_factor;

          /* If the delta is too low for the equations to work,
           * bump the values up a bit.
           */
          if (ABS (priv->dx) < 1)
            priv->dx = (priv->dx > 0) ? 1 : -1;
          if (ABS (priv->dy) < 1)
            priv->dy = (priv->dy > 0) ? 1 : -1;

          /* We want n, where x / y^n < z,
           * x = Distance to move per frame
           * y = Deceleration rate
           * z = maximum distance from target
           *
           * Rearrange to n = log (x / z) / log (y)
           * To simplify, z = 1, so n = log (x) / log (y)
           */
          y = priv->decel_rate;
          nx = logf (ABS (priv->dx)) / logf (y);
          ny = logf (ABS (priv->dy)) / logf (y);
          n = MAX (nx, ny);

          duration = MAX (1, (gint)(MAX (nx, ny) * (1000/60.0)));

          LOG_DEBUG (scroll, "%s: checking duration: x_pos = %i, y_pos = %i, "
                     "event_x = %f, event_y = %f, y = %f, nx = %f, ny = %f, "
                     "n = %f, frac = %f, x_origin = %f, y_origin = %f, "
                     "time_diff = %lu, duration = %u, "
                     "priv->last_motion = %u, priv->dx = %f, "
                     "priv->dy = %f, priv->decel_rate = %f, "
                     "priv->overshoot = %f, priv->accumulated_delta = %f, "
                     "priv->acceleration_factor = %f",
                     G_STRFUNC, x_pos, y_pos, event_x, event_y,
                     y, nx, ny, n, frac, x_origin, y_origin, time_diff,
                     duration, priv->last_motion, priv->dx, priv->dy,
                     priv->decel_rate, priv->overshoot,
                     priv->accumulated_delta, priv->acceleration_factor);

          if (duration > 250)
            {
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
              mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child),
                                             &hadjust, &vadjust);
              ax = (1.0 - 1.0 / pow (y, n + 1)) / (1.0 - 1.0 / y);
              ay = (1.0 - 1.0 / pow (y, n + 1)) / (1.0 - 1.0 / y);

              /* Solving for dx */
              if (hadjust &&
                  priv->in_automatic_scroll != MX_AUTOMATIC_SCROLL_VERTICAL)
                {
                  mx_adjustment_get_values (hadjust, &value, &lower, &upper,
                                            &step_increment, NULL, &page_size);

                  /* Make sure we pick the next nearest step increment in the
                   * same direction as the push.
                   */
                  priv->dx *= n;
                  if (priv->snap_on_page)
                    {
                      if (ABS (priv->dx) < step_increment / 2)
                        d = round ((value + priv->dx - lower) / step_increment);
                      else if (priv->dx > 0)
                        d = ceil ((value + priv->dx - lower) / step_increment);
                      else
                        d = floor ((value + priv->dx - lower) / step_increment);

                      if (priv->overshoot <= 0.0)
                        d = CLAMP ((d * step_increment) + lower,
                                   lower, upper - page_size) - value;
                      else
                        d = ((d * step_increment) + lower) - value;
                    }
                  else
                    {
                      if (priv->overshoot <= 0.0)
                        d = CLAMP (value + priv->dx + lower,
                                   lower, upper - page_size) - value;
                      else
                        d = priv->dx;
                    }

                  priv->dx = d / ax;
                }

              /* Solving for dy */
              if (vadjust &&
                  priv->in_automatic_scroll != MX_AUTOMATIC_SCROLL_HORIZONTAL)
                {
                  mx_adjustment_get_values (vadjust, &value, &lower, &upper,
                                            &step_increment, NULL, &page_size);

                  priv->dy *= n;
                  if (priv->snap_on_page)
                    {
                      if (ABS (priv->dy) < step_increment / 2)
                        d = round ((value + priv->dy - lower) / step_increment);
                      else if (priv->dy > 0)
                        d = ceil ((value + priv->dy - lower) / step_increment);
                      else
                        d = floor ((value + priv->dy - lower) / step_increment);

                      if (priv->overshoot <= 0.0)
                        d = CLAMP ((d * step_increment) + lower,
                                   lower, upper - page_size) - value;
                      else
                        d = ((d * step_increment) + lower) - value;
                    }
                  else
                    {
                      if (priv->overshoot <= 0.0)
                        d = CLAMP (value + priv->dy + lower,
                                   lower, upper - page_size) - value;
                      else
                        d = priv->dy;
                    }

                  priv->dy = d / ay;
                }

              LOG_DEBUG (scroll, "%s: release: x_pos = %i, y_pos = %i, "
                         "event_x = %f, event_y = %f, value = %f, lower = %f, "
                         "upper = %f, step_increment = %f, page_size = %f, "
                         "d = %f, ax = %f, ay = %f, y = %f, nx = %f, ny = %f, "
                         "n = %f, frac = %f, x_origin = %f, y_origin = %f, "
                         "time_diff = %lu, duration = %u, "
                         "priv->last_motion = %u, priv->dx = %f, "
                         "priv->dy = %f, priv->decel_rate = %f, "
                         "priv->overshoot = %f, priv->accumulated_delta = %f, "
                         "priv->acceleration_factor = %f",
                         G_STRFUNC, x_pos, y_pos, event_x, event_y, value,
                         lower, upper, step_increment, page_size, d, ax, ay, y,
                         nx, ny, n, frac, x_origin, y_origin, time_diff,
                         duration, priv->last_motion, priv->dx, priv->dy,
                         priv->decel_rate, priv->overshoot,
                         priv->accumulated_delta, priv->acceleration_factor);

              priv->deceleration_timeline = clutter_timeline_new (duration);

              g_signal_connect (priv->deceleration_timeline, "new_frame",
                                G_CALLBACK (deceleration_new_frame_cb), scroll);
              g_signal_connect (priv->deceleration_timeline, "stopped",
                                G_CALLBACK (deceleration_stopped_cb), scroll);
              priv->accumulated_delta = 0;
              priv->hmoving = priv->vmoving = TRUE;
              clutter_timeline_start (priv->deceleration_timeline);
              decelerating = TRUE;
              set_state (scroll, MX_KINETIC_SCROLL_VIEW_STATE_SCROLLING);
            }
        }
      else
        {
          LOG_DEBUG (scroll, "%s: failed to transform point (%f, %f)",
                     G_STRFUNC, x_pos, y_pos);
        }
    }
  else
    {
      LOG_DEBUG (scroll, "%s: no child set", G_STRFUNC);
    }

  if (priv->use_grab)
    {
      if (priv->sequence)
        clutter_input_device_sequence_ungrab (priv->device, priv->sequence);
      else
        clutter_input_device_ungrab (priv->device);
    }

  priv->sequence = NULL;
  priv->device = NULL;

  /* Reset motion event buffer */
  priv->last_motion = 0;

  if (!decelerating)
    clamp_adjustments (scroll, priv->clamp_duration, TRUE, TRUE);

  return TRUE;
}

static gboolean
press_event (MxKineticScrollView *scroll,
             gfloat               x,
             gfloat               y)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  ClutterActor *actor = (ClutterActor *) scroll;
  ClutterActor *stage = clutter_actor_get_stage (actor);
  MxKineticScrollViewMotion *motion;

  /* Reset automatic-scroll setting */
  priv->in_automatic_scroll = MX_AUTOMATIC_SCROLL_NONE;
  priv->align_tested = 0;

  /* Reset motion buffer */
  priv->last_motion = 0;
  motion = &g_array_index (priv->motion_buffer, MxKineticScrollViewMotion, 0);
  motion->x = x;
  motion->y = y;

  LOG_DEBUG (scroll, "initial point(%fx%f)", x, y);

  if (clutter_actor_transform_stage_point (actor, x, y,
                                           &motion->x, &motion->y))
    {
      guint threshold;
      MxSettings *settings = mx_settings_get_default ();

      g_get_current_time (&motion->time);

      if (priv->deceleration_timeline)
        {
          clutter_timeline_stop (priv->deceleration_timeline);
          g_clear_object (&priv->deceleration_timeline);
        }

      if (priv->use_captured)
        {
          g_signal_connect (scroll,
                            "captured-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
        }
      else
        {
          g_signal_connect (scroll,
                            "motion-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
          g_signal_connect (scroll,
                            "touch-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
        }

      g_signal_connect (stage,
                        "captured-event",
                        G_CALLBACK (button_release_event_cb),
                        scroll);

      /* If there's a zero drag threshold, start the drag immediately */
      g_object_get (G_OBJECT (settings),
                    "drag-threshold", &threshold, NULL);
      if (threshold == 0)
        {
          priv->in_drag = TRUE;
          clutter_stage_set_motion_events_enabled (CLUTTER_STAGE (stage),
                                                   FALSE);

          if (priv->use_grab)
            {
              if (priv->sequence)
                clutter_input_device_sequence_grab (priv->device,
                                                    priv->sequence,
                                                    CLUTTER_ACTOR (scroll));
              else
                clutter_input_device_grab (priv->device, CLUTTER_ACTOR (scroll));
            }

          /* Swallow the press event */
          return TRUE;
        }
      else
        priv->in_drag = FALSE;
    }

  return FALSE;
}

static gboolean
button_press_event_cb (ClutterActor        *actor,
                       ClutterEvent        *event,
                       MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv = scroll->priv;
  gfloat x, y;

  if (priv->device != NULL)
    return FALSE;

  if (priv->cancel_event == event)
    {
      LOG_DEBUG (scroll, "%s: cancel", G_STRFUNC);
      return FALSE;
    }

  switch (clutter_event_type (event))
    {
    case CLUTTER_BUTTON_PRESS:
      if (clutter_event_get_button (event) == priv->button)
        {
          priv->device = clutter_event_get_device (event);
          priv->sequence = clutter_event_get_event_sequence (event);
          priv->source_press_actor = clutter_event_get_source (event);
          clutter_event_get_coords (event, &x, &y);
          if (press_event (scroll, x, y))
            {
              if (priv->use_grab)
                {
                  emit_cursor_actor_cancel (scroll, event);
                  /* return FALSE; */
                }
              return TRUE;
            }
        }
      break;

    case CLUTTER_TOUCH_BEGIN:
      if (priv->sequence == NULL)
        {
          priv->device = clutter_event_get_device (event);
          priv->sequence = clutter_event_get_event_sequence (event);
          priv->source_press_actor = clutter_event_get_source (event);
          clutter_event_get_coords (event, &x, &y);
          if (press_event (scroll, x, y))
            {
              if (priv->use_grab)
                {
                  emit_cursor_actor_cancel (scroll, event);
                  /* return FALSE; */
                }
              return TRUE;
            }
        }
      break;

    default:
      break;
    }

  return FALSE;
}

static gboolean
mx_kinetic_scroll_view_event (ClutterActor *actor,
                              ClutterEvent *event)
{
  MxKineticScrollView *scroll = (MxKineticScrollView *) actor;
  MxKineticScrollViewPrivate *priv = scroll->priv;

  if (!priv->in_drag)
    return FALSE;

  if (priv->cancel_event == event)
    {
      LOG_DEBUG (scroll, "%s: cancel", G_STRFUNC);
      return FALSE;
    }

  switch (clutter_event_type (event))
    {
    case CLUTTER_MOTION:
    case CLUTTER_TOUCH_UPDATE:
      return motion_event_cb (actor, event, scroll);

    case CLUTTER_BUTTON_RELEASE:
    case CLUTTER_TOUCH_END:
    case CLUTTER_TOUCH_CANCEL:
      return button_release_event_cb (clutter_actor_get_stage (actor),
                                      event,
                                      scroll);

    default:
      break;
    }

  return FALSE;
}


static void
mx_kinetic_scroll_view_actor_added (ClutterActor *container,
                                    ClutterActor *actor)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (container)->priv;

  if (MX_IS_TOOLTIP (actor))
    return;

  if (priv->child)
    {
      clutter_actor_remove_child (container, priv->child);
      priv->child = NULL;
    }

  if (MX_IS_SCROLLABLE (actor))
    {
      MxAdjustment *hadjust, *vadjust;

      priv->child = actor;

      /* Make sure the adjustments have been created so the child
       * will initialise them during its allocation (necessary for
       * MxBoxLayout, for example)
       */
      mx_scrollable_get_adjustments (MX_SCROLLABLE (actor), &hadjust, &vadjust);
    }
  else
    g_warning ("Attempting to add an actor of type %s to "
               "a MxKineticScrollView, but the actor does "
               "not implement MxScrollable.",
               g_type_name (G_OBJECT_TYPE (actor)));
}

static void
mx_kinetic_scroll_view_actor_removed (ClutterActor *container,
                                      ClutterActor *actor)
{
  MxKineticScrollViewPrivate *priv = MX_KINETIC_SCROLL_VIEW (container)->priv;

  if (priv->child == actor)
    priv->child = NULL;
}

static void
mx_kinetic_scroll_view_init (MxKineticScrollView *self)
{
  MxKineticScrollViewPrivate *priv = self->priv =
    KINETIC_SCROLL_VIEW_PRIVATE (self);

  priv->motion_buffer =
    g_array_sized_new (FALSE, TRUE, sizeof (MxKineticScrollViewMotion), 30);
  g_array_set_size (priv->motion_buffer, 3);
  priv->decel_rate = 1.1f;
  priv->button = 1;
  priv->scroll_policy = MX_SCROLL_POLICY_BOTH;
  priv->align_tested = 0;
  priv->in_automatic_scroll = MX_AUTOMATIC_SCROLL_NONE;
  priv->acceleration_factor = 1.0;
  priv->clamp_duration = 250;
  priv->clamp_mode = CLUTTER_EASE_OUT_QUAD;
  priv->snap_on_page = TRUE;

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
  g_signal_connect (self, "button-press-event",
                    G_CALLBACK (button_press_event_cb), self);
  g_signal_connect (self, "touch-event",
                    G_CALLBACK (button_press_event_cb), self);
  g_signal_connect (self, "actor-added",
                    G_CALLBACK (mx_kinetic_scroll_view_actor_added), NULL);
  g_signal_connect (self, "actor-removed",
                    G_CALLBACK (mx_kinetic_scroll_view_actor_removed), NULL);
}

/**
 * mx_kinetic_scroll_view_new:
 *
 * Creates a new #MxKineticScrollView.
 *
 * Returns: a newly allocated #MxKineticScrollView
 *
 * Since: 1.2
 */
ClutterActor *
mx_kinetic_scroll_view_new ()
{
  return g_object_new (MX_TYPE_KINETIC_SCROLL_VIEW, NULL);
}

/**
 * mx_kinetic_scroll_view_stop:
 * @scroll: A #MxKineticScrollView
 *
 * Stops any current movement due to kinetic scrolling.
 *
 * Since: 1.2
 */
void
mx_kinetic_scroll_view_stop (MxKineticScrollView *scroll)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_clear_object (&priv->deceleration_timeline);
    }
}

/**
 * mx_kinetic_scroll_view_set_deceleration:
 * @scroll: A #MxKineticScrollView
 * @rate: The deceleration rate
 *
 * Sets the deceleration rate when a drag is finished on the kinetic
 * scroll-view. This is the value that the momentum is divided by
 * every 60th of a second.
 *
 * Since: 1.2
 */
void
mx_kinetic_scroll_view_set_deceleration (MxKineticScrollView *scroll,
                                         gdouble              rate)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));
  g_return_if_fail (rate > 1.0);

  priv = scroll->priv;

  if (priv->decel_rate != rate)
    {
      priv->decel_rate = rate;
      g_object_notify (G_OBJECT (scroll), "deceleration");
    }
}

/**
 * mx_kinetic_scroll_view_get_deceleration:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the deceleration rate of the kinetic scroll-view.
 *
 * Returns: The deceleration rate of the kinetic scroll-view
 *
 * Since: 1.2
 */
gdouble
mx_kinetic_scroll_view_get_deceleration (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 1.01);
  return scroll->priv->decel_rate;
}

/*
void
mx_kinetic_scroll_view_set_buffer_size (MxKineticScrollView *scroll,
                                        guint                size)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));
  g_return_if_fail (size > 0);

  priv = scroll->priv;
  if (priv->motion_buffer->len != size)
    {
      g_array_set_size (priv->motion_buffer, size);
      g_object_notify (G_OBJECT (scroll), "buffer-size");
    }
}

guint
mx_kinetic_scroll_view_get_buffer_size (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 0);
  return scroll->priv->motion_buffer->len;
}
*/

/**
 * mx_kinetic_scroll_view_set_mouse_button:
 * @scroll: A #MxKineticScrollView
 * @button: A mouse button number
 *
 * Sets the mouse button number used to initiate drag events on the kinetic
 * scroll-view.
 *
 * Since: 1.2
 */
void
mx_kinetic_scroll_view_set_mouse_button (MxKineticScrollView *scroll,
                                         guint32              button)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->button != button)
    {
      priv->button = button;
      g_object_notify (G_OBJECT (scroll), "mouse-button");
    }
}

/**
 * mx_kinetic_scroll_view_get_mouse_button:
 * @scroll: A #MxKineticScrollView
 *
 * Gets the #MxKineticScrollView:mouse-button property
 *
 * Returns: The mouse button number used to initiate drag events on the
 *          kinetic scroll-view
 *
 * Since: 1.2
 */
guint32
mx_kinetic_scroll_view_get_mouse_button (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 0);
  return scroll->priv->button;
}

/**
 * mx_kinetic_scroll_view_set_use_captured:
 * @scroll: A #MxKineticScrollView
 * @use_captured: %TRUE to use captured events
 *
 * Sets whether to use captured events to initiate drag events. This can be
 * used to block events that would initiate scrolling from reaching the child
 * actor.
 *
 * Since: 1.2
 */
void
mx_kinetic_scroll_view_set_use_captured (MxKineticScrollView *scroll,
                                         gboolean             use_captured)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;
  if (priv->use_captured != use_captured)
    {
      priv->use_captured = use_captured;

      g_signal_handlers_disconnect_by_func (scroll,
                                            button_press_event_cb,
                                            scroll);

      if (use_captured)
        {
          g_signal_connect (scroll,
                            "captured-event",
                            G_CALLBACK (button_press_event_cb),
                            scroll);
        }
      else
        {
          g_signal_connect (scroll,
                            "button-press-event",
                            G_CALLBACK (button_press_event_cb),
                            scroll);
          g_signal_connect (scroll,
                            "touch-event",
                            G_CALLBACK (button_press_event_cb),
                            scroll);
        }

      g_object_notify (G_OBJECT (scroll), "use-captured");
    }
}

/**
 * mx_kinetic_scroll_view_get_use_captured:
 * @scroll: A #MxKineticScrollView
 *
 * Gets the #MxKineticScrollView:use-captured property.
 *
 * Returns: %TRUE if captured-events should be used to initiate scrolling
 *
 * Since: 1.2
 */
gboolean
mx_kinetic_scroll_view_get_use_captured (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), FALSE);
  return scroll->priv->use_captured;
}

/**
 * mx_kinetic_scroll_view_set_use_grab:
 * @scroll: A #MxKineticScrollView
 * @use_grab: %TRUE to use grab events
 *
 * Sets whether to use grab events to initiate drag events. This can be
 * used to block events that would initiate scrolling from reaching the child
 * actor.
 *
 * Since: 2.0
 */
void
mx_kinetic_scroll_view_set_use_grab (MxKineticScrollView *scroll,
                                     gboolean             use_grab)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;
  if (priv->use_grab != use_grab)
    {
      priv->use_grab = use_grab;

      g_object_notify (G_OBJECT (scroll), "use-grab");
    }
}

/**
 * mx_kinetic_scroll_view_get_use_grab:
 * @scroll: A #MxKineticScrollView
 *
 * Gets the #MxKineticScrollView:use-grab property.
 *
 * Returns: %TRUE if grab-events should be used to initiate scrolling
 *
 * Since: 2.0
 */
gboolean
mx_kinetic_scroll_view_get_use_grab (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), FALSE);
  return scroll->priv->use_grab;
}

/**
 * mx_kinetic_scroll_view_set_overshoot:
 * @scroll: A #MxKineticScrollView
 * @overshoot: The rate at which the view will decelerate when scrolling beyond
 *             its boundaries.
 *
 * Sets the rate at which the view will decelerate when scrolling beyond its
 * boundaries. The deceleration rate will be multiplied by this value every
 * 60th of a second when the view is scrolling outside of the range set by its
 * adjustments.
 *
 * See mx_kinetic_scroll_view_set_deceleration()
 *
 * Since: 1.2
 */
void
mx_kinetic_scroll_view_set_overshoot (MxKineticScrollView *scroll,
                                      gdouble              overshoot)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;
  if (priv->overshoot != overshoot)
    {
      priv->overshoot = overshoot;
      g_object_notify (G_OBJECT (scroll), "overshoot");
    }
}

/**
 * mx_kinetic_scroll_view_get_overshoot:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the deceleration rate multiplier used when the scroll-view is
 * scrolling beyond its boundaries.
 *
 * Since: 1.2
 */
gdouble
mx_kinetic_scroll_view_get_overshoot (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 0.0);
  return scroll->priv->overshoot;
}

/**
 * mx_kinetic_scroll_view_set_scroll_policy:
 * @scroll: A #MxKineticScrollView
 * @policy: A #MxScrollPolicy
 *
 * Sets the scrolling policy for the kinetic scroll-view. This controls the
 * possible axes of movement, and can affect the minimum size of the widget.
 */
void
mx_kinetic_scroll_view_set_scroll_policy (MxKineticScrollView  *scroll,
                                          MxScrollPolicy policy)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->scroll_policy != policy)
    {
      priv->scroll_policy = policy;

      g_object_notify (G_OBJECT (scroll), "scroll-policy");
    }
}

/**
 * mx_kinetic_scroll_view_get_scroll_policy:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the scrolling policy of the kinetic scroll-view.
 *
 * Returns: A #MxScrollPolicy
 */
MxScrollPolicy
mx_kinetic_scroll_view_get_scroll_policy (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 0);

  return scroll->priv->scroll_policy;
}

/**
 * mx_kinetic_scroll_view_set_acceleration_factor:
 * @scroll: A #MxKineticScrollView
 * @acceleration_factor: The acceleration factor
 *
 * Factor applied to the initial momentum.
 *
 * Since: 1.4
 */
void
mx_kinetic_scroll_view_set_acceleration_factor (MxKineticScrollView *scroll,
                                                gdouble              acceleration_factor)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));
  g_return_if_fail (acceleration_factor >= 0.0);

  priv = scroll->priv;

  if (priv->acceleration_factor != acceleration_factor)
    {
      priv->acceleration_factor = acceleration_factor;
      g_object_notify (G_OBJECT (scroll), "acceleration-factor");
    }
}

/**
 * mx_kinetic_scroll_view_get_acceleration_factor:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the initial acceleration factor of the kinetic scroll-view.
 *
 * Returns: The initial acceleration factor of the kinetic scroll-view
 *
 * Since: 1.4
 */
gdouble
mx_kinetic_scroll_view_get_acceleration_factor (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 1.0);
  return scroll->priv->acceleration_factor;
}

/**
 * mx_kinetic_scroll_view_set_clamp_duration:
 * @scroll: A #MxKineticScrollView
 * @clamp_duration: Clamp duration
 *
 * Duration of the adjustment clamp animation.
 *
 * Since: 1.4
 */
void
mx_kinetic_scroll_view_set_clamp_duration (MxKineticScrollView *scroll,
                                           guint                clamp_duration)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->clamp_duration != clamp_duration)
    {
      priv->clamp_duration = clamp_duration;
      g_object_notify (G_OBJECT (scroll), "clamp-duration");
    }
}

/**
 * mx_kinetic_scroll_view_get_clamp_duration:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the duration of the adjustment clamp animation.
 *
 * Returns: Clamp duration
 *
 * Since: 1.4
 */
guint
mx_kinetic_scroll_view_get_clamp_duration (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), 250);
  return scroll->priv->clamp_duration;
}

/**
 * mx_kinetic_scroll_view_set_clamp_mode:
 * @scroll: A #MxKineticScrollView
 * @clamp_mode: Clamp mode
 *
 * Animation mode to use for the adjustment clamp animation.
 *
 * Since: 1.4
 */
void
mx_kinetic_scroll_view_set_clamp_mode (MxKineticScrollView *scroll,
                                       gulong               clamp_mode)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->clamp_mode != clamp_mode)
    {
      priv->clamp_mode = clamp_mode;
      g_object_notify (G_OBJECT (scroll), "clamp-mode");
    }
}

/**
 * mx_kinetic_scroll_view_get_clamp_mode:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves the animation mode to use for the adjustment clamp animation.
 *
 * Returns: Clamp mode
 *
 * Since: 1.4
 */
gulong
mx_kinetic_scroll_view_get_clamp_mode (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), CLUTTER_EASE_OUT_QUAD);
  return scroll->priv->clamp_mode;
}

/**
 * mx_kinetic_scroll_view_set_clamp_to_center:
 * @scroll: A #MxKineticScrollView
 * @clamp_to_center: Clamp to center
 *
 * Set whether to clamp to step increments based on the center of the page.
 *
 * Since: 1.4
 */
void
mx_kinetic_scroll_view_set_clamp_to_center (MxKineticScrollView *scroll,
                                            gboolean             clamp_to_center)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->clamp_to_center != clamp_to_center)
    {
      priv->clamp_to_center = !!clamp_to_center;
      g_object_notify (G_OBJECT (scroll), "clamp-to-center");
    }
}

/**
 * mx_kinetic_scroll_view_get_clamp_to_center:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves whether to clamp to step increments based on the center of the page.
 *
 * Returns: Clamp to center
 *
 * Since: 1.4
 */
gboolean
mx_kinetic_scroll_view_get_clamp_to_center (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), FALSE);
  return scroll->priv->clamp_to_center;
}

/**
 * mx_kinetic_scroll_view_get_snap_on_page:
 * @scroll: A #MxKineticScrollView
 *
 * Retrieves whether animations end on step increments.
 *
 * Returns: #true if animations end on step increments, #false otherwise.
 *
 * Since: 2.0
 */
gboolean
mx_kinetic_scroll_view_get_snap_on_page (MxKineticScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll), FALSE);
  return scroll->priv->snap_on_page;
}

/**
 * mx_kinetic_scroll_view_set_snap_on_page:
 * @scroll: A #MxKineticScrollView
 * @snap_on_page: #true to stop animations on step increments
 *
 * Set whether to stop animations on step increments.
 *
 * Since: 2.0
 */
void
mx_kinetic_scroll_view_set_snap_on_page (MxKineticScrollView *scroll,
                                         gboolean snap_on_page)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->snap_on_page != snap_on_page)
    {
      priv->snap_on_page = !!snap_on_page;
      g_object_notify (G_OBJECT (scroll), "snap-on-page");
    }
}

static void
_mx_scroll_view_ensure_visible_axis (MxAdjustment *adjust,
                                     gdouble       lower,
                                     gdouble       upper)
{
  gdouble new_value, adjust_lower, adjust_upper, adjust_page_size;

  gboolean changed = FALSE;

  mx_adjustment_get_values (adjust,
                            &new_value,
                            &adjust_lower,
                            &adjust_upper,
                            NULL,
                            NULL,
                            &adjust_page_size);

  /* Sanitise input values */
  lower = CLAMP (lower, adjust_lower, adjust_upper - adjust_page_size);
  upper = CLAMP (upper, adjust_lower + adjust_page_size, adjust_upper);

  /* Ensure the bottom is visible */
  if (new_value + adjust_page_size < upper)
    {
      new_value = upper - adjust_page_size;
      changed = TRUE;
    }

  /* Ensure the top is visible */
  if (lower < new_value)
    {
      new_value = lower;
      changed = TRUE;
    }

  if (changed)
    mx_adjustment_interpolate (adjust, new_value,
                               250, CLUTTER_EASE_OUT_CUBIC);
}

/**
 * mx_kinetic_scroll_view_ensure_visible:
 * @scroll: A #MxKineticScrollView
 * @geometry: The region to make visible
 *
 * Ensures that a given region is visible in the ScrollView, with the top-left
 * taking precedence.
 *
 * Since: 2.0
 */
void
mx_kinetic_scroll_view_ensure_visible (MxKineticScrollView   *scroll,
                                       const ClutterGeometry *geometry)
{
  MxKineticScrollViewPrivate *priv;
  MxAdjustment *hadjustment, *vadjustment;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));
  g_return_if_fail (geometry != NULL);

  priv = scroll->priv;

  mx_scrollable_get_adjustments (MX_SCROLLABLE (priv->child),
                                 &hadjustment,
                                 &vadjustment);

  _mx_scroll_view_ensure_visible_axis (hadjustment,
                                       geometry->x,
                                       geometry->x + geometry->width);
  _mx_scroll_view_ensure_visible_axis (vadjustment,
                                       geometry->y,
                                       geometry->y + geometry->height);
}

/**
 * mx_kinetic_scroll_view_get_input:
 * @scroll: A #MxKineticScrollView
 * @device: (allow-none) (out) (transfer none): a pointer to a #ClutterInputDevice pointer
 * @sequence: (allow-none) (out) (transfer none): a pointer to a #ClutterEventSequence pointer
 *
 * Retrieves informations about the current input device driving the
 * scrolling.
 *
 * Since: 2.0
 */
void
mx_kinetic_scroll_view_get_input (MxKineticScrollView   *scroll,
                                  ClutterInputDevice   **device,
                                  ClutterEventSequence **sequence)
{
  MxKineticScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_KINETIC_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (device != NULL)
    *device = priv->device;

  if (sequence != NULL)
    *sequence = priv->sequence;
}
