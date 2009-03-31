/*
 * Copyright (C) 2008 Intel Corporation
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
 * Written by: Robert Staudinger <robsta@openedhand.com>.
 */

/**
 * SECTION:nbtk-scroll-button
 * @short_description: Scroll button widget
 *
 * A button widget that continuously emits "clicked" signals when the first 
 * mouse button is held down on it.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>
#include "nbtk-scroll-button.h"

#define NBTK_SCROLL_BUTTON_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_SCROLL_BUTTON, NbtkScrollButtonPrivate))

#define INITIAL_REPEAT_TIMEOUT 500
#define SUBSEQUENT_REPEAT_TIMEOUT 200

struct _NbtkScrollButtonPrivate
{
  guint source_id;
  gboolean is_initial;
};

G_DEFINE_TYPE (NbtkScrollButton, nbtk_scroll_button, NBTK_TYPE_BUTTON);

static void
nbtk_scroll_button_notify_reactive (NbtkScrollButton  *self,
                                    GParamSpec        *pspec,
                                    gpointer           data)
{
  NbtkScrollButtonPrivate *priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (self);

  if (!clutter_actor_get_reactive (CLUTTER_ACTOR (self)) &&
      priv->source_id)
    {
      g_source_remove (priv->source_id);
      priv->source_id = 0;
    }
}

static gboolean
nbtk_scroll_button_emit_clicked (NbtkScrollButton *self)
{
  NbtkScrollButtonPrivate *priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (self);

  g_signal_emit_by_name (self, "clicked");

  if (priv->is_initial)
    {
      /* Switch to subsequent timeout. */
      priv->is_initial = FALSE;
      priv->source_id = g_timeout_add (SUBSEQUENT_REPEAT_TIMEOUT,
                                       (GSourceFunc) nbtk_scroll_button_emit_clicked,
                                       self);
      return FALSE;
    }

  return TRUE;
}

static gboolean
nbtk_scroll_button_button_press_event (ClutterActor       *actor,
                                       ClutterButtonEvent *event)
{
  NbtkScrollButtonPrivate *priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (actor);

  if (priv->source_id)
    {
      g_source_remove (priv->source_id);
      priv->source_id = 0;
    }

  if (event->button == 1)
    {
      /* Emit first click immediately ... */
      g_signal_emit_by_name (actor, "clicked");

      /* ... and more repeatedly. */
      priv->is_initial = TRUE;
      priv->source_id = g_timeout_add (INITIAL_REPEAT_TIMEOUT,
                                       (GSourceFunc) nbtk_scroll_button_emit_clicked,
                                       actor);
    }

  return TRUE;
}

static gboolean
nbtk_scroll_button_button_release_event (ClutterActor       *actor,
                                         ClutterButtonEvent *event)
{
  NbtkScrollButtonPrivate *priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (actor);

  if (priv->source_id)
    {
      g_source_remove (priv->source_id);
      priv->source_id = 0;
    }

  return TRUE;
}

static gboolean
nbtk_scroll_button_leave_event (ClutterActor         *actor,
                                ClutterCrossingEvent *event)
{
  NbtkScrollButtonPrivate *priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (actor);

  if (priv->source_id)
    {
      g_source_remove (priv->source_id);
      priv->source_id = 0;
    }

  CLUTTER_ACTOR_CLASS (nbtk_scroll_button_parent_class)
    ->leave_event (actor, event);

  return TRUE;
}

static void
nbtk_scroll_button_class_init (NbtkScrollButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (object_class, sizeof (NbtkScrollButtonPrivate));

  actor_class->button_press_event = nbtk_scroll_button_button_press_event;
  actor_class->button_release_event = nbtk_scroll_button_button_release_event;
  actor_class->leave_event = nbtk_scroll_button_leave_event;
}

static void
nbtk_scroll_button_init (NbtkScrollButton *self)
{
  self->priv = NBTK_SCROLL_BUTTON_GET_PRIVATE (self);
  
  g_signal_connect (self, "notify::reactive", 
                    G_CALLBACK (nbtk_scroll_button_notify_reactive), NULL);
}

NbtkWidget *
nbtk_scroll_button_new (void)
{
  return (NbtkWidget *) g_object_new (NBTK_TYPE_SCROLL_BUTTON, NULL);
}

