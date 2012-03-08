/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-progress-bar.c: Progress bar widget
 *
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:mx-progress-bar
 * @short_description: visual representation of progress
 *
 * #MxProgressBar visually represents the progress of an action or a value in
 * a range.
 *
 */

#include "mx-progress-bar.h"
#include "mx-progress-bar-fill.h"
#include "mx-texture-frame.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxProgressBar, mx_progress_bar, MX_TYPE_WIDGET)

#define PROGRESS_BAR_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_PROGRESS_BAR, MxProgressBarPrivate))

struct _MxProgressBarPrivate
{
  ClutterActor *fill;
  gdouble       progress;
};

enum
{
  PROP_0,

  PROP_PROGRESS
};

static void
mx_progress_bar_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MxProgressBar *self = MX_PROGRESS_BAR (object);

  switch (property_id)
    {
    case PROP_PROGRESS:
      g_value_set_double (value, mx_progress_bar_get_progress (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_progress_bar_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  MxProgressBar *self = MX_PROGRESS_BAR (object);

  switch (property_id)
    {
    case PROP_PROGRESS:
      mx_progress_bar_set_progress (self, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_progress_bar_dispose (GObject *object)
{
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (object)->priv;

  if (priv->fill)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->fill));
      priv->fill = NULL;
    }

  G_OBJECT_CLASS (mx_progress_bar_parent_class)->dispose (object);
}

static void
mx_progress_bar_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_progress_bar_parent_class)->finalize (object);
}

static void
mx_progress_bar_paint (ClutterActor *actor)
{
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_progress_bar_parent_class)->paint (actor);

  if (priv->progress)
    clutter_actor_paint (priv->fill);
}

static void
mx_progress_bar_allocate_fill (MxProgressBar         *self,
                               const ClutterActorBox *box,
                               ClutterAllocationFlags flags)
{
  ClutterActorBox box_data;
  MxProgressBarPrivate *priv = self->priv;

  if (!box)
    {
      clutter_actor_get_allocation_box (CLUTTER_ACTOR (self), &box_data);
      box = &box_data;
    }

  if (priv->progress)
    {
      ClutterActorBox child_box;
      MxPadding padding;

      mx_widget_get_padding (MX_WIDGET (self), &padding);

      child_box.x1 = padding.left;
      child_box.y1 = padding.top;
      child_box.y2 = (box->y2 - box->y1) - padding.bottom;
      child_box.x2 = ((box->x2 - box->x1 - padding.left - padding.right) *
                      priv->progress) + padding.left;

      clutter_actor_allocate (priv->fill, &child_box, flags);
    }
}

static void
mx_progress_bar_allocate (ClutterActor          *actor,
                          const ClutterActorBox *box,
                          ClutterAllocationFlags flags)
{
  MxProgressBar *self = MX_PROGRESS_BAR (actor);

  CLUTTER_ACTOR_CLASS (mx_progress_bar_parent_class)->
  allocate (actor, box, flags);

  mx_progress_bar_allocate_fill (self, box, flags);
}

static void
mx_progress_bar_map (ClutterActor *actor)
{
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (actor)->priv;
  CLUTTER_ACTOR_CLASS (mx_progress_bar_parent_class)->map (actor);
  clutter_actor_map (priv->fill);
}

static void
mx_progress_bar_unmap (ClutterActor *actor)
{
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (actor)->priv;

  if (priv->fill)
    clutter_actor_unmap (priv->fill);

  CLUTTER_ACTOR_CLASS (mx_progress_bar_parent_class)->unmap (actor);
}

static void
mx_progress_bar_apply_style (MxWidget *widget,
                             MxStyle  *style)
{
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (widget)->priv;

  if (priv->fill != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->fill), style);
}

static void
mx_progress_bar_get_preferred_width (ClutterActor *actor,
                                     gfloat        for_height,
                                     gfloat       *min_width_p,
                                     gfloat       *nat_width_p)
{
  MxPadding padding;
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (actor)->priv;
  gfloat height;

  clutter_actor_get_preferred_width (priv->fill,
                                     for_height,
                                     min_width_p,
                                     nat_width_p);

  clutter_actor_get_preferred_height (priv->fill,
                                      -1,
                                      &height,
                                      NULL);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  /* Add an arbitrary amount to the width for preferred width, so that the
   * indicator is visible and can display some values */
  if (nat_width_p)
    *nat_width_p += padding.left + padding.right + height * 4;
}

static void
mx_progress_bar_get_preferred_height (ClutterActor *actor,
                                      gfloat        for_width,
                                      gfloat       *min_height_p,
                                      gfloat       *nat_height_p)
{
  MxPadding padding;
  MxProgressBarPrivate *priv = MX_PROGRESS_BAR (actor)->priv;

  clutter_actor_get_preferred_height (priv->fill,
                                      for_width,
                                      min_height_p,
                                      nat_height_p);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;
  if (nat_height_p)
    *nat_height_p += padding.top + padding.bottom;
}

static void
mx_progress_bar_class_init (MxProgressBarClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxProgressBarPrivate));

  object_class->get_property = mx_progress_bar_get_property;
  object_class->set_property = mx_progress_bar_set_property;
  object_class->dispose = mx_progress_bar_dispose;
  object_class->finalize = mx_progress_bar_finalize;

  actor_class->paint = mx_progress_bar_paint;
  actor_class->get_preferred_width = mx_progress_bar_get_preferred_width;
  actor_class->get_preferred_height = mx_progress_bar_get_preferred_height;
  actor_class->allocate = mx_progress_bar_allocate;
  actor_class->map = mx_progress_bar_map;
  actor_class->unmap = mx_progress_bar_unmap;

  widget_class->apply_style = mx_progress_bar_apply_style;

  pspec = g_param_spec_double ("progress",
                               "Progress",
                               "Progress",
                               0.0, 1.0, 0.0, MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PROGRESS, pspec);
}

static void
mx_progress_bar_init (MxProgressBar *self)
{
  MxProgressBarPrivate *priv = self->priv = PROGRESS_BAR_PRIVATE (self);

  priv->fill = g_object_new (MX_TYPE_PROGRESS_BAR_FILL,
                             "clip-to-allocation", TRUE,
                             NULL);
  clutter_actor_set_parent (priv->fill, CLUTTER_ACTOR (self));
}

/**
 * mx_progress_bar_new:
 *
 * Create a new progress bar
 *
 * Returns: a new #MxProgressBar
 */
ClutterActor *
mx_progress_bar_new (void)
{
  return g_object_new (MX_TYPE_PROGRESS_BAR, NULL);
}

/**
 * mx_progress_bar_set_progress:
 * @bar: A #MxProgressBar
 * @progress: A value between 0.0 and 1.0
 *
 * Set the progress of the progress bar
 *
 */
void
mx_progress_bar_set_progress (MxProgressBar *bar,
                              gdouble        progress)
{
  MxProgressBarPrivate *priv;

  g_return_if_fail (MX_IS_PROGRESS_BAR (bar));

  priv = bar->priv;

  if (priv->progress != progress)
    {
      priv->progress = progress;
      mx_progress_bar_allocate_fill (bar, NULL, 0);
      clutter_actor_queue_redraw (CLUTTER_ACTOR (bar));
      g_object_notify (G_OBJECT (bar), "progress");
    }
}

/**
 * mx_progress_bar_get_progress:
 * @bar: A #MxProgressBar
 *
 * Get the progress of the progress bar
 *
 * Returns: A value between 0.0 and 1.0
 */
gdouble
mx_progress_bar_get_progress (MxProgressBar *bar)
{
  g_return_val_if_fail (MX_IS_PROGRESS_BAR (bar), 0.0);

  return bar->priv->progress;
}
