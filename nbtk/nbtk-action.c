/*
 * nbtk-action.c: NbtkAction object
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
 * SECTION: nbtk-action
 * @short_description: Represents a user action
 *
 * Actions represent operations that the user can perform, such as items in a
 * menu or toolbar.
 */

#include "nbtk-action.h"

G_DEFINE_TYPE (NbtkAction, nbtk_action, G_TYPE_INITIALLY_UNOWNED)

#define ACTION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_ACTION, NbtkActionPrivate))

struct _NbtkActionPrivate
{
  gchar    *name;
  gboolean  active;
};

enum
{
  PROP_0,

  PROP_NAME,
  PROP_ACTIVE
};

enum
{
  ACTIVATED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
nbtk_action_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkAction *action = NBTK_ACTION (object);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string (value, nbtk_action_get_name (action));
      break;

    case PROP_ACTIVE:
      g_value_set_boolean (value, nbtk_action_get_active (action));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_action_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkAction *action = NBTK_ACTION (object);

  switch (property_id)
    {
    case PROP_NAME:
      nbtk_action_set_name (action, g_value_get_string (value));
      break;

    case PROP_ACTIVE:
      nbtk_action_set_active (action, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_action_dispose (GObject *object)
{
  G_OBJECT_CLASS (nbtk_action_parent_class)->dispose (object);
}

static void
nbtk_action_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_action_parent_class)->finalize (object);
}

static void
nbtk_action_class_init (NbtkActionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkActionPrivate));

  object_class->get_property = nbtk_action_get_property;
  object_class->set_property = nbtk_action_set_property;
  object_class->dispose = nbtk_action_dispose;
  object_class->finalize = nbtk_action_finalize;

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

  signals[ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkActionClass, activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
nbtk_action_init (NbtkAction *self)
{
  NbtkActionPrivate *priv = self->priv = ACTION_PRIVATE (self);
  priv->active = TRUE;
}

/**
 * nbtk_action_new:
 *
 * Create a new, blank, NbtkAction
 *
 * Returns: a newly allocated NbtkAction
 */
NbtkAction *
nbtk_action_new (void)
{
  return g_object_new (NBTK_TYPE_ACTION, NULL);
}

/**
 * nbtk_action_new_full:
 * @name: name of the action
 * @activated_cb: callback to connect to the activated signal
 * @user_data: user data to be passed to the callback
 *
 * Create a new NbtkAction with the name and callback set
 *
 * Returns: a newly allocation NbtkAction
 */
NbtkAction *
nbtk_action_new_full (const gchar *name,
                      GCallback    activated_cb,
                      gpointer     user_data)
{
  NbtkAction *action = g_object_new (NBTK_TYPE_ACTION, "name", name, NULL);

  if (activated_cb)
    g_signal_connect (action, "activated", activated_cb, user_data);

  return action;
}

/**
 * nbtk_action_get_name:
 * @action: A #NbtkAction
 *
 * Get the name of the action
 *
 * Returns: name of the action, owned by NbtkAction
 */
const gchar *
nbtk_action_get_name (NbtkAction *action)
{
  g_return_if_fail (NBTK_IS_ACTION (action));

  return action->priv->name;
}

/**
 * nbtk_action_set_name:
 * @action: A #NbtkAction
 * @name: new name to set
 *
 * Set the name of the action
 *
 */
void
nbtk_action_set_name (NbtkAction  *action,
                      const gchar *name)
{
  NbtkActionPrivate *priv;

  g_return_if_fail (NBTK_IS_ACTION (action));

  priv = action->priv;

  if (!g_strcmp0 (priv->name, name))
    {
      g_free (priv->name);
      priv->name = g_strdup (name);

      g_object_notify (G_OBJECT (action), "name");
    }

}

/**
 * nbtk_action_get_active:
 * @action: A #NbtkAction
 *
 * Get the value of the active property
 *
 * Returns: #TRUE if the action is active
 */
gboolean
nbtk_action_get_active (NbtkAction *action)
{
  g_return_if_fail (NBTK_IS_ACTION (action));

  return action->priv->active;
}

/**
 * nbtk_action_set_active:
 * @action: A #NbtkAction
 * @active: the value to set
 *
 * Set the value of the active property
 *
 */
void
nbtk_action_set_active (NbtkAction *action,
                        gboolean    active)
{
  NbtkActionPrivate *priv;

  g_return_if_fail (NBTK_IS_ACTION (action));

  priv = action->priv;

  if (priv->active != active)
    {
      priv->active = active;
      g_object_notify (G_OBJECT (action), "active");
    }
}

