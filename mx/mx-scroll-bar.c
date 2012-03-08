/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scroll-bar.c: Scroll bar actor
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
#include "mx-frame.h"
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

  guint         handle_min_size;

  /* Trough-click handling. */
  enum { NONE, UP, DOWN }  paging_direction;
  guint             paging_source_id;
  guint             paging_event_no;

  gboolean          stepper_forward;
  guint             stepper_source_id;

  MxOrientation     orientation;
};

enum
{
  PROP_0,

  PROP_ADJUSTMENT,
  PROP_ORIENTATION
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

    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
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

    case PROP_ORIENTATION:
      mx_scroll_bar_set_orientation (bar, g_value_get_enum (value));
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

  if (priv->bw_stepper)
    {
      clutter_actor_unparent (priv->bw_stepper);
      priv->bw_stepper = NULL;
    }

  if (priv->fw_stepper)
    {
      clutter_actor_unparent (priv->fw_stepper);
      priv->fw_stepper = NULL;
    }

  if (priv->trough)
    {
      clutter_actor_unparent (priv->trough);
      priv->trough = NULL;
    }

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

  if (priv->bw_stepper)
    clutter_actor_unmap (priv->bw_stepper);

  if (priv->fw_stepper)
    clutter_actor_unmap (priv->fw_stepper);

  if (priv->trough)
    clutter_actor_unmap (priv->trough);

  if (priv->handle)
    clutter_actor_unmap (priv->handle);
}

static void
mx_scroll_bar_apply_style (MxWidget *widget,
                           MxStyle  *style)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (widget)->priv;

  if (priv->bw_stepper != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->bw_stepper), style);

  if (priv->fw_stepper != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->fw_stepper), style);

  if (priv->trough != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->trough), style);

  if (priv->handle != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->handle), style);
}

static void
mx_scroll_bar_get_preferred_width (ClutterActor *actor,
                                   gfloat        for_height,
                                   gfloat       *min_width,
                                   gfloat       *pref_width)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;
  MxPadding padding;
  gfloat width;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
    {
      gfloat bg_w, h_w, bs_w, fs_w;

      clutter_actor_get_preferred_width (priv->bw_stepper, -1, NULL, &bs_w);
      clutter_actor_get_preferred_width (priv->fw_stepper, -1, NULL, &fs_w);
      clutter_actor_get_preferred_width (priv->trough, -1, NULL, &bg_w);
      clutter_actor_get_preferred_width (priv->handle, -1, NULL, &h_w);

      h_w += padding.left + padding.right;
      bs_w += padding.left + padding.right;
      fs_w += padding.left + padding.right;

      width = MAX (bg_w, MAX (h_w, MAX (bs_w, fs_w)));
    }
  else
    {
      gfloat fs_w, bs_w;

      clutter_actor_get_preferred_width (priv->bw_stepper, -1, NULL, &bs_w);
      clutter_actor_get_preferred_width (priv->fw_stepper, -1, NULL, &fs_w);

      width = padding.left + fs_w + priv->handle_min_size + bs_w
        + padding.right;

    }

  if (min_width)
    *min_width = width;

  if (pref_width)
    *pref_width = width;
}

static void
mx_scroll_bar_get_preferred_height (ClutterActor *actor,
                                    gfloat        for_width,
                                    gfloat       *min_height,
                                    gfloat       *pref_height)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (actor)->priv;
  MxPadding padding;
  gfloat height;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (priv->orientation == MX_ORIENTATION_HORIZONTAL)
    {
      gfloat bg_h, h_h, bs_h, fs_h;

      clutter_actor_get_preferred_height (priv->bw_stepper, -1, NULL, &bs_h);
      clutter_actor_get_preferred_height (priv->fw_stepper, -1, NULL, &fs_h);
      clutter_actor_get_preferred_height (priv->trough, -1, NULL, &bg_h);
      clutter_actor_get_preferred_height (priv->handle, -1, NULL, &h_h);

      h_h += padding.top + padding.bottom;
      bs_h += padding.top + padding.bottom;
      fs_h += padding.top + padding.bottom;

      height = MAX (bg_h, MAX (h_h, MAX (bs_h, fs_h)));
    }
  else
    {
      gfloat fs_h, bs_h;

      clutter_actor_get_preferred_height (priv->bw_stepper, -1, NULL, &bs_h);
      clutter_actor_get_preferred_height (priv->fw_stepper, -1, NULL, &fs_h);

      height = padding.top + fs_h + priv->handle_min_size + bs_h
        + padding.bottom;
    }

  if (min_height)
    *min_height = height;

  if (pref_height)
    *pref_height = height;

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
  CLUTTER_ACTOR_CLASS (mx_scroll_bar_parent_class)->allocate (actor, box,
                                                              flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  /* calculate the child area */
  x = padding.left;
  y = padding.top;
  width = (box->x2 - box->x1) - padding.left - padding.right;
  height = (box->y2 - box->y1) - padding.top - padding.bottom;

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
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
      gfloat handle_size, position, avail_size, handle_pos;
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

      value = mx_adjustment_get_value (priv->adjustment);

      if ((upper == lower)
          || (page_size >= (upper - lower)))
        increment = 1.0;
      else
        increment = page_size / (upper - lower);

      min_size = priv->handle_min_size;

      mx_stylable_get (MX_STYLABLE (actor),
                       "mx-max-size", &max_size,
                       NULL);

      if (upper - lower - page_size <= 0)
        position = 0;
      else
        position = (value - lower) / (upper - lower - page_size);

      if (priv->orientation == MX_ORIENTATION_VERTICAL)
        {
          avail_size = height - stepper_size * 2;
          handle_size = increment * avail_size;
          handle_size = CLAMP (handle_size, min_size, max_size);

          handle_box.x1 = x;
          handle_pos = bw_box.y2 + position * (avail_size - handle_size);
          handle_box.y1 = CLAMP (handle_pos,
                                 bw_box.y2, fw_box.y1 - min_size);

          handle_box.x2 = handle_box.x1 + width;
          handle_box.y2 = CLAMP (handle_pos + handle_size,
                                 bw_box.y2 + min_size, fw_box.y1);
        }
      else
        {
          avail_size = width - stepper_size * 2;
          handle_size = increment * avail_size;
          handle_size = CLAMP (handle_size, min_size, max_size);

          handle_pos = bw_box.x2 + position * (avail_size - handle_size);
          handle_box.x1 = CLAMP (handle_pos,
                                 bw_box.x2, fw_box.x1 - min_size);
          handle_box.y1 = y;

          handle_box.x2 = CLAMP (handle_pos + handle_size,
                                 bw_box.x2 + min_size, fw_box.x1);
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
mx_scroll_bar_style_changed (MxWidget *widget, MxStyleChangedFlags flags)
{
  MxScrollBarPrivate *priv = MX_SCROLL_BAR (widget)->priv;

  mx_stylable_get (MX_STYLABLE (widget),
                   "mx-min-size", &priv->handle_min_size,
                   NULL);

  mx_stylable_style_changed ((MxStylable *) priv->bw_stepper, flags);
  mx_stylable_style_changed ((MxStylable *) priv->fw_stepper, flags);
  mx_stylable_style_changed ((MxStylable *) priv->trough, flags);
  mx_stylable_style_changed ((MxStylable *) priv->handle, flags);

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

  gobject_class = G_OBJECT_CLASS (mx_scroll_bar_parent_class);
  obj = gobject_class->constructor (type, n_properties, properties);

  bar  = MX_SCROLL_BAR (obj);

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
        mx_adjustment_interpolate_relative (priv->adjustment, -step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    case CLUTTER_SCROLL_DOWN:
    case CLUTTER_SCROLL_RIGHT:
      if (value == upper)
        return FALSE;
      else
        mx_adjustment_interpolate_relative (priv->adjustment, step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    }

  return TRUE;
}

static void
mx_scroll_bar_class_init (MxScrollBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxScrollBarPrivate));

  object_class->get_property = mx_scroll_bar_get_property;
  object_class->set_property = mx_scroll_bar_set_property;
  object_class->dispose      = mx_scroll_bar_dispose;
  object_class->constructor  = mx_scroll_bar_constructor;

  actor_class->get_preferred_width  = mx_scroll_bar_get_preferred_width;
  actor_class->get_preferred_height = mx_scroll_bar_get_preferred_height;
  actor_class->allocate       = mx_scroll_bar_allocate;
  actor_class->paint          = mx_scroll_bar_paint;
  actor_class->pick           = mx_scroll_bar_pick;
  actor_class->scroll_event   = mx_scroll_bar_scroll_event;
  actor_class->map            = mx_scroll_bar_map;
  actor_class->unmap          = mx_scroll_bar_unmap;

  widget_class->apply_style = mx_scroll_bar_apply_style;

  g_object_class_install_property
                 (object_class,
                 PROP_ADJUSTMENT,
                 g_param_spec_object ("adjustment",
                                      "Adjustment",
                                      "The adjustment",
                                      MX_TYPE_ADJUSTMENT,
                                      MX_PARAM_READWRITE));

  pspec = g_param_spec_enum ("orientation",
                             "Orientation",
                             "The orientation of the scrollbar",
                             MX_TYPE_ORIENTATION,
                             MX_ORIENTATION_HORIZONTAL,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ORIENTATION, pspec);

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

      pspec = g_param_spec_uint ("mx-min-size",
                                 "Minimum grabber size",
                                 "Minimum size of the scroll grabber, in px",
                                 0, G_MAXUINT, 32,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface,
                                          MX_TYPE_SCROLL_BAR, pspec);

      pspec = g_param_spec_uint ("mx-max-size",
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

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
    size = clutter_actor_get_height (priv->trough)
           - clutter_actor_get_height (priv->handle);
  else
    size = clutter_actor_get_width (priv->trough)
           - clutter_actor_get_width (priv->handle);

  if (size == 0)
    return;

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
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
          mx_stylable_set_style_pseudo_class (MX_STYLABLE (bar->priv->handle),
                                              NULL);
        }
      else
        {
          /* allow the release event to continue to the handle for processing */
          return FALSE;
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

  return FALSE;
}

static gboolean
trough_paging_cb (MxScrollBar *self)
{
  gfloat handle_pos, event_pos, tx, ty;
  gdouble value;
  gdouble page_increment;
  gboolean ret;
  gulong mode;

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

  if (self->priv->orientation == MX_ORIENTATION_VERTICAL)
    handle_pos = clutter_actor_get_y (self->priv->handle);
  else
    handle_pos = clutter_actor_get_x (self->priv->handle);

  clutter_actor_transform_stage_point (CLUTTER_ACTOR (self->priv->trough),
                                       self->priv->move_x,
                                       self->priv->move_y,
                                       &tx, &ty);

  if (self->priv->orientation == MX_ORIENTATION_VERTICAL)
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

  mx_adjustment_interpolate (self->priv->adjustment, value, 250, mode);

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
stepper_move_on (MxScrollBarPrivate *priv,
                 gint                mode)
{
  double value, inc;

  g_object_get (priv->adjustment,
                "step-increment", &inc,
                "value", &value,
                NULL);

  if (priv->stepper_forward)
    value = value + inc;
  else
    value = value - inc;

  mx_adjustment_interpolate (priv->adjustment, value,
                             PAGING_SUBSEQUENT_REPEAT_TIMEOUT, mode);
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

  return FALSE;
}

static gboolean
stepper_button_release_cb (ClutterActor       *actor,
                           ClutterButtonEvent *event,
                           MxScrollBar        *self)
{

  if (self->priv->stepper_source_id)
    {
      g_source_remove (self->priv->stepper_source_id);
      self->priv->stepper_source_id = 0;
    }

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
  mx_stylable_set_style_class (MX_STYLABLE (self->priv->bw_stepper),
                               "backward-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->bw_stepper),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->bw_stepper, "button-press-event",
                    G_CALLBACK (stepper_button_press_event_cb), self);
  g_signal_connect (self->priv->bw_stepper, "button-release-event",
                    G_CALLBACK (stepper_button_release_cb), self);
  g_signal_connect (self->priv->bw_stepper, "leave-event",
                    G_CALLBACK (stepper_button_release_cb), self);

  self->priv->fw_stepper = (ClutterActor *) mx_button_new ();
  mx_stylable_set_style_class (MX_STYLABLE (self->priv->fw_stepper),
                               "forward-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->fw_stepper),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->fw_stepper, "button-press-event",
                    G_CALLBACK (stepper_button_press_event_cb), self);
  g_signal_connect (self->priv->fw_stepper, "button-release-event",
                    G_CALLBACK (stepper_button_release_cb), self);
  g_signal_connect (self->priv->fw_stepper, "leave-event",
                    G_CALLBACK (stepper_button_release_cb), self);

  self->priv->trough = mx_frame_new ();
  clutter_actor_set_reactive ((ClutterActor *) self->priv->trough, TRUE);
  mx_stylable_set_style_class (MX_STYLABLE (self->priv->trough), "htrough");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->trough),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->trough, "button-press-event",
                    G_CALLBACK (trough_button_press_event_cb), self);
  g_signal_connect (self->priv->trough, "button-release-event",
                    G_CALLBACK (trough_button_release_event_cb), self);
  g_signal_connect (self->priv->trough, "leave-event",
                    G_CALLBACK (trough_leave_event_cb), self);

  self->priv->handle = (ClutterActor *) mx_button_new ();
  mx_stylable_set_style_class (MX_STYLABLE (self->priv->handle), "hhandle");
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->handle),
                            CLUTTER_ACTOR (self));
  g_signal_connect (self->priv->handle, "button-press-event",
                    G_CALLBACK (handle_button_press_event_cb), self);

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_scroll_bar_style_changed), NULL);
  g_signal_connect (self, "notify::reactive",
                    G_CALLBACK (mx_scroll_bar_notify_reactive), NULL);
}

/**
 * mx_scroll_bar_new:
 *
 * Create a new #MxScrollBar
 *
 * Returns: a new #MxScrollBar
 */
ClutterActor *
mx_scroll_bar_new (void)
{
  return g_object_new (MX_TYPE_SCROLL_BAR, NULL);
}

/**
 * mx_scroll_bar_new_with_adjustment:
 * @adjustment: an #MxAdjustment
 *
 * Create a new #MxScrollBar with the given adjustment set
 *
 * Returns: a new #MxScrollBar
 */
ClutterActor *
mx_scroll_bar_new_with_adjustment (MxAdjustment *adjustment)
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

/**
 * mx_scroll_bar_get_adjustment:
 * @bar: a #MxScrollBar
 *
 * Gets the adjustment object that stores the current position
 * of the scrollbar.
 *
 * Return value: (transfer none): the adjustment
 */
MxAdjustment *
mx_scroll_bar_get_adjustment (MxScrollBar *bar)
{
  g_return_val_if_fail (MX_IS_SCROLL_BAR (bar), NULL);

  return bar->priv->adjustment;
}


MxOrientation
mx_scroll_bar_get_orientation (MxScrollBar *bar)
{
  g_return_val_if_fail (MX_IS_SCROLL_BAR (bar), MX_ORIENTATION_HORIZONTAL);

  return bar->priv->orientation;
}

void
mx_scroll_bar_set_orientation (MxScrollBar   *bar,
                               MxOrientation  orientation)
{
  MxScrollBarPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_BAR (bar));

  priv = bar->priv;

  if (orientation != priv->orientation)
    {

      priv->orientation = orientation;

      if (bar->priv->orientation)
        {
          mx_stylable_set_style_class (MX_STYLABLE (priv->bw_stepper),
                                       "up-stepper");
          mx_stylable_set_style_class (MX_STYLABLE (priv->fw_stepper),
                                       "down-stepper");
          mx_stylable_set_style_class (MX_STYLABLE (priv->handle),
                                       "vhandle");
          mx_stylable_set_style_class (MX_STYLABLE (priv->trough),
                                       "vtrough");
        }
      else
        {
          mx_stylable_set_style_class (MX_STYLABLE (priv->fw_stepper),
                                       "forward-stepper");
          mx_stylable_set_style_class (MX_STYLABLE (priv->bw_stepper),
                                       "backward-stepper");
          mx_stylable_set_style_class (MX_STYLABLE (priv->handle),
                                       "hhandle");
          mx_stylable_set_style_class (MX_STYLABLE (priv->trough),
                                       "htrough");
        }

      clutter_actor_queue_relayout (CLUTTER_ACTOR (bar));

      g_object_notify (G_OBJECT (bar), "orientation");
    }
}
