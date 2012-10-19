/*
 * mx-frame.h: frame actor
 *
 * Copyright 2009 Intel Corporation
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
 */

#include "mx-frame.h"
#include "mx-tooltip.h"

G_DEFINE_TYPE (MxFrame, mx_frame, MX_TYPE_WIDGET)

#define FRAME_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_FRAME, MxFramePrivate))

struct _MxFramePrivate
{
  ClutterActor *child;
};


static void
mx_frame_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_frame_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_frame_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_frame_parent_class)->dispose (object);
}

static void
mx_frame_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_frame_parent_class)->finalize (object);
}

static void
mx_frame_allocate (ClutterActor          *self,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  MxFramePrivate *priv = ((MxFrame *) self)->priv;
  ClutterActorBox childbox;

  CLUTTER_ACTOR_CLASS (mx_frame_parent_class)->allocate (self, box, flags);

  if (priv->child)
    {
      mx_widget_get_available_area (MX_WIDGET (self), box, &childbox);
      clutter_actor_allocate (priv->child, &childbox, flags);
    }
}

static void
mx_frame_paint (ClutterActor *self)
{
  MxFramePrivate *priv = ((MxFrame *) self)->priv;

  CLUTTER_ACTOR_CLASS (mx_frame_parent_class)->paint (self);

  if (priv->child)
    clutter_actor_paint (priv->child);
}

static void
mx_frame_pick (ClutterActor       *self,
               const ClutterColor *color)
{
  MxFramePrivate *priv = ((MxFrame *) self)->priv;

  CLUTTER_ACTOR_CLASS (mx_frame_parent_class)->pick (self, color);

  if (priv->child)
    clutter_actor_paint (priv->child);
}

static void
mx_frame_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width,
                              gfloat       *pref_width)
{
  MxFramePrivate *priv = ((MxFrame *) actor)->priv;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width)
    *min_width = 0;

  if (pref_width)
    *pref_width = 0;

  if (priv->child)
    {
      clutter_actor_get_preferred_width (priv->child,
                                         for_height - padding.top - padding.bottom,
                                         min_width, pref_width);
    }

  if (min_width)
    *min_width += padding.left + padding.right;

  if (pref_width)
    *pref_width += padding.left + padding.right;

}

static void
mx_frame_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height,
                               gfloat       *pref_height)
{
  MxFramePrivate *priv = ((MxFrame *) actor)->priv;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_height)
    *min_height = 0;

  if (pref_height)
    *pref_height = 0;

  if (priv->child)
    {
      clutter_actor_get_preferred_height (priv->child,
                                          for_width - padding.left - padding.right,
                                          min_height, pref_height);
    }

  if (min_height)
    *min_height += padding.top + padding.bottom;

  if (pref_height)
    *pref_height += padding.top + padding.bottom;
}

static void
mx_frame_class_init (MxFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFramePrivate));

  object_class->get_property = mx_frame_get_property;
  object_class->set_property = mx_frame_set_property;
  object_class->dispose = mx_frame_dispose;
  object_class->finalize = mx_frame_finalize;

  actor_class->allocate = mx_frame_allocate;
  actor_class->pick = mx_frame_pick;
  actor_class->paint = mx_frame_paint;
  actor_class->get_preferred_width = mx_frame_get_preferred_width;
  actor_class->get_preferred_height = mx_frame_get_preferred_height;
}

static void
mx_frame_actor_added (ClutterActor *container,
                      ClutterActor *actor)
{
  MxFramePrivate *priv = MX_FRAME (container)->priv;

  if (MX_IS_TOOLTIP (actor))
    return;

  if (priv->child)
    clutter_actor_remove_child (container, priv->child);

  priv->child = actor;
}

static void
mx_frame_actor_removed (ClutterActor *container,
                        ClutterActor *actor)
{
  MxFramePrivate *priv = MX_FRAME (container)->priv;

  if (priv->child == actor)
    priv->child = NULL;
}

static void
mx_frame_init (MxFrame *self)
{
  self->priv = FRAME_PRIVATE (self);

  g_signal_connect (self, "actor-added", G_CALLBACK (mx_frame_actor_added),
                    NULL);
  g_signal_connect (self, "actor-removed", G_CALLBACK (mx_frame_actor_removed),
                    NULL);
}

/**
 * mx_frame_new:
 *
 * Create a new #MxFrame
 *
 * Returns: a newly allocated #MxFrame
 */
ClutterActor *
mx_frame_new (void)
{
  return g_object_new (MX_TYPE_FRAME, NULL);
}
