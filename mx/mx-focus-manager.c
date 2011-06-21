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

static GQuark focus_manager_quark = 0;

struct _MxFocusManagerPrivate
{
  ClutterActor *stage;

  MxFocusable *focused;
  MxFocusable *focused_toplevel;
  guint        refocus_idle;
};

enum
{
  PROP_STAGE = 1,
  PROP_FOCUSED
};

static void mx_focus_manager_set_focused (MxFocusManager *manager, MxFocusable *focusable);

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
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_focus_manager_dispose (GObject *object)
{
  MxFocusManager *self = MX_FOCUS_MANAGER (object);
  MxFocusManagerPrivate *priv = self->priv;

  if (priv->refocus_idle)
    {
      g_source_remove (priv->refocus_idle);
      priv->refocus_idle = 0;
    }

  if (priv->stage)
    {
      g_object_set_qdata (G_OBJECT (priv->stage), focus_manager_quark, NULL);
      priv->stage = NULL;
    }

  if (priv->focused || priv->focused_toplevel)
    mx_focus_manager_set_focused (self, NULL);

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
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_STAGE, pspec);

  pspec = g_param_spec_object ("focused",
                               "Focused",
                               "The object that currently has focus",
                               CLUTTER_TYPE_ACTOR,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_FOCUSED, pspec);

  focus_manager_quark = g_quark_from_static_string ("mx-focus-manager");
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
  g_object_unref (manager);
}

static gboolean
mx_focus_manager_refocus_cb (MxFocusManager *manager)
{
  MxFocusManagerPrivate *priv = manager->priv;

  priv->refocus_idle = 0;

  if (priv->focused_toplevel)
    mx_focus_manager_push_focus_with_hint (manager,
                                           priv->focused_toplevel,
                                           MX_FOCUS_HINT_PRIOR);
  else
    mx_focus_manager_set_focused (manager, NULL);

  return FALSE;
}

static void
mx_focus_manager_focused_mapped_cb (ClutterActor   *focused,
                                    GParamSpec     *pspec,
                                    MxFocusManager *manager)
{
  MxFocusManagerPrivate *priv = manager->priv;

  if (CLUTTER_ACTOR_IS_MAPPED (focused))
    return;

  /* We can't refocus straight away, as during the mapped notification,
   * the actor is still valid (and will just receive focus again).
   *
   * This may also trigger a redraw, which would trigger this Clutter bug:
   * http://bugzilla.clutter-project.org/show_bug.cgi?id=2621
   * (patch submitted).
   */
  if (!priv->refocus_idle)
    priv->refocus_idle =
      g_idle_add_full (G_PRIORITY_HIGH,
                       (GSourceFunc)mx_focus_manager_refocus_cb,
                       manager, NULL);
}

static void
mx_focus_manager_set_focused (MxFocusManager *manager,
                              MxFocusable    *focusable)
{
  MxFocusManagerPrivate *priv = manager->priv;

  if (priv->focused)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->focused),
                                    (gpointer *)&priv->focused);
      g_signal_handlers_disconnect_by_func (priv->focused,
                                            mx_focus_manager_focused_mapped_cb,
                                            manager);
    }

  if (priv->focused_toplevel)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->focused_toplevel),
                                    (gpointer *)&priv->focused_toplevel);
      priv->focused_toplevel = NULL;
    }

  priv->focused = focusable;

  if (priv->focused)
    {
      ClutterActor *parent =
        clutter_actor_get_parent (CLUTTER_ACTOR (priv->focused));

      if (priv->refocus_idle)
        {
          g_source_remove (priv->refocus_idle);
          priv->refocus_idle = 0;
        }

      g_signal_connect_after (priv->focused, "notify::mapped",
                              G_CALLBACK (mx_focus_manager_focused_mapped_cb),
                              manager);
      g_object_add_weak_pointer (G_OBJECT (priv->focused),
                                 (gpointer *)&priv->focused);

      /* Keep track of the highest-level focusable so we can push the focus
       * back onto it if the focused actor disappears.
       */
      while (parent)
        {
          if (MX_IS_FOCUSABLE (parent))
            priv->focused_toplevel = MX_FOCUSABLE (parent);
          parent = clutter_actor_get_parent (parent);
        }

      if (priv->focused_toplevel)
        g_object_add_weak_pointer (G_OBJECT (priv->focused_toplevel),
                                   (gpointer *)&priv->focused_toplevel);
    }
}

static void
mx_focus_manager_start_focus (MxFocusManager *manager, MxFocusHint hint)
{
  MxFocusManagerPrivate *priv = manager->priv;
  MxFocusable *focusable, *new_focused;

  GList *children, *l;

  children = clutter_container_get_children (CLUTTER_CONTAINER (priv->stage));
  focusable = NULL;

  if (hint == MX_FOCUS_HINT_LAST)
    children = g_list_reverse (children);

  for (l = children; l; l = g_list_next (l))
    {
      if (MX_IS_FOCUSABLE (l->data))
        {
          focusable = MX_FOCUSABLE (l->data);
          new_focused = mx_focusable_accept_focus (focusable, hint);
          mx_focus_manager_set_focused (manager, new_focused);

          if (new_focused)
            break;
        }
    }

  g_list_free (children);
}

static gboolean
mx_focus_manager_ensure_focused (MxFocusManager *manager,
                                 ClutterStage   *stage,
                                 MxFocusHint     hint)
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
        {
          mx_focus_manager_start_focus (manager, hint);
        }
      else
        {
          MxFocusable *new_focused;

          new_focused = mx_focusable_accept_focus (MX_FOCUSABLE (actor),
                                                   MX_FOCUS_HINT_PRIOR);
          mx_focus_manager_set_focused (manager, new_focused);
        }

      if (priv->focused)
        g_object_notify (G_OBJECT (manager), "focused");
    }

  return (priv->focused) ? TRUE : FALSE;
}

static gboolean
mx_focus_manager_event_cb (ClutterStage   *stage,
                           ClutterEvent   *event,
                           MxFocusManager *manager)
{
  MxFocusable *old_focus;
  MxFocusManagerPrivate *priv = manager->priv;

  if (event->type != CLUTTER_KEY_PRESS)
    return FALSE;

  old_focus = priv->focused;
  switch (event->key.keyval)
    {
    case CLUTTER_KEY_Tab:
    case CLUTTER_KEY_ISO_Left_Tab:
      if (event->key.modifier_state & CLUTTER_SHIFT_MASK)
        mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_PREVIOUS);
      else
        mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_NEXT);
      break;

    case CLUTTER_KEY_Right:
      mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_RIGHT);
      break;

    case CLUTTER_KEY_Left:
      mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_LEFT);
      break;

    case CLUTTER_KEY_Up:
      mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_UP);
      break;

    case CLUTTER_KEY_Down:
      mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_DOWN);
      break;

    default:
      return FALSE;
    }

  if (priv->focused == old_focus)
    return FALSE;
  else
    return TRUE;
}

/**
 * mx_focus_manager_get_for_stage:
 * @stage: A #ClutterStage
 *
 * Get the MxFocusManager associated with a stage, or create one if none exist
 * for the specified stage.
 *
 * Returns: (transfer none): An #MxFocusManager
 */
MxFocusManager *
mx_focus_manager_get_for_stage (ClutterStage *stage)
{
  MxFocusManager *manager;

  g_return_val_if_fail (CLUTTER_IS_STAGE (stage), NULL);

  manager = g_object_get_qdata (G_OBJECT (stage), focus_manager_quark);

  if (manager == NULL)
    {
      MxFocusManagerPrivate *priv;

      manager = g_object_new (MX_TYPE_FOCUS_MANAGER, NULL);
      priv = manager->priv;

      priv->stage = CLUTTER_ACTOR (stage);

      g_object_set_qdata (G_OBJECT (stage), focus_manager_quark, manager);
      g_object_weak_ref (G_OBJECT (stage),
                         (GWeakNotify) mx_focus_manager_weak_notify, manager);

      g_signal_connect (G_OBJECT (stage), "event",
                        G_CALLBACK (mx_focus_manager_event_cb),
                        manager);
      g_object_notify (G_OBJECT (manager), "stage");
    }

  return manager;
}

/**
 * mx_focus_manager_get_stage:
 * @manager: A #MxFocusManager
 *
 * Get the stage the MxFocusManager is associated with
 *
 * Returns: (transfer none): A #ClutterStage
 */
ClutterStage*
mx_focus_manager_get_stage (MxFocusManager *manager)
{
  g_return_val_if_fail (MX_IS_FOCUS_MANAGER (manager), NULL);

  return CLUTTER_STAGE (manager->priv->stage);
}

/**
 * mx_focus_manager_get_focused:
 * @manager: A #MxFocusManager
 *
 * Get the currently focused #MxFocusable
 *
 * Returns: (transfer none): MxFocusable
 */
MxFocusable*
mx_focus_manager_get_focused (MxFocusManager *manager)
{
  g_return_val_if_fail (MX_IS_FOCUS_MANAGER (manager), NULL);

  return manager->priv->focused;
}

/**
 * mx_focus_manager_push_focus:
 * @manager: the focus manager
 * @focusable: the object to set focus on
 *
 * Sets the currently focused actor, with an #MxFocusHint of
 * %MX_FOCUS_HINT_PRIOR.
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
      MxFocusable *new_focused;

      if (priv->focused)
        {
          /* notify the current focusable that focus is being moved */
          mx_focusable_move_focus (priv->focused, MX_FOCUS_DIRECTION_OUT, priv->focused);
        }

      new_focused = mx_focusable_accept_focus (focusable, MX_FOCUS_HINT_PRIOR);
      mx_focus_manager_set_focused (manager, new_focused);

      g_object_notify (G_OBJECT (manager), "focused");
    }
}

/**
 * mx_focus_manager_push_focus_with_hint:
 * @manager: the focus manager
 * @focusable: the object to set focus on
 * @hint: an #MxFocusHint
 *
 * Similar to #mx_focus_manager_push_focus, but allows the hint to be specified.
 *
 * Note: the final focused object may not be the same as @focusable if
 * @focusable does not accept focus directly.
 *
 * Since: 1.2
 */
void
mx_focus_manager_push_focus_with_hint (MxFocusManager *manager,
                                       MxFocusable    *focusable,
                                       MxFocusHint     hint)
{
  MxFocusManagerPrivate *priv;

  g_return_if_fail (MX_IS_FOCUS_MANAGER (manager));
  g_return_if_fail (MX_IS_FOCUSABLE (focusable));

  priv = manager->priv;

  if (priv->focused != focusable)
    {
      MxFocusable *new_focused;

      if (priv->focused)
        {
          /* notify the current focusable that focus is being moved */
          mx_focusable_move_focus (priv->focused, MX_FOCUS_DIRECTION_OUT,
                                   priv->focused);
        }

      new_focused = mx_focusable_accept_focus (focusable, hint);
      mx_focus_manager_set_focused (manager, new_focused);

      g_object_notify (G_OBJECT (manager), "focused");
    }
}

/**
 * mx_focus_manager_move_focus
 * @manager: the focus manager
 * @direction: The direction to move focus in
 *
 * Moves the current focus in the given direction.
 */
void
mx_focus_manager_move_focus (MxFocusManager   *manager,
                             MxFocusDirection  direction)
{
  MxFocusable *old_focus, *new_focused;
  MxFocusManagerPrivate *priv;

  g_return_if_fail (MX_IS_FOCUS_MANAGER (manager));

  priv = manager->priv;

  old_focus = priv->focused;

  if (priv->focused)
    {
      new_focused = mx_focusable_move_focus (priv->focused,
                                             direction,
                                             priv->focused);
      mx_focus_manager_set_focused (manager, new_focused);
    }

  if (!priv->focused)
    {
      /* If we're going next or previous, we wrap around, otherwise
       * re-focus the last actor.
       */
      switch (direction)
        {
        case MX_FOCUS_DIRECTION_NEXT:
          if (old_focus)
            mx_focus_manager_start_focus (manager, MX_FOCUS_HINT_FIRST);
          else
            mx_focus_manager_ensure_focused (manager,
                                             CLUTTER_STAGE (priv->stage),
                                             MX_FOCUS_HINT_FIRST);
          break;

        case MX_FOCUS_DIRECTION_PREVIOUS:
          if (old_focus)
            mx_focus_manager_start_focus (manager, MX_FOCUS_HINT_LAST);
          else
            mx_focus_manager_ensure_focused (manager,
                                             CLUTTER_STAGE (priv->stage),
                                             MX_FOCUS_HINT_LAST);
          break;

        default:
          /* re-focus the original */
          if (old_focus && (direction != MX_FOCUS_DIRECTION_OUT))
            {
              new_focused = mx_focusable_accept_focus (old_focus, 0);
              mx_focus_manager_set_focused (manager, new_focused);
            }
          else
            mx_focus_manager_ensure_focused (manager,
                                             CLUTTER_STAGE (priv->stage),
                                             MX_FOCUS_HINT_FIRST);
          break;
        }
    }

  /* Notify if the focus has changed */
  if (priv->focused != old_focus)
    g_object_notify (G_OBJECT (manager), "focused");
}

