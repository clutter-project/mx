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
  gint          current_page;
  ClutterActor *current_actor;

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
mx_notebook_actor_added (ClutterContainer *container,
                         ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;
  ClutterActorIter iter;
  ClutterActor *child;

  if (!priv->current_actor)
    {
      /* First actor */
      priv->current_actor = actor;
      priv->current_page = 0;
      g_object_notify (G_OBJECT (container), "current-page");
    }
  else
    {
      /* If the added actor precedes the current one, we need to
         update current-page property. */
      clutter_actor_iter_init (&iter, CLUTTER_ACTOR (container));
      while (clutter_actor_iter_next (&iter, &child))
        {
          if (actor == child)
            {
              priv->current_page++;
              g_object_notify (G_OBJECT (container), "current-page");
              break;
            }

          if (priv->current_actor == child)
            break;
        }
    }
}

static void
mx_notebook_actor_removed (ClutterContainer *container,
                           ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;
  ClutterActorIter iter;
  ClutterActor *child;
  gint n_children = clutter_actor_get_n_children (CLUTTER_ACTOR (container));

  if (actor == priv->current_actor)
    {
      if (n_children < 1)
        {
          /* No more children */
          priv->current_actor = NULL;
          priv->current_page = -1;
          g_object_notify (G_OBJECT (container), "current-page");
        }
      else
        {
          /* Take previous children */
          if (priv->current_page >= n_children)
            mx_notebook_set_current_page (MX_NOTEBOOK (container),
                                          n_children - 1);
          else
            mx_notebook_set_current_page (MX_NOTEBOOK (container),
                                          MAX (0, priv->current_page - 1));
        }
    }
  else
    {
      /* Removed actor isn't the current. If the removed actor
         precedes the current one, we need to update current-page
         property. */
      gint nth = 0;

      clutter_actor_iter_init (&iter, CLUTTER_ACTOR (container));
      while (clutter_actor_iter_next (&iter, &child))
        {
          if (child == priv->current_actor)
            break;
          nth++;
        }

      if (priv->current_page != nth)
        {
          priv->current_page = nth;
          g_object_notify (G_OBJECT (container), "current-page");
        }
    }
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->actor_added = mx_notebook_actor_added;
  iface->actor_removed = mx_notebook_actor_removed;
}

static MxFocusable *
mx_notebook_accept_focus (MxFocusable *focusable,
                          MxFocusHint  hint)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (focusable)->priv;

  if (priv->current_actor && MX_IS_FOCUSABLE (priv->current_actor))
    return mx_focusable_accept_focus (MX_FOCUSABLE (priv->current_actor), hint);
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
      g_value_set_int (value, priv->current_page);
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
                                    g_value_get_int (value));
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
  MxPadding padding;
  ClutterActorIter iter;
  ClutterActor *child;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width_p)
    *min_width_p = 0;

  if (natural_width_p)
    *natural_width_p = 0;

  clutter_actor_iter_init (&iter, actor);
  while (clutter_actor_iter_next (&iter, &child))
    {
      gfloat child_min, child_nat;

      clutter_actor_get_preferred_width (child, for_height,
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
  MxPadding padding;
  ClutterActorIter iter;
  ClutterActor *child;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_height_p)
    *min_height_p = 0;

  if (natural_height_p)
    *natural_height_p = 0;

  clutter_actor_iter_init (&iter, actor);
  while (clutter_actor_iter_next (&iter, &child))
    {
      gfloat child_min, child_nat;

      clutter_actor_get_preferred_height (child, for_width,
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
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;
  ClutterActorIter iter;
  ClutterActor *child;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->paint (actor);

  clutter_actor_iter_init (&iter, actor);
  while (clutter_actor_iter_next (&iter, &child))
    {
      if (child == priv->current_actor)
        continue;

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }

  if (priv->current_actor)
    clutter_actor_paint (priv->current_actor);
}

static void
mx_notebook_pick (ClutterActor       *actor,
                  const ClutterColor *color)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->pick (actor, color);

  if (priv->current_actor)
    clutter_actor_paint (priv->current_actor);
}

static void
mx_notebook_allocate (ClutterActor          *actor,
                      const ClutterActorBox *box,
                      ClutterAllocationFlags flags)
{
  MxPadding padding;
  ClutterActorBox childbox;
  ClutterActorIter iter;
  ClutterActor *child;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->allocate (actor, box, flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  childbox.x1 = 0 + padding.left;
  childbox.x2 = box->x2 - box->x1 - padding.right;

  childbox.y1 = 0 + padding.top;
  childbox.y2 = box->y2 - box->y1 - padding.bottom;

  clutter_actor_iter_init (&iter, actor);
  while (clutter_actor_iter_next (&iter, &child))
    {
      if (CLUTTER_ACTOR_IS_VISIBLE (child))
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
    /* up, left (1, 3) */
    mx_notebook_previous_page (book);
  else
    /* down, right (2, 4) */
    mx_notebook_next_page (book);

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

  pspec = g_param_spec_int ("current-page",
                            "Current page",
                            "The current ClutterActor being displayed",
                            -1, G_MAXINT, -1,
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
  MxNotebookPrivate *priv;

  self->priv = priv = NOTEBOOK_PRIVATE (self);

  priv->current_page = -1;
}

ClutterActor *
mx_notebook_new (void)
{
  return g_object_new (MX_TYPE_NOTEBOOK, NULL);
}

/**
 * mx_notebook_set_current_page:
 * @self: A #MxNotebook
 * @page_num: A page number
 *
 * Change the current page number in @self.
 */
void
mx_notebook_set_current_page (MxNotebook *self,
                              gint        page_num)
{
  MxNotebookPrivate *priv;
  gint n_children;

  g_return_if_fail (MX_IS_NOTEBOOK (self));

  priv = self->priv;

  n_children = clutter_actor_get_n_children (CLUTTER_ACTOR (self));

  g_return_if_fail (page_num >= 0 && page_num < n_children);

  priv->current_page = page_num;
  priv->current_actor = clutter_actor_get_child_at_index (CLUTTER_ACTOR (self),
                                                          page_num);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));
}

/**
 * mx_notebook_get_current_page:
 * @self: A #MxNotebook
 *
 * Get the number of the current page in @self.
 *
 * Returns: the number of the current page in @self.
 */
gint
mx_notebook_get_current_page (MxNotebook *self)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (self), -1);

  return self->priv->current_page;
}

/**
 * mx_notebook_get_n_pages:
 * @self: A #MxNotebook
 *
 * Get the number of pages in @self.
 *
 * Returns: the number of pages in @self.
 */
gint
mx_notebook_get_n_pages (MxNotebook *self)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (self), -1);

  return clutter_actor_get_n_children (CLUTTER_ACTOR (self));
}

/**
 * mx_notebook_get_nth_page:
 * @self: A #MxNotebook
 * @page_num: A page number
 *
 * Get the actor at @page_num.
 *
 * Returns: (transfer none): the #ClutterActor at @page_num.
 */
ClutterActor *
mx_notebook_get_nth_page (MxNotebook *self,
                          gint        page_num)
{
  MxNotebookPrivate *priv;

  g_return_val_if_fail (MX_IS_NOTEBOOK (self), NULL);

  priv = self->priv;

  if (priv->current_page < 0)
    return NULL;
  else if (clutter_actor_get_n_children (CLUTTER_ACTOR (self)) <= page_num)
    return NULL;

  return clutter_actor_get_child_at_index (CLUTTER_ACTOR (self), page_num);
}

/**
 * mx_notebook_previous_page:
 * @self: A #MxNotebook
 *
 * Change the current page to previous one.
 */
void
mx_notebook_previous_page (MxNotebook *self)
{
  MxNotebookPrivate *priv;
  gint n_children;

  g_return_if_fail (MX_IS_NOTEBOOK (self));
  priv = self->priv;

  if (priv->current_page == -1)
    return;

  n_children = clutter_actor_get_n_children (CLUTTER_ACTOR (self));

  mx_notebook_set_current_page (self,
                                (priv->current_page - 1) % n_children);
}

/**
 * mx_notebook_next_page:
 * @self: A #MxNotebook
 *
 * Change the current page to next one.
 */
void
mx_notebook_next_page (MxNotebook *self)
{
  MxNotebookPrivate *priv;
  gint n_children;

  g_return_if_fail (MX_IS_NOTEBOOK (self));
  priv = self->priv;

  if (priv->current_page == -1)
    return;

  n_children = clutter_actor_get_n_children (CLUTTER_ACTOR (self));

  mx_notebook_set_current_page (self,
                                (priv->current_page + 1) % n_children);
}

void
mx_notebook_set_enable_gestures (MxNotebook *self,
                                 gboolean    enabled)
{
  MxNotebookPrivate *priv;

  g_return_if_fail (MX_IS_NOTEBOOK (self));

  priv = self->priv;

  if (priv->enable_gestures != enabled)
    {
      priv->enable_gestures = enabled;

#ifndef HAVE_CLUTTER_GESTURE
      g_warning ("Gestures are disabled as Clutter Gesture is not available");
#else
      if (enabled && !priv->gesture)
        {
          priv->gesture = clutter_gesture_new (CLUTTER_ACTOR (self));
          clutter_gesture_set_gesture_mask (priv->gesture,
                                            CLUTTER_ACTOR (self),
                                            GESTURE_MASK_SLIDE);
          g_signal_connect (priv->gesture, "gesture-slide-event",
                            G_CALLBACK (mx_notebook_gesture_slide_event_cb),
                            self);
          clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
        }
#endif

      g_object_notify (G_OBJECT (self), "enable-gestures");
    }
}

gboolean
mx_notebook_get_enable_gestures (MxNotebook *self)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (self), FALSE);

  return self->priv->enable_gestures;
}
