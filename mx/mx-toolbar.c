/*
 * mx-toolbar.c: toolbar actor
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

#include "mx-toolbar.h"
#include "mx-private.h"
#include <clutter/clutter.h>

G_DEFINE_TYPE (MxToolbar, mx_toolbar, MX_TYPE_BIN)

#define TOOLBAR_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_TOOLBAR, MxToolbarPrivate))


#define PADDING 10

enum {
  PROP_CLOSE_BUTTON = 1
};

struct _MxToolbarPrivate
{
  gboolean has_close_button;
  ClutterActor *close_button;
};

static void
mx_toolbar_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (object)->priv;

  switch (property_id)
    {
  case PROP_CLOSE_BUTTON:
    g_value_set_boolean (value, priv->has_close_button);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_toolbar_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  switch (property_id)
    {
  case PROP_CLOSE_BUTTON:
    mx_toolbar_set_has_close_button (MX_TOOLBAR (object),
                                     g_value_get_boolean (value));
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_toolbar_dispose (GObject *object)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (object)->priv;

  G_OBJECT_CLASS (mx_toolbar_parent_class)->dispose (object);

  if (priv->close_button)
    {
      clutter_actor_unparent (priv->close_button);
      priv->close_button = NULL;
    }
}

static void
mx_toolbar_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_toolbar_parent_class)->finalize (object);
}

static void
mx_toolbar_map (ClutterActor *actor)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->map (actor);

  if (priv->close_button)
    clutter_actor_map (priv->close_button);
}

static void
mx_toolbar_unmap (ClutterActor *actor)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;

  if (priv->close_button)
    clutter_actor_unmap (priv->close_button);

  CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->unmap (actor);
}

static void
mx_toolbar_get_preferred_width (ClutterActor *actor,
                                gfloat        for_height,
                                gfloat       *min_width,
                                gfloat       *pref_width)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;
  MxPadding padding;
  gfloat min_child, pref_child, min_close, pref_close;
  ClutterActor *child;

  /* if there is no close button, then use the default preferred width
   * function from MxBin */
  if (!priv->has_close_button)
    {
      CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->
        get_preferred_width (actor,
                             for_height,
                             min_width,
                             pref_width);
      return;
    }

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_height = for_height - padding.top - padding.bottom;

  if (priv->close_button)
    {
      clutter_actor_get_preferred_width (priv->close_button,
                                         for_height,
                                         &min_close,
                                         &pref_close);
    }
  else
    {
      min_close = 0;
      pref_close = 0;
    }

  child = mx_bin_get_child (MX_BIN (actor));
  if (child)
    {
      clutter_actor_get_preferred_width (child,
                                         for_height,
                                         &min_child,
                                         &pref_child);
    }
  else
    {
      min_child = 0;
      pref_child = 0;
    }

  if (min_width)
    *min_width = padding.left + padding.right + min_close + min_child + PADDING;

  if (pref_width)
    *pref_width = padding.left + padding.right + pref_close + pref_child + PADDING;
}

static void
mx_toolbar_get_preferred_height (ClutterActor *actor,
                                 gfloat        for_width,
                                 gfloat       *min_height,
                                 gfloat       *pref_height)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;
  MxPadding padding;
  gfloat min_child, pref_child, min_close, pref_close;
  ClutterActor *child;

  /* if there is no close button, then use the default preferred width
   * function from MxBin */
  if (!priv->has_close_button)
    {
      CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->
        get_preferred_height (actor,
                              for_width,
                              min_height,
                              pref_height);
      return;
    }

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_width = for_width - padding.left - padding.right;

  if (priv->close_button)
    {
      clutter_actor_get_preferred_height (priv->close_button,
                                          -1,
                                          &min_close,
                                          &pref_close);
    }
  else
    {
      min_close = 0;
      pref_close = 0;
    }

  child = mx_bin_get_child (MX_BIN (actor));
  if (child)
    {
      clutter_actor_get_preferred_height (child,
                                          -1,
                                          &min_child,
                                          &pref_child);
    }
  else
    {
      min_child = 0;
      pref_child = 0;
    }

  if (min_height)
    *min_height = padding.top + padding.bottom + MAX (min_close, min_child);

  if (pref_height)
    *pref_height = padding.top + padding.bottom + MAX (pref_close, pref_child);
}

static void
mx_toolbar_allocate (ClutterActor           *actor,
                     const ClutterActorBox  *box,
                     ClutterAllocationFlags  flags)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;
  ClutterActorBox childbox, avail;
  gfloat close_w;

  CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->allocate (actor, box, flags);

  mx_widget_get_available_area (MX_WIDGET (actor), box, &avail);

  if (priv->close_button)
    {
      gfloat pref_h;

      clutter_actor_get_preferred_size (priv->close_button,
                                        NULL, NULL, &close_w, &pref_h);
      childbox.x1 = avail.x2 - close_w;
      childbox.y1 = avail.y1;
      childbox.x2 = avail.x2;
      childbox.y2 = avail.y2;

      clutter_actor_allocate (priv->close_button, &childbox, flags);
    }
  else
    {
      close_w = 0;
    }

  childbox.x1 = avail.x1;
  childbox.y1 = avail.y1;
  childbox.x2 = avail.x2 - close_w - PADDING;
  childbox.y2 = avail.y2;

  mx_bin_allocate_child (MX_BIN (actor), &childbox, flags);
}

static void
mx_toolbar_pick (ClutterActor       *actor,
                 const ClutterColor *color)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->pick (actor, color);

  if (priv->close_button
      && clutter_actor_should_pick_paint (priv->close_button))
    clutter_actor_paint (priv->close_button);
}

static void
mx_toolbar_paint (ClutterActor *actor)
{
  MxToolbarPrivate *priv = MX_TOOLBAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_toolbar_parent_class)->paint (actor);

  if (priv->close_button)
    clutter_actor_paint (priv->close_button);
}

static void
mx_toolbar_class_init (MxToolbarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxToolbarPrivate));

  object_class->get_property = mx_toolbar_get_property;
  object_class->set_property = mx_toolbar_set_property;
  object_class->dispose = mx_toolbar_dispose;
  object_class->finalize = mx_toolbar_finalize;

  actor_class->map = mx_toolbar_map;
  actor_class->unmap = mx_toolbar_unmap;
  actor_class->get_preferred_width = mx_toolbar_get_preferred_width;
  actor_class->get_preferred_height = mx_toolbar_get_preferred_height;
  actor_class->allocate = mx_toolbar_allocate;
  actor_class->pick = mx_toolbar_pick;
  actor_class->paint = mx_toolbar_paint;


  pspec = g_param_spec_boolean ("has-close-button",
                                "Has Close Button",
                                "Whether to show a close button on the toolbar",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLOSE_BUTTON, pspec);
}

static void
mx_toolbar_init (MxToolbar *self)
{
  self->priv = TOOLBAR_PRIVATE (self);

  mx_toolbar_set_has_close_button (self, TRUE);

  mx_bin_set_alignment (MX_BIN (self), MX_ALIGN_START, MX_ALIGN_MIDDLE);
}

ClutterActor *
mx_toolbar_new (void)
{
  return g_object_new (MX_TYPE_TOOLBAR, NULL);
}


void
mx_toolbar_set_has_close_button (MxToolbar *toolbar,
                                 gboolean   has_close_button)
{
  MxToolbarPrivate *priv;

  g_return_if_fail (MX_IS_TOOLBAR (toolbar));

  priv = toolbar->priv;

  if (priv->has_close_button != has_close_button)
    {
      priv->has_close_button = has_close_button;

      if (!has_close_button)
        {
          if (priv->close_button)
            {
              clutter_actor_unparent (priv->close_button);
              priv->close_button = NULL;
            }
        }
      else
        {
          priv->close_button = mx_button_new ();
          clutter_actor_set_parent (priv->close_button,
                                    CLUTTER_ACTOR (toolbar));
        }

      clutter_actor_queue_relayout (CLUTTER_ACTOR (toolbar));

      g_object_notify (G_OBJECT (toolbar), "has-close-button");
    }
}

gboolean
mx_toolbar_get_has_close_button (MxToolbar *toolbar)
{
  g_return_val_if_fail (MX_IS_TOOLBAR (toolbar), FALSE);

  return toolbar->priv->has_close_button;
}
