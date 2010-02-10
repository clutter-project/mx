/*
 * mx-focus-manager: Keyboard focus manager object
 *
 * Copyright 2010 Intel Corporation
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
 *
 */

#include "mx-focus-manager.h"
#include "mx-focusable.h"
#include "mx-private.h"

#include <clutter/clutter-keysyms.h>

G_DEFINE_TYPE (MxFocusManager, mx_focus_manager, G_TYPE_OBJECT)

#define FOCUS_MANAGER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_FOCUS_MANAGER, MxFocusManagerPrivate))

struct _MxFocusManagerPrivate
{
  ClutterActor *stage;

  MxFocusable *focused;
};

enum
{
  PROP_STAGE = 1,
  PROP_FOCUSED
};

static void
mx_focus_manager_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MxFocusManagerPrivate *priv = MX_FOCUS_MANAGER (object)->priv;

  switch (property_id)
    {
  case PROP_STAGE:
    g_value_set_object (value, priv->stage);
    break;

  case PROP_FOCUSED:
    g_value_set_object (value, priv->focused);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_focus_manager_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MxFocusManager *manager = MX_FOCUS_MANAGER (object);
  switch (property_id)
    {
  case PROP_STAGE:
    mx_focus_manager_set_stage (manager, g_value_get_object (value));
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_focus_manager_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_focus_manager_parent_class)->dispose (object);
}

static void
mx_focus_manager_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_focus_manager_parent_class)->finalize (object);
}

static void
mx_focus_manager_class_init (MxFocusManagerClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFocusManagerPrivate));

  object_class->get_property = mx_focus_manager_get_property;
  object_class->set_property = mx_focus_manager_set_property;
  object_class->dispose = mx_focus_manager_dispose;
  object_class->finalize = mx_focus_manager_finalize;

  pspec = g_param_spec_object ("stage",
                               "Stage",
                               "Top level container for focusables",
                               CLUTTER_TYPE_STAGE,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_STAGE, pspec);

  pspec = g_param_spec_object ("focused",
                               "Focused",
                               "The object that currently has focus",
                               CLUTTER_TYPE_STAGE,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_STAGE, pspec);
}

static void
mx_focus_manager_init (MxFocusManager *self)
{
  self->priv = FOCUS_MANAGER_PRIVATE (self);
}

static void
mx_focus_manager_weak_notify (MxFocusManager *manager,
                              ClutterStage   *stage)
{
  manager->priv->stage = NULL;
}

static void
mx_focus_manager_start_focus (MxFocusManager *manager, MxFocusHint hint)
{
  MxFocusManagerPrivate *priv = manager->priv;
  MxFocusable *focusable;

  GList *children, *l;

  children = clutter_container_get_children (CLUTTER_CONTAINER (priv->stage));
  focusable = NULL;

  if (hint == MX_LAST)
    children = g_list_reverse (children);

  for (l = children; l; l = g_list_next (l))
    {
      if (MX_IS_FOCUSABLE (l->data))
        {
          focusable = MX_FOCUSABLE (l->data);
          break;
        }
    }

  g_list_free (children);

  if (focusable)
    {
      priv->focused = mx_focusable_accept_focus (focusable, hint);
    }
}

static gboolean
mx_focus_manager_ensure_focused (MxFocusManager *manager, ClutterStage *stage)
{
  ClutterActor *actor;
  MxFocusManagerPrivate *priv = manager->priv;

  if (!priv->focused)
    {
      /* try and guess the current focusable by working up the scene graph from
       * the actor that currently has key focus until a focusable is found */
      actor = clutter_stage_get_key_focus (stage);

      while (actor)
        {
          if (MX_IS_FOCUSABLE (actor))
            break;

          actor = clutter_actor_get_parent (actor);
        }

      /* if we didn't find a focusable, try any children of the stage */
      if (!actor)
        mx_focus_manager_start_focus (manager, MX_FIRST);
      else
        priv->focused = mx_focusable_accept_focus (MX_FOCUSABLE (actor),
                                                   MX_FIRST);
    }

  return (priv->focused) ? TRUE : FALSE;
}

static gboolean
mx_focus_manager_event_cb (ClutterStage   *stage,
                           ClutterEvent   *event,
                           MxFocusManager *manager)
{
  MxFocusHint hint;
  MxDirection direction;
  MxFocusManagerPrivate *priv = manager->priv;

  if (event->type != CLUTTER_KEY_PRESS)
    return FALSE;

  switch (event->key.keyval)
    {
  case CLUTTER_Tab:

    if (!priv->focused)
      return mx_focus_manager_ensure_focused (manager, stage);

    if (event->key.modifier_state & CLUTTER_SHIFT_MASK)
      {
        direction = MX_PREVIOUS;
        hint = MX_LAST;
      }
    else
      {
        direction = MX_NEXT;
        hint = MX_FIRST;
      }

    priv->focused = mx_focusable_move_focus (priv->focused, direction,
                                             priv->focused);

    /* if focusable is NULL, then we reached the end of the focus chain */
    if (!priv->focused)
      mx_focus_manager_start_focus (manager, hint);

    return TRUE;
/*
  case CLUTTER_Right:
    if (!mx_focusable_move_focus (focusable, MX_RIGHT))
      result = mx_focusable_move_focus (focusable, MX_NEXT);
    else
      result = TRUE;
    break;
*/
  default:
      return FALSE;
    }
}

MxFocusManager *
mx_focus_manager_new (ClutterStage *stage)
{
  return g_object_new (MX_TYPE_FOCUS_MANAGER, "stage", stage, NULL);
}

ClutterStage*
mx_focus_manager_get_stage (MxFocusManager *manager)
{
  g_return_val_if_fail (MX_IS_FOCUS_MANAGER (manager), NULL);

  return CLUTTER_STAGE (manager->priv->stage);
}

void
mx_focus_manager_set_stage (MxFocusManager *manager,
                            ClutterStage   *stage)
{
  MxFocusManagerPrivate *priv;

  g_return_if_fail (MX_IS_FOCUS_MANAGER (manager));
  g_return_if_fail (CLUTTER_IS_STAGE (stage));

  priv = manager->priv;

  if (priv->stage != (ClutterActor *) stage)
    {

      if (priv->stage)
        g_object_weak_unref (G_OBJECT (priv->stage),
                             (GWeakNotify) mx_focus_manager_weak_notify,
                             manager);

      priv->stage = CLUTTER_ACTOR (stage);

      g_object_weak_ref (G_OBJECT (priv->stage),
                         (GWeakNotify) mx_focus_manager_weak_notify,
                         manager);

      g_signal_connect (priv->stage, "event",
                        G_CALLBACK (mx_focus_manager_event_cb),
                        manager);

      g_object_notify (G_OBJECT (manager), "stage");
    }

}

MxFocusable*
mx_focus_manager_get_focused (MxFocusManager *manager)
{
  g_return_val_if_fail (MX_IS_FOCUS_MANAGER (manager), NULL);

  return manager->priv->focused;
}

/**
 * mx_focus_manager_push_focus:
 * @manager: the focus manager
 * @focused: the object to set focus on
 *
 * Note: the final focused object may not be the same as @focusable if
 * @focusable does not accept focus directly.
 */
void
mx_focus_manager_push_focus (MxFocusManager *manager,
                             MxFocusable    *focusable)
{
  MxFocusManagerPrivate *priv;

  g_return_if_fail (MX_IS_FOCUS_MANAGER (manager));
  g_return_if_fail (MX_IS_FOCUSABLE (focusable));

  priv = manager->priv;

  if (priv->focused != focusable)
    {
      if (priv->focused)
        {
          /* notify the current focusable that focus is being moved */
          mx_focusable_move_focus (priv->focused, MX_OUT, priv->focused);
        }

      priv->focused = mx_focusable_accept_focus (focusable, MX_FIRST);

      g_object_notify (G_OBJECT (manager), "focused");
    }
}

