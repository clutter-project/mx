/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-popup.c: popup menu class
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

/**
 * SECTION:mx-popup
 * @short_description: a popup actor representing a list of user actions
 *
 * #MxPopup displays a list of user actions, defined by a list of
 * #MxAction<!-- -->s. The popup list will appear above all other actors.
 */

#include "mx-popup.h"
#include "mx-label.h"
#include "mx-button.h"
#include "mx-box-layout.h"
#include "mx-icon-theme.h"

G_DEFINE_TYPE (MxPopup, mx_popup, MX_TYPE_FLOATING_WIDGET)

#define POPUP_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_POPUP, MxPopupPrivate))

#define SPACING 8
#define DEFAULT_ICON_SIZE 16

typedef struct
{
  MxAction *action;
  MxWidget *button;
} MxPopupChild;

struct _MxPopupPrivate
{
  GArray  *children;
  gboolean transition_out;


  ClutterActor *stage;
  gulong captured_event_handler;
};

enum
{
  ACTION_ACTIVATED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static gboolean mx_popup_captured_event_handler (ClutterActor *actor,
                                                 ClutterEvent *event,
                                                 ClutterActor *popup);

static void
mx_popup_get_property (GObject    *object,
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
mx_popup_set_property (GObject      *object,
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
mx_popup_free_action_at (MxPopup *popup,
                         gint     index,
                         gboolean remove_action)
{
  MxPopupPrivate *priv = popup->priv;
  MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
                                        index);

  clutter_actor_unparent (CLUTTER_ACTOR (child->button));
  g_object_unref (child->action);

  if (remove_action)
    g_array_remove_index (priv->children, index);
}

static void
mx_popup_dispose (GObject *object)
{
  MxPopup *popup = MX_POPUP (object);
  MxPopupPrivate *priv = popup->priv;

  if (priv->children)
    {
      gint i;
      for (i = 0; i < priv->children->len; i++)
        mx_popup_free_action_at (popup, i, FALSE);
      g_array_free (priv->children, TRUE);
      priv->children = NULL;
    }


  G_OBJECT_CLASS (mx_popup_parent_class)->dispose (object);
}

static void
mx_popup_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_popup_parent_class)->finalize (object);
}

static void
mx_popup_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  gint i;
  MxPadding padding;
  gfloat min_width, nat_width;

  MxPopupPrivate *priv = MX_POPUP (actor)->priv;

  /* Add padding and the size of the widest child */
  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  min_width = nat_width = 0;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat child_min_width, child_nat_width;
      MxPopupChild *child;

      child = &g_array_index (priv->children, MxPopupChild, i);

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
mx_popup_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  gint i;
  MxPadding padding;
  gfloat min_height, nat_height;

  MxPopupPrivate *priv = MX_POPUP (actor)->priv;

  /* Add padding and the cumulative height of the children */
  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  min_height = nat_height = padding.top + padding.bottom;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat child_min_height, child_nat_height;

      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
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
mx_popup_allocate (ClutterActor          *actor,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  gint i;
  MxPadding padding;
  ClutterActorBox child_box;
  MxPopupPrivate *priv = MX_POPUP (actor)->priv;

  /* Allocate children */
  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  child_box.x1 = padding.left;
  child_box.y1 = padding.top;
  child_box.x2 = box->x2 - box->x1 - padding.right;
  for (i = 0; i < priv->children->len; i++)
    {
      gfloat natural_height;

      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
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
  CLUTTER_ACTOR_CLASS (mx_popup_parent_class)->allocate (actor, box, flags);
}

static void
mx_popup_floating_paint (ClutterActor *popup)
{
  gint i;
  MxPopupPrivate *priv = MX_POPUP (popup)->priv;

  /* Chain up to get background */
  MX_FLOATING_WIDGET_CLASS (mx_popup_parent_class)->floating_paint (popup);

  /* Paint children */
  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
                                            i);
      clutter_actor_paint (CLUTTER_ACTOR (child->button));
    }
}

static void
mx_popup_floating_pick (ClutterActor       *popup,
                        const ClutterColor *color)
{
  gint i;
  MxPopupPrivate *priv = MX_POPUP (popup)->priv;

  /* chain up to get bounding rectangle */

  MX_FLOATING_WIDGET_CLASS (mx_popup_parent_class)->floating_pick (popup, color);

  /* pick children */
  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild, i);

      if (clutter_actor_should_pick_paint (CLUTTER_ACTOR (child->button)))
        {
          clutter_actor_paint (CLUTTER_ACTOR (child->button));
        }
    }
}

static void
stage_weak_notify (MxPopup      *popup,
                   ClutterStage *stage)
{
  popup->priv->stage = NULL;
}

static void
mx_popup_map (ClutterActor *actor)
{
  gint i;
  MxPopupPrivate *priv = MX_POPUP (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_popup_parent_class)->map (actor);

  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
                                            i);
      clutter_actor_map (CLUTTER_ACTOR (child->button));
    }

  /* set up a capture so we can close the menu if the user clicks outside it */
  priv->stage = clutter_actor_get_stage (actor);
  g_object_weak_ref (G_OBJECT (priv->stage), (GWeakNotify) stage_weak_notify,
                     actor);

  priv->captured_event_handler =
    g_signal_connect (priv->stage,
                      "captured-event",
                      G_CALLBACK (mx_popup_captured_event_handler),
                      actor);
}

static void
mx_popup_unmap (ClutterActor *actor)
{
  gint i;
  MxPopupPrivate *priv = MX_POPUP (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_popup_parent_class)->unmap (actor);

  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
                                            i);
      clutter_actor_unmap (CLUTTER_ACTOR (child->button));
    }

  if (priv->stage)
    {
      g_signal_handler_disconnect (priv->stage, priv->captured_event_handler);
      priv->captured_event_handler = 0;

      g_object_weak_unref (G_OBJECT (priv->stage),
                           (GWeakNotify) stage_weak_notify,
                           actor);
      priv->stage = NULL;
    }
}

static gboolean
mx_popup_event (ClutterActor *actor,
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

static gboolean
mx_popup_captured_event_handler (ClutterActor *actor,
                                 ClutterEvent *event,
                                 ClutterActor *popup)
{
  int i;
  ClutterActor *source;
  MxPopupPrivate *priv = MX_POPUP (popup)->priv;

  /* allow the event to continue if it is applied to the popup or any of its
   * children
   */
  source = clutter_event_get_source (event);
  if (source == popup)
    return FALSE;
  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child;

      child = &g_array_index (priv->children, MxPopupChild, i);

      if (source == (ClutterActor*) child->button)
        return FALSE;
    }

  /* hide the menu if the user clicks outside the menu */
  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      if (clutter_actor_get_animation (popup))
        {
          clutter_animation_completed (clutter_actor_get_animation (popup));

          return FALSE;
        }


      clutter_actor_set_reactive (popup, FALSE);
      clutter_actor_animate (popup, CLUTTER_LINEAR, 250,
                             "opacity", (guchar) 0,
                             "signal-swapped::completed", clutter_actor_hide,
                             popup,
                             NULL);
    }


  return TRUE;
}

static void
mx_popup_show (ClutterActor *actor)
{
  ClutterAnimation *animation = NULL;

  /* set reactive and opacity, since these may have been set by the fade-out
   * animation (e.g. from captured_event_handler or button_release_cb) */
  if ((animation = clutter_actor_get_animation (actor)))
    {
      clutter_animation_completed (animation);
    }
  clutter_actor_set_reactive (actor, TRUE);
  clutter_actor_set_opacity (actor, 0xff);

  /* chain up to run show after re-setting properties above */
  CLUTTER_ACTOR_CLASS (mx_popup_parent_class)->show (actor);
}

static void
mx_popup_hide (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (mx_popup_parent_class)->hide (actor);
}

static void
mx_popup_style_changed (MxPopup *popup)
{
  MxPopupPrivate *priv = popup->priv;
  int i;

  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child;

      child = &g_array_index (priv->children, MxPopupChild, i);

      mx_widget_ensure_style (child->button);
    }
}

static void
mx_popup_class_init (MxPopupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxFloatingWidgetClass *float_class = MX_FLOATING_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxPopupPrivate));

  object_class->get_property = mx_popup_get_property;
  object_class->set_property = mx_popup_set_property;
  object_class->dispose = mx_popup_dispose;
  object_class->finalize = mx_popup_finalize;

  actor_class->show = mx_popup_show;
  actor_class->hide = mx_popup_hide;
  actor_class->get_preferred_width = mx_popup_get_preferred_width;
  actor_class->get_preferred_height = mx_popup_get_preferred_height;
  actor_class->allocate = mx_popup_allocate;
  actor_class->map = mx_popup_map;
  actor_class->unmap = mx_popup_unmap;
  actor_class->event = mx_popup_event;

  float_class->floating_paint = mx_popup_floating_paint;
  float_class->floating_pick = mx_popup_floating_pick;

  signals[ACTION_ACTIVATED] =
    g_signal_new ("action-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxPopupClass, action_activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, MX_TYPE_ACTION);
}

static void
mx_popup_init (MxPopup *self)
{
  MxPopupPrivate *priv = self->priv = POPUP_PRIVATE (self);

  priv->children = g_array_new (FALSE, FALSE, sizeof (MxPopupChild));

  g_object_set (G_OBJECT (self),
                "show-on-set-parent", FALSE,
                NULL);

  g_signal_connect (self, "style-changed", G_CALLBACK (mx_popup_style_changed),
                    NULL);
}

/**
 * mx_popup_new:
 *
 * Create a new #MxPopup
 *
 * Returns: a newly allocated #MxPopup
 */
ClutterActor *
mx_popup_new (void)
{
  return g_object_new (MX_TYPE_POPUP, NULL);
}

static void
mx_popup_button_release_cb (MxButton     *button,
                            ClutterEvent *event,
                            MxAction     *action)
{
  MxPopup *popup;

  popup = MX_POPUP (clutter_actor_get_parent (CLUTTER_ACTOR (button)));

  /* set the popup unreactive to prevent other items being hilighted */
  clutter_actor_set_reactive ((ClutterActor*) popup, FALSE);


  g_object_ref (popup);
  g_object_ref (action);

  g_signal_emit (popup, signals[ACTION_ACTIVATED], 0, action);
  g_signal_emit_by_name (action, "activated");

  clutter_actor_animate (CLUTTER_ACTOR (popup), CLUTTER_LINEAR, 250,
                         "opacity", (guchar) 0,
                         "signal-swapped::completed", clutter_actor_hide, popup,
                         NULL);

  g_object_unref (action);
  g_object_unref (popup);

}

/**
 * mx_popup_add_action:
 * @popup: A #MxPopup
 * @action: A #MxAction
 *
 * Append @action to @popup.
 *
 */
void
mx_popup_add_action (MxPopup  *popup,
                     MxAction *action)
{
  MxPopupChild child;
  ClutterActor *label;
  ClutterTexture *icon;

  g_return_if_fail (MX_IS_POPUP (popup));
  g_return_if_fail (MX_IS_ACTION (action));

  MxPopupPrivate *priv = popup->priv;

  child.action = g_object_ref_sink (action);
  /* TODO: Connect to notify signals in case action properties change */
  child.button = g_object_new (MX_TYPE_BOX_LAYOUT,
                               "reactive", TRUE,
                               "spacing", SPACING,
                               NULL);

  if (mx_action_get_icon (action))
    {
      icon = mx_icon_theme_lookup_texture (mx_icon_theme_get_default (),
                                           mx_action_get_icon (action),
                                           DEFAULT_ICON_SIZE);

      if (icon)
        {
          clutter_container_add_actor (CLUTTER_CONTAINER (child.button),
                                       (ClutterActor *)icon);
          clutter_container_child_set (CLUTTER_CONTAINER (child.button),
                                       (ClutterActor *)icon,
                                       "y-fill", FALSE,
                                       NULL);

        }
    }

  label = mx_label_new_with_text (mx_action_get_display_name (action));

  clutter_container_add_actor (CLUTTER_CONTAINER (child.button),
                               label);

  clutter_container_child_set (CLUTTER_CONTAINER (child.button), label,
                               "y-fill", FALSE,
                               NULL);

  g_signal_connect (child.button, "button-release-event",
                    G_CALLBACK (mx_popup_button_release_cb), action);
  clutter_actor_set_parent (CLUTTER_ACTOR (child.button),
                            CLUTTER_ACTOR (popup));

  g_array_append_val (priv->children, child);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (popup));
}

/**
 * mx_popup_remove_action:
 * @popup: A #MxPopup
 * @action: A #MxAction
 *
 * Remove @action from @popup.
 *
 */
void
mx_popup_remove_action (MxPopup  *popup,
                        MxAction *action)
{
  gint i;

  g_return_if_fail (MX_IS_POPUP (popup));
  g_return_if_fail (MX_IS_ACTION (action));

  MxPopupPrivate *priv = popup->priv;

  for (i = 0; i < priv->children->len; i++)
    {
      MxPopupChild *child = &g_array_index (priv->children, MxPopupChild,
                                            i);

      if (child->action == action)
        {
          mx_popup_free_action_at (popup, i, TRUE);
          break;
        }
    }
}

/**
 * mx_popup_clear:
 * @popup: A #MxPopup
 *
 * Remove all the actions from @popup.
 *
 */
void
mx_popup_clear (MxPopup *popup)
{
  gint i;

  g_return_if_fail (MX_IS_POPUP (popup));

  MxPopupPrivate *priv = popup->priv;

  if (!priv->children->len)
    return;

  for (i = 0; i < priv->children->len; i++)
    mx_popup_free_action_at (popup, i, FALSE);

  g_array_remove_range (priv->children, 0, priv->children->len);
}

