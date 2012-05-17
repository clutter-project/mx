/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scroll-view.h: Container with scroll-bars
 *
 * Copyright 2008 OpenedHand
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
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

/**
 * SECTION:mx-scroll-view
 * @short_description: a container for scrollable children
 *
 * #MxScrollView is a single child container for actors that implement
 * #MxScrollable. It provides scrollbars around the edge of the child to
 * allow the user to move around the scrollable area.
 *
 * <figure id="mx-scroll-view">
 *   <title>#MxScrollView around an #MxBoxLayout</title>
 *   <para>An example of an #MxScrollView wrapped around an #MxBoxLayout 
 *   actor (which implements #MxScrollable). The #MxBoxLayout contains 
 *   nine #ClutterRectangle instances, but the stage is too small for 
 *   all of them to be visible. The #MxScrollView adds the appropriate
 *   horizontal scroll, which makes it possible to scroll to the end
 *   of the row of rectangles.</para>
 *   <graphic fileref="MxScrollView.png" format="PNG"/>
 * </figure>
 */

#include "mx-scroll-view.h"
#include "mx-marshal.h"
#include "mx-scroll-bar.h"
#include "mx-scrollable.h"
#include "mx-stylable.h"
#include "mx-enum-types.h"
#include "mx-private.h"
#include <clutter/clutter.h>

#include "config.h"
#ifdef HAVE_CLUTTER_GESTURE
#include <clutter-gesture/clutter-gesture.h>
#endif

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxScrollView, mx_scroll_view, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

#define SCROLL_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                                             MX_TYPE_SCROLL_VIEW, \
                                                             MxScrollViewPrivate))

struct _MxScrollViewPrivate
{
  /* a pointer to the child; this is actually stored
   * inside MxBin:child, but we keep it to avoid
   * calling mx_bin_get_child() every time we need it
   */
  ClutterActor *child;

  ClutterActor *hscroll;
  ClutterActor *vscroll;

  guint         mouse_scroll : 1;
  guint         enable_gestures : 1;

  guint         scrollbar_width;
  guint         scrollbar_height;

  MxScrollPolicy scroll_policy;

#ifdef HAVE_CLUTTER_GESTURE
  ClutterGesture *gesture;
  ClutterAnimation *animation;
#endif
};

enum {
  PROP_0,

  PROP_MOUSE_SCROLL,
  PROP_ENABLE_GESTURES,
  PROP_SCROLL_POLICY
};

static void
mx_scroll_view_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MxScrollViewPrivate *priv = ((MxScrollView *) object)->priv;

  switch (property_id)
    {
    case PROP_MOUSE_SCROLL:
      g_value_set_boolean (value, priv->mouse_scroll);
      break;

    case PROP_ENABLE_GESTURES:
      g_value_set_boolean (value, priv->enable_gestures);
      break;

    case PROP_SCROLL_POLICY:
      g_value_set_enum (value, priv->scroll_policy);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_scroll_view_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  MxScrollView *view = MX_SCROLL_VIEW (object);

  switch (property_id)
    {
    case PROP_MOUSE_SCROLL:
      mx_scroll_view_set_enable_mouse_scrolling (view,
                                                 g_value_get_boolean (value));
      break;

    case PROP_ENABLE_GESTURES:
      mx_scroll_view_set_enable_gestures (view, g_value_get_boolean (value));
      break;

    case PROP_SCROLL_POLICY:
      mx_scroll_view_set_scroll_policy (view, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_scroll_view_dispose (GObject *object)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (object)->priv;

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

#ifdef HAVE_CLUTTER_GESTURE
  if (priv->gesture)
    {
      g_object_unref (priv->gesture);
      priv->gesture = NULL;
    }

  if (priv->animation)
    {
      g_object_unref (priv->animation);
      priv->animation = NULL;
    }
#endif

  /* Chaining up will remove the child actor */
  G_OBJECT_CLASS (mx_scroll_view_parent_class)->dispose (object);
}

static void
mx_scroll_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_scroll_view_parent_class)->finalize (object);
}

static void
mx_scroll_view_paint (ClutterActor *actor)
{
  ClutterActorBox box;
  gfloat w, h;
  MxAdjustment *vadjustment = NULL, *hadjustment = NULL;
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (actor)->priv;
  ClutterColor *color;

  guint8 r, g, b;
  const gint shadow = 15;

  mx_stylable_get (MX_STYLABLE (actor), "background-color", &color, NULL);

  r = color->red;
  g = color->green;
  b = color->blue;
  clutter_color_free (color);

  /* If there is a child to paint, clip it */
  if (priv->child)
    {
      clutter_actor_get_allocation_box (priv->child, &box);
      cogl_clip_push_rectangle (0, 0, (box.x2 - box.x1), (box.y2 - box.y1));
    }

  CLUTTER_ACTOR_CLASS (mx_scroll_view_parent_class)->paint (actor);

  if (priv->child)
    cogl_clip_pop ();


  clutter_actor_get_allocation_box (actor, &box);

  w = box.x2 - box.x1;
  h = box.y2 - box.y1;

  /* paint our custom children */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    {
      clutter_actor_paint (priv->hscroll);
      clutter_actor_get_allocation_box (priv->hscroll, &box);
      h -= (box.y2 - box.y1);

      hadjustment = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->hscroll));
    }
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    {
      clutter_actor_paint (priv->vscroll);
      clutter_actor_get_allocation_box (priv->vscroll, &box);
      w -= (box.x2 - box.x1);
      vadjustment = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->vscroll));
    }

  /* set up the matrial using dummy set source call */
  cogl_set_source_color4ub (0, 0, 0, 0);

  if (vadjustment)
    {
      gdouble len;
      if ((len = mx_adjustment_get_value (vadjustment)) > 0)
        {
          CoglTextureVertex top[4] = { { 0,}, };

          if (len > shadow)
            len = shadow;

          top[1].x = w;
          top[2].x = w;
          top[2].y = len;
          top[3].y = len;

          cogl_color_set_from_4ub (&top[0].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&top[1].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&top[2].color, 0, 0, 0, 0);
          cogl_color_set_from_4ub (&top[3].color, 0, 0, 0, 0);
          cogl_polygon (top, 4, TRUE);
        }

      if ((len = (mx_adjustment_get_upper (vadjustment)
                 - mx_adjustment_get_page_size (vadjustment))
         - mx_adjustment_get_value (vadjustment)) > 0)
        {
          CoglTextureVertex bottom[4] = { {0, }, };

          if (len > shadow)
            len = shadow;

          bottom[0].x = w;
          bottom[0].y = h;
          bottom[1].y = h;
          bottom[2].y = h - len;
          bottom[3].x = w;
          bottom[3].y = h - len;

          cogl_color_set_from_4ub (&bottom[0].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&bottom[1].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&bottom[2].color, 0, 0, 0, 0);
          cogl_color_set_from_4ub (&bottom[3].color, 0, 0, 0, 0);
          cogl_polygon (bottom, 4, TRUE);
        }
    }


  if (hadjustment)
    {
      gdouble len;

      if ((len = mx_adjustment_get_value (hadjustment)) > 0)
        {

          CoglTextureVertex left[4] = { { 0, }, };

          if (len > shadow)
            len = shadow;

          left[0].y = h;
          left[2].x = len;
          left[3].x = len;
          left[3].y = h;

          cogl_color_set_from_4ub (&left[0].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&left[1].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&left[2].color, 0, 0, 0, 0);
          cogl_color_set_from_4ub (&left[3].color, 0, 0, 0, 0);
          cogl_polygon (left, 4, TRUE);
        }


      if ((len = (mx_adjustment_get_upper (hadjustment)
                 - mx_adjustment_get_page_size (hadjustment))
         - mx_adjustment_get_value (hadjustment)) > 0)
        {
          CoglTextureVertex right[4] = { { 0, }, };

          if (len > shadow)
            len = shadow;

          right[0].x = w;
          right[1].x = w;
          right[1].y = h;
          right[2].x = w - len;
          right[2].y = h;
          right[3].x = w - len;


          cogl_color_set_from_4ub (&right[0].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&right[1].color, r, g, b, 0xff);
          cogl_color_set_from_4ub (&right[2].color, 0, 0, 0, 0);
          cogl_color_set_from_4ub (&right[3].color, 0, 0, 0, 0);
          cogl_polygon (right, 4, TRUE);
        }
    }


}

static void
mx_scroll_view_pick (ClutterActor       *actor,
                     const ClutterColor *color)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (actor)->priv;
  ClutterActorBox      box;

  /* Chain up so we get a bounding box pained (if we are reactive) */
  if (priv->child)
    {
      clutter_actor_get_allocation_box (priv->child, &box);
      cogl_clip_push_rectangle (0, 0, (box.x2 - box.x1), (box.y2 - box.y1));
    }

  CLUTTER_ACTOR_CLASS (mx_scroll_view_parent_class)->pick (actor, color);

  if (priv->child)
    cogl_clip_pop ();

  /* paint our custom children */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    clutter_actor_paint (priv->hscroll);
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    clutter_actor_paint (priv->vscroll);
}

static void
mx_scroll_view_get_preferred_width (ClutterActor *actor,
                                    gfloat        for_height,
                                    gfloat       *min_width_p,
                                    gfloat       *natural_width_p)
{
  MxPadding padding;
  gfloat child_min_w, child_nat_w;
  gfloat vscroll_w;

  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  /* Our natural width is the natural width of the child */
  clutter_actor_get_preferred_width (priv->child,
                                     for_height,
                                     &child_min_w,
                                     &child_nat_w);

  /* Add space for the scroll-bar if we can determine it will be necessary */
  vscroll_w = 0;
  if (for_height >= 0)
    {
      gfloat natural_height;

      clutter_actor_get_preferred_height (priv->child, -1.0,
                                          NULL,
                                          &natural_height);
      if (for_height < natural_height)
        vscroll_w = priv->scrollbar_width;
    }

  if (min_width_p)
    {
      *min_width_p = padding.left + padding.right + vscroll_w;

      /* if the scroll policy is not set to always or horizontal, then the
       * minimum size of the scroll view is the minimum size of the child */

      if (!(priv->scroll_policy == MX_SCROLL_POLICY_BOTH
            || priv->scroll_policy == MX_SCROLL_POLICY_HORIZONTAL))
        {
          *min_width_p += child_min_w;
        }
    }

  /* Add space for padding */
  if (natural_width_p)
    *natural_width_p = padding.left + padding.right + child_nat_w + vscroll_w;
}

static void
mx_scroll_view_get_preferred_height (ClutterActor *actor,
                                     gfloat        for_width,
                                     gfloat       *min_height_p,
                                     gfloat       *natural_height_p)
{
  MxPadding padding;
  gfloat min_child_h, nat_child_h;
  gfloat scroll_h;

  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  /* Our natural height is the natural height of the child */
  clutter_actor_get_preferred_height (priv->child,
                                      for_width,
                                      &min_child_h,
                                      &nat_child_h);

  /* Add space for the scroll-bar if we can determine it will be necessary */
  scroll_h = 0;
  if (for_width >= 0)
    {
      gfloat natural_width;

      clutter_actor_get_preferred_width (priv->child, -1.0,
                                         NULL,
                                         &natural_width);
      if (for_width < natural_width)
        scroll_h = priv->scrollbar_height;
    }

  /* Add space for padding */
  if (min_height_p)
    {
      *min_height_p = padding.top + padding.bottom + scroll_h;

      if (!(priv->scroll_policy == MX_SCROLL_POLICY_BOTH
            || priv->scroll_policy == MX_SCROLL_POLICY_VERTICAL))
        {
          *min_height_p += min_child_h;
        }
    }

  if (natural_height_p)
    *natural_height_p = padding.top + nat_child_h + padding.bottom + scroll_h;
}

static void
mx_scroll_view_allocate (ClutterActor          *actor,
                         const ClutterActorBox *box,
                         ClutterAllocationFlags flags)
{
  MxPadding padding;
  ClutterActorBox child_box;
  gfloat avail_width, avail_height, sb_width, sb_height;

  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_scroll_view_parent_class)->
    allocate (actor, box, flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  avail_width = (box->x2 - box->x1) - padding.left - padding.right;
  avail_height = (box->y2 - box->y1) - padding.top - padding.bottom;

  sb_width = priv->scrollbar_width;
  sb_height = priv->scrollbar_height;

  if (!CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    sb_width = 0;

  if (!CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    sb_height = 0;

  /* Vertical scrollbar */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->vscroll))
    {
      child_box.x1 = avail_width - sb_width;
      child_box.y1 = padding.top;
      child_box.x2 = avail_width;
      child_box.y2 = child_box.y1 + avail_height - sb_height;

      clutter_actor_allocate (priv->vscroll, &child_box, flags);
    }

  /* Horizontal scrollbar */
  if (CLUTTER_ACTOR_IS_VISIBLE (priv->hscroll))
    {
      child_box.x1 = padding.left;
      child_box.x2 = child_box.x1 + avail_width - sb_width;
      child_box.y1 = avail_height - sb_height;
      child_box.y2 = avail_height;

      clutter_actor_allocate (priv->hscroll, &child_box, flags);
    }


  /* Child */
  child_box.x1 = padding.left;
  child_box.x2 = avail_width - sb_width;
  child_box.y1 = padding.top;
  child_box.y2 = avail_height - sb_height;

  if (priv->child)
    clutter_actor_allocate (priv->child, &child_box, flags);
}

static void
mx_scroll_view_style_changed (MxWidget *widget, MxStyleChangedFlags flags)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (widget)->priv;

  mx_stylable_style_changed (MX_STYLABLE (priv->hscroll), flags);
  mx_stylable_style_changed (MX_STYLABLE (priv->vscroll), flags);

  mx_stylable_get (MX_STYLABLE (widget),
                   "x-mx-scrollbar-width", &priv->scrollbar_width,
                   "x-mx-scrollbar-height", &priv->scrollbar_height,
                   NULL);
}

static gboolean
mx_scroll_view_scroll_event (ClutterActor       *self,
                             ClutterScrollEvent *event)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (self)->priv;
  gdouble lower, value, upper, step;
  MxAdjustment *vadjustment, *hadjustment;

  /* don't handle scroll events if requested not to */
  if (!priv->mouse_scroll)
    return FALSE;

  hadjustment = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->hscroll));
  vadjustment = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->vscroll));

  switch (event->direction)
    {
    case CLUTTER_SCROLL_UP:
    case CLUTTER_SCROLL_DOWN:
      if (vadjustment)
        g_object_get (vadjustment,
                      "lower", &lower,
                      "step-increment", &step,
                      "value", &value,
                      "upper", &upper,
                      NULL);
      else
        return FALSE;
      break;
    case CLUTTER_SCROLL_LEFT:
    case CLUTTER_SCROLL_RIGHT:
      if (hadjustment)
        g_object_get (hadjustment,
                      "lower", &lower,
                      "step-increment", &step,
                      "value", &value,
                      "upper", &upper,
                      NULL);
      else
        return FALSE;
      break;
    }

  switch (event->direction)
    {
    case CLUTTER_SCROLL_UP:
      if (value == lower)
        return FALSE;
      else
        mx_adjustment_interpolate_relative (vadjustment, -step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    case CLUTTER_SCROLL_DOWN:
      if (value == upper)
        return FALSE;
      else
        mx_adjustment_interpolate_relative (vadjustment, step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    case CLUTTER_SCROLL_LEFT:
      if (value == lower)
        return FALSE;
      else
        mx_adjustment_interpolate_relative (hadjustment, -step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    case CLUTTER_SCROLL_RIGHT:
      if (value == upper)
        return FALSE;
      else
        mx_adjustment_interpolate_relative (hadjustment, step,
                                            250, CLUTTER_EASE_OUT_CUBIC);
      break;
    }

  return TRUE;
}

static void
mx_scroll_view_apply_style (MxWidget *widget,
                            MxStyle  *style)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (widget)->priv;

  if (priv->hscroll != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->hscroll), style);

  if (priv->vscroll != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->vscroll), style);
}

static void
mx_scroll_view_class_init (MxScrollViewClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxScrollViewPrivate));

  object_class->get_property = mx_scroll_view_get_property;
  object_class->set_property = mx_scroll_view_set_property;
  object_class->dispose= mx_scroll_view_dispose;
  object_class->finalize = mx_scroll_view_finalize;

  actor_class->paint = mx_scroll_view_paint;
  actor_class->pick = mx_scroll_view_pick;
  actor_class->get_preferred_width = mx_scroll_view_get_preferred_width;
  actor_class->get_preferred_height = mx_scroll_view_get_preferred_height;
  actor_class->allocate = mx_scroll_view_allocate;
  actor_class->scroll_event = mx_scroll_view_scroll_event;

  widget_class->apply_style = mx_scroll_view_apply_style;

  pspec = g_param_spec_boolean ("enable-mouse-scrolling",
                                "Enable Mouse Scrolling",
                                "Enable automatic mouse wheel scrolling",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class,
                                   PROP_MOUSE_SCROLL,
                                   pspec);

  pspec = g_param_spec_boolean ("enable-gestures",
                                "Enable Gestures",
                                "Enable use of pointer gestures for scrolling "
                                "if Mx was built with ClutterGesture support",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ENABLE_GESTURES, pspec);

  pspec = g_param_spec_enum ("scroll-policy",
                             "Scroll Policy",
                             "The scroll policy",
                             MX_TYPE_SCROLL_POLICY,
                             MX_SCROLL_POLICY_BOTH,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SCROLL_POLICY, pspec);
}

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("x-mx-scrollbar-width",
                                 "Vertical scroll-bar thickness",
                                 "Thickness of vertical scrollbar, in px",
                                 0, G_MAXUINT, 24,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_SCROLL_VIEW, pspec);

      pspec = g_param_spec_uint ("x-mx-scrollbar-height",
                                 "Horizontal scroll-bar thickness",
                                 "Thickness of horizontal scrollbar, in px",
                                 0, G_MAXUINT, 24,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_SCROLL_VIEW, pspec);
    }
}

static void
child_adjustment_changed_cb (MxAdjustment *adjustment,
                             ClutterActor *bar)
{
  MxScrollView *scroll;
  gdouble lower, upper, page_size;

  scroll = MX_SCROLL_VIEW (clutter_actor_get_parent (bar));

  /* Determine if this scroll-bar should be visible */
  mx_adjustment_get_values (adjustment, NULL,
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
child_hadjustment_notify_cb (GObject    *gobject,
                             GParamSpec *arg1,
                             gpointer    user_data)
{
  MxAdjustment *hadjust;

  ClutterActor *actor = CLUTTER_ACTOR (gobject);
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (user_data)->priv;

  hadjust = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->hscroll));
  if (hadjust)
    g_signal_handlers_disconnect_by_func (hadjust,
                                          child_adjustment_changed_cb,
                                          priv->hscroll);

  mx_scrollable_get_adjustments (MX_SCROLLABLE (actor), &hadjust, NULL);
  if (hadjust)
    {
      mx_scroll_bar_set_adjustment (MX_SCROLL_BAR(priv->hscroll), hadjust);
      g_signal_connect (hadjust, "changed", G_CALLBACK (
                          child_adjustment_changed_cb), priv->hscroll);
      child_adjustment_changed_cb (hadjust, priv->hscroll);
    }
}

static void
child_vadjustment_notify_cb (GObject    *gobject,
                             GParamSpec *arg1,
                             gpointer    user_data)
{
  MxAdjustment *vadjust;

  ClutterActor *actor = CLUTTER_ACTOR (gobject);
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (user_data)->priv;

  vadjust = mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->vscroll));
  if (vadjust)
    g_signal_handlers_disconnect_by_func (vadjust,
                                          child_adjustment_changed_cb,
                                          priv->vscroll);

  mx_scrollable_get_adjustments (MX_SCROLLABLE(actor), NULL, &vadjust);
  if (vadjust)
    {
      mx_scroll_bar_set_adjustment (MX_SCROLL_BAR(priv->vscroll), vadjust);
      g_signal_connect (vadjust, "changed", G_CALLBACK (
                          child_adjustment_changed_cb), priv->vscroll);
      child_adjustment_changed_cb (vadjust, priv->vscroll);
    }
}

#ifdef HAVE_CLUTTER_GESTURE
static gboolean
mx_scroll_view_gesture_slide_event_cb (ClutterGesture           *gesture,
                                       ClutterGestureSlideEvent *event,
                                       MxScrollView             *scrollview)
{
  MxScrollViewPrivate *priv = scrollview->priv;
  gdouble step, value, final;
  MxAdjustment *adjustment;

  ClutterInterval *interval;
  ClutterTimeline *timeline;

  if (!priv->enable_gestures)
    return FALSE;

  if (!priv->animation)
    {
      priv->animation = clutter_animation_new ();

      clutter_animation_set_duration (priv->animation, 1000);
      clutter_animation_set_mode (priv->animation,
                                  CLUTTER_EASE_OUT_CUBIC);
    }

  if (event->direction > 2)
    adjustment =
      mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->hscroll));
  else
    adjustment =
      mx_scroll_bar_get_adjustment (MX_SCROLL_BAR(priv->vscroll));

  g_object_get (adjustment,
                "page-increment", &step,
                "value", &value,
                NULL);
  clutter_animation_set_object (priv->animation,
                                G_OBJECT (adjustment));

  if (event->direction % 2)
    /* up, left (1, 3) */
    final = value + step;
  else
    /* down, right (2, 4) */
    final = value - step;

  timeline = clutter_animation_get_timeline (priv->animation);
  interval = clutter_animation_get_interval (priv->animation, "value");

  if (!interval)
    {
      interval = clutter_interval_new (G_TYPE_DOUBLE, value, final);
      clutter_animation_bind_interval (priv->animation, "value",
                                       interval);
      clutter_timeline_start (timeline);

    }
  else
    {
      GValue *end, *start;

      end = clutter_interval_peek_final_value (interval);
      start = clutter_interval_peek_initial_value (interval);

      g_value_set_double (end, final);
      g_value_set_double (start, value);

      clutter_timeline_rewind (timeline);
      clutter_timeline_start (timeline);
    }

  return TRUE;
}

#endif

static void
mx_scroll_view_init (MxScrollView *self)
{
  MxScrollViewPrivate *priv = self->priv = SCROLL_VIEW_PRIVATE (self);

  priv->hscroll = mx_scroll_bar_new ();
  priv->vscroll = g_object_new (MX_TYPE_SCROLL_BAR,
                                "orientation", MX_ORIENTATION_VERTICAL,
                                NULL);

  priv->scroll_policy = MX_SCROLL_POLICY_BOTH;

  clutter_actor_set_parent (priv->hscroll, CLUTTER_ACTOR (self));
  clutter_actor_set_parent (priv->vscroll, CLUTTER_ACTOR (self));
  clutter_actor_hide (priv->hscroll);
  clutter_actor_hide (priv->vscroll);

  /* mouse scroll is enabled by default, so we also need to be reactive */
  priv->mouse_scroll = TRUE;
  g_object_set (G_OBJECT (self), "reactive", TRUE, NULL);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_scroll_view_style_changed), NULL);

}

static void
mx_scroll_view_actor_added (ClutterContainer *container,
                            ClutterActor     *actor)
{
  MxScrollView *self = MX_SCROLL_VIEW (container);
  MxScrollViewPrivate *priv = self->priv;

  if (MX_IS_SCROLLABLE (actor))
    {
      priv->child = actor;

      /* Get adjustments for scroll-bars */
      g_signal_connect (actor, "notify::horizontal-adjustment",
                        G_CALLBACK (child_hadjustment_notify_cb),
                        container);
      g_signal_connect (actor, "notify::vertical-adjustment",
                        G_CALLBACK (child_vadjustment_notify_cb),
                        container);
      child_hadjustment_notify_cb (G_OBJECT (actor), NULL, container);
      child_vadjustment_notify_cb (G_OBJECT (actor), NULL, container);
    }
  else
    {
      g_warning ("Attempting to add an actor of type %s to "
                 "a MxScrollView, but the actor does "
                 "not implement MxScrollable.",
                 g_type_name (G_OBJECT_TYPE (actor)));
    }
}

static void
mx_scroll_view_actor_removed (ClutterContainer *container,
                              ClutterActor     *actor)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (container)->priv;

  if (actor == priv->child)
    {
      g_object_ref (priv->child);

      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_hadjustment_notify_cb,
                                            container);
      g_signal_handlers_disconnect_by_func (priv->child,
                                            child_vadjustment_notify_cb,
                                            container);
      mx_scrollable_set_adjustments ((MxScrollable*) priv->child, NULL, NULL);

      g_object_unref (priv->child);
      priv->child = NULL;
    }
}

static void
mx_scroll_view_foreach_with_internals (ClutterContainer *container,
                                       ClutterCallback   callback,
                                       gpointer          user_data)
{
  MxScrollViewPrivate *priv = MX_SCROLL_VIEW (container)->priv;

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
  iface->foreach_with_internals = mx_scroll_view_foreach_with_internals;

  iface->actor_added = mx_scroll_view_actor_added;
  iface->actor_removed = mx_scroll_view_actor_removed;
}

/**
 * mx_scroll_view_get_hscroll_bar:
 * @scroll: a #MxScrollView
 *
 * Gets the horizontal scrollbar of the scrollbiew
 *
 * Return value: (transfer none): the horizontal #MxScrollbar
 */
ClutterActor *
mx_scroll_view_new (void)
{
  return g_object_new (MX_TYPE_SCROLL_VIEW, NULL);
}

void
mx_scroll_view_set_enable_mouse_scrolling (MxScrollView *scroll,
                                           gboolean      enabled)
{
  MxScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->mouse_scroll != enabled)
    {
      priv->mouse_scroll = enabled;

      /* make sure we can receive mouse wheel events */
      if (enabled)
        clutter_actor_set_reactive ((ClutterActor *) scroll, TRUE);

      g_object_notify (G_OBJECT (scroll), "enable-mouse-scrolling");
    }
}

gboolean
mx_scroll_view_get_enable_mouse_scrolling (MxScrollView *scroll)
{
  MxScrollViewPrivate *priv;

  g_return_val_if_fail (MX_IS_SCROLL_VIEW (scroll), FALSE);

  priv = scroll->priv;

  return priv->mouse_scroll;
}

void
mx_scroll_view_set_enable_gestures (MxScrollView *scroll,
                                    gboolean      enabled)
{
  MxScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->enable_gestures != enabled)
    {
      priv->enable_gestures = enabled;

#ifndef HAVE_CLUTTER_GESTURE
      g_warning ("Gestures are disabled as Clutter Gesture is not available");
#else
      if (enabled && !priv->gesture)
        {
          priv->gesture = clutter_gesture_new (CLUTTER_ACTOR (scroll));
          clutter_gesture_set_gesture_mask (priv->gesture,
                                            CLUTTER_ACTOR (scroll),
                                            GESTURE_MASK_SLIDE);
          g_signal_connect (priv->gesture, "gesture-slide-event",
                            G_CALLBACK (mx_scroll_view_gesture_slide_event_cb),
                            scroll);
          clutter_actor_set_reactive (CLUTTER_ACTOR (scroll), TRUE);
        }
#endif

      g_object_notify (G_OBJECT (scroll), "enable-gestures");
    }
}

gboolean
mx_scroll_view_get_enable_gestures (MxScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_SCROLL_VIEW (scroll), FALSE);

  return scroll->priv->enable_gestures;
}


void
mx_scroll_view_set_scroll_policy (MxScrollView  *scroll,
                                  MxScrollPolicy policy)
{
  MxScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  if (priv->scroll_policy != policy)
    {
      priv->scroll_policy = policy;

      g_object_notify (G_OBJECT (scroll), "scroll-policy");

      clutter_actor_queue_relayout (CLUTTER_ACTOR (scroll));
    }
}

MxScrollPolicy
mx_scroll_view_get_scroll_policy (MxScrollView *scroll)
{
  g_return_val_if_fail (MX_IS_SCROLL_VIEW (scroll), 0);

  return scroll->priv->scroll_policy;
}

static void
_mx_scroll_view_ensure_visible_axis (MxScrollBar  *bar,
                                     gdouble       lower,
                                     gdouble       upper)
{
  gdouble new_value, adjust_lower, adjust_upper, adjust_page_size;

  gboolean changed = FALSE;
  MxAdjustment *adjust = mx_scroll_bar_get_adjustment (bar);

  mx_adjustment_get_values (adjust,
                            &new_value,
                            &adjust_lower,
                            &adjust_upper,
                            NULL,
                            NULL,
                            &adjust_page_size);

  /* Sanitise input values */
  lower = CLAMP (lower, adjust_lower, adjust_upper - adjust_page_size);
  upper = CLAMP (upper, adjust_lower + adjust_page_size, adjust_upper);

  /* Ensure the bottom is visible */
  if (new_value + adjust_page_size < upper)
    {
      new_value = upper - adjust_page_size;
      changed = TRUE;
    }

  /* Ensure the top is visible */
  if (lower < new_value)
    {
      new_value = lower;
      changed = TRUE;
    }

  if (changed)
    mx_adjustment_interpolate (adjust, new_value,
                               250, CLUTTER_EASE_OUT_CUBIC);
}

/**
 * mx_scroll_view_ensure_visible:
 * @scroll: A #MxScrollView
 * @geometry: The region to make visible
 *
 * Ensures that a given region is visible in the ScrollView, with the top-left
 * taking precedence.
 *
 */
void
mx_scroll_view_ensure_visible (MxScrollView          *scroll,
                               const ClutterGeometry *geometry)
{
  MxScrollViewPrivate *priv;

  g_return_if_fail (MX_IS_SCROLL_VIEW (scroll));

  priv = scroll->priv;

  _mx_scroll_view_ensure_visible_axis (MX_SCROLL_BAR (priv->hscroll),
                                       geometry->x,
                                       geometry->x + geometry->width);
  _mx_scroll_view_ensure_visible_axis (MX_SCROLL_BAR (priv->vscroll),
                                       geometry->y,
                                       geometry->y + geometry->height);
}
