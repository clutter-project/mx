/*
 * nbtk-popup.c: popup menu class
 *
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "nbtk-popup.h"

G_DEFINE_TYPE (NbtkPopup, nbtk_popup, NBTK_TYPE_WIDGET)

#define POPUP_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_POPUP, NbtkPopupPrivate))

typedef struct
{
  NbtkAction *action;
  NbtkWidget *button;
} NbtkPopupChild;

struct _NbtkPopupPrivate
{
  GArray   *children;
  gboolean  transition_out;
};

enum
{
  ACTION_ACTIVATED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
nbtk_popup_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_popup_set_property (GObject      *object,
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
nbtk_popup_free_action_at (NbtkPopup *popup,
                           gint       index,
                           gboolean   remove_action)
{
  NbtkPopupPrivate *priv = popup->priv;
  NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                          index);

  clutter_actor_unparent (CLUTTER_ACTOR (child->button));
  g_object_unref (child->action);

  if (remove_action)
    g_array_remove_index (priv->children, index);
}

static void
nbtk_popup_dispose (GObject *object)
{
  NbtkPopup *popup = NBTK_POPUP (object);
  NbtkPopupPrivate *priv = popup->priv;

  if (priv->children)
    {
      gint i;
      for (i = 0; i < priv->children->len; i++)
        nbtk_popup_free_action_at (popup, i, FALSE);
      g_array_free (priv->children, TRUE);
      priv->children = NULL;
    }

  G_OBJECT_CLASS (nbtk_popup_parent_class)->dispose (object);
}

static void
nbtk_popup_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_popup_parent_class)->finalize (object);
}

static void
nbtk_popup_get_preferred_width (ClutterActor *actor,
                                gfloat        for_height,
                                gfloat       *min_width_p,
                                gfloat       *natural_width_p)
{
  gint i;
  NbtkPadding padding;
  gfloat min_width, nat_width;

  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  /* Add padding and the size of the widest child */
  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  min_width = nat_width = 0;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat child_min_width, child_nat_width;
      NbtkPopupChild *child;

      child = &g_array_index (priv->children, NbtkPopupChild, i);

      clutter_actor_get_preferred_width (CLUTTER_ACTOR (child->button),
                                         for_height,
                                         &child_min_width,
                                         &child_nat_width);

      if (child_min_width > min_width)
        min_width = child_min_width;
      if (child_nat_width > nat_width)
        nat_width = child_nat_width;
    }

  if (min_width_p)
    *min_width_p = min_width + padding.left + padding.right;
  if (natural_width_p)
    *natural_width_p = nat_width + padding.left + padding.right;
}

static void
nbtk_popup_get_preferred_height (ClutterActor *actor,
                                 gfloat        for_width,
                                 gfloat       *min_height_p,
                                 gfloat       *natural_height_p)
{
  gint i;
  NbtkPadding padding;
  gfloat min_height, nat_height;

  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  /* Add padding and the cumulative height of the children */
  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  min_height = nat_height = padding.top + padding.bottom;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat child_min_height, child_nat_height;

      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);

      clutter_actor_get_preferred_height (CLUTTER_ACTOR (child->button),
                                          for_width,
                                          &child_min_height,
                                          &child_nat_height);

      min_height += child_min_height + 1;
      nat_height += child_nat_height + 1;
    }

  if (min_height_p)
    *min_height_p = min_height;
  if (natural_height_p)
    *natural_height_p = nat_height;
}

static void
nbtk_popup_allocate (ClutterActor          *actor,
                     const ClutterActorBox *box,
                     ClutterAllocationFlags flags)
{
  gint i;
  NbtkPadding padding;
  ClutterActorBox child_box;
  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  /* Allocate children */
  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  child_box.x1 = padding.left;
  child_box.y1 = padding.top;
  child_box.x2 = box->x2 - box->x1 - padding.right;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat natural_height;

      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);

      clutter_actor_get_preferred_height (CLUTTER_ACTOR (child->button),
                                          child_box.x2 - child_box.x1,
                                          NULL,
                                          &natural_height);
      child_box.y2 = child_box.y1 + natural_height;

      clutter_actor_allocate (CLUTTER_ACTOR (child->button), &child_box, flags);

      child_box.y1 = child_box.y2 + 1;
    }

  /* Chain up and allocate background */
  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->allocate (actor, box, flags);
}

static void
nbtk_popup_paint (ClutterActor *actor)
{
  gint i;
  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  /* Chain up to get background */
  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->paint (actor);

  /* Paint children */
  for (i = 0; i < priv->children->len; i++)
    {
      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);
      clutter_actor_paint (CLUTTER_ACTOR (child->button));
    }
}

static void
nbtk_popup_pick (ClutterActor       *actor,
                 const ClutterColor *color)
{
  gint i;
  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->pick (actor, color);

  /* pick children */
  for (i = 0; i < priv->children->len; i++)
    {
      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);
      if (clutter_actor_should_pick_paint (CLUTTER_ACTOR (actor)))
        {
          clutter_actor_paint ((ClutterActor*) child->button);
        }
    }
}

static void
nbtk_popup_map (ClutterActor *actor)
{
  gint i;
  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->map (actor);

  for (i = 0; i < priv->children->len; i++)
    {
      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);
      clutter_actor_map (CLUTTER_ACTOR (child->button));
    }
}

static void
nbtk_popup_unmap (ClutterActor *actor)
{
  gint i;
  NbtkPopupPrivate *priv = NBTK_POPUP (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->unmap (actor);

  for (i = 0; i < priv->children->len; i++)
    {
      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);
      clutter_actor_unmap (CLUTTER_ACTOR (child->button));
    }
}

static gboolean
nbtk_popup_event (ClutterActor *actor,
                  ClutterEvent *event)
{
  /* We swallow mouse events so that they don't fall through to whatever's
   * beneath us.
   */
  switch (event->type)
    {
    case CLUTTER_MOTION:
    case CLUTTER_BUTTON_PRESS:
    case CLUTTER_BUTTON_RELEASE:
    case CLUTTER_SCROLL:
      return TRUE;
    default:
      return FALSE;
    }
}

static void
nbtk_popup_show (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (nbtk_popup_parent_class)->show (actor);

  /* set reactive, since this may have been unset by a previous activation
   * (see: nbtk_popup_button_release_cb) */
  clutter_actor_set_reactive (actor, TRUE);
}

static void
nbtk_popup_class_init (NbtkPopupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkPopupPrivate));

  object_class->get_property = nbtk_popup_get_property;
  object_class->set_property = nbtk_popup_set_property;
  object_class->dispose = nbtk_popup_dispose;
  object_class->finalize = nbtk_popup_finalize;

  actor_class->show = nbtk_popup_show;
  actor_class->get_preferred_width = nbtk_popup_get_preferred_width;
  actor_class->get_preferred_height = nbtk_popup_get_preferred_height;
  actor_class->allocate = nbtk_popup_allocate;
  actor_class->paint = nbtk_popup_paint;
  actor_class->pick = nbtk_popup_pick;
  actor_class->map = nbtk_popup_map;
  actor_class->unmap = nbtk_popup_unmap;
  actor_class->event = nbtk_popup_event;

  signals[ACTION_ACTIVATED] =
    g_signal_new ("action-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkPopupClass, action_activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, NBTK_TYPE_ACTION);
}

static void
nbtk_popup_init (NbtkPopup *self)
{
  NbtkPopupPrivate *priv = self->priv = POPUP_PRIVATE (self);

  priv->children = g_array_new (FALSE, FALSE, sizeof (NbtkPopupChild));

  g_object_set (G_OBJECT (self),
                "show-on-set-parent", FALSE,
                NULL);
}

/**
 * nbtk_popup_new:
 *
 * Create a new #NbtkPopup
 *
 * Returns: a newly allocated #NbtkPopup
 */
NbtkWidget *
nbtk_popup_new (void)
{
  return g_object_new (NBTK_TYPE_POPUP, NULL);
}

static void
nbtk_popup_button_release_cb (NbtkButton   *button,
                              ClutterEvent *event,
                              NbtkAction   *action)
{
  NbtkPopup *popup;

  popup = NBTK_POPUP (clutter_actor_get_parent (CLUTTER_ACTOR (button)));

  /* set the popup unreactive to prevent other items being hilighted */
  clutter_actor_set_reactive ((ClutterActor*) popup, FALSE);

  g_signal_emit (popup, signals[ACTION_ACTIVATED], 0, action);
  g_signal_emit_by_name (action, "activated");

}

/**
 * nbtk_popup_add_action:
 * @popup: A #NbtkPopup
 * @action: A #NbtkAction
 *
 * Append @action to @popup.
 *
 */
void
nbtk_popup_add_action (NbtkPopup  *popup,
                       NbtkAction *action)
{
  NbtkPopupChild child;

  NbtkPopupPrivate *priv = popup->priv;

  child.action = g_object_ref_sink (action);
  /* TODO: Connect to notify signals in case action properties change */
  child.button = g_object_new (NBTK_TYPE_BUTTON,
                               "label", nbtk_action_get_name (action),
                               "transition-duration", 0,
                               NULL);

  nbtk_bin_set_alignment (NBTK_BIN (child.button),
                          NBTK_ALIGN_LEFT,
                          NBTK_ALIGN_CENTER);
  g_signal_connect (child.button, "button-release-event",
                    G_CALLBACK (nbtk_popup_button_release_cb), action);
  clutter_actor_set_parent (CLUTTER_ACTOR (child.button),
                            CLUTTER_ACTOR (popup));

  g_array_append_val (priv->children, child);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (popup));
}

/**
 * nbtk_popup_remove_action:
 * @popup: A #NbtkPopup
 * @action: A #NbtkAction
 *
 * Remove @action from @popup.
 *
 */
void
nbtk_popup_remove_action (NbtkPopup  *popup,
                          NbtkAction *action)
{
  gint i;

  NbtkPopupPrivate *priv = popup->priv;

  for (i = 0; i < priv->children->len; i++)
    {
      NbtkPopupChild *child = &g_array_index (priv->children, NbtkPopupChild,
                                              i);

      if (child->action == action)
        {
          nbtk_popup_free_action_at (popup, i, TRUE);
          break;
        }
    }
}

/**
 * nbtk_popup_clear:
 * @popup: A #NbtkPopup
 *
 * Remove all the actions from @popup.
 *
 */
void
nbtk_popup_clear (NbtkPopup *popup)
{
  gint i;

  NbtkPopupPrivate *priv = popup->priv;

  if (!priv->children->len)
    return;

  for (i = 0; i < priv->children->len; i++)
    nbtk_popup_free_action_at (popup, i, FALSE);

  g_array_remove_range (priv->children, 0, priv->children->len);
}

