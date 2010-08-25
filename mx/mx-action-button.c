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
  PROP_ACTION
};

enum {
  LAST_SIGNAL,
};

struct _MxActionButtonPrivate {
  MxAction *action;

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
}

static void
mx_action_button_init (MxActionButton *self)
{
  MxActionButtonPrivate *priv = GET_PRIVATE (self);
  ClutterActor *hbox;

  self->priv = priv;

  hbox = mx_box_layout_new ();
  mx_bin_set_child (MX_BIN (self), hbox);

  priv->icon = mx_icon_new ();
  priv->label = mx_label_new ();

  clutter_container_add (CLUTTER_CONTAINER (hbox), priv->icon,
                         priv->label, NULL);
  mx_box_layout_child_set_expand (MX_BOX_LAYOUT (hbox), priv->label, TRUE);
  mx_box_layout_child_set_y_fill (MX_BOX_LAYOUT (hbox), priv->label, FALSE);
}

/**
 * mx_action_button_new:
 * @action: An #MxAction
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
 * @button: An #MxButton
 * @action: An #MxAction
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
 * @button: An #MxActionButton
 *
 * Retrieves the #MxAction associated with @button.
 *
 * Return value: An @MxAction
 */
MxAction *
mx_action_button_get_action (MxActionButton *button)
{
  g_return_val_if_fail (MX_IS_ACTION_BUTTON (button), NULL);

  return button->priv->action;
}
