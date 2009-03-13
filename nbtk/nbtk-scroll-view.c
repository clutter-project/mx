/* nbtk-scroll-view.h: Container with scroll-bars
 *
 * Copyright (C) 2008 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Nbtk by: Robert Staudinger <robsta@openedhand.com>
 */

#include "nbtk-scroll-view.h"
#include "nbtk-marshal.h"
#include "nbtk-scroll-bar.h"
#include "nbtk-scrollable.h"
#include "nbtk-stylable.h"
#include <clutter/clutter.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void nbtk_stylable_iface_init (NbtkStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkScrollView, nbtk_scroll_view, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (NBTK_TYPE_STYLABLE,
                                                nbtk_stylable_iface_init))

#define SCROLL_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                NBTK_TYPE_SCROLL_VIEW, \
                                NbtkScrollViewPrivate))

struct _NbtkScrollViewPrivate
{
  ClutterActor   *child;

  ClutterActor   *hscroll;
  ClutterActor   *vscroll;

  NbtkAdjustment *hadjustment;
  NbtkAdjustment *vadjustment;
};

enum {
  PROP_0,

  PROP_HSCROLL,
  PROP_VSCROLL,
  PROP_CHILD,
};

static void
nbtk_scroll_view_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  NbtkScrollViewPrivate *priv = ((NbtkScrollView *)object)->priv;

  switch (property_id)
    {
    case PROP_HSCROLL :
      g_value_set_object (value, priv->hscroll);
      break;
    case PROP_VSCROLL :
      g_value_set_object (value, priv->vscroll);
      break;
    case PROP_CHILD :
      g_value_set_object (value, priv->child);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_scroll_view_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_scroll_view_dispose (GObject *object)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (object)->priv;

  if (priv->child)
    clutter_container_remove_actor (CLUTTER_CONTAINER (object), priv->child);

  if (priv->vscroll)
    {
      clutter_actor_unparent (priv->vscroll);
      priv->vscroll = NULL;
    }

  if (priv->hscroll)
    {
      clutter_actor_unparent (priv->hscroll);
      priv->hscroll = NULL;
    }

  G_OBJECT_CLASS (nbtk_scroll_view_parent_class)->dispose (object);
}

static void
nbtk_scroll_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_scroll_view_parent_class)->finalize (object);
}

static void
nbtk_scroll_view_paint (ClutterActor *actor)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_scroll_view_parent_class)->paint (actor);

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    clutter_actor_paint (priv->hscroll);
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    clutter_actor_paint (priv->vscroll);
}

static void
nbtk_scroll_view_pick (ClutterActor *actor, const ClutterColor *color)
{
  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (nbtk_scroll_view_parent_class)->pick (actor, color);

  /* Trigger pick on children */
  nbtk_scroll_view_paint (actor);
}

static void
nbtk_scroll_view_get_preferred_width (ClutterActor *actor,
                                      ClutterUnit   for_height,
                                      ClutterUnit  *min_width_p,
                                      ClutterUnit  *natural_width_p)
{
  NbtkPadding padding;
  guint xthickness;

  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  nbtk_stylable_get (NBTK_STYLABLE (actor),
                     "xthickness", &xthickness,
                     NULL);

  /* Our natural width is the natural width of the child */
  clutter_actor_get_preferred_width (priv->child,
                                     for_height,
                                     NULL,
                                     natural_width_p);

  /* Add space for the scroll-bar if we can determine it will be necessary */
  if ((for_height >= 0) && natural_width_p)
    {
      ClutterUnit natural_height;

      clutter_actor_get_preferred_height (priv->child, -1.0,
                                          NULL,
                                          &natural_height);
      if (for_height < natural_height)
        *natural_width_p += CLUTTER_UNITS_FROM_INT (xthickness);
    }

  /* Add space for padding */
  if (min_width_p)
    *min_width_p = padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
nbtk_scroll_view_get_preferred_height (ClutterActor *actor,
                                       ClutterUnit   for_width,
                                       ClutterUnit  *min_height_p,
                                       ClutterUnit  *natural_height_p)
{
  NbtkPadding padding;
  guint ythickness;

  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  nbtk_stylable_get (NBTK_STYLABLE (actor),
                     "ythickness", &ythickness,
                     NULL);

  /* Our natural height is the natural height of the child */
  clutter_actor_get_preferred_height (priv->child,
                                      for_width,
                                      NULL,
                                      natural_height_p);

  /* Add space for the scroll-bar if we can determine it will be necessary */
  if ((for_width >= 0) && natural_height_p)
    {
      ClutterUnit natural_width;

      clutter_actor_get_preferred_width (priv->child, -1.0,
                                         NULL,
                                         &natural_width);
      if (for_width < natural_width)
        *natural_height_p += CLUTTER_UNITS_FROM_INT (ythickness);
    }

  /* Add space for padding */
  if (min_height_p)
    *min_height_p = padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;
}

static void
nbtk_scroll_view_allocate (ClutterActor          *actor,
                           const ClutterActorBox *box,
                           gboolean               absolute_origin_changed)
{
  NbtkPadding padding;
  ClutterActorBox child_box;
  guint xthickness, ythickness;
  ClutterUnit xthicknessu, ythicknessu;

  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (nbtk_scroll_view_parent_class)->
    allocate (actor, box, absolute_origin_changed);

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  nbtk_stylable_get (NBTK_STYLABLE (actor),
                     "xthickness", &xthickness,
                     "ythickness", &ythickness,
                     NULL);
  xthicknessu = CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll) ?
    CLUTTER_UNITS_FROM_INT (xthickness) : 0;
  ythicknessu = CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll) ?
    CLUTTER_UNITS_FROM_INT (ythickness) : 0;

  /* Vertical scrollbar */
  child_box.x1 = box->x2 - box->x1 - padding.top;
  child_box.x2 = MAX(0, (box->y2 - box->y1 - ythicknessu)) +
                     child_box.x1 - padding.top - padding.bottom;
  child_box.y1 = padding.right;
  child_box.y2 = MIN(xthicknessu, box->x2 - box->x1) + padding.right;

  clutter_actor_allocate (priv->vscroll,
                          &child_box,
                          absolute_origin_changed);

  /* Horizontal scrollbar */
  child_box.x1 = padding.left;
  child_box.x2 = MAX(0, box->x2 - box->x1 - xthicknessu - padding.right);
  child_box.y1 = MAX(0, box->y2 - box->y1 - ythicknessu) - padding.bottom;
  child_box.y2 = box->y2 - box->y1 - padding.bottom;

  clutter_actor_allocate (priv->hscroll,
                          &child_box,
                          absolute_origin_changed);

  /* Child */
  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  if (CLUTTER_ACTOR_IS_REACTIVE (priv->vscroll))
    child_box.x2 -= xthicknessu;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;
  if (CLUTTER_ACTOR_IS_REACTIVE (priv->hscroll))
    child_box.y2 -= ythicknessu;

  child_box.x1 += padding.left;
  child_box.x2 -= padding.left + padding.right;
  child_box.y1 += padding.top;
  child_box.y2 -= padding.top + padding.bottom;

  if (priv->child)
    {
      clutter_actor_allocate (priv->child, &child_box, absolute_origin_changed);
      clutter_actor_set_clipu (priv->child,
                               child_box.x1,
                               child_box.y1,
                               child_box.x2 - child_box.x1,
                               child_box.y2 - child_box.y1);
    }

}

static void
nbtk_scroll_view_class_init (NbtkScrollViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkScrollViewPrivate));

  object_class->get_property = nbtk_scroll_view_get_property;
  object_class->set_property = nbtk_scroll_view_set_property;
  object_class->dispose= nbtk_scroll_view_dispose;
  object_class->finalize = nbtk_scroll_view_finalize;

  actor_class->paint = nbtk_scroll_view_paint;
  actor_class->pick = nbtk_scroll_view_pick;
  actor_class->get_preferred_width = nbtk_scroll_view_get_preferred_width;
  actor_class->get_preferred_height = nbtk_scroll_view_get_preferred_height;
  actor_class->allocate = nbtk_scroll_view_allocate;

  g_object_class_install_property (object_class,
                                   PROP_HSCROLL,
                                   g_param_spec_object ("hscroll",
                                                        "NbtkScrollBar",
                                                        "Horizontal scroll indicator",
                                                        NBTK_TYPE_SCROLL_BAR,
                                                        G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                                   PROP_VSCROLL,
                                   g_param_spec_object ("vscroll",
                                                       "NbtkScrollBar",
                                                       "Vertical scroll indicator",
                                                       NBTK_TYPE_SCROLL_BAR,
                                                       G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                                   PROP_CHILD,
                                   g_param_spec_object ("child",
                                                        "ClutterActor",
                                                        "Child actor",
                                                        CLUTTER_TYPE_ACTOR,
                                                        G_PARAM_READABLE));
}

static void
nbtk_stylable_iface_init (NbtkStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      pspec = g_param_spec_uint ("xthickness",
                                  "Vertical scroll-bar thickness",
                                  "Thickness of vertical scrollbar, in px",
                                  0, G_MAXUINT, 32,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_SCROLL_VIEW, pspec);

      pspec = g_param_spec_uint ("ythickness",
                                  "Horizontal scroll-bar thickness",
                                  "Thickness of horizontal scrollbar, in px",
                                  0, G_MAXUINT, 32,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_SCROLL_VIEW, pspec);
    }
}

static void
child_adjustment_changed_cb (NbtkAdjustment *adjustment,
                             ClutterActor   *bar)
{
  NbtkScrollView *scroll;
  gdouble lower, upper, page_size;

  scroll = NBTK_SCROLL_VIEW (clutter_actor_get_parent (bar));

  /* Determine if this scroll-bar should be visible */
  nbtk_adjustment_get_values (adjustment, NULL,
                              &lower, &upper,
                              NULL, NULL,
                              &page_size);

  if ((upper - lower) > page_size)
    clutter_actor_show (bar);
  else
    clutter_actor_hide (bar);

  /* Request a resize */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (scroll));
}

static void
child_hadjustment_notify_cb (GObject *gobject,
                             GParamSpec *arg1,
                             gpointer user_data)
{
  NbtkAdjustment *hadjust;

  ClutterActor *actor = CLUTTER_ACTOR (gobject);
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (user_data)->priv;

  hadjust = nbtk_scroll_bar_get_adjustment (NBTK_SCROLL_BAR(priv->hscroll));
  if (hadjust)
    g_signal_handlers_disconnect_by_func (hadjust,
                                          child_adjustment_changed_cb,
                                          priv->hscroll);

  nbtk_scrollable_get_adjustments (NBTK_SCROLLABLE (actor), &hadjust, NULL);
  if (hadjust)
    {
      nbtk_scroll_bar_set_adjustment (NBTK_SCROLL_BAR(priv->hscroll), hadjust);
      g_signal_connect (hadjust, "changed", G_CALLBACK (
                        child_adjustment_changed_cb), priv->hscroll);
      child_adjustment_changed_cb (hadjust, priv->hscroll);
    }
}

static void
child_vadjustment_notify_cb (GObject *gobject,
                             GParamSpec *arg1,
                             gpointer user_data)
{
  NbtkAdjustment *vadjust;

  ClutterActor *actor = CLUTTER_ACTOR (gobject);
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (user_data)->priv;

  vadjust = nbtk_scroll_bar_get_adjustment (NBTK_SCROLL_BAR(priv->vscroll));
  if (vadjust)
    g_signal_handlers_disconnect_by_func (vadjust,
                                          child_adjustment_changed_cb,
                                          priv->vscroll);

  nbtk_scrollable_get_adjustments (NBTK_SCROLLABLE(actor), NULL, &vadjust);
  if (vadjust)
    {
      nbtk_scroll_bar_set_adjustment (NBTK_SCROLL_BAR(priv->vscroll), vadjust);
      g_signal_connect (vadjust, "changed", G_CALLBACK (
                        child_adjustment_changed_cb), priv->vscroll);
      child_adjustment_changed_cb (vadjust, priv->vscroll);
    }
}

static void
nbtk_scroll_view_init (NbtkScrollView *self)
{
  NbtkScrollViewPrivate *priv = self->priv = SCROLL_VIEW_PRIVATE (self);

  priv->hscroll = nbtk_scroll_bar_new (NULL);
  priv->vscroll = nbtk_scroll_bar_new (NULL);

  clutter_actor_set_parent (priv->hscroll, CLUTTER_ACTOR (self));
  clutter_actor_set_parent (priv->vscroll, CLUTTER_ACTOR (self));

  clutter_actor_show (priv->hscroll);
  clutter_actor_show (priv->vscroll);
  clutter_actor_set_rotation (priv->vscroll, CLUTTER_Z_AXIS, 90.0, 0, 0, 0);
}

static void
nbtk_scroll_view_add_actor (ClutterContainer *container,
                            ClutterActor     *actor)
{
  NbtkScrollView *self = NBTK_SCROLL_VIEW (container);
  NbtkScrollViewPrivate *priv = self->priv;

  if (priv->child)
    {
      g_warning ("Attempting to add an actor of type %s to "
                 "a NbtkScrollView that already contains "
                 "an actor of type %s.",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (priv->child)));
    }
  else
    {
      if (NBTK_IS_SCROLLABLE(actor))
        {
          priv->child = actor;
          clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

          /* Get adjustments for scroll-bars */
          g_signal_connect (actor, "notify::hadjustment",
                            G_CALLBACK (child_hadjustment_notify_cb),
                            container);
          g_signal_connect (actor, "notify::vadjustment",
                            G_CALLBACK (child_vadjustment_notify_cb),
                            container);
          child_hadjustment_notify_cb (G_OBJECT (actor), NULL, container);
          child_vadjustment_notify_cb (G_OBJECT (actor), NULL, container);

          /* Notify that child has been set */
          g_signal_emit_by_name (container, "actor-added", priv->child);
          g_object_notify (G_OBJECT (container), "child");

          clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
        }
      else
        {
          g_warning ("Attempting to add an actor of type %s to "
                     "a NbtkScrollView, but the actor does "
                     "not implement NbtkScrollable.",
                     g_type_name (G_OBJECT_TYPE (actor)));
        }
    }
}

static void
nbtk_scroll_view_remove_actor (ClutterContainer *container,
                               ClutterActor     *actor)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (container)->priv;

  if (actor == priv->child)
    {
      g_object_ref (priv->child);

      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_hadjustment_notify_cb,
                                            container);
      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_vadjustment_notify_cb,
                                            container);

      clutter_actor_unparent (priv->child);

      g_signal_emit_by_name (container, "actor-removed", priv->child);

      g_object_unref (priv->child);
      priv->child = NULL;

      g_object_notify (G_OBJECT (container), "child");

      if (CLUTTER_ACTOR_IS_VISIBLE (container))
        clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
    }
}

static void
nbtk_scroll_view_foreach (ClutterContainer *container,
                          ClutterCallback   callback,
                          gpointer          callback_data)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (container)->priv;

  if (priv->child)
    callback (priv->child, callback_data);
}

static void
nbtk_scroll_view_lower (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
nbtk_scroll_view_raise (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
nbtk_scroll_view_sort_depth_order (ClutterContainer *container)
{
  /* single child */
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_scroll_view_add_actor;
  iface->remove = nbtk_scroll_view_remove_actor;
  iface->foreach = nbtk_scroll_view_foreach;
  iface->lower = nbtk_scroll_view_lower;
  iface->raise = nbtk_scroll_view_raise;
  iface->sort_depth_order = nbtk_scroll_view_sort_depth_order;
}

NbtkWidget *
nbtk_scroll_view_new (void)
{
  return NBTK_WIDGET (g_object_new (NBTK_TYPE_SCROLL_VIEW, NULL));
}

ClutterActor *
nbtk_scroll_view_get_hscroll_bar (NbtkScrollView *scroll)
{
  NbtkScrollViewPrivate *priv;

  g_return_val_if_fail (NBTK_IS_SCROLL_VIEW (scroll), NULL);

  priv = scroll->priv;

  return priv->hscroll;
}

ClutterActor *
nbtk_scroll_view_get_vscroll_bar (NbtkScrollView *scroll)
{
  NbtkScrollViewPrivate *priv;

  g_return_val_if_fail (NBTK_IS_SCROLL_VIEW (scroll), NULL);

  priv = scroll->priv;

  return priv->vscroll;
}

ClutterActor *
nbtk_scroll_view_get_child (NbtkScrollView *scroll)
{
  NbtkScrollViewPrivate *priv;

  g_return_val_if_fail (NBTK_IS_SCROLL_VIEW (scroll), NULL);

  priv = scroll->priv;

  return priv->child;
}
