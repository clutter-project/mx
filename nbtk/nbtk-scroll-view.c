/*
 * nbtk-scroll-view.h: Container with scroll-bars
 *
 * Copyright 2008 OpenedHand
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
 *
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Nbtk by: Robert Staudinger <robsta@openedhand.com>
 *
 */

#include "nbtk-scroll-view.h"
#include "nbtk-marshal.h"
#include "nbtk-scroll-bar.h"
#include "nbtk-scrollable.h"
#include "nbtk-stylable.h"
#include <clutter/clutter.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void nbtk_stylable_iface_init (NbtkStylableIface *iface);

static ClutterContainerIface *nbtk_scroll_view_parent_iface = NULL;

G_DEFINE_TYPE_WITH_CODE (NbtkScrollView, nbtk_scroll_view, NBTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (NBTK_TYPE_STYLABLE,
                                                nbtk_stylable_iface_init))

#define SCROLL_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                NBTK_TYPE_SCROLL_VIEW, \
                                NbtkScrollViewPrivate))

struct _NbtkScrollViewPrivate
{
  /* a pointer to the child; this is actually stored
   * inside NbtkBin:child, but we keep it to avoid
   * calling nbtk_bin_get_child() every time we need it
   */
  ClutterActor   *child;

  ClutterActor   *hscroll;
  ClutterActor   *vscroll;

  gfloat          row_size;
  gfloat          column_size;

  gboolean        row_size_set : 1;
  gboolean        column_size_set : 1;
};

enum {
  PROP_0,

  PROP_HSCROLL,
  PROP_VSCROLL
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

  priv->child = NULL;

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

  /* NbtkBin will paint the child */
  CLUTTER_ACTOR_CLASS (nbtk_scroll_view_parent_class)->paint (actor);

  /* paint our custom children */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    clutter_actor_paint (priv->hscroll);
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    clutter_actor_paint (priv->vscroll);
}

static void
nbtk_scroll_view_pick (ClutterActor *actor, const ClutterColor *color)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (nbtk_scroll_view_parent_class)->pick (actor, color);

  /* paint our custom children */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    clutter_actor_paint (priv->hscroll);
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    clutter_actor_paint (priv->vscroll);
}

static void
nbtk_scroll_view_get_preferred_width (ClutterActor *actor,
                                      gfloat        for_height,
                                      gfloat       *min_width_p,
                                      gfloat       *natural_width_p)
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
      gfloat natural_height;

      clutter_actor_get_preferred_height (priv->child, -1.0,
                                          NULL,
                                          &natural_height);
      if (for_height < natural_height)
        *natural_width_p += xthickness;
    }

  /* Add space for padding */
  if (min_width_p)
    *min_width_p = padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
nbtk_scroll_view_get_preferred_height (ClutterActor *actor,
                                       gfloat        for_width,
                                       gfloat       *min_height_p,
                                       gfloat       *natural_height_p)
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
      gfloat natural_width;

      clutter_actor_get_preferred_width (priv->child, -1.0,
                                         NULL,
                                         &natural_width);
      if (for_width < natural_width)
        *natural_height_p += ythickness;
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
                           ClutterAllocationFlags flags)
{
  NbtkPadding padding;
  ClutterActorBox child_box;
  guint xthickness, ythickness;
  gfloat xthicknessu, ythicknessu;
  ClutterActorClass *parent_parent_class;

  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (actor)->priv;

  /* Chain up to the parent's parent class
   *
   * We do this because we do not want NbtkBin to allocate the child, as we
   * give it a different allocation later, depending on whether the scrollbars
   * are visible
   */
  parent_parent_class
    = g_type_class_peek_parent (nbtk_scroll_view_parent_class);

  CLUTTER_ACTOR_CLASS (parent_parent_class)->
    allocate (actor, box, flags);


  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  nbtk_stylable_get (NBTK_STYLABLE (actor),
                     "xthickness", &xthickness,
                     "ythickness", &ythickness,
                     NULL);
  xthicknessu = CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll) ?
    xthickness : 0;
  ythicknessu = CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll) ?
    ythickness : 0;

  /* Vertical scrollbar */
  child_box.x1 = box->x2 - box->x1 - padding.top;
  child_box.x2 = MAX(0, (box->y2 - box->y1 - ythicknessu)) +
                     child_box.x1 - padding.top - padding.bottom;
  child_box.y1 = padding.right;
  child_box.y2 = MIN(xthicknessu, box->x2 - box->x1) + padding.right;

  clutter_actor_allocate (priv->vscroll,
                          &child_box,
                          flags);

  /* Horizontal scrollbar */
  child_box.x1 = padding.left;
  child_box.x2 = MAX(0, box->x2 - box->x1 - xthicknessu - padding.right);
  child_box.y1 = MAX(0, box->y2 - box->y1 - ythicknessu) - padding.bottom;
  child_box.y2 = box->y2 - box->y1 - padding.bottom;

  clutter_actor_allocate (priv->hscroll,
                          &child_box,
                          flags);

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
      clutter_actor_allocate (priv->child, &child_box, flags);
      clutter_actor_set_clip (priv->child,
                              child_box.x1,
                              child_box.y1,
                              child_box.x2 - child_box.x1,
                              child_box.y2 - child_box.y1);
    }

}

static void
nbtk_scroll_view_style_changed (NbtkWidget *widget)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (widget)->priv;

  NBTK_WIDGET_CLASS (nbtk_scroll_view_parent_class)->style_changed (widget);

  if (priv->child)
    {
      NBTK_WIDGET_GET_CLASS (NBTK_WIDGET (priv->child))
        ->style_changed (NBTK_WIDGET (priv->child));
    }
  NBTK_WIDGET_GET_CLASS (NBTK_WIDGET (priv->hscroll))
    ->style_changed (NBTK_WIDGET (priv->hscroll));
  NBTK_WIDGET_GET_CLASS (NBTK_WIDGET (priv->vscroll))
    ->style_changed (NBTK_WIDGET (priv->vscroll));
}

static void
nbtk_scroll_view_class_init (NbtkScrollViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);

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

  widget_class->style_changed = nbtk_scroll_view_style_changed;

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
}

static void
nbtk_stylable_iface_init (NbtkStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("xthickness",
                                  "Vertical scroll-bar thickness",
                                  "Thickness of vertical scrollbar, in px",
                                  0, G_MAXUINT, 24,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_SCROLL_VIEW, pspec);

      pspec = g_param_spec_uint ("ythickness",
                                  "Horizontal scroll-bar thickness",
                                  "Thickness of horizontal scrollbar, in px",
                                  0, G_MAXUINT, 24,
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
      /* Force scroll step if neede. */
      if (priv->column_size_set)
        {
          g_object_set (hadjust,
                        "step-increment", priv->column_size,
                        NULL);
        }

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
      /* Force scroll step if neede. */
      if (priv->row_size_set)
        {
          g_object_set (vadjust,
                        "step-increment", priv->row_size,
                        NULL);
        }

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

  priv->hscroll = CLUTTER_ACTOR (nbtk_scroll_bar_new (NULL));
  priv->vscroll = CLUTTER_ACTOR (nbtk_scroll_bar_new (NULL));

  clutter_actor_set_parent (priv->hscroll, CLUTTER_ACTOR (self));
  clutter_actor_set_parent (priv->vscroll, CLUTTER_ACTOR (self));

  clutter_actor_show (priv->hscroll);
  clutter_actor_show (priv->vscroll);
  clutter_actor_set_rotation (priv->vscroll, CLUTTER_Z_AXIS, 90.0, 0, 0, 0);

  g_object_set (G_OBJECT (self), "reactive", FALSE, NULL);
}

static void
nbtk_scroll_view_add (ClutterContainer *container,
                      ClutterActor     *actor)
{
  NbtkScrollView *self = NBTK_SCROLL_VIEW (container);
  NbtkScrollViewPrivate *priv = self->priv;

  if (NBTK_IS_SCROLLABLE (actor))
    {
      priv->child = actor;

      /* chain up to NbtkBin::add() */
      nbtk_scroll_view_parent_iface->add (container, actor);

      /* Get adjustments for scroll-bars */
      g_signal_connect (actor, "notify::hadjustment",
                        G_CALLBACK (child_hadjustment_notify_cb),
                        container);
      g_signal_connect (actor, "notify::vadjustment",
                        G_CALLBACK (child_vadjustment_notify_cb),
                        container);
      child_hadjustment_notify_cb (G_OBJECT (actor), NULL, container);
      child_vadjustment_notify_cb (G_OBJECT (actor), NULL, container);
    }
  else
    {
      g_warning ("Attempting to add an actor of type %s to "
                 "a NbtkScrollView, but the actor does "
                 "not implement NbtkScrollable.",
                 g_type_name (G_OBJECT_TYPE (actor)));
    }
}

static void
nbtk_scroll_view_remove (ClutterContainer *container,
                         ClutterActor     *actor)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (container)->priv;

  if (actor == priv->child)
    {
      g_object_ref (priv->child);

      /* chain up to NbtkBin::remove() */
      nbtk_scroll_view_parent_iface->remove (container, actor);

      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_hadjustment_notify_cb,
                                            container);
      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_vadjustment_notify_cb,
                                            container);

      g_object_unref (priv->child);
      priv->child = NULL;
    }
}

static void
nbtk_scroll_view_foreach_with_internals (ClutterContainer *container,
                                         ClutterCallback   callback,
                                         gpointer          user_data)
{
  NbtkScrollViewPrivate *priv = NBTK_SCROLL_VIEW (container)->priv;

  if (priv->child != NULL)
    callback (priv->child, user_data);

  if (priv->hscroll != NULL)
    callback (priv->hscroll, user_data);

  if (priv->vscroll != NULL)
    callback (priv->vscroll, user_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  /* store a pointer to the NbtkBin implementation of
   * ClutterContainer so that we can chain up when
   * overriding the methods
   */
  nbtk_scroll_view_parent_iface = g_type_interface_peek_parent (iface);

  iface->add = nbtk_scroll_view_add;
  iface->remove = nbtk_scroll_view_remove;
  iface->foreach_with_internals = nbtk_scroll_view_foreach_with_internals;
}

NbtkWidget *
nbtk_scroll_view_new (void)
{
  return g_object_new (NBTK_TYPE_SCROLL_VIEW, NULL);
}

ClutterActor *
nbtk_scroll_view_get_hscroll_bar (NbtkScrollView *scroll)
{
  g_return_val_if_fail (NBTK_IS_SCROLL_VIEW (scroll), NULL);

  return scroll->priv->hscroll;
}

ClutterActor *
nbtk_scroll_view_get_vscroll_bar (NbtkScrollView *scroll)
{
  g_return_val_if_fail (NBTK_IS_SCROLL_VIEW (scroll), NULL);

  return scroll->priv->vscroll;
}

gfloat
nbtk_scroll_view_get_column_size (NbtkScrollView *scroll)
{
  NbtkAdjustment  *adjustment;
  gdouble          column_size;

  g_return_val_if_fail (scroll, 0);

  adjustment = nbtk_scroll_bar_get_adjustment (
                NBTK_SCROLL_BAR (scroll->priv->hscroll));
  g_object_get (adjustment,
                "step-increment", &column_size,
                NULL);

  return column_size;
}

void
nbtk_scroll_view_set_column_size (NbtkScrollView *scroll,
                                  gfloat          column_size)
{
  NbtkAdjustment  *adjustment;

  g_return_if_fail (scroll);

  if (column_size < 0)
    {
      scroll->priv->column_size_set = FALSE;
      scroll->priv->column_size = -1;
    }
  else
    {
      scroll->priv->column_size_set = TRUE;
      scroll->priv->column_size = column_size;

      adjustment = nbtk_scroll_bar_get_adjustment (
                    NBTK_SCROLL_BAR (scroll->priv->hscroll));

      if (adjustment)
        g_object_set (adjustment,
                      "step-increment", (gdouble) scroll->priv->column_size,
                      NULL);
     }
}

gfloat
nbtk_scroll_view_get_row_size (NbtkScrollView *scroll)
{
  NbtkAdjustment  *adjustment;
  gdouble row_size;

  g_return_val_if_fail (scroll, 0);

  adjustment = nbtk_scroll_bar_get_adjustment (
                NBTK_SCROLL_BAR (scroll->priv->vscroll));
  g_object_get (adjustment,
                "step-increment", &row_size,
                NULL);

  return row_size;
}

void
nbtk_scroll_view_set_row_size (NbtkScrollView *scroll,
                               gfloat          row_size)
{
  NbtkAdjustment  *adjustment;

  g_return_if_fail (scroll);

  if (row_size < 0)
    {
      scroll->priv->row_size_set = FALSE;
      scroll->priv->row_size = -1;
    }
  else
    {
      scroll->priv->row_size_set = TRUE;
      scroll->priv->row_size = row_size;

      adjustment = nbtk_scroll_bar_get_adjustment (
                    NBTK_SCROLL_BAR (scroll->priv->vscroll));

      if (adjustment)
        g_object_set (adjustment,
                      "step-increment", (gdouble) scroll->priv->row_size,
                      NULL);
    }
}

