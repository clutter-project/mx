/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-action.c: MxAction object
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
 */

/**
 * SECTION: mx-action
 * @short_description: Represents a user action
 *
 * Actions represent operations that the user can perform, such as items in a
 * menu or toolbar.
 */

#include "mx-action.h"

G_DEFINE_TYPE (MxAction, mx_action, G_TYPE_INITIALLY_UNOWNED)

#define ACTION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_ACTION, MxActionPrivate))

struct _MxActionPrivate
{
  gchar   *name;
  gchar   *display_name;
  gchar   *icon;
  gboolean active;
};

enum
{
  PROP_0,

  PROP_NAME,
  PROP_DISPLAY_NAME,
  PROP_ICON,
  PROP_ACTIVE
};

enum
{
  ACTIVATED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
mx_action_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxAction *action = MX_ACTION (object);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string (value, mx_action_get_name (action));
      break;

    case PROP_DISPLAY_NAME:
      g_value_set_string (value, mx_action_get_display_name (action));
      break;

    case PROP_ICON:
      g_value_set_string (value, mx_action_get_icon (action));
      break;

    case PROP_ACTIVE:
      g_value_set_boolean (value, mx_action_get_active (action));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_action_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxAction *action = MX_ACTION (object);

  switch (property_id)
    {
    case PROP_NAME:
      mx_action_set_name (action, g_value_get_string (value));
      break;

    case PROP_DISPLAY_NAME:
      mx_action_set_display_name (action, g_value_get_string (value));
      break;

    case PROP_ICON:
      mx_action_set_icon (action, g_value_get_string (value));
      break;

    case PROP_ACTIVE:
      mx_action_set_active (action, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_action_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_action_parent_class)->dispose (object);
}

static void
mx_action_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_action_parent_class)->finalize (object);
}

static void
mx_action_class_init (MxActionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxActionPrivate));

  object_class->get_property = mx_action_get_property;
  object_class->set_property = mx_action_set_property;
  object_class->dispose = mx_action_dispose;
  object_class->finalize = mx_action_finalize;

  g_object_class_install_property (object_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "Action name.",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_DISPLAY_NAME,
                                   g_param_spec_string ("display-name",
                                                        "Display name",
                                                        "Localised name to use "
                                                        "for display.",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_ICON,
                                   g_param_spec_string ("icon",
                                                        "Icon name",
                                                        "Icon name or path to "
                                                        "to be used if this "
                                                        "action is displayed",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));



  g_object_class_install_property (object_class,
                                   PROP_ACTIVE,
                                   g_param_spec_boolean ("active",
                                                         "Active",
                                                         "Whether the action "
                                                         "is active.",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_NAME |
                                                         G_PARAM_STATIC_NICK |
                                                         G_PARAM_STATIC_BLURB));

  /**
   * MxAction::activated
   * @action: the object that received the signal
   *
   * Emitted when the MxAction is activated.
   *
   */
  signals[ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActionClass, activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
mx_action_init (MxAction *self)
{
  MxActionPrivate *priv = self->priv = ACTION_PRIVATE (self);
  priv->active = TRUE;
}

/**
 * mx_action_new:
 *
 * Create a new, blank, MxAction
 *
 * Returns: a newly allocated MxAction
 */
MxAction *
mx_action_new (void)
{
  return g_object_new (MX_TYPE_ACTION, NULL);
}

/**
 * mx_action_new_full:
 * @name: name of the action
 * @display_name: name of the action to display to the user
 * @activated_cb: callback to connect to the activated signal
 * @user_data: user data to be passed to the callback
 *
 * Create a new MxAction with the name and callback set
 *
 * Returns: a newly allocation MxAction
 */
MxAction *
mx_action_new_full (const gchar *name,
                    const gchar *display_name,
                    GCallback    activated_cb,
                    gpointer     user_data)
{
  MxAction *action = g_object_new (MX_TYPE_ACTION,
                                   "name", name,
                                   "display-name", display_name,
                                   NULL);

  if (activated_cb)
    g_signal_connect (action, "activated", activated_cb, user_data);

  return action;
}

/**
 * mx_action_get_name:
 * @action: A #MxAction
 *
 * Get the name of the action
 *
 * Returns: name of the action, owned by MxAction
 */
const gchar *
mx_action_get_name (MxAction *action)
{
  g_return_val_if_fail (MX_IS_ACTION (action), NULL);

  return action->priv->name;
}

/**
 * mx_action_set_name:
 * @action: A #MxAction
 * @name: new name to set
 *
 * Set the name of the action
 *
 */
void
mx_action_set_name (MxAction    *action,
                    const gchar *name)
{
  MxActionPrivate *priv;

  g_return_if_fail (MX_IS_ACTION (action));

  priv = action->priv;

  if (g_strcmp0 (priv->name, name))
    {
      g_free (priv->name);
      priv->name = g_strdup (name);

      g_object_notify (G_OBJECT (action), "name");
    }

}

/**
 * mx_action_get_active:
 * @action: A #MxAction
 *
 * Get the value of the active property
 *
 * Returns: #TRUE if the action is active
 */
gboolean
mx_action_get_active (MxAction *action)
{
  g_return_val_if_fail (MX_IS_ACTION (action), FALSE);

  return action->priv->active;
}

/**
 * mx_action_set_active:
 * @action: A #MxAction
 * @active: the value to set
 *
 * Set the value of the active property
 *
 */
void
mx_action_set_active (MxAction *action,
                      gboolean  active)
{
  MxActionPrivate *priv;

  g_return_if_fail (MX_IS_ACTION (action));

  priv = action->priv;

  if (priv->active != active)
    {
      priv->active = active;
      g_object_notify (G_OBJECT (action), "active");
    }
}

/**
 * mx_action_get_display_name:
 * @action: A #MxAction
 *
 * Get the display name of the action
 *
 * Returns: display-name of the action, owned by MxAction
 */
const gchar *
mx_action_get_display_name (MxAction *action)
{
  g_return_val_if_fail (MX_IS_ACTION (action), NULL);

  return action->priv->display_name;
}

/**
 * mx_action_get_icon:
 * @action: A #MxAction
 *
 * Get the icon of the action
 *
 * Returns: icon of the action, owned by MxAction
 */
const gchar *
mx_action_get_icon (MxAction *action)
{
  g_return_val_if_fail (MX_IS_ACTION (action), NULL);

  return action->priv->icon;
}

/**
 * mx_action_set_display_name:
 * @action: A #MxAction
 * @name: new display name to set
 *
 * Set the name of the action to display to the user
 *
 */
void
mx_action_set_display_name (MxAction    *action,
                            const gchar *name)
{
  MxActionPrivate *priv;

  g_return_if_fail (MX_IS_ACTION (action));

  priv = action->priv;

  if (g_strcmp0 (priv->display_name, name))
    {
      g_free (priv->display_name);
      priv->display_name = g_strdup (name);

      g_object_notify (G_OBJECT (action), "display-name");
    }
}

/**
 * mx_action_set_icon:
 * @action: A #MxAction
 * @name: new icon to set
 *
 * The icon to be used in a visual representation of an action.
 *
 */
void
mx_action_set_icon (MxAction    *action,
                    const gchar *name)
{
  MxActionPrivate *priv;

  g_return_if_fail (MX_IS_ACTION (action));

  priv = action->priv;

  if (g_strcmp0 (priv->icon, name))
    {
      g_free (priv->icon);
      priv->icon = g_strdup (name);

      g_object_notify (G_OBJECT (action), "icon");
    }
}
