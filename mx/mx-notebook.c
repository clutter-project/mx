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
#ifdef HAVE_CLUTTER_GESTURE
#include <clutter-gesture/clutter-gesture.h>
#endif

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxNotebook, mx_notebook, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))

#define NOTEBOOK_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_NOTEBOOK, MxNotebookPrivate))

struct _MxNotebookPrivate
{
  gint    current_page;

  GSList *children;
  gint    n_children;

  gboolean enable_gestures;

#if HAVE_CLUTTER_GESTURE
  ClutterGesture *gesture;
#endif
};

enum
{
  PROP_PAGE = 1,
  PROP_ENABLE_GESTURES
};

static void
mx_notebook_update_children (MxNotebook *book)
{
  MxNotebookPrivate *priv = book->priv;
  gint i;
  GSList *l;

  i = 0;
  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);

      if (i == priv->current_page)
        {
          {
            clutter_actor_set_opacity (child, 0);
            clutter_actor_show (child);
            clutter_actor_animate (child, CLUTTER_LINEAR, 250,
                                   "opacity", 255, NULL);
          }
        }
      else
      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        {
          clutter_actor_animate (child, CLUTTER_LINEAR, 250,
                                 "opacity", 0, NULL);
        }

      i++;
    }
}

static void
mx_notebook_add (ClutterContainer *container,
                 ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
  priv->children = g_slist_append (priv->children, actor);

  priv->n_children++;

  /* hide the actor until its page is displayed */
  if ((priv->n_children - 1) != priv->current_page)
    clutter_actor_hide (actor);

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
mx_notebook_remove (ClutterContainer *container,
                    ClutterActor     *actor)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  GSList *item = NULL;

  item = g_slist_find (priv->children, actor);

  if (item == NULL)
    {
      g_warning ("Actor of type '%s' is not a child of container of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  g_object_ref (actor);

  priv->children = g_slist_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);

  priv->n_children--;

  /* check if the last page was just removed and it was also the visible page */
  if (priv->current_page > (priv->n_children - 1))
    priv->current_page = priv->n_children - 1;


  mx_notebook_update_children (MX_NOTEBOOK (container));
}

static void
mx_notebook_foreach (ClutterContainer *container,
                     ClutterCallback   callback,
                     gpointer          callback_data)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (container)->priv;

  g_slist_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_notebook_add;
  iface->remove = mx_notebook_remove;
  iface->foreach = mx_notebook_foreach;
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
    case PROP_PAGE:
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
    case PROP_PAGE:
      mx_notebook_set_page (MX_NOTEBOOK (object), g_value_get_int (value));
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
  MxNotebook *notebook;

  G_OBJECT_CLASS (mx_notebook_parent_class)->dispose (object);

  notebook = MX_NOTEBOOK (object);

#ifdef HAVE_CLUTTER_GESTURE
  if (notebook->priv->gesture)
    {
      g_object_unref (notebook->priv->gesture);
      notebook->priv->gesture = NULL;
    }
#endif

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
  GSList *l;
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
  GSList *l;
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
  GSList *l;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->paint (actor);

  for (l = MX_NOTEBOOK (actor)->priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }

}

static void
mx_notebook_pick (ClutterActor       *actor,
                  const ClutterColor *color)
{
  GSList *l;

  CLUTTER_ACTOR_CLASS (mx_notebook_parent_class)->pick (actor, color);

  for (l = MX_NOTEBOOK (actor)->priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);

      if (clutter_actor_should_pick_paint (child))
        clutter_actor_paint (child);
    }

}

static void
mx_notebook_allocate (ClutterActor          *actor,
                      const ClutterActorBox *box,
                      ClutterAllocationFlags flags)
{
  MxNotebookPrivate *priv = MX_NOTEBOOK (actor)->priv;
  GSList *l;
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
  MxNotebookPrivate *priv = book->priv;

  if (!priv->enable_gestures)
    return FALSE;

  if (event->direction % 2)
    /* up, left (1, 3) */
    mx_notebook_set_page (book, priv->current_page - 1);
  else
    /* down, right (2, 4) */
    mx_notebook_set_page (book, priv->current_page + 1);

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

  pspec = g_param_spec_int ("page",
                            "Page",
                            "Index of the child to display",
                            0, G_MAXINT, 0,
                            MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PAGE, pspec);

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
mx_notebook_set_page (MxNotebook *book,
                      gint        page)
{
  MxNotebookPrivate *priv;

  g_return_if_fail (MX_IS_NOTEBOOK (book));

  priv = book->priv;

  if (page == priv->current_page)
    return;

  /* check for invalid page numbers */
  if (page < 0 || page > (priv->n_children - 1))
    return;

  priv->current_page = page;

  /* ensure the correct child is visible */
  mx_notebook_update_children (book);

  g_object_notify (G_OBJECT (book), "page");
}

gint
mx_notebook_get_page (MxNotebook *book)
{
  g_return_val_if_fail (MX_IS_NOTEBOOK (book), -1);

  return book->priv->current_page;
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
