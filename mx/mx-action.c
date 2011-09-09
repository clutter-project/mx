/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-action.c: MxAction object
 *
 * Copyright 2009, 2011 Intel Corporation.
 * Copyright © 2010 Codethink Limited
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
#include "mx-types.h"

static void g_action_iface_init (GActionInterface *iface);

G_DEFINE_TYPE_WITH_CODE (MxAction, mx_action, G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION,
                                                g_action_iface_init))

#define ACTION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_ACTION, MxActionPrivate))

struct _MxActionPrivate
{
  /* GAction */
  gchar        *name;
  GVariantType *parameter_type;
  guint         enabled   : 1;
  guint         state_set : 1;
  GVariant     *state;

  gchar        *display_name;
  gchar        *icon;
};

enum
{
  PROP_0,

  PROP_NAME,
  PROP_DISPLAY_NAME,
  PROP_ICON,
  PROP_ACTIVE,

  /* GAction properties (+ _NAME above) */
  PROP_PARAMETER_TYPE,
  PROP_ENABLED,
  PROP_STATE_TYPE,
  PROP_STATE
};

enum
{
  ACTIVATED,  /* deprecated */
  ACTIVATE,   /* GAction */

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

/*
 * GAction implementation
 *
 * Most of the GAction implementation has been derived from GLib's
 * GSimpleAction.
 */

static const gchar *
mx_g_action_get_name (GAction *action)
{
  MxAction *mx_action = MX_ACTION (action);

  return mx_action->priv->name;
}

const GVariantType *
mx_action_get_parameter_type (GAction *action)
{
  MxAction *mx_action = MX_ACTION (action);

  return mx_action->priv->parameter_type;
}

static const GVariantType *
mx_action_get_state_type (GAction *action)
{
  MxAction *mx_action = MX_ACTION (action);

  if (mx_action->priv->state != NULL)
    return g_variant_get_type (mx_action->priv->state);
  else
    return NULL;
}

static GVariant *
mx_action_get_state_hint (GAction *action)
{
  return NULL;
}

static gboolean
mx_action_get_enabled (GAction *action)
{
  MxAction *mx_action = MX_ACTION (action);

  return mx_action->priv->enabled;
}

static void
mx_action_set_state (GAction  *action,
                     GVariant *value)
{
  MxAction *mx_action = MX_ACTION (action);

  g_return_if_fail (value != NULL);

  {
    const GVariantType *state_type;

    state_type = mx_action->priv->state ?
                 g_variant_get_type (mx_action->priv->state) : NULL;
    g_return_if_fail (state_type != NULL);
    g_return_if_fail (g_variant_is_of_type (value, state_type));
  }

  g_variant_ref_sink (value);

  if (!g_variant_equal (mx_action->priv->state, value))
    {
      if (mx_action->priv->state)
        g_variant_unref (mx_action->priv->state);

      mx_action->priv->state = g_variant_ref (value);

      g_object_notify (G_OBJECT (mx_action), "state");
    }

  g_variant_unref (value);
}

static GVariant *
mx_action_get_state (GAction *action)
{
  MxAction *mx_action = MX_ACTION (action);

  return mx_action->priv->state ? g_variant_ref (mx_action->priv->state) : NULL;
}

static void
mx_action_activate (GAction  *action,
                    GVariant *parameter)
{
  MxAction *mx_action = MX_ACTION (action);

  g_return_if_fail (mx_action->priv->parameter_type == NULL ?
                    parameter == NULL :
                    (parameter != NULL &&
                     g_variant_is_of_type (parameter,
                                           mx_action->priv->parameter_type)));

  if (parameter != NULL)
    g_variant_ref_sink (parameter);

  if (mx_action->priv->enabled)
    {
      g_signal_emit (mx_action, signals[ACTIVATE], 0, parameter);
      /* This is for the GAction implementation to emulate the old and
       * deprecated MxAction behaviour */
      g_signal_emit (mx_action, signals[ACTIVATED], 0);
    }

  if (parameter != NULL)
    g_variant_unref (parameter);
}

void
g_action_iface_init (GActionInterface *iface)
{
  iface->get_name = mx_g_action_get_name;
  iface->get_parameter_type = mx_action_get_parameter_type;
  iface->get_state_type = mx_action_get_state_type;
  iface->get_state_hint = mx_action_get_state_hint;
  iface->get_enabled = mx_action_get_enabled;
  iface->get_state = mx_action_get_state;
  iface->set_state = mx_action_set_state;
  iface->activate = mx_action_activate;
}

static void
mx_action_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxAction *action = MX_ACTION (object);
  GAction *g_action = G_ACTION (object);

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

    case PROP_PARAMETER_TYPE:
      g_value_set_boxed (value, mx_action_get_parameter_type (g_action));
      break;

    case PROP_ENABLED:
      g_value_set_boolean (value, mx_action_get_enabled (g_action));
      break;

    case PROP_STATE_TYPE:
      g_value_set_boxed (value, mx_action_get_state_type (g_action));
      break;

    case PROP_STATE:
      g_value_take_variant (value, mx_action_get_state (g_action));
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
  MxActionPrivate *priv = action->priv;
  GAction *g_action = G_ACTION (object);

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

    case PROP_PARAMETER_TYPE:
      /* construct only */
      priv->parameter_type = g_value_get_boxed (value);
      break;

    case PROP_ENABLED:
      priv->enabled = g_value_get_boolean (value);
      break;

    case PROP_STATE:
      {
        GVariant *state = g_value_get_variant (value);

        if (state)
          mx_action_set_state (g_action, state);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
activated_emitted_cb (MxAction *action,
                      gpointer  user_data)
{
  /* we can't resuse g_action_activate() or mx_action_activate() as it would
   * create an infinte loop */
  if (action->priv->enabled)
    g_signal_emit (action, signals[ACTIVATE], 0, NULL);
}

static void
mx_action_constructed (GObject *object)
{
  /* we connect to activate to emulate the old behaviour when using
   * g_action_activate () */
  g_signal_connect (object, "activated",
                    G_CALLBACK (activated_emitted_cb), NULL);

  G_OBJECT_CLASS (mx_action_parent_class)->constructed (object);
}

static void
mx_action_finalize (GObject *object)
{
  MxActionPrivate *priv = MX_ACTION (object)->priv;

  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }

  if (priv->display_name)
    {
      g_free (priv->display_name);
      priv->display_name = NULL;
    }

  if (priv->icon)
    {
      g_free (priv->icon);
      priv->icon = NULL;
    }

  if (priv->parameter_type)
    g_variant_type_free (priv->parameter_type);
  if (priv->state)
    g_variant_unref (priv->state);

  G_OBJECT_CLASS (mx_action_parent_class)->finalize (object);
}

static void
mx_action_class_init (MxActionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxActionPrivate));

  object_class->get_property = mx_action_get_property;
  object_class->set_property = mx_action_set_property;
  object_class->constructed = mx_action_constructed;
  object_class->finalize = mx_action_finalize;

  /**
   * MxAction:name:
   *
   * The name of the action.  This is mostly meaningful for identifying
   * the action once it has been added to a #GActionGroup.
   *
   * Since: 1.4
   */
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
                                                        G_PARAM_STATIC_BLURB |
                                                        MX_PARAM_TRANSLATEABLE
                                                        ));

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



  /* deprecated in favor on "enabled" (part of GAction) */
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
   * MxAction:parameter-type:
   *
   * The type of the parameter that must be given when activating the
   * action.
   *
   * Since: 1.4
   */
  g_object_class_install_property (object_class,
                                   PROP_PARAMETER_TYPE,
                                   g_param_spec_boxed ("parameter-type",
                                                       "Parameter Type",
                                                       "The type of GVariant "
                                                       "passed to activate()",
                                                       G_TYPE_VARIANT_TYPE,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * MxAction:enabled:
   *
   * If @action is currently enabled.
   *
   * If the action is disabled then calls to g_action_activate() and
   * g_action_change_state() have no effect.
   *
   * Since: 1.4
   */
  g_object_class_install_property (object_class,
                                   PROP_ENABLED,
                                   g_param_spec_boolean ("enabled",
                                                         "Enabled",
                                                         "If the action can be "
                                                         "activated",
                                                         TRUE,
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  /**
   * MxAction:state-type:
   *
   * The #GVariantType of the state that the action has, or %NULL if the
   * action is stateless.
   *
   * Since: 1.4
   */
  g_object_class_install_property (object_class,
                                   PROP_STATE_TYPE,
                                   g_param_spec_boxed ("state-type",
                                                       "State Type",
                                                       "The type of the state "
                                                       "kept by the action",
                                                       G_TYPE_VARIANT_TYPE,
                                                       G_PARAM_READABLE |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * MxAction:state:
   *
   * The state of the action, or %NULL if the action is stateless.
   *
   * Since: 1.4
   */
  g_object_class_install_property (object_class,
                                   PROP_STATE,
                                   g_param_spec_variant ("state",
                                                         "State",
                                                         "The state the action "
                                                         "is in",
                                                         G_VARIANT_TYPE_ANY,
                                                         NULL,
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  /**
   * MxAction::activated
   * @action: the object that received the signal
   *
   * Emitted when the MxAction is activated.
   *
   * Deprecated: 1.4: Use MxAction::activate instead.
   */
  signals[ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActionClass, activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * MxAction::activate:
   * @action: the #MxAction
   * @parameter: (allow-none): the parameter to the activation
   *
   * Indicates that the action was just activated.
   *
   * @parameter will always be of the expected type.  In the event that
   * an incorrect type was given, no signal will be emitted.
   *
   * Since: 1.4
   */
  signals[ACTIVATE] = g_signal_new ("activate",
                                    MX_TYPE_ACTION,
                                    G_SIGNAL_RUN_LAST,
                                    0, NULL, NULL,
                                    g_cclosure_marshal_VOID__VARIANT,
                                    G_TYPE_NONE, 1,
                                    G_TYPE_VARIANT);
}

static void
mx_action_init (MxAction *self)
{
  MxActionPrivate *priv = self->priv = ACTION_PRIVATE (self);
  priv->enabled = TRUE;
}

/**
 * mx_action_new:
 *
 * Creates a new, blank, #MxAction
 *
 * Returns: (transfer full): a newly allocated #MxAction
 */
MxAction *
mx_action_new (void)
{
  return g_object_new (MX_TYPE_ACTION, NULL);
}

/**
 * mx_action_new_with_parameter:
 * @name: the name of the action
 * @parameter_type: (allow-none): the type of parameter to the activate
 * function
 *
 * Creates a new action with a parameter.
 *
 * The created action is stateless.  See mx_action_new_stateful().
 *
 * Returns: a new #MxAction
 *
 * Since: 1.4
 */
MxAction *
mx_action_new_with_parameter (const gchar        *name,
                              const GVariantType *parameter_type)
{
  MxAction *action;

  g_return_val_if_fail (name != NULL, NULL);

  action = g_object_new (MX_TYPE_ACTION,
                         "name", name,
                         "parameter-type", parameter_type,
                         NULL);

  return action;
}

/**
 * mx_action_new_stateful:
 * @name: the name of the action
 * @parameter_type: (allow-none): the type of the parameter to the activate
 * function
 * @state: the initial state of the action
 *
 * Creates a new stateful action.
 *
 * @state is the initial state of the action.  All future state values
 * must have the same #GVariantType as the initial state.
 *
 * Returns: a new #MxAction
 *
 * Since: 1.4
 */
MxAction *
mx_action_new_stateful (const gchar        *name,
                        const GVariantType *parameter_type,
                        GVariant           *state)
{
  MxAction *action;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (state != NULL, NULL);

  action = g_object_new (MX_TYPE_ACTION,
                         "name", name,
                         "parameter-type", parameter_type,
                         "state", state,
                         NULL);

  return action;
}

/**
 * mx_action_new_full:
 * @name: name of the action
 * @display_name: name of the action to display to the user
 * @activated_cb: (type Mx.ActionCallbackFunc) (scope async) (allow-none): callback to connect to the activated signal
 * @user_data: user data to be passed to the callback
 *
 * Creates a new #MxAction with the name and callback set
 *
 * Returns: (transfer full): a newly allocated #MxAction
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

  return action->priv->enabled;
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

  if (priv->enabled != active)
    {
      priv->enabled = active;
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
