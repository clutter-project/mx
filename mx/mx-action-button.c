/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-action-button.c: MxActionButton object
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
 * @short_description: A button that activates an MxAction
 *
 * A button that when clicked activates an MxAction.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include <mx/mx-action-button.h>
#include <mx/mx-box-layout.h>
#include <mx/mx-box-layout-child.h>
#include <mx/mx-icon.h>
#include <mx/mx-label.h>

enum {
  PROP_0,
  PROP_ACTION,
  PROP_ICON_POSITION,
  PROP_ICON_VISIBLE,
  PROP_TEXT_VISIBLE
};

enum {
  LAST_SIGNAL,
};

struct _MxActionButtonPrivate {
  MxAction   *action;
  MxPosition  icon_position;
  guint       icon_visible : 1;
  guint       text_visible : 1;

  ClutterActor *hbox;
  ClutterActor *icon;
  ClutterActor *label;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_ACTION_BUTTON, MxActionButtonPrivate))
G_DEFINE_TYPE (MxActionButton, mx_action_button, MX_TYPE_BUTTON);

static void
mx_action_button_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_action_button_parent_class)->finalize (object);
}

static void
mx_action_button_dispose (GObject *object)
{
  MxActionButton *self = (MxActionButton *) object;
  MxActionButtonPrivate *priv = self->priv;

  if (priv->action)
    {
      g_object_unref (priv->action);
      priv->action = NULL;
    }

  G_OBJECT_CLASS (mx_action_button_parent_class)->dispose (object);
}

static void
mx_action_button_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MxActionButton *self = (MxActionButton *) object;

  switch (prop_id)
    {
    case PROP_ACTION:
      mx_action_button_set_action (self, g_value_get_object (value));
      break;

    case PROP_ICON_POSITION:
      mx_action_button_set_icon_position (self, g_value_get_enum (value));
      break;

    case PROP_ICON_VISIBLE:
      mx_action_button_set_icon_visible (self, g_value_get_boolean (value));
      break;

    case PROP_TEXT_VISIBLE:
      mx_action_button_set_text_visible (self, g_value_get_boolean (value));
      break;

    default:
      break;
    }
}

static void
mx_action_button_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MxActionButton *self = (MxActionButton *) object;
  MxActionButtonPrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_ACTION:
      g_value_set_object (value, priv->action);
      break;

    case PROP_ICON_POSITION:
      g_value_set_enum (value, priv->icon_position);
      break;

    case PROP_ICON_VISIBLE:
      g_value_set_boolean (value, priv->icon_visible);
      break;

    case PROP_TEXT_VISIBLE:
      g_value_set_boolean (value, priv->text_visible);
      break;

    default:
      break;
    }
}

static void
mx_action_button_clicked (MxButton *button)
{
  MxActionButton *action_button = MX_ACTION_BUTTON (button);
  MxActionButtonPrivate *priv = action_button->priv;

  /* Why is this not mx_action_activated ()? */
  g_signal_emit_by_name (priv->action, "activated");
}

static void
mx_action_button_class_init (MxActionButtonClass *klass)
{
  GObjectClass *o_class = (GObjectClass *) klass;
  MxButtonClass *b_class = (MxButtonClass *) klass;
  GParamSpec *pspec;

  o_class->dispose = mx_action_button_dispose;
  o_class->finalize = mx_action_button_finalize;
  o_class->set_property = mx_action_button_set_property;
  o_class->get_property = mx_action_button_get_property;

  b_class->clicked = mx_action_button_clicked;

  g_type_class_add_private (klass, sizeof (MxActionButtonPrivate));

  pspec = g_param_spec_object ("action", "Action", "Associated action",
                               MX_TYPE_ACTION,
                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (o_class, PROP_ACTION, pspec);

  pspec = g_param_spec_enum ("icon-position", "Icon position",
                             "The position of the icon, relative to the text",
                             MX_TYPE_POSITION, MX_POSITION_LEFT,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (o_class, PROP_ICON_POSITION, pspec);

  pspec = g_param_spec_boolean ("icon-visible", "Icon visible",
                                "Whether to show the icon",
                                TRUE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (o_class, PROP_ICON_VISIBLE, pspec);

  pspec = g_param_spec_boolean ("text-visible", "Text visible",
                                "Whether to show the text",
                                TRUE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (o_class, PROP_TEXT_VISIBLE, pspec);
}

static void
mx_action_button_update_contents (MxActionButton *self)
{
  gboolean icon_visible;
  MxActionButtonPrivate *priv = self->priv;

  /* If the icon doesn't have a name set, treat it as
   * not-visible.
   */
  if (priv->icon_visible && mx_icon_get_icon_name (MX_ICON (priv->icon)))
    icon_visible = TRUE;
  else
    icon_visible = FALSE;

  /* Handle the simple cases first */
  if (!icon_visible &&
      !priv->text_visible)
    {
      clutter_actor_hide (priv->hbox);
      return;
    }

  if (icon_visible &&
      !priv->text_visible)
    {
      clutter_actor_show (priv->icon);
      clutter_actor_hide (priv->label);
      clutter_actor_lower_bottom (priv->icon);
      return;
    }

  if (!icon_visible &&
      priv->text_visible)
    {
      clutter_actor_hide (priv->icon);
      clutter_actor_show (priv->label);
      clutter_actor_lower_bottom (priv->label);
      return;
    }

  /* Both the icon and text are visible, handle this case */
  clutter_actor_show (priv->icon);
  clutter_actor_show (priv->label);
  switch (priv->icon_position)
    {
    case MX_POSITION_TOP:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_VERTICAL);
      clutter_actor_lower_bottom (priv->icon);
      break;

    case MX_POSITION_RIGHT:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_HORIZONTAL);
      clutter_actor_raise_top (priv->icon);
      break;

    case MX_POSITION_BOTTOM:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_VERTICAL);
      clutter_actor_raise_top (priv->icon);
      break;

    case MX_POSITION_LEFT:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_HORIZONTAL);
      clutter_actor_lower_bottom (priv->icon);
      break;
    }
}

static void
mx_action_button_init (MxActionButton *self)
{
  MxActionButtonPrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->icon_visible = TRUE;
  priv->text_visible = TRUE;

  priv->hbox = mx_box_layout_new ();
  mx_bin_set_child (MX_BIN (self), priv->hbox);

  priv->icon = mx_icon_new ();
  priv->label = mx_label_new ();

  clutter_container_add (CLUTTER_CONTAINER (priv->hbox), priv->icon,
                         priv->label, NULL);
  mx_box_layout_child_set_expand (MX_BOX_LAYOUT (priv->hbox),
                                  priv->label, TRUE);
  mx_box_layout_child_set_y_fill (MX_BOX_LAYOUT (priv->hbox),
                                  priv->label, FALSE);
}

/**
 * mx_action_button_new:
 * @action: A #MxAction
 *
 * Creates a new #MxActionButton that activates @action when clicked.
 *
 * Return value: A newly created #MxActionButton that should be unreffed when
 * finished with.
 */
ClutterActor *
mx_action_button_new (MxAction *action)
{
  return g_object_new (MX_TYPE_ACTION_BUTTON,
                       "action", action,
                       NULL);
}

/**
 * mx_action_button_set_action:
 * @button: A #MxButton
 * @action: A #MxAction
 *
 * Sets @action as the action for @button. @Button will take its label and
 * icon from @action.
 */
void
mx_action_button_set_action (MxActionButton *button,
                             MxAction       *action)
{
  MxActionButtonPrivate *priv;

  g_return_if_fail (MX_IS_ACTION_BUTTON (button));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = button->priv;

  priv->action = g_object_ref_sink (action);

  mx_icon_set_icon_name (MX_ICON (priv->icon), mx_action_get_icon (action));
  mx_label_set_text (MX_LABEL (priv->label),
                     mx_action_get_display_name (action));
}

/**
 * mx_action_button_get_action:
 * @button: A #MxActionButton
 *
 * Retrieves the #MxAction associated with @button.
 *
 * Returns: A #MxAction
 */
MxAction *
mx_action_button_get_action (MxActionButton *button)
{
  g_return_val_if_fail (MX_IS_ACTION_BUTTON (button), NULL);

  return button->priv->action;
}

/**
 * mx_action_button_set_icon_position:
 * @button: A #MxActionButton
 * @position: A #MxPosition
 *
 * Sets the icon position, relative to the text on the button.
 */
void
mx_action_button_set_icon_position (MxActionButton *button,
                                    MxPosition      position)
{
  MxActionButtonPrivate *priv;

  g_return_if_fail (MX_IS_ACTION_BUTTON (button));

  priv = button->priv;
  if (priv->icon_position != position)
    {
      priv->icon_position = position;
      mx_action_button_update_contents (button);
      g_object_notify (G_OBJECT (button), "icon-position");
    }
}

/**
 * mx_action_button_get_icon_position:
 * @button: A #MxActionButton
 *
 * Retrieves the icon's relative position to the text.
 *
 * Returns: A #MxPosition
 */
MxPosition
mx_action_button_get_icon_position (MxActionButton *button)
{
  g_return_val_if_fail (MX_IS_ACTION_BUTTON (button), MX_POSITION_LEFT);
  return button->priv->icon_position;
}

/**
 * mx_action_button_set_icon_visible:
 * @button: A #MxActionButton
 * @visible: %TRUE if the icon should be visible
 *
 * Sets the visibility of the icon associated with the button's action.
 */
void
mx_action_button_set_icon_visible (MxActionButton *button,
                                   gboolean        visible)
{
  MxActionButtonPrivate *priv;

  g_return_if_fail (MX_IS_ACTION_BUTTON (button));

  priv = button->priv;
  if (priv->icon_visible != visible)
    {
      priv->icon_visible = visible;
      mx_action_button_update_contents (button);
      g_object_notify (G_OBJECT (button), "icon-visible");
    }
}

/**
 * mx_action_button_get_icon_visible:
 * @button: A #MxActionButton
 *
 * Retrieves the visibility of the icon associated with the button's action.
 *
 * Returns: %TRUE if the icon is visible, %FALSE otherwise
 */
gboolean
mx_action_button_get_icon_visible (MxActionButton *button)
{
  g_return_val_if_fail (MX_IS_ACTION_BUTTON (button), FALSE);
  return button->priv->icon_visible;
}

/**
 * mx_action_button_set_text_visible:
 * @button: A #MxActionButton
 * @visible: %TRUE if the text should be visible
 *
 * Sets the visibility of the text associated with the button's action.
 */
void
mx_action_button_set_text_visible (MxActionButton *button,
                                   gboolean        visible)
{
  MxActionButtonPrivate *priv;

  g_return_if_fail (MX_IS_ACTION_BUTTON (button));

  priv = button->priv;
  if (priv->text_visible != visible)
    {
      priv->text_visible = visible;
      mx_action_button_update_contents (button);
      g_object_notify (G_OBJECT (button), "text-visible");
    }
}

/**
 * mx_action_button_get_text_visible:
 * @button: A #MxActionButton
 *
 * Retrieves the visibility of the text associated with the button's action.
 *
 * Returns: %TRUE if the text is visible, %FALSE otherwise
 */
gboolean
mx_action_button_get_text_visible (MxActionButton *button)
{
  g_return_val_if_fail (MX_IS_ACTION_BUTTON (button), FALSE);
  return button->priv->text_visible;
}
