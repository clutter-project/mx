/*
 * mx-path-bar-button.c: A button actor for the path bar
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-path-bar-button.h"
#include <math.h>

G_DEFINE_TYPE (MxPathBarButton, mx_path_bar_button, MX_TYPE_BUTTON)

#define PATH_BAR_BUTTON_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_PATH_BAR_BUTTON, MxPathBarButtonPrivate))

struct _MxPathBarButtonPrivate
{
  gdouble transition;
};

enum
{
  PROP_0,

  PROP_TRANSITION
};

static void
mx_path_bar_button_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  MxPathBarButtonPrivate *priv = MX_PATH_BAR_BUTTON (object)->priv;

  switch (property_id)
    {
    case PROP_TRANSITION:
      g_value_set_double (value, priv->transition);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_path_bar_button_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  MxPathBarButtonPrivate *priv = MX_PATH_BAR_BUTTON (object)->priv;

  switch (property_id)
    {
    case PROP_TRANSITION:
      priv->transition = g_value_get_double (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (object));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_path_bar_button_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_path_bar_button_parent_class)->dispose (object);
}

static void
mx_path_bar_button_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_path_bar_button_parent_class)->finalize (object);
}

static void
mx_path_bar_button_get_preferred_width (ClutterActor *actor,
                                        gfloat        for_height,
                                        gfloat       *min_width_p,
                                        gfloat       *nat_width_p)
{
  MxPathBarButtonPrivate *priv = MX_PATH_BAR_BUTTON (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_path_bar_button_parent_class)->
    get_preferred_width (actor, for_height, min_width_p, nat_width_p);

  if (min_width_p)
    *min_width_p = ceilf ((*min_width_p) * priv->transition);
  if (nat_width_p)
    *nat_width_p = ceilf ((*nat_width_p) * priv->transition);
}

static void
mx_path_bar_button_class_init (MxPathBarButtonClass *klass)
{
  GParamSpec *pspec;

  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class  = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxPathBarButtonPrivate));

  object_class->get_property = mx_path_bar_button_get_property;
  object_class->set_property = mx_path_bar_button_set_property;
  object_class->dispose = mx_path_bar_button_dispose;
  object_class->finalize = mx_path_bar_button_finalize;

  actor_class->get_preferred_width = mx_path_bar_button_get_preferred_width;

  pspec = g_param_spec_double ("transition",
                               "Transition",
                               "Transition animation progress.",
                               0.0, 1.0, 0.0, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TRANSITION, pspec);
}

static void
mx_path_bar_button_init (MxPathBarButton *self)
{
  self->priv = PATH_BAR_BUTTON_PRIVATE (self);

  g_object_set (G_OBJECT (self), "clip-to-allocation", TRUE, NULL);
}

ClutterActor *
mx_path_bar_button_new (const gchar *label)
{
  return g_object_new (MX_TYPE_PATH_BAR_BUTTON, "label", label, NULL);
}
