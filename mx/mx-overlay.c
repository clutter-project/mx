/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-overlay.c: A layer mecanism for tooltips and menus.
 *
 * Copyright 2012 Intel Corporation.
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
 * Written by: Lionel Landwerlin <lionel.g.landwerlin@linux.intel.com>
 *
 */

#include "mx-overlay.h"

G_DEFINE_TYPE (MxOverlay, mx_overlay, CLUTTER_TYPE_ACTOR)

#define OVERLAY_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_OVERLAY, MxOverlayPrivate))

struct _MxOverlayPrivate
{
  gboolean needs_layout;
};

static void
mx_overlay_get_property (GObject    *object,
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
mx_overlay_set_property (GObject      *object,
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
mx_overlay_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_overlay_parent_class)->dispose (object);
}

static void
mx_overlay_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_overlay_parent_class)->finalize (object);
}

static void
mx_overlay_queue_relayout (ClutterActor *self)
{
  MxOverlayPrivate *priv = MX_OVERLAY (self)->priv;

  priv->needs_layout = TRUE;
  clutter_actor_queue_redraw (self);
}

static void
mx_overlay_paint (ClutterActor *self)
{
  MxOverlayPrivate *priv = MX_OVERLAY (self)->priv;
  ClutterActorIter iter;
  ClutterActor *child;

  clutter_actor_iter_init (&iter, self);

  if (priv->needs_layout)
    {
      while (clutter_actor_iter_next (&iter, &child))
        {
          clutter_actor_allocate_preferred_size (child,
                                                 CLUTTER_ALLOCATION_NONE);
          clutter_actor_paint (child);
        }
      priv->needs_layout = FALSE;
    }
  else
    {
      while (clutter_actor_iter_next (&iter, &child))
        clutter_actor_paint (child);
    }
}

static void
mx_overlay_class_init (MxOverlayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxOverlayPrivate));

  object_class->get_property = mx_overlay_get_property;
  object_class->set_property = mx_overlay_set_property;
  object_class->dispose = mx_overlay_dispose;
  object_class->finalize = mx_overlay_finalize;

  actor_class->queue_relayout = mx_overlay_queue_relayout;
  actor_class->paint = mx_overlay_paint;
}

static void
mx_overlay_init (MxOverlay *self)
{
  self->priv = OVERLAY_PRIVATE (self);
}

ClutterActor *
mx_overlay_new (void)
{
  return g_object_new (MX_TYPE_OVERLAY, NULL);
}
