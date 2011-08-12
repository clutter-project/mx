/*
 * mx-floating-widget: always-on-top base actor
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#include "mx-floating-widget.h"

G_DEFINE_ABSTRACT_TYPE (MxFloatingWidget, mx_floating_widget, MX_TYPE_WIDGET)

#define FLOATING_WIDGET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_FLOATING_WIDGET, MxFloatingWidgetPrivate))

struct _MxFloatingWidgetPrivate
{
  ClutterActor *stage;

  CoglMatrix paint_matrix;
  CoglMatrix pick_matrix;

  gulong pick_handler;
  gulong paint_handler;
};


static void
mx_floating_widget_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_floating_widget_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_floating_widget_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_floating_widget_parent_class)->dispose (object);
}

static void
mx_floating_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_floating_widget_parent_class)->finalize (object);
}

static void
stage_weak_notify (MxFloatingWidget *widget,
                   ClutterStage     *stage)
{
  widget->priv->stage = NULL;
}

static void
mx_floating_widget_pick_from_stage (ClutterActor       *stage,
                                    const ClutterColor *color,
                                    MxFloatingWidget   *widget)
{
  MxFloatingWidgetClass *klass;
  MxFloatingWidgetPrivate *priv = widget->priv;
  gboolean has_clip;

  if (!CLUTTER_ACTOR_IS_REACTIVE (widget))
    return;

  klass = MX_FLOATING_WIDGET_CLASS (G_OBJECT_GET_CLASS (widget));

  cogl_push_matrix ();

  cogl_set_modelview_matrix (&(priv->pick_matrix));

  has_clip = clutter_actor_has_clip (CLUTTER_ACTOR (widget));

  if (has_clip)
    {
      gfloat x, y, w, h;
      clutter_actor_get_clip (CLUTTER_ACTOR (widget), &x, &y, &w, &h);
      cogl_clip_push_rectangle (x, y, x + w, y + h);
    }

  if (klass->floating_pick)
    klass->floating_pick (CLUTTER_ACTOR (widget), color);

  if (has_clip)
    cogl_clip_pop ();

  cogl_pop_matrix ();
}

static void
mx_floating_widget_paint_from_stage (ClutterActor     *stage,
                                     MxFloatingWidget *widget)
{
  MxFloatingWidgetClass *klass;
  MxFloatingWidgetPrivate *priv = widget->priv;
  gboolean has_clip;

  klass = MX_FLOATING_WIDGET_CLASS (G_OBJECT_GET_CLASS (widget));

  cogl_push_matrix ();

  cogl_set_modelview_matrix (&(priv->paint_matrix));

  has_clip = clutter_actor_has_clip (CLUTTER_ACTOR (widget));

  if (has_clip)
    {
      gfloat x, y, w, h;
      clutter_actor_get_clip (CLUTTER_ACTOR (widget), &x, &y, &w, &h);
      cogl_clip_push_rectangle (x, y, x + w, y + h);
    }

  if (klass->floating_paint)
    klass->floating_paint (CLUTTER_ACTOR (widget));

  if (has_clip)
    cogl_clip_pop ();

  cogl_pop_matrix ();
}

static void
mx_floating_widget_map (ClutterActor *actor)
{
  MxFloatingWidgetPrivate *priv = MX_FLOATING_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_floating_widget_parent_class)->map (actor);

  /* connect after the paint and pick signals on the stage have run, so that
   * we can be sure to be painted and picked above all other actors else */
  priv->stage = clutter_actor_get_stage (actor);
  g_object_weak_ref (G_OBJECT (priv->stage), (GWeakNotify) stage_weak_notify,
                     actor);

  priv->paint_handler =
    g_signal_connect_after (priv->stage,
                            "paint",
                            G_CALLBACK (mx_floating_widget_paint_from_stage),
                            actor);

  priv->pick_handler =
    g_signal_connect_after (priv->stage,
                            "pick",
                            G_CALLBACK (mx_floating_widget_pick_from_stage),
                            actor);
}

static void
mx_floating_widget_unmap (ClutterActor *actor)
{
  MxFloatingWidgetPrivate *priv = MX_FLOATING_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_floating_widget_parent_class)->unmap (actor);

  if (priv->stage)
    {
      g_signal_handler_disconnect (priv->stage, priv->paint_handler);
      priv->paint_handler = 0;

      g_signal_handler_disconnect (priv->stage, priv->pick_handler);
      priv->pick_handler = 0;

      g_object_weak_unref (G_OBJECT (priv->stage),
                           (GWeakNotify) stage_weak_notify,
                           actor);
      priv->stage = NULL;
    }

}

static void
mx_floating_widget_paint_null (ClutterActor *actor)
{
  /* store the matrix for later
   * the widget is painted after the stage has painted, so that it appears above
   * everything else */
  cogl_get_modelview_matrix (&(MX_FLOATING_WIDGET (actor)->priv->paint_matrix));
}

static void
mx_floating_widget_pick_null (ClutterActor       *actor,
                              const ClutterColor *color)
{
  /* store the matrix for later
   * the widget is picked after the stage has picked, so that it appears above
   * everything else */
  cogl_get_modelview_matrix (&(MX_FLOATING_WIDGET (actor)->priv->pick_matrix));
}


static void
mx_floating_widget_floating_pick (ClutterActor      *widget,
                                  const ClutterColor *color)
{
  /* default implementation is to chain up to the original pick */
  CLUTTER_ACTOR_CLASS (mx_floating_widget_parent_class)->pick (widget, color);
}

static void
mx_floating_widget_floating_paint (ClutterActor *widget)
{
  /* default implementation is to chain up to the original paint */
  CLUTTER_ACTOR_CLASS (mx_floating_widget_parent_class)->paint (widget);
}

static void
mx_floating_widget_class_init (MxFloatingWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxFloatingWidgetClass *float_class = MX_FLOATING_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFloatingWidgetPrivate));

  object_class->get_property = mx_floating_widget_get_property;
  object_class->set_property = mx_floating_widget_set_property;
  object_class->dispose = mx_floating_widget_dispose;
  object_class->finalize = mx_floating_widget_finalize;

  actor_class->map = mx_floating_widget_map;
  actor_class->unmap = mx_floating_widget_unmap;
  actor_class->pick = mx_floating_widget_pick_null;
  actor_class->paint = mx_floating_widget_paint_null;

  float_class->floating_pick = mx_floating_widget_floating_pick;
  float_class->floating_paint = mx_floating_widget_floating_paint;
}

static void
mx_floating_widget_init (MxFloatingWidget *self)
{
  self->priv = FLOATING_WIDGET_PRIVATE (self);
}

