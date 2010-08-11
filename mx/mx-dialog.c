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
 * SECTION: mx-dialog
 * @short_description: A modal dialog actor
 *
 * A dialog actor with a content area and a button bar.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include <mx/mx-action-button.h>
#include <mx/mx-button-group.h>
#include <mx/mx-box-layout.h>
#include <mx/mx-dialog.h>
#include <mx/mx-frame.h>

enum {
  PROP_0,
};

enum {
  LAST_SIGNAL,
};

struct _MxDialogPrivate {
  ClutterActor *layout;
  ClutterActor *content;

  ClutterActor *button_box;
  MxButtonGroup *button_group;
  GList *actions;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_DIALOG, MxDialogPrivate))
G_DEFINE_TYPE (MxDialog, mx_dialog, MX_TYPE_MODAL_FRAME);

static void
mx_dialog_finalize (GObject *object)
{
  MxDialog *self = (MxDialog *) object;
  MxDialogPrivate *priv = self->priv;

  if (priv->actions)
    {
      g_list_free (priv->actions);
      priv->actions = NULL;
    }

  G_OBJECT_CLASS (mx_dialog_parent_class)->finalize (object);
}

static void
mx_dialog_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_dialog_parent_class)->dispose (object);
}

static void
mx_dialog_class_init (MxDialogClass *klass)
{
  GObjectClass *o_class = (GObjectClass *) klass;

  o_class->dispose      = mx_dialog_dispose;
  o_class->finalize     = mx_dialog_finalize;

  g_type_class_add_private (klass, sizeof (MxDialogPrivate));
}

static void
mx_dialog_init (MxDialog *self)
{
  MxDialogPrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->layout = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->layout),
                                 MX_ORIENTATION_VERTICAL);
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->layout);
  clutter_actor_set_name (priv->layout, "dialog-window");

  priv->content = mx_frame_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->layout), priv->content);

  priv->button_box = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->button_box),
                                 MX_ORIENTATION_HORIZONTAL);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->layout),
                               priv->button_box);

  priv->button_group = mx_button_group_new ();
}

/**
 * mx_dialog_new:
 *
 * Creates a new #MxDialog actor
 *
 * Return value: A #ClutterActor
 */
ClutterActor *
mx_dialog_new (void)
{
  return g_object_new (MX_TYPE_DIALOG, NULL);
}

/**
 * mx_dialog_set_content:
 * @self: A #MxDialog
 * @content: A #ClutterActor containing the contents
 *
 * Puts @content in the content area of @dialog
 */
void
mx_dialog_set_content (MxDialog     *self,
                       ClutterActor *content)
{
  MxDialogPrivate *priv;

  g_return_if_fail (MX_IS_DIALOG (self));
  priv = self->priv;

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content), content);
}

/**
 * mx_dialog_add_action:
 * @self: A #MxDialog
 * @action: A #MxAction
 *
 * Adds a #MxActionButton that represents @action to the button area of @dialog
 */
void
mx_dialog_add_action (MxDialog *self,
                      MxAction *action)
{
  MxDialogPrivate *priv;
  ClutterActor *button;

  g_return_if_fail (MX_IS_DIALOG (self));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = self->priv;

  priv->actions = g_list_append (priv->actions, action);

  button = mx_action_button_new (action);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->button_box), button);
  mx_button_group_add (priv->button_group, (MxButton *) button);
}

/**
 * mx_dialog_remove_action:
 * @self: A #MxDialog
 * @action: A #MxAction
 *
 * Removes the button associated with @action from the button area of @dialog
 */
void
mx_dialog_remove_action (MxDialog *self,
                         MxAction *action)
{
  MxDialogPrivate *priv;

  g_return_if_fail (MX_IS_DIALOG (self));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = self->priv;

  priv->actions = g_list_remove (priv->actions, action);

  /* FIXME: Remove button */
}
