/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-notebook: notebook actor
 *
 * Copyright 2009, 2010 Intel Corporation.
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

#include <config.h>
#include "mx-notebook.h"
#include "mx-private.h"
#include "mx-focusable.h"
#ifdef HAVE_CLUTTER_GESTURE
#include <clutter-gesture/clutter-gesture.h>
#endif

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxNotebook, mx_notebook, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init))

#define NOTEBOOK_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_NOTEBOOK, MxNotebookPrivate))

struct _MxNotebookPrivate
{
  ClutterActor *current_page;

  GList *children;

  gboolean enable_gestures;

#if HAVE_CLUTTER_GESTURE
  ClutterGesture *gesture;
#endif
};

enum
{
  PROP_CURRENT_PAGE = 1,
  PROP_ENABLE_GESTURES
};

static void
mx_notebook_show_complete_cb (MxNotebook *book)
{
  MxNotebookPrivate *priv = book->priv;
  GList *l;

  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);
      if (child != priv->current_page)
        {
          clutter_actor_hide (child);
          clutter_actor_set_opacity (child, 0x00);
        }
    }
}

static void
mx_notebook_update_children (MxNotebook *book)
{
  MxNotebookPrivate *priv = book->priv;
  GList *l;

  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);
      ClutterAnimation *anim = clutter_actor_get_animation (child);

      if (anim)
        {
          /* A bit of a hack - we want to just abort the animation,
           * but there's no way of aborting an animation that was
           * started with clutter_actor_animate().
           */
          guint8 opacity = clutter_actor_get_opacity (child);
          g_signal_handlers_disconnect_by_func (anim,
                                                mx_notebook_show_complete_cb,
                                                book);
          clutter_animation_completed (anim);
          clutter_actor_set_opacity (child, opacity);
        }

      if (child == priv->current_page)
        {
          clutter_actor_show (child);
          clutter_actor_animate (child, CLUTTER_LINEAR, 250,
                                 "opacity", 255,
                                 "signal-swapped::completed",
                                   mx_notebook_show_complete_cb,
                                   book,
                                 NULL);
        }
    }
}

static void
mx_notebook_add (ClutterContainer *container,
                 ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
  priv->children = g_list_append (priv->children, actor);

  if (!priv->current_page)
    {
      priv->current_page = actor;
      clutter_actor_set_opacity (actor, 0xff);
      g_object_notify (G_OBJECT (container), "current-page");
    }
  else
    clutter_actor_hide (actor);

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
mx_notebook_remove (ClutterContainer *container,
                    ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  GList *item = NULL;

  item = g_list_find (priv->children, actor);

  if (item == NULL)
    {
      g_warning ("Actor of type '%s' is not a child of container of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  /* If it was the current page, select either the previous or
   * the next, whichever exists first.
   */
  if (actor == priv->current_page)
    {
      priv->current_page = item->prev ? item->prev->data :
        (item->next ? item->next->data : NULL);
      g_object_notify (G_OBJECT (container), "current-page");
    }

  g_object_ref (actor);

  priv->children = g_list_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);

  mx_notebook_update_children (MX_NOTEBOOK (container));
}

static void
mx_notebook_foreach (ClutterContainer *container,
                     ClutterCallback   callback,
                     gpointer          callback_data)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  g_list_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_notebook_add;
  iface->remove = mx_notebook_remove;
  iface->foreach = mx_notebook_foreach;
}

static MxFocusable *
mx_notebook_accept_focus (MxFocusable *focusable,
                          MxFocusHint  hint)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (focusable)->priv;

  if (priv->current_page && MX_IS_FOCUSABLE (priv->current_page))
    return mx_focusable_accept_focus (MX_FOCUSABLE (priv->current_page), hint);
  else
    return NULL;
}

static MxFocusable *
mx_notebook_move_focus (MxFocusable      *focusable,
                        MxFocusDirection  direction,
                        MxFocusable      *from)
{
  return NULL;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_notebook_accept_focus;
  iface->move_focus = mx_notebook_move_focus;
}

static void
mx_notebook_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (object)->priv;

  switch (property_id)
    {
    case PROP_CURRENT_PAGE:
      g_value_set_object (value, priv->current_page);
      break;

    case PROP_ENABLE_GESTURES:
      g_value_set_boolean (value, priv->enable_gestures);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_notebook_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_CURRENT_PAGE:
      mx_notebook_set_current_page (MX_NOTEBOOK (object),
                                    (ClutterActor *)g_value_get_object (value));
      break;

    case PROP_ENABLE_GESTURES:
      mx_notebook_set_enable_gestures (MX_NOTEBOOK (object),
                                       g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_notebook_dispose (GObject *object)
{
#ifdef HAVE_CLUTTER_GESTURE
  MxNotebookPrivate *priv = MX_NOTEBOOK (object)->priv;

  if (priv->gesture)
    {
      g_object_unref (priv->gesture);
      priv->gesture = NULL;
    }
#endif

  G_OBJECT_CLASS (mx_notebook_parent_class)->dispose (object);
}

static void
mx_notebook_destroy (ClutterActor *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;

  g_list_foreach (priv->children, (GFunc) clutter_actor_destroy, NULL);

  if (CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->destroy)
    CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->destroy (actor);
}

static void
mx_notebook_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_notebook_parent_class)->finalize (object);
}

static void
mx_notebook_get_preferred_width (ClutterActor *actor,
                                 gfloat        for_height,
                                 gfloat       *min_width_p,
                                 gfloat       *natural_width_p)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;
  GList *l;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width_p)
    *min_width_p = 0;

  if (natural_width_p)
    *natural_width_p = 0;

  for (l = priv->children; l; l = l->next)
    {
      gfloat child_min, child_nat;

      clutter_actor_get_preferred_width (CLUTTER_ACTOR (l->data), for_height,
                                         &child_min, &child_nat);

      if (min_width_p)
        *min_width_p = MAX ((*min_width_p), child_min);

      if (natural_width_p)
        *natural_width_p = MAX ((*natural_width_p), child_nat);
    }

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;

}

static void
mx_notebook_get_preferred_height (ClutterActor *actor,
                                  gfloat        for_width,
                                  gfloat       *min_height_p,
                                  gfloat       *natural_height_p)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;
  GList *l;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_height_p)
    *min_height_p = 0;

  if (natural_height_p)
    *natural_height_p = 0;

  for (l = priv->children; l; l = l->next)
    {
      gfloat child_min, child_nat;

      clutter_actor_get_preferred_height (CLUTTER_ACTOR (l->data), for_width,
                                          &child_min, &child_nat);

      if (min_height_p)
        *min_height_p = MAX (*min_height_p, child_min);

      if (natural_height_p)
        *natural_height_p = MAX (*natural_height_p, child_nat);
    }

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;

}

static void
mx_notebook_paint (ClutterActor *actor)
{
  GList *l;

  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->paint (actor);

  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);

      if (child == priv->current_page)
        continue;

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }

  if (priv->current_page)
    clutter_actor_paint (priv->current_page);
}

static void
mx_notebook_pick (ClutterActor       *actor,
                  const ClutterColor *color)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->pick (actor, color);

  if (priv->current_page)
    clutter_actor_paint (priv->current_page);
}

static void
mx_notebook_allocate (ClutterActor          *actor,
                      const ClutterActorBox *box,
                      ClutterAllocationFlags flags)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;
  GList *l;
  MxPadding padding;
  ClutterActorBox childbox;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->allocate (actor, box, flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  childbox.x1 = 0 + padding.left;
  childbox.x2 = box->x2 - box->x1 - padding.right;

  childbox.y1 = 0 + padding.top;
  childbox.y2 = box->y2 - box->y1 - padding.bottom;

  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child;

      child = CLUTTER_ACTOR (l->data);

      if (CLUTTER_ACTOR_IS_VISIBLE (l->data))
        clutter_actor_allocate (child, &childbox, flags);
    }
}

#ifdef HAVE_CLUTTER_GESTURE
static gboolean
mx_notebook_gesture_slide_event_cb (ClutterGesture           *gesture,
                                    ClutterGestureSlideEvent *event,
                                    MxNotebook               *book)
{
  GList *item;
  MxNotebookPrivate *priv = book->priv;

  if (!priv->enable_gestures || !priv->current_page)
    return FALSE;

  item = g_list_find (priv->children, priv->current_page);
  if (!item)
    {
      g_warning ("Current page not found in child list");
      return FALSE;
    }

  if (event->direction % 2)
    {
      /* up, left (1, 3) */
      if (item->prev)
        mx_notebook_set_current_page (book, (ClutterActor *)item->prev->data);
      else
        mx_notebook_set_current_page (book,
                                      (ClutterActor *)g_list_last (item)->data);
    }
  else
    {
      /* down, right (2, 4) */
      if (item->next)
        mx_notebook_set_current_page (book, (ClutterActor *)item->next->data);
      else
        mx_notebook_set_current_page (book,
                                      (ClutterActor *)priv->children->data);
    }

  return TRUE;
}
#endif

static void
mx_notebook_class_init (MxNotebookClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxNotebookPrivate));

  object_class->get_property = mx_notebook_get_property;
  object_class->set_property = mx_notebook_set_property;
  object_class->dispose = mx_notebook_dispose;
  object_class->finalize = mx_notebook_finalize;

  actor_class->allocate = mx_notebook_allocate;
  actor_class->get_preferred_width = mx_notebook_get_preferred_width;
  actor_class->get_preferred_height = mx_notebook_get_preferred_height;
  actor_class->paint = mx_notebook_paint;
  actor_class->pick = mx_notebook_pick;
  actor_class->destroy = mx_notebook_destroy;

  pspec = g_param_spec_object ("current-page",
                               "Current page",
                               "The current ClutterActor being displayed",
                               CLUTTER_TYPE_ACTOR,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CURRENT_PAGE, pspec);

  pspec = g_param_spec_boolean ("enable-gestures",
                                "Enable Gestures",
                                "Enable use of pointer gestures to switch page",
                                FALSE,
                                G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ENABLE_GESTURES, pspec);
}

static void
mx_notebook_init (MxNotebook *self)
{
  self->priv = NOTEBOOK_PRIVATE (self);
}

ClutterActor *
mx_notebook_new (void)
{
  return g_object_new (MX_TYPE_NOTEBOOK, NULL);
}

void
mx_notebook_set_current_page (MxNotebook   *book,
                              ClutterActor *page)
{
  MxNotebookPrivate *priv;

  g_return_if_fail (MX_IS_NOTEBOOK (book));
  g_return_if_fail (CLUTTER_IS_ACTOR (page));

  priv = book->priv;

  if (page == priv->current_page)
    return;

  priv->current_page = page;

  /* ensure the correct child is visible */
  mx_notebook_update_children (book);

  g_object_notify (G_OBJECT (book), "current-page");
}

/**
 * mx_notebook_get_current_page:
 * @notebook: A #MxNotebook
 *
 * Get the current page
 *
 * Returns: (transfer none): the current page
 */
ClutterActor *
mx_notebook_get_current_page (MxNotebook *notebook)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (notebook), NULL);

  return notebook->priv->current_page;
}

void
mx_notebook_set_enable_gestures (MxNotebook *book,
                                 gboolean    enabled)
{
  MxNotebookPrivate *priv;

  g_return_if_fail (MX_IS_NOTEBOOK (book));

  priv = book->priv;

  if (priv->enable_gestures != enabled)
    {
      priv->enable_gestures = enabled;

#ifndef HAVE_CLUTTER_GESTURE
      g_warning ("Gestures are disabled as Clutter Gesture is not available");
#else
      if (enabled && !priv->gesture)
        {
          priv->gesture = clutter_gesture_new (CLUTTER_ACTOR (book));
          clutter_gesture_set_gesture_mask (priv->gesture,
                                            CLUTTER_ACTOR (book),
                                            GESTURE_MASK_SLIDE);
          g_signal_connect (priv->gesture, "gesture-slide-event",
                            G_CALLBACK (mx_notebook_gesture_slide_event_cb),
                            book);
          clutter_actor_set_reactive (CLUTTER_ACTOR (book), TRUE);
        }
#endif

      g_object_notify (G_OBJECT (book), "enable-gestures");
    }
}

gboolean
mx_notebook_get_enable_gestures (MxNotebook *book)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (book), FALSE);

  return book->priv->enable_gestures;
}
