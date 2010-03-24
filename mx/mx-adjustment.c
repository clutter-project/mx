/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-adjustment.c: Adjustment object
 *
 * Copyright (C) 2008 OpenedHand
 * Copyright (c) 2009, 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@openedhand.com>, inspired by GtkAdjustment
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

/**
 * SECTION:mx-adjustment
 * @short_description: A GObject representing an adjustable bounded value
 *
 * The #MxAdjustment object represents a range of values bounded between a
 * minimum and maximum, together with step and page increments and a page size.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>
#include <clutter/clutter.h>

#include "mx-adjustment.h"
#include "mx-marshal.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxAdjustment, mx_adjustment, G_TYPE_OBJECT)

#define ADJUSTMENT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_ADJUSTMENT, MxAdjustmentPrivate))

struct _MxAdjustmentPrivate
{
  /* Do not sanity-check values while constructing,
   * not all properties may be set yet. */
  guint is_constructing : 1;

  gdouble  lower;
  gdouble  upper;
  gdouble  value;
  gdouble  step_increment;
  gdouble  page_increment;
  gdouble  page_size;

  /* For signal emission/notification */
  guint lower_source;
  guint upper_source;
  guint value_source;
  guint step_inc_source;
  guint page_inc_source;
  guint page_size_source;
  guint changed_source;

  /* For interpolation */
  ClutterTimeline *interpolation;
  gdouble          old_position;
  gdouble          new_position;
  ClutterAlpha    *interpolate_alpha;

  /* For elasticity */
  gboolean      elastic;
  guint         bounce_source;
  ClutterAlpha *bounce_alpha;
};

enum
{
  PROP_0,

  PROP_LOWER,
  PROP_UPPER,
  PROP_VALUE,
  PROP_STEP_INC,
  PROP_PAGE_INC,
  PROP_PAGE_SIZE,

  PROP_ELASTIC,
};

enum
{
  CHANGED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static gboolean _mx_adjustment_set_lower          (MxAdjustment *adjustment,
                                                   gdouble       lower);
static gboolean _mx_adjustment_set_upper          (MxAdjustment *adjustment,
                                                   gdouble       upper);
static gboolean _mx_adjustment_set_step_increment (MxAdjustment *adjustment,
                                                   gdouble       step);
static gboolean _mx_adjustment_set_page_increment (MxAdjustment *adjustment,
                                                   gdouble       page);
static gboolean _mx_adjustment_set_page_size      (MxAdjustment *adjustment,
                                                   gdouble       size);
static void mx_adjustment_clamp_page (MxAdjustment *adjustment,
                                      gdouble       lower,
                                      gdouble       upper);

static void
mx_adjustment_constructed (GObject *object)
{
  GObjectClass *g_class;
  MxAdjustment *self = MX_ADJUSTMENT (object);

  g_class = G_OBJECT_CLASS (mx_adjustment_parent_class);
  /* The docs say we're suppose to chain up, but would crash without
   * some extra care. */
  if (g_class && g_class->constructed &&
      g_class->constructed != mx_adjustment_constructed)
    {
      g_class->constructed (object);
    }

  MX_ADJUSTMENT (self)->priv->is_constructing = FALSE;
  mx_adjustment_clamp_page (self, self->priv->lower, self->priv->upper);
}

static void
mx_adjustment_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  MxAdjustmentPrivate *priv = MX_ADJUSTMENT (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LOWER:
      g_value_set_double (value, priv->lower);
      break;

    case PROP_UPPER:
      g_value_set_double (value, priv->upper);
      break;

    case PROP_VALUE:
      g_value_set_double (value, priv->value);
      break;

    case PROP_STEP_INC:
      g_value_set_double (value, priv->step_increment);
      break;

    case PROP_PAGE_INC:
      g_value_set_double (value, priv->page_increment);
      break;

    case PROP_PAGE_SIZE:
      g_value_set_double (value, priv->page_size);
      break;

    case PROP_ELASTIC:
      g_value_set_boolean (value, priv->elastic);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_adjustment_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  MxAdjustment *adj = MX_ADJUSTMENT (gobject);

  switch (prop_id)
    {
    case PROP_LOWER:
      mx_adjustment_set_lower (adj, g_value_get_double (value));
      break;

    case PROP_UPPER:
      mx_adjustment_set_upper (adj, g_value_get_double (value));
      break;

    case PROP_VALUE:
      mx_adjustment_set_value (adj, g_value_get_double (value));
      break;

    case PROP_STEP_INC:
      mx_adjustment_set_step_increment (adj, g_value_get_double (value));
      break;

    case PROP_PAGE_INC:
      mx_adjustment_set_page_increment (adj, g_value_get_double (value));
      break;

    case PROP_PAGE_SIZE:
      mx_adjustment_set_page_size (adj, g_value_get_double (value));
      break;

    case PROP_ELASTIC:
      mx_adjustment_set_elastic (adj, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
stop_interpolation (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->interpolation)
    {
      clutter_timeline_stop (priv->interpolation);
      g_object_unref (priv->interpolation);
      priv->interpolation = NULL;

      if (priv->bounce_alpha)
        {
          g_object_unref (priv->bounce_alpha);
          priv->bounce_alpha = NULL;
        }
    }

  if (priv->bounce_source)
    {
      g_source_remove (priv->bounce_source);
      priv->bounce_source = 0;
    }
}

static void
mx_adjustment_remove_idle (guint *source)
{
  if (*source)
    {
      g_source_remove (*source);
      *source = 0;
    }
}

static void
mx_adjustment_dispose (GObject *object)
{
  MxAdjustmentPrivate *priv = MX_ADJUSTMENT (object)->priv;

  stop_interpolation (MX_ADJUSTMENT (object));

  /* Remove idle handlers */
  mx_adjustment_remove_idle (&priv->value_source);
  mx_adjustment_remove_idle (&priv->lower_source);
  mx_adjustment_remove_idle (&priv->upper_source);
  mx_adjustment_remove_idle (&priv->page_inc_source);
  mx_adjustment_remove_idle (&priv->step_inc_source);
  mx_adjustment_remove_idle (&priv->page_size_source);
  mx_adjustment_remove_idle (&priv->changed_source);

  if (priv->interpolate_alpha)
    {
      g_object_unref (priv->interpolate_alpha);
      priv->interpolate_alpha = NULL;
    }

  G_OBJECT_CLASS (mx_adjustment_parent_class)->dispose (object);
}

static void
mx_adjustment_class_init (MxAdjustmentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxAdjustmentPrivate));

  object_class->constructed = mx_adjustment_constructed;
  object_class->get_property = mx_adjustment_get_property;
  object_class->set_property = mx_adjustment_set_property;
  object_class->dispose = mx_adjustment_dispose;

  g_object_class_install_property (object_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
                                                        "Lower",
                                                        "Lower bound",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
                                                        "Upper",
                                                        "Upper bound",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_VALUE,
                                   g_param_spec_double ("value",
                                                        "Value",
                                                        "Current value",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_STEP_INC,
                                   g_param_spec_double ("step-increment",
                                                        "Step Increment",
                                                        "Step increment",
                                                        0.0,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_PAGE_INC,
                                   g_param_spec_double ("page-increment",
                                                        "Page Increment",
                                                        "Page increment",
                                                        0.0,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_PAGE_SIZE,
                                   g_param_spec_double ("page-size",
                                                        "Page Size",
                                                        "Page size",
                                                        0.0,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        MX_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_ELASTIC,
                                   g_param_spec_boolean ("elastic",
                                                         "Elastic",
                                                         "Make interpolation "
                                                         "behave in an "
                                                         "'elastic' way and "
                                                         "stop clamping value.",
                                                         FALSE,
                                                         MX_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  /**
   * MxAdjustment::changed:
   *
   * Emitted when any of the adjustment values have changed
   */
  signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxAdjustmentClass, changed),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
mx_adjustment_init (MxAdjustment *self)
{
  self->priv = ADJUSTMENT_PRIVATE (self);

  self->priv->is_constructing = TRUE;
}

MxAdjustment *
mx_adjustment_new (void)
{
  return g_object_new (MX_TYPE_ADJUSTMENT, NULL);
}

MxAdjustment *
mx_adjustment_new_with_values (gdouble value,
                               gdouble lower,
                               gdouble upper,
                               gdouble step_increment,
                               gdouble page_increment,
                               gdouble page_size)
{
  return g_object_new (MX_TYPE_ADJUSTMENT,
                       "value", value,
                       "lower", lower,
                       "upper", upper,
                       "step-increment", step_increment,
                       "page-increment", page_increment,
                       "page-size", page_size,
                       NULL);
}

gdouble
mx_adjustment_get_value (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv;

  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0);

  priv = adjustment->priv;

  return priv->value;
}

static gboolean
mx_adjustment_value_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->value_source = 0;

  g_object_notify (G_OBJECT (adjustment), "value");

  return FALSE;
}

static gboolean
mx_adjustment_lower_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->lower_source = 0;
  g_object_notify (G_OBJECT (adjustment), "lower");

  return FALSE;
}

static gboolean
mx_adjustment_upper_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->upper_source = 0;
  g_object_notify (G_OBJECT (adjustment), "upper");

  return FALSE;
}

static gboolean
mx_adjustment_step_inc_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->step_inc_source = 0;
  g_object_notify (G_OBJECT (adjustment), "step-increment");

  return FALSE;
}

static gboolean
mx_adjustment_page_inc_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->page_inc_source = 0;
  g_object_notify (G_OBJECT (adjustment), "page-increment");

  return FALSE;
}

static gboolean
mx_adjustment_page_size_notify_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->page_size_source = 0;
  g_object_notify (G_OBJECT (adjustment), "page-size");

  return FALSE;
}

static gboolean
mx_adjustment_emit_changed_cb (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->changed_source = 0;
  g_signal_emit (adjustment, signals[CHANGED], 0);

  return FALSE;
}

void
mx_adjustment_set_value (MxAdjustment *adjustment,
                         gdouble       value)
{
  MxAdjustmentPrivate *priv;

  g_return_if_fail (MX_IS_ADJUSTMENT (adjustment));

  priv = adjustment->priv;

  /* Defer clamp until after construction. */
  if (!priv->is_constructing)
    {
      if (!priv->elastic)
        value = CLAMP (value,
                       priv->lower,
                       MAX (priv->lower, priv->upper - priv->page_size));
    }

  if (priv->value != value)
    {
      stop_interpolation (adjustment);

      priv->value = value;

      g_object_notify (G_OBJECT (adjustment), "value");
    }
}

static void
mx_adjustment_clamp_page (MxAdjustment *adjustment,
                          gdouble       lower,
                          gdouble       upper)
{
  MxAdjustmentPrivate *priv;
  gboolean changed;

  g_return_if_fail (MX_IS_ADJUSTMENT (adjustment));

  priv = adjustment->priv;

  stop_interpolation (adjustment);

  lower = CLAMP (lower, priv->lower, priv->upper - priv->page_size);
  upper = CLAMP (upper, priv->lower + priv->page_size, priv->upper);

  changed = FALSE;

  if (priv->value + priv->page_size > upper)
    {
      priv->value = upper - priv->page_size;
      changed = TRUE;
    }

  if (priv->value < lower)
    {
      priv->value = lower;
      changed = TRUE;
    }

  if (changed && !priv->value_source)
    priv->value_source =
      g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                       (GSourceFunc)mx_adjustment_value_notify_cb,
                       adjustment,
                       NULL);
}

static void
mx_adjustment_emit_changed (MxAdjustment *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (!priv->changed_source)
    priv->changed_source =
      g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                       (GSourceFunc)mx_adjustment_emit_changed_cb,
                       adjustment,
                       NULL);
}

static gboolean
_mx_adjustment_set_lower (MxAdjustment *adjustment,
                          gdouble       lower)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->lower != lower)
    {
      priv->lower = lower;

      mx_adjustment_emit_changed (adjustment);

      if (!priv->lower_source)
        priv->lower_source =
          g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                           (GSourceFunc)mx_adjustment_lower_notify_cb,
                           adjustment,
                           NULL);

      /* Defer clamp until after construction. */
      if (!priv->is_constructing)
        mx_adjustment_clamp_page (adjustment, priv->lower, priv->upper);

      return TRUE;
    }

  return FALSE;
}

void
mx_adjustment_set_lower (MxAdjustment *adjustment,
                         gdouble       lower)
{
  _mx_adjustment_set_lower (adjustment, lower);
}

gdouble
mx_adjustment_get_lower (MxAdjustment *adjustment)
{
  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0.0);
  return adjustment->priv->lower;
}

static gboolean
_mx_adjustment_set_upper (MxAdjustment *adjustment,
                          gdouble       upper)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->upper != upper)
    {
      priv->upper = upper;

      mx_adjustment_emit_changed (adjustment);

      if (!priv->upper_source)
        priv->upper_source =
          g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                           (GSourceFunc)mx_adjustment_upper_notify_cb,
                           adjustment,
                           NULL);

      /* Defer clamp until after construction. */
      if (!priv->is_constructing)
        mx_adjustment_clamp_page (adjustment, priv->lower, priv->upper);

      return TRUE;
    }

  return FALSE;
}

void
mx_adjustment_set_upper (MxAdjustment *adjustment,
                         gdouble       upper)
{
  _mx_adjustment_set_upper (adjustment, upper);
}

gdouble
mx_adjustment_get_upper (MxAdjustment *adjustment)
{
  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0.0);
  return adjustment->priv->upper;
}

static gboolean
_mx_adjustment_set_step_increment (MxAdjustment *adjustment,
                                   gdouble       step)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->step_increment != step)
    {
      priv->step_increment = step;

      mx_adjustment_emit_changed (adjustment);

      if (!priv->step_inc_source)
        priv->step_inc_source =
          g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                           (GSourceFunc)mx_adjustment_step_inc_notify_cb,
                           adjustment,
                           NULL);

      return TRUE;
    }

  return FALSE;
}

void
mx_adjustment_set_step_increment (MxAdjustment *adjustment,
                                  gdouble       step)
{
  _mx_adjustment_set_step_increment (adjustment, step);
}

gdouble
mx_adjustment_get_step_increment (MxAdjustment *adjustment)
{
  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0.0);
  return adjustment->priv->step_increment;
}

static gboolean
_mx_adjustment_set_page_increment (MxAdjustment *adjustment,
                                   gdouble       page)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->page_increment != page)
    {
      priv->page_increment = page;

      mx_adjustment_emit_changed (adjustment);

      if (!priv->page_inc_source)
        priv->page_inc_source =
          g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                           (GSourceFunc)mx_adjustment_page_inc_notify_cb,
                           adjustment,
                           NULL);

      return TRUE;
    }

  return FALSE;
}

void
mx_adjustment_set_page_increment (MxAdjustment *adjustment,
                                  gdouble       page)
{
  _mx_adjustment_set_page_increment (adjustment, page);
}

gdouble
mx_adjustment_get_page_increment (MxAdjustment *adjustment)
{
  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0.0);
  return adjustment->priv->page_increment;
}

static gboolean
_mx_adjustment_set_page_size (MxAdjustment *adjustment,
                              gdouble       size)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->page_size != size)
    {
      priv->page_size = size;

      mx_adjustment_emit_changed (adjustment);

      if (!priv->page_size_source)
        priv->page_size_source =
          g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
                           (GSourceFunc)mx_adjustment_page_size_notify_cb,
                           adjustment,
                           NULL);

      /* Well explicitely clamp after construction. */
      if (!priv->is_constructing)
        mx_adjustment_clamp_page (adjustment, priv->lower, priv->upper);

      return TRUE;
    }

  return FALSE;
}

void
mx_adjustment_set_page_size (MxAdjustment *adjustment,
                             gdouble       size)
{
  _mx_adjustment_set_page_size (adjustment, size);
}

gdouble
mx_adjustment_get_page_size (MxAdjustment *adjustment)
{
  g_return_val_if_fail (MX_IS_ADJUSTMENT (adjustment), 0.0);
  return adjustment->priv->page_size;
}

void
mx_adjustment_set_values (MxAdjustment *adjustment,
                          gdouble       value,
                          gdouble       lower,
                          gdouble       upper,
                          gdouble       step_increment,
                          gdouble       page_increment,
                          gdouble       page_size)
{
  MxAdjustmentPrivate *priv;
  gboolean emit_changed = FALSE;

  g_return_if_fail (MX_IS_ADJUSTMENT (adjustment));
  g_return_if_fail (page_size >= 0 && page_size <= G_MAXDOUBLE);
  g_return_if_fail (step_increment >= 0 && step_increment <= G_MAXDOUBLE);
  g_return_if_fail (page_increment >= 0 && page_increment <= G_MAXDOUBLE);

  priv = adjustment->priv;

  stop_interpolation (adjustment);

  emit_changed = FALSE;

  g_object_freeze_notify (G_OBJECT (adjustment));

  emit_changed |= _mx_adjustment_set_lower (adjustment, lower);
  emit_changed |= _mx_adjustment_set_upper (adjustment, upper);
  emit_changed |= _mx_adjustment_set_step_increment (adjustment,
                                                     step_increment);
  emit_changed |= _mx_adjustment_set_page_increment (adjustment,
                                                     page_increment);
  emit_changed |= _mx_adjustment_set_page_size (adjustment, page_size);

  if (value != priv->value)
    {
      mx_adjustment_set_value (adjustment, value);
      emit_changed = TRUE;
    }

  if (emit_changed)
    mx_adjustment_emit_changed (adjustment);

  g_object_thaw_notify (G_OBJECT (adjustment));
}

void
mx_adjustment_get_values (MxAdjustment *adjustment,
                          gdouble      *value,
                          gdouble      *lower,
                          gdouble      *upper,
                          gdouble      *step_increment,
                          gdouble      *page_increment,
                          gdouble      *page_size)
{
  MxAdjustmentPrivate *priv;

  g_return_if_fail (MX_IS_ADJUSTMENT (adjustment));

  priv = adjustment->priv;

  if (lower)
    *lower = priv->lower;

  if (upper)
    *upper = priv->upper;

  if (value)
    *value = mx_adjustment_get_value (adjustment);

  if (step_increment)
    *step_increment = priv->step_increment;

  if (page_increment)
    *page_increment = priv->page_increment;

  if (page_size)
    *page_size = priv->page_size;
}

static void
interpolation_new_frame_cb (ClutterTimeline *timeline,
                            guint            msecs,
                            MxAdjustment    *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  priv->interpolation = NULL;

  if (priv->elastic)
    {
      gdouble progress = clutter_alpha_get_alpha (priv->bounce_alpha) / 1.0;
      gdouble dx = priv->old_position
                   + (priv->new_position - priv->old_position)
                   * progress;

      mx_adjustment_set_value (adjustment, dx);
    }
  else
    mx_adjustment_set_value (adjustment,
                             priv->old_position +
                             (priv->new_position - priv->old_position) *
                             clutter_alpha_get_alpha (priv->interpolate_alpha));

  priv->interpolation = timeline;
}

static void
interpolation_completed_cb (ClutterTimeline *timeline,
                            MxAdjustment    *adjustment)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  stop_interpolation (adjustment);
  mx_adjustment_set_value (adjustment, priv->new_position);
}

/* Note, there's super-optimal code that does a similar thing in
 * clutter-alpha.c
 *
 * Tried this instead of CLUTTER_ALPHA_SINE_INC, but I think SINE_INC looks
 * better. Leaving code here in case this is revisited.
 */
/*
   static guint32
   bounce_alpha_func (ClutterAlpha *alpha,
                   gpointer      user_data)
   {
   ClutterFixed progress, angle;
   ClutterTimeline *timeline = clutter_alpha_get_timeline (alpha);

   progress = clutter_timeline_get_progressx (timeline);
   angle = clutter_qmulx (CFX_PI_2 + CFX_PI_4/2, progress);

   return clutter_sinx (angle) +
    (CFX_ONE - clutter_sinx (CFX_PI_2 + CFX_PI_4/2));
   }
 */

void
mx_adjustment_interpolate (MxAdjustment *adjustment,
                           gdouble       value,
                           guint         duration,
                           gulong        mode)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  stop_interpolation (adjustment);

  if (duration <= 1)
    {
      mx_adjustment_set_value (adjustment, value);
      return;
    }

  priv->old_position = priv->value;
  priv->new_position = value;

  priv->interpolation = clutter_timeline_new (duration);

  if (priv->elastic)
    priv->bounce_alpha = clutter_alpha_new_full (priv->interpolation,
                                                 CLUTTER_LINEAR);

  if (priv->interpolate_alpha)
    g_object_unref (priv->interpolate_alpha);

  priv->interpolate_alpha = clutter_alpha_new_full (priv->interpolation,
                                                    mode);

  g_signal_connect (priv->interpolation,
                    "new-frame",
                    G_CALLBACK (interpolation_new_frame_cb),
                    adjustment);
  g_signal_connect (priv->interpolation,
                    "completed",
                    G_CALLBACK (interpolation_completed_cb),
                    adjustment);

  clutter_timeline_start (priv->interpolation);
}

void
mx_adjustment_interpolate_relative (MxAdjustment *adjustment,
                                    gdouble       offset,
                                    guint         duration,
                                    gulong        mode)
{
  MxAdjustmentPrivate *priv = adjustment->priv;

  if (priv->interpolation)
    offset += priv->new_position;
  else
    offset += priv->value;

  mx_adjustment_interpolate (adjustment,
                             offset,
                             duration,
                             mode);
}

gboolean
mx_adjustment_get_elastic (MxAdjustment *adjustment)
{
  return adjustment->priv->elastic;
}

void
mx_adjustment_set_elastic (MxAdjustment *adjustment,
                           gboolean      elastic)
{
  adjustment->priv->elastic = elastic;
}

gboolean
mx_adjustment_clamp (MxAdjustment *adjustment,
                     gboolean      interpolate,
                     guint         duration,
                     gulong        mode)
{
  MxAdjustmentPrivate *priv = adjustment->priv;
  gdouble dest = priv->value;

  if (priv->value < priv->lower)
    dest = priv->lower;

  if (priv->value > priv->upper - priv->page_size)
    dest = priv->upper - priv->page_size;

  if (dest != priv->value)
    {
      if (interpolate)
        mx_adjustment_interpolate (adjustment, dest, duration, mode);
      else
        mx_adjustment_set_value (adjustment, dest);

      return TRUE;
    }

  return FALSE;
}
