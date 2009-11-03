/*
 * mx-button-group.c: A group handler for buttons
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#include "mx-button-group.h"

#include "mx-private.h"

G_DEFINE_TYPE (MxButtonGroup, mx_button_group, G_TYPE_INITIALLY_UNOWNED)

#define BUTTON_GROUP_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_BUTTON_GROUP, MxButtonGroupPrivate))

struct _MxButtonGroupPrivate
{
  MxButton *active_button;
  GSList *children;
};

enum
{
  PROP_ACTIVE_BUTTON = 1
};

static void
mx_button_group_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MxButtonGroup *group = MX_BUTTON_GROUP (object);

  switch (property_id)
    {
    case PROP_ACTIVE_BUTTON:
      g_value_set_object (value, group->priv->active_button);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_button_group_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  MxButtonGroup *group = MX_BUTTON_GROUP (object);

  switch (property_id)
    {
    case PROP_ACTIVE_BUTTON:
      mx_button_group_set_active_button (group,
                                         g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_button_group_dispose (GObject *object)
{
  MxButtonGroupPrivate *priv = MX_BUTTON_GROUP (object)->priv;

  if (priv->children)
    {
      g_slist_foreach (priv->children, (GFunc) g_object_unref, NULL);
      g_slist_free (priv->children);
      priv->children = NULL;
    }

  priv->active_button = NULL;

  G_OBJECT_CLASS (mx_button_group_parent_class)->dispose (object);
}

static void
mx_button_group_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_button_group_parent_class)->finalize (object);
}

static void
mx_button_group_class_init (MxButtonGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxButtonGroupPrivate));

  object_class->get_property = mx_button_group_get_property;
  object_class->set_property = mx_button_group_set_property;
  object_class->dispose = mx_button_group_dispose;
  object_class->finalize = mx_button_group_finalize;

  pspec = g_param_spec_object ("active-button",
                               "Active Button",
                               "The currently active (toggled) button",
                               MX_TYPE_BUTTON,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ACTIVE_BUTTON, pspec);
}

static void
mx_button_group_init (MxButtonGroup *self)
{
  self->priv = BUTTON_GROUP_PRIVATE (self);
}

MxButtonGroup *
mx_button_group_new (void)
{
  return g_object_new (MX_TYPE_BUTTON_GROUP, NULL);
}

static void
button_checked_notify_cb (MxButton      *button,
                          GParamSpec    *pspec,
                          MxButtonGroup *group)
{
  if (mx_button_get_checked (button))
    mx_button_group_set_active_button (group, button);
  else
    mx_button_group_set_active_button (group, NULL);
}

void
mx_button_group_add (MxButtonGroup   *group,
                     MxButton        *button)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (MX_IS_BUTTON (button));

  g_object_ref (button);
  group->priv->children = g_slist_prepend (group->priv->children, button);

  g_signal_connect (button, "notify::checked",
                    G_CALLBACK (button_checked_notify_cb), group);
}

void
mx_button_group_remove (MxButtonGroup   *group,
                        MxButton        *button)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (MX_IS_BUTTON (button));

  group->priv->children = g_slist_remove (group->priv->children, button);

  g_signal_handlers_disconnect_by_func (button, button_checked_notify_cb,
                                        group);

  g_object_unref (button);
}

void
mx_button_group_foreach (MxButtonGroup   *group,
                         ClutterCallback  callback,
                         gpointer         userdata)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (callback == NULL);

  g_slist_foreach (group->priv->children, (GFunc) callback, userdata);
}

void
mx_button_group_set_active_button (MxButtonGroup *group,
                                   MxButton      *button)
{
  MxButtonGroupPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (button == NULL || MX_IS_BUTTON (button));

  priv = group->priv;
  if (button == priv->active_button)
    return;

  mx_button_set_checked (priv->active_button, FALSE);

  if (button)
    mx_button_set_checked (button, TRUE);

  priv->active_button = button;

  g_object_notify (G_OBJECT (group), "active-button");
}

MxButton *
mx_button_group_get_active_button (MxButtonGroup *group)
{
  MxButtonGroupPrivate *priv;

  g_return_val_if_fail (MX_IS_BUTTON_GROUP (group), NULL);

  priv = group->priv;

  return priv->active_button;
}
