/*
 * mx-viewport.c: Viewport actor
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

/**
 * SECTION:mx-viewport
 * @short_description: single child scrollable container
 *
 * #MxViewport allows non-scrollable children to be scrollable by implementing
 * the #MxScrollable and #ClutterContainer interface.
 *
 * Do not use #MxViewport if you need good performance as it does can not
 * be selective about the area of its child that is painted/picked. Therefore
 * if the child is very large or contains a lot of children, you will experience
 * poor performance.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <clutter/clutter.h>

#include "mx-viewport.h"
#include "mx-adjustment.h"
#include "mx-scrollable.h"
#include "mx-private.h"
#include "mx-bin.h"

static void scrollable_interface_init (MxScrollableInterface *iface);

static void scrollable_set_adjustments (MxScrollable *scrollable,
                                        MxAdjustment *hadjustment,
                                        MxAdjustment *vadjustment);

static void scrollable_get_adjustments (MxScrollable  *scrollable,
                                        MxAdjustment **hadjustment,
                                        MxAdjustment **vadjustment);

G_DEFINE_TYPE_WITH_CODE (MxViewport, mx_viewport, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                scrollable_interface_init))

#define VIEWPORT_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_VIEWPORT, \
  MxViewportPrivate))

struct _MxViewportPrivate
{
  gfloat x;
  gfloat y;
  gfloat z;

  MxAdjustment *hadjustment;
  MxAdjustment *vadjustment;

  gboolean sync_adjustments;
};

enum
{
  PROP_0,

  PROP_X_ORIGIN,
  PROP_Y_ORIGIN,
  PROP_Z_ORIGIN,
  PROP_HADJUST,
  PROP_VADJUST,
  PROP_SYNC_ADJUST
};

static void
mx_viewport_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  MxAdjustment *adjustment;

  MxViewportPrivate *priv = MX_VIEWPORT (object)->priv;

  switch (prop_id)
    {
    case PROP_X_ORIGIN:
      g_value_set_float (value, priv->x);
      break;

    case PROP_Y_ORIGIN:
      g_value_set_float (value, priv->y);
      break;

    case PROP_Z_ORIGIN:
      g_value_set_float (value, priv->z);
      break;

    case PROP_HADJUST :
      scrollable_get_adjustments (MX_SCROLLABLE (object), &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST :
      scrollable_get_adjustments (MX_SCROLLABLE (object), NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    case PROP_SYNC_ADJUST :
      g_value_set_boolean (value, priv->sync_adjustments);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_viewport_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  MxViewport *viewport = MX_VIEWPORT (object);
  MxViewportPrivate *priv = viewport->priv;

  switch (prop_id)
    {
    case PROP_X_ORIGIN:
      mx_viewport_set_origin (viewport,
                                g_value_get_float (value),
                                priv->y,
                                priv->z);
      break;

    case PROP_Y_ORIGIN:
      mx_viewport_set_origin (viewport,
                                priv->x,
                                g_value_get_float (value),
                                priv->z);
      break;

    case PROP_Z_ORIGIN:
      mx_viewport_set_origin (viewport,
                                priv->x,
                                priv->y,
                                g_value_get_float (value));
      break;

    case PROP_HADJUST :
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  g_value_get_object (value),
                                  priv->vadjustment);
      break;

    case PROP_VADJUST :
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  priv->hadjustment,
                                  g_value_get_object (value));
      break;

    case PROP_SYNC_ADJUST :
      priv->sync_adjustments = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_viewport_dispose (GObject *gobject)
{
  MxViewportPrivate *priv = MX_VIEWPORT (gobject)->priv;

  if (priv->hadjustment)
    {
      g_object_unref (priv->hadjustment);
      priv->hadjustment = NULL;
    }

  if (priv->vadjustment)
    {
      g_object_unref (priv->vadjustment);
      priv->vadjustment = NULL;
    }

  G_OBJECT_CLASS (mx_viewport_parent_class)->dispose (gobject);
}

static ClutterActor *
get_child_and_natural_size (MxViewport  *self,
                            gfloat   *natural_width,
                            gfloat   *natural_height)
{
  /* MxBin is a single-child container,
    * let it grow as big as it wants. */
  ClutterActor        *child;
  ClutterRequestMode   mode;

  child = mx_bin_get_child (MX_BIN (self));
  if (child)
    {

      g_object_get (G_OBJECT (child), "request-mode", &mode, NULL);
      if (mode == CLUTTER_REQUEST_HEIGHT_FOR_WIDTH)
        {
          clutter_actor_get_preferred_width (child, -1, NULL,
                                             natural_width);
          clutter_actor_get_preferred_height (child, *natural_width, NULL,
                                              natural_height);
        }
      else
        {
          clutter_actor_get_preferred_height (child, -1, NULL,
                                              natural_height);
          clutter_actor_get_preferred_width (child, *natural_height, NULL,
                                             natural_width);
        }

      return child;
    }

    return NULL;
}

static void
mx_viewport_paint (ClutterActor *self)
{
  MxViewportPrivate *priv = MX_VIEWPORT (self)->priv;

  cogl_push_matrix ();

  cogl_translate ((int) priv->x * -1,
                  (int) priv->y * -1,
                  (int) priv->z * -1);

  CLUTTER_ACTOR_CLASS (mx_viewport_parent_class)->paint (self);

  cogl_pop_matrix ();
}

static void
mx_viewport_pick (ClutterActor       *self,
                    const ClutterColor *color)
{
  mx_viewport_paint (self);
}

static void
mx_viewport_allocate (ClutterActor          *self,
                        const ClutterActorBox *box,
                        ClutterAllocationFlags flags)
{
  MxViewportPrivate   *priv = MX_VIEWPORT (self)->priv;
  ClutterActor          *child;
  ClutterActorBox natural_box;
  gfloat     natural_width, natural_height;
  gfloat     available_width, available_height;

  /* Chain up. */
  CLUTTER_ACTOR_CLASS (mx_viewport_parent_class)->
    allocate (self, box, flags);

  available_width = box->x2 - box->x1;
  available_height = box->y2 - box->y1;

  natural_box.x1 = 0;
  natural_box.y1 = 0;

  if (NULL != (child = get_child_and_natural_size (MX_VIEWPORT (self),
                                                   &natural_width,
                                                   &natural_height)))
    {
      natural_box.x2 = natural_width;
      natural_box.y2 = natural_height;
      clutter_actor_allocate (child, &natural_box, flags);
    }
  else
    {
      natural_box.x2 = available_width;
      natural_box.y2 = available_height;
    }

  /* Refresh adjustments */
  if (priv->sync_adjustments)
    {
      gdouble prev_value;

      if (priv->hadjustment)
        {
          g_object_set (G_OBJECT (priv->hadjustment),
                       "lower", 0.0,
                       "page-size", available_width,
                       "upper", natural_width,
                       NULL);

          /* Make sure value is clamped */
          prev_value = mx_adjustment_get_value (priv->hadjustment);
          mx_adjustment_set_value (priv->hadjustment, prev_value);
        }

      if (priv->vadjustment)
        {
          g_object_set (G_OBJECT (priv->vadjustment),
                       "lower", 0.0,
                       "page-size", available_height,
                       "upper", natural_height,
                       NULL);

          prev_value = mx_adjustment_get_value (priv->vadjustment);
          mx_adjustment_set_value (priv->vadjustment, prev_value);
        }
    }
}

static void
mx_viewport_class_init (MxViewportClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxViewportPrivate));

  gobject_class->get_property = mx_viewport_get_property;
  gobject_class->set_property = mx_viewport_set_property;
  gobject_class->dispose = mx_viewport_dispose;

  actor_class->paint = mx_viewport_paint;
  actor_class->pick = mx_viewport_pick;
  actor_class->allocate = mx_viewport_allocate;


  pspec = g_param_spec_float ("x-origin",
                              "X Origin",
                              "Origin's X coordinate in pixels",
                              -G_MAXFLOAT, G_MAXFLOAT, 0,
                              MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_X_ORIGIN, pspec);

  pspec = g_param_spec_float ("y-origin",
                              "Y Origin",
                              "Origin's Y coordinate in pixels",
                              -G_MAXFLOAT, G_MAXFLOAT, 0,
                              MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Y_ORIGIN, pspec);

  pspec = g_param_spec_float ("z-origin",
                              "Z Origin",
                              "Origin's Z coordinate in pixels",
                              -G_MAXFLOAT, G_MAXFLOAT, 0,
                              MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Z_ORIGIN, pspec);

  pspec = g_param_spec_boolean ("sync-adjustments",
                                "Synchronise adjustments",
                                "Whether to synchronise adjustments with "
                                "viewport size",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_SYNC_ADJUST, pspec);

  g_object_class_override_property (gobject_class, PROP_HADJUST, "hadjustment");

  g_object_class_override_property (gobject_class, PROP_VADJUST, "vadjustment");
}

static void
hadjustment_value_notify_cb (MxAdjustment *adjustment,
                             GParamSpec     *pspec,
                             MxViewport   *viewport)
{
  MxViewportPrivate *priv = viewport->priv;
  gdouble value;

  value = mx_adjustment_get_value (adjustment);

  mx_viewport_set_origin (viewport,
                            (float) (value),
                            priv->y,
                            priv->z);
}

static void
vadjustment_value_notify_cb (MxAdjustment *adjustment, GParamSpec *arg1,
                             MxViewport *viewport)
{
  MxViewportPrivate *priv = viewport->priv;
  gdouble value;

  value = mx_adjustment_get_value (adjustment);

  mx_viewport_set_origin (viewport,
                            priv->x,
                            (float) (value),
                            priv->z);
}

static void
scrollable_set_adjustments (MxScrollable *scrollable,
                            MxAdjustment *hadjustment,
                            MxAdjustment *vadjustment)
{
  MxViewportPrivate *priv = MX_VIEWPORT (scrollable)->priv;

  if (hadjustment != priv->hadjustment)
    {
      if (priv->hadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->hadjustment,
                                                hadjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->hadjustment);
        }

      if (hadjustment)
        {
          g_object_ref (hadjustment);
          g_signal_connect (hadjustment, "notify::value",
                            G_CALLBACK (hadjustment_value_notify_cb),
                            scrollable);
        }

      priv->hadjustment = hadjustment;
    }

  if (vadjustment != priv->vadjustment)
    {
      if (priv->vadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->vadjustment,
                                                vadjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->vadjustment);
        }

      if (vadjustment)
        {
          g_object_ref (vadjustment);
          g_signal_connect (vadjustment, "notify::value",
                            G_CALLBACK (vadjustment_value_notify_cb),
                            scrollable);
        }

      priv->vadjustment = vadjustment;
    }
}

static void
scrollable_get_adjustments (MxScrollable *scrollable,
                            MxAdjustment **hadjustment,
                            MxAdjustment **vadjustment)
{
  MxViewportPrivate *priv;
  ClutterActor *actor, *stage;

  g_return_if_fail (MX_IS_VIEWPORT (scrollable));

  priv = ((MxViewport *) scrollable)->priv;

  actor = CLUTTER_ACTOR (scrollable);
  stage = clutter_actor_get_stage (actor);
  if (G_UNLIKELY (stage == NULL))
    stage = clutter_stage_get_default ();

  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          MxAdjustment *adjustment;
          gdouble width, stage_width, increment;

          width = clutter_actor_get_width (actor);
          stage_width = clutter_actor_get_width (stage);
          increment = MAX (1.0, MIN (stage_width, width));

          adjustment = mx_adjustment_new (priv->x,
                                            0,
                                            width,
                                            1.0,
                                            increment,
                                            increment);

          scrollable_set_adjustments (scrollable,
                                      adjustment,
                                      priv->vadjustment);

          *hadjustment = adjustment;
        }
    }

  if (vadjustment)
    {
      if (priv->vadjustment)
        *vadjustment = priv->vadjustment;
      else
        {
          MxAdjustment *adjustment;
          gdouble height, stage_height, increment;

          height = clutter_actor_get_height (actor);
          stage_height = clutter_actor_get_height (stage);
          increment = MAX (1.0, MIN (stage_height, height));

          adjustment = mx_adjustment_new (priv->y,
                                            0,
                                            height,
                                            1.0,
                                            increment,
                                            increment);

          scrollable_set_adjustments (scrollable,
                                      priv->hadjustment,
                                      adjustment);

          *vadjustment = adjustment;
        }
    }
}

static void
scrollable_interface_init (MxScrollableInterface *iface)
{
  iface->set_adjustments = scrollable_set_adjustments;
  iface->get_adjustments = scrollable_get_adjustments;
}

static void
mx_viewport_init (MxViewport *self)
{
  self->priv = VIEWPORT_PRIVATE (self);

  self->priv->sync_adjustments = TRUE;

  g_object_set (G_OBJECT (self), "reactive", FALSE, NULL);
}

MxWidget *
mx_viewport_new (void)
{
  return g_object_new (MX_TYPE_VIEWPORT, NULL);
}

void
mx_viewport_set_origin (MxViewport *viewport,
                          gfloat        x,
                          gfloat        y,
                          gfloat        z)
{
  MxViewportPrivate *priv;

  g_return_if_fail (MX_IS_VIEWPORT (viewport));

  priv = viewport->priv;

  g_object_freeze_notify (G_OBJECT (viewport));

  if (x != priv->x)
    {
      priv->x = x;
      g_object_notify (G_OBJECT (viewport), "x-origin");

      if (priv->hadjustment)
        mx_adjustment_set_value (priv->hadjustment,
                                   (float) (x));
    }

  if (y != priv->y)
    {
      priv->y = y;
      g_object_notify (G_OBJECT (viewport), "y-origin");

      if (priv->vadjustment)
        mx_adjustment_set_value (priv->vadjustment,
                                   (float) (y));
    }

  if (z != priv->z)
    {
      priv->z = z;
      g_object_notify (G_OBJECT (viewport), "z-origin");
    }

  g_object_thaw_notify (G_OBJECT (viewport));

  clutter_actor_queue_redraw (CLUTTER_ACTOR (viewport));
}

void
mx_viewport_get_origin (MxViewport *viewport,
                          gfloat       *x,
                          gfloat       *y,
                          gfloat       *z)
{
  MxViewportPrivate *priv;

  g_return_if_fail (MX_IS_VIEWPORT (viewport));

  priv = viewport->priv;

  if (x)
    *x = priv->x;

  if (y)
    *y = priv->y;

  if (z)
    *z = priv->z;
}

