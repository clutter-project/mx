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

  guint allow_no_active : 1;
};

enum
{
  PROP_ACTIVE_BUTTON = 1,
  PROP_ALLOW_NO_ACTIVE
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

    case PROP_ALLOW_NO_ACTIVE:
      g_value_set_boolean (value, group->priv->allow_no_active);
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
      mx_button_group_set_active_button (group, g_value_get_object (value));
      break;

    case PROP_ALLOW_NO_ACTIVE:
      mx_button_group_set_allow_no_active (group, g_value_get_boolean (value));
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

  pspec = g_param_spec_boolean ("allow-no-active",
                                "Allow No Active",
                                "Allow no buttons to be active (toggled)",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ALLOW_NO_ACTIVE, pspec);
}

static void
mx_button_group_init (MxButtonGroup *self)
{
  self->priv = BUTTON_GROUP_PRIVATE (self);
}

/**
 * mx_button_group_new:
 *
 * Create a new #MxButtonGroup.
 *
 * Returns: a newly allocated #MxButtonGroup.
 */
MxButtonGroup *
mx_button_group_new (void)
{
  return g_object_new (MX_TYPE_BUTTON_GROUP, NULL);
}

static void
button_toggled_notify_cb (MxButton      *button,
                          GParamSpec    *pspec,
                          MxButtonGroup *group)
{
  if (mx_button_get_toggled (button))
    mx_button_group_set_active_button (group, button);
  else
    mx_button_group_set_active_button (group, NULL);
}

static gboolean
button_click_intercept (MxButton           *button,
                        ClutterButtonEvent *event,
                        MxButtonGroup      *group)
{
  if (button == group->priv->active_button && !group->priv->allow_no_active)
    return TRUE;
  else
    return FALSE;
}

static void
button_weak_notify (MxButtonGroup *group,
                    MxButton      *button)
{
  MxButtonGroupPrivate *priv = group->priv;
  GSList *l, *prev = NULL, *next = NULL;

  for (l = priv->children; l; l = g_slist_next (l))
    {
      if ((MxButton *) l->data == button)
        {

          next = g_slist_next (l);
          group->priv->children = g_slist_remove (priv->children,
                                                  button);
          break;
        }
      prev = l;
    }

  if (priv->active_button == button)
    {
      /* Try and select another button if the one we've removed is active.
       * But we shouldn't do this in the case where we allow no active button.
       */
      if (priv->allow_no_active)
        {
          mx_button_group_set_active_button (group, NULL);
        }
      else if (prev)
        {
          mx_button_group_set_active_button (group, (MxButton *) prev->data);
        }
      else if (next)
        {
          mx_button_group_set_active_button (group, (MxButton *) next->data);
        }
      else if (priv->children)
        {
          mx_button_group_set_active_button (group,
                                             (MxButton *) priv->children->data);
        }
      else
        {
          mx_button_group_set_active_button (group, NULL);
        }
    }
}

/**
 * mx_button_group_add:
 * @group: A #MxButtonGroup
 * @button: A #MxButton
 *
 * Add @button to the #MxButtonGroup.
 *
 */
void
mx_button_group_add (MxButtonGroup   *group,
                     MxButton        *button)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (MX_IS_BUTTON (button));

  group->priv->children = g_slist_prepend (group->priv->children, button);

  g_signal_connect (button, "notify::toggled",
                    G_CALLBACK (button_toggled_notify_cb), group);
  g_signal_connect (button, "button-press-event",
                    G_CALLBACK (button_click_intercept), group);
  g_signal_connect (button, "button-release-event",
                    G_CALLBACK (button_click_intercept), group);

  g_object_weak_ref (G_OBJECT (button), (GWeakNotify) button_weak_notify,
                     group);
}

/**
 * mx_button_group_remove:
 * @group: A #MxButtonGroup
 * @button: A #MxButton
 *
 * Remove @button from the #MxButtonGroup
 *
 */
void
mx_button_group_remove (MxButtonGroup   *group,
                        MxButton        *button)
{
  GSList *l, *prev = NULL, *next;
  MxButtonGroupPrivate *priv;
  gboolean found;

  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (MX_IS_BUTTON (button));

  priv = group->priv;

  /* check the button exists in this group */
  found = FALSE;
  for (l = priv->children; l; l = g_slist_next (l))
    {
      if ((MxButton*) l->data == button)
        {
          found = TRUE;
          break;
        }
      prev = l;
    }
  if (!found)
    return;

  next = g_slist_next (l);
  priv->children = g_slist_remove (priv->children, button);

  g_signal_handlers_disconnect_by_func (button, button_toggled_notify_cb,
                                        group);
  g_signal_handlers_disconnect_by_func (button, button_click_intercept, group);

  g_object_weak_unref (G_OBJECT (button), (GWeakNotify) button_weak_notify,
                       group);

  if (priv->active_button == button)
    {
      /* Try and select another button if the one we've removed is active.
       * But we shouldn't do this in the case where we allow no active button.
       */
      if (priv->allow_no_active)
        {
          mx_button_group_set_active_button (group, NULL);
        }
      else if (prev)
        {
          mx_button_group_set_active_button (group, (MxButton *) prev->data);
        }
      else if (next)
        {
          mx_button_group_set_active_button (group, (MxButton *) next->data);
        }
      else if (priv->children)
        {
          mx_button_group_set_active_button (group,
                                             (MxButton *) priv->children->data);
        }
      else
        {
          mx_button_group_set_active_button (group, NULL);
        }
    }
}

/**
 * mx_button_group_foreach:
 * @group: A #MxButtonGroup
 * @callback: (scope call): A #ClutterCallback
 * @userdata: (closure): A #gpointer
 *
 * Calls @callback for each button in the group.
 *
 */
void
mx_button_group_foreach (MxButtonGroup   *group,
                         ClutterCallback  callback,
                         gpointer         userdata)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));
  g_return_if_fail (callback != NULL);

  g_slist_foreach (group->priv->children, (GFunc) callback, userdata);
}

/**
 * mx_button_group_set_active_button:
 * @group: A #MxButtonGroup
 * @button: (allow-none): A #MxButton
 *
 * Set the current active button in the group. The previous active button will
 * have #MxButton:toggled set to #FALSE.
 *
 */
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

  if (priv->active_button)
    mx_button_set_toggled (priv->active_button, FALSE);

  if (button)
    mx_button_set_toggled (button, TRUE);

  priv->active_button = button;

  g_object_notify (G_OBJECT (group), "active-button");
}

/**
 * mx_button_group_get_active_button:
 * @group: A #MxButtonGroup
 *
 * Get the current active button
 *
 * Returns: (transfer none): the currently active button
 */
MxButton *
mx_button_group_get_active_button (MxButtonGroup *group)
{
  MxButtonGroupPrivate *priv;

  g_return_val_if_fail (MX_IS_BUTTON_GROUP (group), NULL);

  priv = group->priv;

  return priv->active_button;
}

/**
 * mx_button_group_set_allow_no_active:
 * @group: A #MxButtonGroup
 * @allow_no_active: A #gboolean
 *
 * Set the value of the #MxButtonGroup:allow-no-active property.
 *
 */
void
mx_button_group_set_allow_no_active (MxButtonGroup *group,
                                     gboolean       allow_no_active)
{
  g_return_if_fail (MX_IS_BUTTON_GROUP (group));

  if (group->priv->allow_no_active != allow_no_active)
    {
      group->priv->allow_no_active = allow_no_active;

      g_object_notify (G_OBJECT (group), "allow-no-active");
    }
}

/**
 * mx_button_group_get_allow_no_active:
 * @group: A #MxButtonGroup
 *
 * Get the value of the #MxButtonGroup:allow-no-active property.
 *
 * Returns: the value of the "allow-no-active" property.
 */
gboolean
mx_button_group_get_allow_no_active (MxButtonGroup *group)
{
  g_return_val_if_fail (MX_IS_BUTTON_GROUP (group), FALSE);

  return group->priv->allow_no_active;
}

/**
 * mx_button_group_get_buttons:
 * @group: A #MxButtonGroup
 *
 * Get a list of the buttons in the button group.
 *
 * Returns: (element-type Mx.Button): a list of buttons. The list is
 *   owned by the #MxButtonGroup and should not be modified by the
 *   application.
 */
const GSList *
mx_button_group_get_buttons (MxButtonGroup *group)
{
  g_return_val_if_fail (MX_IS_BUTTON_GROUP (group), NULL);

  return group->priv->children;
}
