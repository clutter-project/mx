/*
 * mx-scroll-bar.c: Scroll bar actor
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
 *
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

/**
 * SECTION:mx-scroll-bar
 * @short_description: a user interface element to control scrollable areas.
 *
 * The #MxScrollBar allows users to scroll scrollable actors, either by
 * the step or page amount, or by manually dragging the handle.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

#include "mx-scroll-bar.h"
#include "mx-bin.h"
#include "mx-marshal.h"
#include "mx-stylable.h"
#include "mx-enum-types.h"
#include "mx-private.h"
#include "mx-button.h"

static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxScrollBar, mx_scroll_bar, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

#define MX_SCROLL_BAR_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_SCROLL_BAR, MxScrollBarPrivate))

#define PAGING_INITIAL_REPEAT_TIMEOUT 500
#define PAGING_SUBSEQUENT_REPEAT_TIMEOUT 200

struct _MxScrollBarPrivate
{
  MxAdjustment *adjustment;

  gulong        capture_handler;
  gfloat        x_origin;
  gfloat        y_origin;

  ClutterActor *bw_stepper;
  ClutterActor *fw_stepper;
  ClutterActor *trough;
  ClutterActor *handle;

  gfloat        move_x;
  gfloat        move_y;

  /* Trough-click handling. */
  enum { NONE, UP, DOWN }  paging_direction;
  guint             paging_source_id;
  guint             paging_event_no;

  gboolean          stepper_forward;
  guint             stepper_source_id;

  ClutterAnimation *paging_animation;

  gboolean          vertical;
};

enum
{
  PROP_0,

  PROP_ADJUSTMENT,
  PROP_VERTICAL
};

enum
{
  SCROLL_START,
  SCROLL_STOP,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static gboolean
handle_button_press_event_cb (ClutterActor       *actor,
                              ClutterButtonEvent *event,
                              MxScrollBar        *bar);

static void
mx_scroll_bar_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (gobject)->priv;

  switch (prop_id)
    {
    case PROP_ADJUSTMENT:
      g_value_set_object (value, priv->adjustment);
      break;

    case PROP_VERTICAL:
      g_value_set_boolean (value, priv->vertical);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_scroll_bar_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  MxScrollBar *bar = MX_SCROLL_BAR (gobject);

  switch (prop_id)
    {
    case PROP_ADJUSTMENT:
      mx_scroll_bar_set_adjustment (bar, g_value_get_object (value));
      break;

    case PROP_VERTICAL:
      bar->priv->vertical = g_value_get_boolean (value);
      if (bar->priv->vertical)
        {
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->bw_stepper),
                                  "up-stepper");
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->fw_stepper),
                                  "down-stepper");
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->handle),
                                  "vhandle");
        }
      else
        {
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->fw_stepper),
                                  "forward-stepper");
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->bw_stepper),
                                  "backward-stepper");
          clutter_actor_set_name (CLUTTER_ACTOR (bar->priv->handle),
                                  "hhandle");
        }
      clutter_actor_queue_relayout ((ClutterActor*) gobject);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_scroll_bar_dispose (GObject *gobject)
{
  MxScrollBar *bar = MX_SCROLL_BAR (gobject);
  MxScrollBarPrivate *priv = bar->priv;

  if (priv->adjustment)
    mx_scroll_bar_set_adjustment (bar, NULL);

  if (priv->handle)
    {
      g_signal_handlers_disconnect_by_func (priv->handle,
                                            G_CALLBACK (handle_button_press_event_cb),
                                            bar);
      clutter_actor_unparent (priv->handle);
      priv->handle = NULL;
    }

  clutter_actor_unparent (priv->bw_stepper);
  priv->bw_stepper = NULL;

  clutter_actor_unparent (priv->fw_stepper);
  priv->fw_stepper = NULL;

  clutter_actor_unparent (priv->trough);
  priv->trough = NULL;

  G_OBJECT_CLASS (mx_scroll_bar_parent_class)->dispose (gobject);
}

static void
mx_scroll_bar_paint (ClutterActor *actor)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->paint (actor);

  clutter_actor_paint (priv->bw_stepper);

  clutter_actor_paint (priv->fw_stepper);

  clutter_actor_paint (priv->trough);

  if (priv->handle && CLUTTER_ACTOR_IS_VISIBLE (priv->handle))
    clutter_actor_paint (priv->handle);
}

static void
mx_scroll_bar_pick (ClutterActor       *actor,
                    const ClutterColor *pick_color)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->pick (actor, pick_color);

  clutter_actor_paint (priv->bw_stepper);
  clutter_actor_paint (priv->fw_stepper);
  clutter_actor_paint (priv->trough);

  if (priv->handle && priv->adjustment)
    clutter_actor_paint (priv->handle);
}

static void
mx_scroll_bar_map (ClutterActor *actor)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->map (actor);

  clutter_actor_map (priv->bw_stepper);
  clutter_actor_map (priv->fw_stepper);
  clutter_actor_map (priv->trough);

  if (priv->handle)
    clutter_actor_map (priv->handle);
}

static void
mx_scroll_bar_unmap (ClutterActor *actor)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->bw_stepper);
  clutter_actor_unmap (priv->fw_stepper);
  clutter_actor_unmap (priv->trough);

  if (priv->handle)
    clutter_actor_unmap (priv->handle);
}

static void
mx_scroll_bar_allocate (ClutterActor          *actor,
                        const ClutterActorBox *box,
                        ClutterAllocationFlags flags)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;
  MxPadding padding;
  ClutterActorBox bw_box, fw_box, trough_box;
  gfloat x, y, width, height, stepper_size;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->
  allocate (actor, box, flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  /* calculate the child area */
  x = padding.left;
  y = padding.top;
  width = (box->x2 - box->x1) - padding.left - padding.right;
  height = (box->y2 - box->y1) - padding.top - padding.bottom;

  if (priv->vertical)
    {
      stepper_size = width;

      /* Backward stepper */
      bw_box.x1 = x;
      bw_box.y1 = y;
      bw_box.x2 = bw_box.x1 + stepper_size;
      bw_box.y2 = bw_box.y1 + stepper_size;
      clutter_actor_allocate (priv->bw_stepper, &bw_box, flags);

      /* Forward stepper */
      fw_box.x1 = x;
      fw_box.y1 = y + height - stepper_size;
      fw_box.x2 = fw_box.x1 + stepper_size;
      fw_box.y2 = fw_box.y1 + stepper_size;
      clutter_actor_allocate (priv->fw_stepper, &fw_box, flags);

      /* Trough */
      trough_box.x1 = x;
      trough_box.y1 = y + stepper_size;
      trough_box.x2 = x + width;
      trough_box.y2 = y + height - stepper_size;
      clutter_actor_allocate (priv->trough, &trough_box, flags);

    }
  else
    {
      stepper_size = height;

      /* Backward stepper */
      bw_box.x1 = x;
      bw_box.y1 = y;
      bw_box.x2 = bw_box.x1 + stepper_size;
      bw_box.y2 = bw_box.y1 + stepper_size;
      clutter_actor_allocate (priv->bw_stepper, &bw_box, flags);

      /* Forward stepper */
      fw_box.x1 = x + width - stepper_size;
      fw_box.y1 = y;
      fw_box.x2 = fw_box.x1 + stepper_size;
      fw_box.y2 = fw_box.y1 + stepper_size;
      clutter_actor_allocate (priv->fw_stepper, &fw_box, flags);

      /* Trough */
      trough_box.x1 = x + stepper_size;
      trough_box.y1 = y;
      trough_box.x2 = x + width - stepper_size;
      trough_box.y2 = y + height;
      clutter_actor_allocate (priv->trough, &trough_box, flags);
    }


  if (priv->adjustment)
    {
      gfloat handle_size, position, avail_size;
      gdouble value, lower, upper, page_size, increment;
      ClutterActorBox handle_box = { 0, };
      guint min_size, max_size;

      mx_adjustment_get_values (priv->adjustment,
                                &value,
                                &lower,
                                &upper,
                                NULL,
                                NULL,
                                &page_size);

      if ((upper == lower)
          || (page_size >= (upper - lower)))
        increment = 1.0;
      else
        increment = page_size / (upper - lower);

      mx_stylable_get (MX_STYLABLE (actor),
                       "min-size", &min_size,
                       "max-size", &max_size,
                       NULL);

      if (upper - lower - page_size <= 0)
        position = 0;
      else
        position = (value - lower) / (upper - lower - page_size);

      if (priv->vertical)
        {
          avail_size = height - stepper_size * 2;
          handle_size = increment * avail_size;
          handle_size = CLAMP (handle_size, min_size, max_size);

          handle_box.x1 = x;
          handle_box.y1 = bw_box.y2 + position * (avail_size - handle_size);

          handle_box.x2 = handle_box.x1 + width;
          handle_box.y2 = handle_box.y1 + handle_size;
        }
      else
        {
          avail_size = width - stepper_size * 2;
          handle_size = increment * avail_size;
          handle_size = CLAMP (handle_size, min_size, max_size);

          handle_box.x1 = bw_box.x2 + position * (avail_size - handle_size);
          handle_box.y1 = y;

          handle_box.x2 = handle_box.x1 + handle_size;
          handle_box.y2 = handle_box.y1 + height;
        }

      /* snap to pixel */
      handle_box.x1 = (int) handle_box.x1;
      handle_box.y1 = (int) handle_box.y1;
      handle_box.x2 = (int) handle_box.x2;
      handle_box.y2 = (int) handle_box.y2;

      clutter_actor_allocate (priv->handle,
                              &handle_box,
                              flags);
    }
}

static void
mx_scroll_bar_style_changed (MxWidget *widget)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (widget)->priv;

  mx_stylable_changed ((MxStylable *) priv->bw_stepper);
  mx_stylable_changed ((MxStylable *) priv->fw_stepper);
  mx_stylable_changed ((MxStylable *) priv->trough);
  mx_stylable_changed ((MxStylable *) priv->handle);

}

static void
bar_reactive_notify_cb (GObject    *gobject,
                        GParamSpec *arg1,
                        gpointer    user_data)
{
  MxScrollBar *bar = MX_SCROLL_BAR (gobject);

  clutter_actor_set_reactive (bar->priv->handle,
                              clutter_actor_get_reactive (CLUTTER_ACTOR (bar)));
}

static GObject*
mx_scroll_bar_constructor (GType                  type,
                           guint                  n_properties,
                           GObjectConstructParam *properties)
{
  GObjectClass *gobject_class;
  GObject *obj;
  MxScrollBar *bar;
  MxScrollBarPrivate *priv;

  gobject_class = G_OBJECT_CLASS (mx_scroll_bar_parent_class);
  obj = gobject_class->constructor (type, n_properties, properties);

  bar  = MX_SCROLL_BAR (obj);
  priv = MX_SCROLL_BAR_GET_PRIVATE (bar);

  g_signal_connect (bar, "notify::reactive",
                    G_CALLBACK (bar_reactive_notify_cb), NULL);

  return obj;
}

static gboolean
mx_scroll_bar_scroll_event (ClutterActor       *actor,
                            ClutterScrollEvent *event)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;
  gdouble lower, step, upper, value;

  if (priv->adjustment)
    {
      g_object_get (priv->adjustment,
                    "lower", &lower,
                    "step-increment", &step,
                    "upper", &upper,
                    "value", &value,
                    NULL);
    }
  else
    {
      return FALSE;
    }

  switch (event->direction)
    {
    case CLUTTER_SCROLL_UP:
    case CLUTTER_SCROLL_LEFT:
      if (value == lower)
        return FALSE;
      else
        mx_adjustment_set_value (priv->adjustment, value - step);
      break;
    case CLUTTER_SCROLL_DOWN:
    case CLUTTER_SCROLL_RIGHT:
      if (value == upper)
        return FALSE;
      else
        mx_adjustment_set_value (priv->adjustment, value + step);
      break;
    }

  return TRUE;
}

static void
mx_scroll_bar_class_init (MxScrollBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxScrollBarPrivate));

  object_class->get_property = mx_scroll_bar_get_property;
  object_class->set_property = mx_scroll_bar_set_property;
  object_class->dispose      = mx_scroll_bar_dispose;
  object_class->constructor  = mx_scroll_bar_constructor;

  actor_class->allocate       = mx_scroll_bar_allocate;
  actor_class->paint          = mx_scroll_bar_paint;
  actor_class->pick           = mx_scroll_bar_pick;
  actor_class->scroll_event   = mx_scroll_bar_scroll_event;
  actor_class->map            = mx_scroll_bar_map;
  actor_class->unmap          = mx_scroll_bar_unmap;

  g_object_class_install_property
                 (object_class,
                 PROP_ADJUSTMENT,
                 g_param_spec_object ("adjustment",
                                      "Adjustment",
                                      "The adjustment",
                                      MX_TYPE_ADJUSTMENT,
                                      MX_PARAM_READWRITE));

  pspec = g_param_spec_boolean ("vertical",
                                "Vertical Orientation",
                                "Vertical Orientation",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_VERTICAL, pspec);

  signals[SCROLL_START] =
    g_signal_new ("scroll-start",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxScrollBarClass, scroll_start),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  signals[SCROLL_STOP] =
    g_signal_new ("scroll-stop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxScrollBarClass, scroll_stop),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("min-size",
                                 "Minimum grabber size",
                                 "Minimum size of the scroll grabber, in px",
                                 0, G_MAXUINT, 32,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface,
                                          MX_TYPE_SCROLL_BAR, pspec);

      pspec = g_param_spec_uint ("max-size",
                                 "Maximum grabber size",
                                 "Maximum size of the scroll grabber, in px",
                                 0, G_MAXINT16, G_MAXINT16,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface,
                                          MX_TYPE_SCROLL_BAR, pspec);
    }
}

static void
move_slider (MxScrollBar *bar,
             gfloat       x,
             gfloat       y)
{
  MxScrollBarPrivate *priv = bar->priv;
  gdouble position, lower, upper, page_size;
  gfloat ux, uy, pos, size;

  if (!priv->adjustment)
    return;

  if (!clutter_actor_transform_stage_point (priv->trough, x, y, &ux, &uy))
    return;

  if (priv->vertical)
    size = clutter_actor_get_height (priv->trough)
           - clutter_actor_get_height (priv->handle);
  else
    size = clutter_actor_get_width (priv->trough)
           - clutter_actor_get_width (priv->handle);

  if (size == 0)
    return;

  if (priv->vertical)
    pos = uy - priv->y_origin;
  else
    pos = ux - priv->x_origin;
  pos = CLAMP (pos, 0, size);

  mx_adjustment_get_values (priv->adjustment,
                            NULL,
                            &lower,
                            &upper,
                            NULL,
                            NULL,
                            &page_size);

  position = ((pos / size)
              * (upper - lower - page_size))
             + lower;

  mx_adjustment_set_value (priv->adjustment, position);
}

static gboolean
handle_capture_event_cb (ClutterActor *trough,
                         ClutterEvent *event,
                         MxScrollBar  *bar)
{
  if (clutter_event_type (event) == CLUTTER_MOTION)
    {
      move_slider (bar,
                   ((ClutterMotionEvent*) event)->x,
                   ((ClutterMotionEvent*) event)->y);
    }
  else if (clutter_event_type (event) == CLUTTER_BUTTON_RELEASE
           && ((ClutterButtonEvent*) event)->button == 1)
    {
      ClutterActor *stage, *target;

      stage = clutter_actor_get_stage(bar->priv->trough);

      if (bar->priv->capture_handler)
        {
          g_signal_handler_disconnect (stage, bar->priv->capture_handler);
          bar->priv->capture_handler = 0;
        }

      clutter_set_motion_events_enabled (TRUE);
      g_signal_emit (bar, signals[SCROLL_STOP], 0);

      /* check if the mouse pointer has left the handle during the drag and
       * remove the hover state if it has */
      target = clutter_stage_get_actor_at_pos ((ClutterStage*) stage,
                                               CLUTTER_PICK_REACTIVE,
                                               ((ClutterButtonEvent*) event)->x,
                                               ((ClutterButtonEvent*) event)->y);
      if (target != bar->priv->handle)
        {
          mx_widget_set_style_pseudo_class ((MxWidget*) bar->priv->handle, NULL);
        }


    }

  return TRUE;
}

static gboolean
handle_button_press_event_cb (ClutterActor       *actor,
                              ClutterButtonEvent *event,
                              MxScrollBar        *bar)
{
  MxScrollBarPrivate *priv = bar->priv;

  if (event->button != 1)
    return FALSE;

  if (!clutter_actor_transform_stage_point (priv->handle,
                                            event->x,
                                            event->y,
                                            &priv->x_origin,
                                            &priv->y_origin))
    return FALSE;

  /* Account for the scrollbar-trough-handle nesting. */
  priv->x_origin += clutter_actor_get_x (priv->trough);
  priv->y_origin += clutter_actor_get_y (priv->trough);

  /* Turn off picking for motion events */
  clutter_set_motion_events_enabled (FALSE);

  priv->capture_handler = g_signal_connect_after (
    clutter_actor_get_stage (priv->trough),
    "captured-event",
    G_CALLBACK (handle_capture_event_cb),
    bar);
  g_signal_emit (bar, signals[SCROLL_START], 0);

  return TRUE;
}

static void
animation_completed_cb (ClutterAnimation   *animation,
                        MxScrollBarPrivate *priv)
{
  g_object_unref (priv->paging_animation);
  priv->paging_animation = NULL;
}

static gboolean
trough_paging_cb (MxScrollBar *self)
{
  gfloat handle_pos, event_pos, tx, ty;
  gdouble value;
  gdouble page_increment;
  gboolean ret;

  gulong mode;
  ClutterAnimation *a;
  GValue v = { 0, };
  ClutterTimeline *t;

  if (self->priv->paging_event_no == 0)
    {
      /* Scroll on after initial timeout. */
      mode = CLUTTER_EASE_OUT_CUBIC;
      ret = FALSE;
      self->priv->paging_event_no = 1;
      self->priv->paging_source_id = g_timeout_add (
        PAGING_INITIAL_REPEAT_TIMEOUT,
        (GSourceFunc) trough_paging_cb,
        self);
    }
  else if (self->priv->paging_event_no == 1)
    {
      /* Scroll on after subsequent timeout. */
      ret = FALSE;
      mode = CLUTTER_EASE_IN_CUBIC;
      self->priv->paging_event_no = 2;
      self->priv->paging_source_id = g_timeout_add (
        PAGING_SUBSEQUENT_REPEAT_TIMEOUT,
        (GSourceFunc) trough_paging_cb,
        self);
    }
  else
    {
      /* Keep scrolling. */
      ret = TRUE;
      mode = CLUTTER_LINEAR;
      self->priv->paging_event_no++;
    }

  /* Do the scrolling */
  mx_adjustment_get_values (self->priv->adjustment,
                            &value, NULL, NULL,
                            NULL, &page_increment, NULL);

  if (self->priv->vertical)
    handle_pos = clutter_actor_get_y (self->priv->handle);
  else
    handle_pos = clutter_actor_get_x (self->priv->handle);

  clutter_actor_transform_stage_point (CLUTTER_ACTOR (self->priv->trough),
                                       self->priv->move_x,
                                       self->priv->move_y,
                                       &tx, &ty);

  if (self->priv->vertical)
    event_pos = ty;
  else
    event_pos = tx;

  if (event_pos > handle_pos)
    {
      if (self->priv->paging_direction == NONE)
        {
          /* Remember direction. */
          self->priv->paging_direction = DOWN;
        }
      if (self->priv->paging_direction == UP)
        {
          /* Scrolled far enough. */
          return FALSE;
        }
      value += page_increment;
    }
  else
    {
      if (self->priv->paging_direction == NONE)
        {
          /* Remember direction. */
          self->priv->paging_direction = UP;
        }
      if (self->priv->paging_direction == DOWN)
        {
          /* Scrolled far enough. */
          return FALSE;
        }
      value -= page_increment;
    }

  if (self->priv->paging_animation)
    {
      clutter_animation_completed (self->priv->paging_animation);
    }

  /* FIXME: Creating a new animation for each scroll is probably not the best
  * idea, but it's a lot less involved than extenind the current animation */
  a = self->priv->paging_animation = g_object_new (CLUTTER_TYPE_ANIMATION,
                                                   "object", self->priv->adjustment,
                                                   "duration", PAGING_SUBSEQUENT_REPEAT_TIMEOUT,
                                                   "mode", mode,
                                                   NULL);
  g_value_init (&v, G_TYPE_DOUBLE);
  g_value_set_double (&v, value);
  clutter_animation_bind (self->priv->paging_animation, "value", &v);
  t = clutter_animation_get_timeline (self->priv->paging_animation);
  g_signal_connect (a, "completed", G_CALLBACK (animation_completed_cb),
                    self->priv);
  clutter_timeline_start (t);

  return ret;
}

static gboolean
trough_button_press_event_cb (ClutterActor       *actor,
                              ClutterButtonEvent *event,
                              MxScrollBar        *self)
{
  g_return_val_if_fail (self, FALSE);

  if (event->button != 1)
    return FALSE;

  if (self->priv->adjustment == NULL)
    return FALSE;

  self->priv->move_x = event->x;
  self->priv->move_y = event->y;
  self->priv->paging_direction = NONE;
  self->priv->paging_event_no = 0;
  trough_paging_cb (self);

  return TRUE;
}

static gboolean
trough_button_release_event_cb (ClutterActor       *actor,
                                ClutterButtonEvent *event,
                                MxScrollBar        *self)
{
  if (event->button != 1)
    return FALSE;

  if (self->priv->paging_source_id)
    {
      g_source_remove (self->priv->paging_source_id);
      self->priv->paging_source_id = 0;
    }

  return TRUE;
}

static gboolean
trough_leave_event_cb (ClutterActor *actor,
                       ClutterEvent *event,
                       MxScrollBar  *self)
{
  if (self->priv->paging_source_id)
    {
      g_source_remove (self->priv->paging_source_id);
      self->priv->paging_source_id = 0;
      return TRUE;
    }

  return FALSE;
}

static void
stepper_animation_completed_cb (ClutterAnimation *a,
                                gpointer          data)
{
  g_object_unref (a);
}

static void
stepper_move_on (MxScrollBarPrivate *priv,
                 gint                mode)
{
  ClutterAnimation *a;
  ClutterTimeline *t;
  GValue v = { 0, };
  double value, inc;

  a = g_object_new (CLUTTER_TYPE_ANIMATION,
                    "object", priv->adjustment,
                    "duration", PAGING_SUBSEQUENT_REPEAT_TIMEOUT,
                    "mode", mode,
                    NULL);

  g_signal_connect (a, "completed", G_CALLBACK (stepper_animation_completed_cb),
                    NULL);

  g_object_get (priv->adjustment,
                "step-increment", &inc,
                "value", &value,
                NULL);

  if (priv->stepper_forward)
    value = value + inc;
  else
    value = value - inc;

  g_value_init (&v, G_TYPE_DOUBLE);
  g_value_set_double (&v, value);
  clutter_animation_bind (a, "value", &v);

  t = clutter_animation_get_timeline (a);
  clutter_timeline_start (t);
}

static gboolean
stepper_button_subsequent_timeout (MxScrollBarPrivate *priv)
{
  stepper_move_on (priv, CLUTTER_LINEAR);

  return TRUE;
}

static gboolean
stepper_button_repeat_timeout (MxScrollBarPrivate *priv)
{
  priv->stepper_source_id = 0;

  stepper_move_on (priv, CLUTTER_EASE_IN_CUBIC);

  priv->stepper_source_id = g_timeout_add (PAGING_SUBSEQUENT_REPEAT_TIMEOUT,
                                           (GSourceFunc)
                                           stepper_button_subsequent_timeout,
                                           priv);
  return FALSE;
}

static gboolean
stepper_button_press_event_cb (ClutterActor       *actor,
                               ClutterButtonEvent *event,
                               MxScrollBar        *bar)
{
  MxScrollBarPrivate *priv = bar->priv;

  if (event->button != 1)
    return FALSE;

  if (bar->priv->adjustment == NULL)
    return FALSE;

  bar->priv->stepper_forward = (actor == priv->fw_stepper);

  stepper_move_on (priv, CLUTTER_EASE_OUT_CUBIC);

  priv->stepper_source_id = g_timeout_add (PAGING_INITIAL_REPEAT_TIMEOUT,
                                           (GSourceFunc)
                                           stepper_button_repeat_timeout,
                                           priv);

  return TRUE;
}

static gboolean
stepper_button_release_cb (ClutterActor       *actor,
                           ClutterButtonEvent *event,
                           MxScrollBar        *self)
{
  if (event->button != 1)
    return FALSE;

  g_source_remove (self->priv->stepper_source_id);

  return FALSE;
}

static void
mx_scroll_bar_notify_reactive (MxScrollBar *self)
{
  MxScrollBarPrivate *priv = self->priv;

  gboolean reactive = CLUTTER_ACTOR_IS_REACTIVE (self);

  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->bw_stepper), reactive);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->fw_stepper), reactive);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->trough), reactive);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->handle), reactive);
}

static void
mx_scroll_bar_init (MxScrollBar *self)
{
  self->priv = MX_SCROLL_BAR_GET_PRIVATE (self);

  self->priv->bw_stepper = (ClutterActor *) mx_button_new ();
  clutter_actor_set_name (CLUTTER_ACTOR (self->priv->bw_stepper),
                          "backward-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->bw_stepper),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->bw_stepper, "button-press-event",
                    G_CALLBACK (stepper_button_press_event_cb), self);
  g_signal_connect (self->priv->bw_stepper, "button-release-event",
                    G_CALLBACK (stepper_button_release_cb), self);

  self->priv->fw_stepper = (ClutterActor *) mx_button_new ();
  clutter_actor_set_name (CLUTTER_ACTOR (self->priv->fw_stepper),
                          "forward-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->fw_stepper),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->fw_stepper, "button-press-event",
                    G_CALLBACK (stepper_button_press_event_cb), self);
  g_signal_connect (self->priv->fw_stepper, "button-release-event",
                    G_CALLBACK (stepper_button_release_cb), self);

  self->priv->trough = (ClutterActor *) mx_bin_new ();
  clutter_actor_set_reactive ((ClutterActor *) self->priv->trough, TRUE);
  clutter_actor_set_name (CLUTTER_ACTOR (self->priv->trough), "trough");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->trough),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->trough, "button-press-event",
                    G_CALLBACK (trough_button_press_event_cb), self);
  g_signal_connect (self->priv->trough, "button-release-event",
                    G_CALLBACK (trough_button_release_event_cb), self);
  g_signal_connect (self->priv->trough, "leave-event",
                    G_CALLBACK (trough_leave_event_cb), self);

  self->priv->handle = (ClutterActor *) mx_button_new ();
  clutter_actor_set_name (CLUTTER_ACTOR (self->priv->handle), "hhandle");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->handle),
                            self->priv->trough);
  g_signal_connect (self->priv->handle, "button-press-event",
                    G_CALLBACK (handle_button_press_event_cb), self);

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_scroll_bar_style_changed), NULL);
  g_signal_connect (self, "notify::reactive",
                    G_CALLBACK (mx_scroll_bar_notify_reactive), NULL);
}

MxWidget *
mx_scroll_bar_new (MxAdjustment *adjustment)
{
  return g_object_new (MX_TYPE_SCROLL_BAR,
                       "adjustment", adjustment,
                       NULL);
}

void
mx_scroll_bar_set_adjustment (MxScrollBar  *bar,
                              MxAdjustment *adjustment)
{
  MxScrollBarPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_BAR (bar));

  priv = bar->priv;
  if (priv->adjustment)
    {
      g_signal_handlers_disconnect_by_func (priv->adjustment,
                                            clutter_actor_queue_relayout,
                                            bar);
      g_signal_handlers_disconnect_by_func (priv->adjustment,
                                            clutter_actor_queue_relayout,
                                            bar);
      g_object_unref (priv->adjustment);
      priv->adjustment = NULL;
    }

  if (adjustment)
    {
      priv->adjustment = g_object_ref (adjustment);

      g_signal_connect_swapped (priv->adjustment, "notify::value",
                                G_CALLBACK (clutter_actor_queue_relayout),
                                bar);
      g_signal_connect_swapped (priv->adjustment, "changed",
                                G_CALLBACK (clutter_actor_queue_relayout),
                                bar);

      clutter_actor_queue_relayout (CLUTTER_ACTOR (bar));
    }
}

MxAdjustment *
mx_scroll_bar_get_adjustment (MxScrollBar *bar)
{
  g_return_val_if_fail (MX_IS_SCROLL_BAR (bar), NULL);

  return bar->priv->adjustment;
}

