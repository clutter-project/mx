/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-box-layout.h: box layout actor
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

/**
 * SECTION:mx-box-layout
 * @short_description: a layout container arranging children in a single line
 *
 * The #MxBoxLayout arranges its children along a single line, where each
 * child can be allocated either its preferred size or larger if the expand
 * option is set. If the fill option is set, the actor will be allocated more
 * than its requested size. If the fill option is not set, but the expand option
 * is enabled, then the position of the actor within the available space can
 * be determined by the alignment child property.
 *
 * <figure id="mx-box-layout-horizontal">
 *   <title>Box layout with horizontal flow</title>
 *   <para>The image shows an #MxBoxLayout with the
 *   #MxBoxLayout:orientation property set to
 *   #MX_ORIENTATION_HORIZONTAL.</para>
 *   <graphic fileref="MxBoxLayout-horizontal.png" format="PNG"/>
 * </figure>
 *
 * <figure id="mx-box-layout-vertical">
 *   <title>Box layout with vertical flow</title>
 *   <para>The image shows an #MxBoxLayout with the
 *   #MxBoxLayout:orientation property set to
 *   #MX_ORIENTATION_VERTICAL.</para>
 *   <graphic fileref="MxBoxLayout-vertical.png" format="PNG"/>
 * </figure>
 */

#include <stdarg.h>
#include <string.h>
#include <gobject/gvaluecollector.h>

#include "mx-box-layout.h"

#include "mx-private.h"
#include "mx-scrollable.h"
#include "mx-box-layout-child.h"
#include "mx-focusable.h"


static void mx_box_container_iface_init (ClutterContainerIface *iface);
static void mx_box_scrollable_interface_init (MxScrollableIface *iface);
static void mx_box_focusable_iface_init (MxFocusableIface *iface);
static void mx_box_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxBoxLayout, mx_box_layout, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                mx_box_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                mx_box_scrollable_interface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_box_focusable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_box_stylable_iface_init));

#define BOX_LAYOUT_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_BOX_LAYOUT, MxBoxLayoutPrivate))

enum {
  PROP_0,

  PROP_ORIENTATION,
  PROP_SPACING,

  PROP_HADJUST,
  PROP_VADJUST,

  PROP_ENABLE_ANIMATIONS,
  PROP_SCROLL_TO_FOCUSED
};

struct _MxBoxLayoutPrivate
{
  GList        *children;

  guint         ignore_css_spacing : 1; /* Should we ignore spacing from
                                           the CSS because the application
                                           set it via set_spacing */
  guint         spacing;

  MxAdjustment *hadjustment;
  MxAdjustment *vadjustment;

  GHashTable      *start_allocations;
  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
  guint            is_animating : 1;
  guint            enable_animations : 1;
  guint            scroll_to_focused : 1;

  MxOrientation orientation;

  MxFocusable *last_focus;
};

void _mx_box_layout_finish_animation (MxBoxLayout *box);

void
_mx_box_layout_start_animation (MxBoxLayout *box)
{
  MxBoxLayoutPrivate *priv = box->priv;

  if (priv->is_animating || !priv->enable_animations)
      return;

  if (!CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (box)))
    return;

  priv->is_animating = TRUE;

  priv->timeline = clutter_timeline_new (300);
  g_signal_connect_swapped (priv->timeline, "new-frame",
                            G_CALLBACK (clutter_actor_queue_relayout), box);
  g_signal_connect_swapped (priv->timeline, "completed",
                            G_CALLBACK (_mx_box_layout_finish_animation), box);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
                                        CLUTTER_EASE_OUT_CUBIC);

  clutter_timeline_start (priv->timeline);
}

void
_mx_box_layout_finish_animation (MxBoxLayout *box)
{
  MxBoxLayoutPrivate *priv = box->priv;

  if (priv->timeline)
    {
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }

  if (priv->alpha)
    {
      g_object_unref (priv->alpha);
      priv->alpha = NULL;
    }

  priv->is_animating = FALSE;
}

/*
 * MxScrollable Interface Implementation
 */
static void
adjustment_value_notify_cb (MxAdjustment *adjustment,
                            GParamSpec   *pspec,
                            MxBoxLayout  *box)
{
  clutter_actor_queue_redraw (CLUTTER_ACTOR (box));
}

static void
scrollable_set_adjustments (MxScrollable *scrollable,
                            MxAdjustment *hadjustment,
                            MxAdjustment *vadjustment)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (scrollable)->priv;

  if (hadjustment != priv->hadjustment)
    {
      if (priv->hadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->hadjustment,
                                                adjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->hadjustment);
        }

      if (hadjustment)
        {
          g_object_ref (hadjustment);
          g_signal_connect (hadjustment, "notify::value",
                            G_CALLBACK (adjustment_value_notify_cb),
                            scrollable);
        }

      priv->hadjustment = hadjustment;
      g_object_notify (G_OBJECT (scrollable), "horizontal-adjustment");
    }

  if (vadjustment != priv->vadjustment)
    {
      if (priv->vadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->vadjustment,
                                                adjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->vadjustment);
        }

      if (vadjustment)
        {
          g_object_ref (vadjustment);
          g_signal_connect (vadjustment, "notify::value",
                            G_CALLBACK (adjustment_value_notify_cb),
                            scrollable);
        }

      priv->vadjustment = vadjustment;
      g_object_notify (G_OBJECT (scrollable), "vertical-adjustment");
    }
}

static void
scrollable_get_adjustments (MxScrollable  *scrollable,
                            MxAdjustment **hadjustment,
                            MxAdjustment **vadjustment)
{
  MxBoxLayoutPrivate *priv;

  priv = (MX_BOX_LAYOUT (scrollable))->priv;

  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          MxAdjustment *adjustment;

          /* create an initial adjustment. this is filled with correct values
           * as soon as allocate() is called */

          adjustment = mx_adjustment_new ();

          scrollable_set_adjustments (scrollable,
                                      adjustment,
                                      priv->vadjustment);

          g_object_unref (adjustment);

          *hadjustment = adjustment;
        }
    }

  if (vadjustment)
    {
      if (priv->vadjustment)
        *vadjustment = priv->vadjustment;
      else
        {
          MxAdjustment *adjustment;

          /* create an initial adjustment. this is filled with correct values
           * as soon as allocate() is called */

          adjustment = mx_adjustment_new ();

          scrollable_set_adjustments (scrollable,
                                      priv->hadjustment,
                                      adjustment);

          g_object_unref (adjustment);

          *vadjustment = adjustment;
        }
    }
}



static void
mx_box_scrollable_interface_init (MxScrollableIface *iface)
{
  iface->set_adjustments = scrollable_set_adjustments;
  iface->get_adjustments = scrollable_get_adjustments;
}

/*
 * ClutterContainer Implementation
 */

static void
fade_in_actor (ClutterActor *actor)
{
  clutter_actor_animate (actor, CLUTTER_LINEAR, 300, "opacity", 0xff, NULL);
}

static void
mx_box_container_add_actor (ClutterContainer *container,
                            ClutterActor     *actor)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  priv->children = g_list_append (priv->children, actor);

  if (priv->enable_animations)
    {
      _mx_box_layout_start_animation (MX_BOX_LAYOUT (container));

      if (priv->timeline)
        {
          /* fade in the new actor when there is room */
          clutter_actor_set_opacity (actor, 0);
          g_signal_connect_swapped (priv->timeline, "completed",
                                    G_CALLBACK (fade_in_actor), actor);
        }
    }

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
mx_box_container_remove_actor (ClutterContainer *container,
                               ClutterActor     *actor)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  GList *item = NULL;

  item = g_list_find (priv->children, actor);

  if (item == NULL)
    {
      g_warning ("Actor of type '%s' is not a child of container of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  g_object_ref (actor);

  if ((ClutterActor *)priv->last_focus == actor)
    priv->last_focus = NULL;

  priv->children = g_list_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  if (priv->enable_animations)
    _mx_box_layout_start_animation (MX_BOX_LAYOUT (container));
  else
    clutter_actor_queue_relayout ((ClutterActor *) container);

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);
}

static void
mx_box_container_foreach (ClutterContainer *container,
                          ClutterCallback   callback,
                          gpointer          callback_data)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;
  ClutterActor       *child;
  GList              *list;

  list = priv->children;
  while (list)
    {
      child = list->data;
      list  = list->next;

      callback (child, callback_data);
    }
}

/*
 * Implementations for raise, lower and sort_by_depth_order are taken from
 * ClutterBox.
 */
static void
mx_box_container_lower (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  priv->children = g_list_remove (priv->children, actor);

  if (sibling == NULL)
    priv->children = g_list_prepend (priv->children, actor);
  else
    {
      gint index_ = g_list_index (priv->children, sibling);

      priv->children = g_list_insert (priv->children, actor, index_);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
}

static void
mx_box_container_raise (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  priv->children = g_list_remove (priv->children, actor);

  if (sibling == NULL)
    priv->children = g_list_append (priv->children, actor);
  else
    {
      gint index_ = g_list_index (priv->children, sibling) + 1;

      priv->children = g_list_insert (priv->children, actor, index_);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
}

static gint
sort_by_depth (gconstpointer a,
               gconstpointer b)
{
  gfloat depth_a = clutter_actor_get_depth ((ClutterActor *) a);
  gfloat depth_b = clutter_actor_get_depth ((ClutterActor *) b);

  if (depth_a < depth_b)
    return -1;

  if (depth_a > depth_b)
    return 1;

  return 0;
}

static void
mx_box_container_sort_depth_order (ClutterContainer *container)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  priv->children = g_list_sort (priv->children, sort_by_depth);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
}

static void
mx_box_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_box_container_add_actor;
  iface->remove = mx_box_container_remove_actor;
  iface->foreach = mx_box_container_foreach;
  iface->lower = mx_box_container_lower;
  iface->raise = mx_box_container_raise;
  iface->sort_depth_order = mx_box_container_sort_depth_order;

  iface->child_meta_type = MX_TYPE_BOX_LAYOUT_CHILD;
}

/*
 * focusable implementation
 */

static void
update_adjustments (MxBoxLayout *self,
                    MxFocusable *focusable)
{
  MxBoxLayoutPrivate *priv = self->priv;
  gdouble value, new_value, page_size;
  ClutterActorBox box = { 0, };
  clutter_actor_get_allocation_box (CLUTTER_ACTOR (focusable), &box);

  if (!priv->scroll_to_focused)
    return;

  if (priv->vadjustment)
    {
      mx_adjustment_get_values (priv->vadjustment,
                                &value, NULL, NULL, NULL, NULL,
                                &page_size);
      if (box.y1 < value)
        new_value = box.y1;
      else if (box.y2 > value + page_size)
        new_value = box.y2 - page_size;
      else
        new_value = value;
      mx_adjustment_interpolate (priv->vadjustment,
                                 new_value,
                                 250, CLUTTER_EASE_OUT_CUBIC);
    }

  if (priv->hadjustment)
    {
      mx_adjustment_get_values (priv->hadjustment,
                                &value, NULL, NULL, NULL, NULL,
                                &page_size);
      if (box.x1 < value)
        new_value = box.x1;
      else if (box.x2 > value + page_size)
        new_value = box.x2 - page_size;
      else
        new_value = value;
      mx_adjustment_interpolate (priv->hadjustment,
                                 new_value,
                                 250, CLUTTER_EASE_OUT_CUBIC);
    }
}

static MxFocusable*
mx_box_layout_move_focus (MxFocusable      *focusable,
                          MxFocusDirection  direction,
                          MxFocusable      *from)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (focusable)->priv;
  GList *l, *childlink;
  MxFocusHint hint;

  /* find the current focus */
  childlink = g_list_find (priv->children, from);

  if (!childlink)
    return NULL;

  priv->last_focus = from;

  hint = mx_focus_hint_from_direction (direction);

  /* convert left/right and up/down into next/previous */
  if (priv->orientation == MX_ORIENTATION_HORIZONTAL)
    {
      if (direction == MX_FOCUS_DIRECTION_LEFT)
        direction = MX_FOCUS_DIRECTION_PREVIOUS;
      else if (direction == MX_FOCUS_DIRECTION_RIGHT)
        direction = MX_FOCUS_DIRECTION_NEXT;
    }
  else
    {
      if (direction == MX_FOCUS_DIRECTION_UP)
        direction = MX_FOCUS_DIRECTION_PREVIOUS;
      else if (direction == MX_FOCUS_DIRECTION_DOWN)
        direction = MX_FOCUS_DIRECTION_NEXT;
    }

  /* find the next widget to focus */
  if (direction == MX_FOCUS_DIRECTION_NEXT)
    {
      for (l = childlink->next; l; l = g_list_next (l))
        {
          if (MX_IS_FOCUSABLE (l->data))
            {
              MxFocusable *focused, *child;

              child = MX_FOCUSABLE (l->data);
              focused = mx_focusable_accept_focus (child, hint);

              if (focused)
                {
                  update_adjustments (MX_BOX_LAYOUT (focusable), focused);
                  return focused;
                }
            }
        }
    }
  else if (direction == MX_FOCUS_DIRECTION_PREVIOUS)
    {
      for (l = g_list_previous (childlink); l; l = g_list_previous (l))
        {
          if (MX_IS_FOCUSABLE (l->data))
            {
              MxFocusable *focused, *child;

              child = MX_FOCUSABLE (l->data);
              focused = mx_focusable_accept_focus (child, hint);

              if (focused)
                {
                  update_adjustments (MX_BOX_LAYOUT (focusable), focused);
                  return focused;
                }
            }
        }
    }

  return NULL;
}

static MxFocusable*
mx_box_layout_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (focusable)->priv;
  MxFocusable *return_focusable;
  MxFocusHint modified_hint;
  GList* list, *l;

  return_focusable = NULL;

  /* Transform the hint based on our orientation */
  modified_hint = hint;
  if (priv->orientation == MX_ORIENTATION_HORIZONTAL)
    {
      if (hint == MX_FOCUS_HINT_FROM_RIGHT)
        modified_hint = MX_FOCUS_HINT_LAST;
      else if (hint == MX_FOCUS_HINT_FROM_LEFT)
        modified_hint = MX_FOCUS_HINT_FIRST;
    }
  else
    {
      if (hint == MX_FOCUS_HINT_FROM_BELOW)
        modified_hint = MX_FOCUS_HINT_LAST;
      else if (hint == MX_FOCUS_HINT_FROM_ABOVE)
        modified_hint = MX_FOCUS_HINT_FIRST;
    }

  /* find the first/last/prior focusable widget */
  switch (modified_hint)
    {
    case MX_FOCUS_HINT_LAST:
      list = g_list_reverse (g_list_copy (priv->children));
      break;

    default:
    case MX_FOCUS_HINT_PRIOR:
      if (priv->last_focus)
        {
          list = g_list_copy (g_list_find (priv->children, priv->last_focus));
          if (list)
            break;
        }
      /* This intentionally runs into the next case */

    case MX_FOCUS_HINT_FIRST:
      list = g_list_copy (priv->children);
      break;
    }

  for (l = list; l; l = g_list_next (l))
    {
      if (MX_IS_FOCUSABLE (l->data))
        {
          return_focusable = mx_focusable_accept_focus (MX_FOCUSABLE (l->data),
                                                        hint);

          if (return_focusable)
            {
              update_adjustments (MX_BOX_LAYOUT (focusable), return_focusable);
              break;
            }
        }
    }

  g_list_free (list);

  return return_focusable;
}

static void
mx_box_focusable_iface_init (MxFocusableIface *iface)
{
  iface->move_focus = mx_box_layout_move_focus;
  iface->accept_focus = mx_box_layout_accept_focus;
}

/* Stylable inplementation */
static void
mx_box_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("x-mx-spacing",
                                 "Spacing",
                                 "The size of the spacing",
                                 0, G_MAXUINT, 0,
                                 MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BOX_LAYOUT, pspec);
    }
}

static void
mx_box_layout_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (object)->priv;
  MxAdjustment *adjustment;

  switch (property_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;

    case PROP_SPACING:
      g_value_set_uint (value, priv->spacing);
      break;

    case PROP_ENABLE_ANIMATIONS:
      g_value_set_boolean (value, priv->enable_animations);
      break;

    case PROP_SCROLL_TO_FOCUSED:
      g_value_set_boolean (value, priv->scroll_to_focused);
      break;

    case PROP_HADJUST:
      scrollable_get_adjustments (MX_SCROLLABLE (object), &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST:
      scrollable_get_adjustments (MX_SCROLLABLE (object), NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_box_layout_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  MxBoxLayout *box = MX_BOX_LAYOUT (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      mx_box_layout_set_orientation (box, g_value_get_enum (value));
      break;

    case PROP_SPACING:
      mx_box_layout_set_spacing (box, g_value_get_uint (value));
      break;

    case PROP_ENABLE_ANIMATIONS:
      mx_box_layout_set_enable_animations (box, g_value_get_boolean (value));
      break;

    case PROP_HADJUST:
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  g_value_get_object (value),
                                  box->priv->vadjustment);
      break;

    case PROP_VADJUST:
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  box->priv->hadjustment,
                                  g_value_get_object (value));
      break;

    case PROP_SCROLL_TO_FOCUSED:
      mx_box_layout_set_scroll_to_focused (box, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_box_layout_dispose (GObject *object)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (object)->priv;

  /* cleanup any running animation */
  _mx_box_layout_finish_animation (MX_BOX_LAYOUT (object));

  /* destroy the children
   * clutter_actor_destroy() will call clutter_container_remove() which will
   * remove the children from the internal list
   */
  while (priv->children)
    clutter_actor_destroy (CLUTTER_ACTOR (priv->children->data));

  if (priv->hadjustment)
    {
      g_object_unref (priv->hadjustment);
      priv->hadjustment = NULL;
    }

  if (priv->vadjustment)
    {
      g_object_unref (priv->vadjustment);
      priv->vadjustment = NULL;
    }

  G_OBJECT_CLASS (mx_box_layout_parent_class)->dispose (object);
}

static void
mx_box_layout_finalize (GObject *object)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (object)->priv;

  if (priv->start_allocations)
    {
      g_hash_table_destroy (priv->start_allocations);
      priv->start_allocations = NULL;
    }

  G_OBJECT_CLASS (mx_box_layout_parent_class)->finalize (object);
}

static void
mx_box_layout_get_preferred_width (ClutterActor *actor,
                                   gfloat        for_height,
                                   gfloat       *min_width_p,
                                   gfloat       *natural_width_p)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  MxPadding padding = { 0, };
  gint n_children = 0;
  GList *l;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width_p)
    *min_width_p = 0;

  if (natural_width_p)
    *natural_width_p = 0;

  if (for_height > 0)
    for_height = MAX (0, for_height - padding.top - padding.bottom);

  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;
      gfloat child_for_height;

      if (!CLUTTER_ACTOR_IS_VISIBLE ((ClutterActor*) l->data))
        continue;

      n_children++;

      if (priv->orientation == MX_ORIENTATION_HORIZONTAL)
        child_for_height = for_height;
      else
        child_for_height = -1;

      clutter_actor_get_preferred_width ((ClutterActor*) l->data,
                                         child_for_height,
                                         &child_min,
                                         &child_nat);

      if (priv->orientation == MX_ORIENTATION_VERTICAL)
        {
          if (min_width_p)
            *min_width_p = MAX (child_min, *min_width_p);

          if (natural_width_p)
            *natural_width_p = MAX (child_nat, *natural_width_p);
        }
      else
        {
          if (min_width_p)
            *min_width_p += child_min;

          if (natural_width_p)
            *natural_width_p += child_nat;

        }
    }


  if (priv->orientation == MX_ORIENTATION_HORIZONTAL && n_children > 1)
    {
      if (min_width_p)
        *min_width_p += priv->spacing * (n_children - 1);

      if (natural_width_p)
        *natural_width_p += priv->spacing * (n_children - 1);
    }

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
mx_box_layout_get_preferred_height (ClutterActor *actor,
                                    gfloat        for_width,
                                    gfloat       *min_height_p,
                                    gfloat       *natural_height_p)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  MxPadding padding = { 0, };
  gint n_children = 0;
  GList *l;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);


  if (min_height_p)
    *min_height_p = 0;

  if (natural_height_p)
    *natural_height_p = 0;

  if (for_width > 0)
    for_width = MAX (0, for_width - padding.left - padding.right);


  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;
      gfloat child_for_width;

      if (!CLUTTER_ACTOR_IS_VISIBLE ((ClutterActor*) l->data))
        continue;

      n_children++;

      if (priv->orientation == MX_ORIENTATION_VERTICAL)
        child_for_width = for_width;
      else
        child_for_width = -1;

      clutter_actor_get_preferred_height ((ClutterActor*) l->data,
                                          child_for_width,
                                          &child_min,
                                          &child_nat);

      if (priv->orientation == MX_ORIENTATION_HORIZONTAL)
        {
          if (min_height_p)
            *min_height_p = MAX (child_min, *min_height_p);

          if (natural_height_p)
            *natural_height_p = MAX (child_nat, *natural_height_p);
        }
      else
        {
          if (min_height_p)
            *min_height_p += child_min;

          if (natural_height_p)
            *natural_height_p += child_nat;
        }
    }

  if (priv->orientation == MX_ORIENTATION_VERTICAL && n_children > 1)
    {
      if (min_height_p)
        *min_height_p += priv->spacing * (n_children - 1);

      if (natural_height_p)
        *natural_height_p += priv->spacing * (n_children - 1);
    }

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;
}

static void
mx_box_layout_allocate (ClutterActor          *actor,
                        const ClutterActorBox *box,
                        ClutterAllocationFlags flags)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  gfloat avail_width, avail_height, pref_width, pref_height;
  MxPadding padding = { 0, };
  gboolean allocate_pref;
  gfloat extra_space = 0;
  gfloat position = 0;
  GList *l;
  gint n_expand_children, n_children;

  CLUTTER_ACTOR_CLASS (mx_box_layout_parent_class)->allocate (actor, box,
                                                              flags);

  if (priv->children == NULL)
    return;

  /* count the number of children with expand set to TRUE and the
   * amount of visible children.
   */
  n_children = n_expand_children = 0;
  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child;

      child = (ClutterActor*) l->data;

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        {
          MxBoxLayoutChild *meta;

          meta = (MxBoxLayoutChild*)
            clutter_container_get_child_meta ((ClutterContainer *) actor,
                                              child);
          n_children++;

          if (meta->expand)
            n_expand_children++;

        }
    }

  /* We have no visible children, so bail out */
  if (n_children == 0)
    return;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  /* do not take off padding just yet, as we are comparing this to the values
   * from get_preferred_height/width which will include padding */
  avail_width  = box->x2 - box->x1;
  avail_height = box->y2 - box->y1;

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
    {
      gfloat min_height;
      mx_box_layout_get_preferred_height (actor, avail_width,
                                          &min_height, &pref_height);
      pref_width = avail_width;

      if (!priv->vadjustment && (pref_height > box->y2 - box->y1))
        {
          /* allocated less than the preferred height and not scrolling */
          allocate_pref = FALSE;
          extra_space = avail_height - min_height;
        }
      else
        allocate_pref = TRUE;
    }
  else
    {
      gfloat min_width;
      mx_box_layout_get_preferred_width (actor, avail_height,
                                         &min_width, &pref_width);
      pref_height = avail_height;

      if (!priv->hadjustment && (pref_width > box->x2 - box->x1))
        {
          /* allocated less than the preferred width and not scrolling */
          allocate_pref = FALSE;
          extra_space = avail_width - min_width;
        }
      else
        allocate_pref = TRUE;
    }

  /* remove the padding values from the available and preferred sizes so we
   * can use them for allocating the children */
  avail_width -= padding.left + padding.right;
  avail_height -= padding.top + padding.bottom;

  pref_width -= padding.left + padding.right;
  pref_height -= padding.top + padding.bottom;

  /* update adjustments for scrolling */
  if (priv->vadjustment)
    {
      gdouble step_inc, page_inc;

      /* Base the adjustment stepping on the size of the first child.
       * In the case where all your children are the same size, this
       * will probably provide the desired behaviour.
       */
      if (priv->children && priv->orientation == MX_ORIENTATION_VERTICAL)
        {
          gfloat child_height;
          ClutterActor *first_child = (ClutterActor *)priv->children->data;
          clutter_actor_get_preferred_height (first_child,
                                              avail_width,
                                              NULL,
                                              &child_height);
          step_inc = child_height;
          page_inc = ((gint)(avail_height / step_inc)) * step_inc;
        }
      else
        {
          step_inc = avail_height / 6;
          page_inc = avail_height;
        }

      g_object_set (G_OBJECT (priv->vadjustment),
                    "lower", 0.0,
                    "upper", pref_height,
                    "page-size", avail_height,
                    "step-increment", step_inc,
                    "page-increment", page_inc,
                    NULL);
    }

  if (priv->hadjustment)
    {
      gdouble step_inc, page_inc;

      if (priv->children && priv->orientation == MX_ORIENTATION_HORIZONTAL)
        {
          gfloat child_width;
          ClutterActor *first_child = (ClutterActor *)priv->children->data;
          clutter_actor_get_preferred_width (first_child,
                                             avail_height,
                                             &child_width,
                                             NULL);
          step_inc = child_width;
          page_inc = ((gint)(avail_width / step_inc)) * step_inc;
        }
      else
        {
          step_inc = avail_width / 6;
          page_inc = avail_width;
        }

      g_object_set (G_OBJECT (priv->hadjustment),
                    "lower", 0.0,
                    "upper", pref_width,
                    "page-size", avail_width,
                    "step-increment", step_inc,
                    "page-increment", page_inc,
                    NULL);
    }

  /* We're allocating our preferred size or higher, so calculate
   * the extra space to give to expanded children.
   */
  if (allocate_pref)
    {
      if (n_expand_children > 0)
        {
          if (priv->orientation == MX_ORIENTATION_VERTICAL)
            extra_space = (avail_height - pref_height) / n_expand_children;
          else
            extra_space = (avail_width - pref_width) / n_expand_children;

          /* don't shrink anything */
          if (extra_space < 0)
            extra_space = 0;
        }
    }

  if (priv->orientation == MX_ORIENTATION_VERTICAL)
    position = padding.top;
  else
    position = padding.left;

  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = (ClutterActor*) l->data;
      ClutterActorBox child_box, old_child_box;
      gfloat child_nat, child_min;
      MxBoxLayoutChild *meta;

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      meta = (MxBoxLayoutChild*)
        clutter_container_get_child_meta ((ClutterContainer *) actor, child);

      if (priv->orientation == MX_ORIENTATION_VERTICAL)
        {
          clutter_actor_get_preferred_height (child, avail_width,
                                              &child_min, &child_nat);

          child_box.y1 = position;

          if (allocate_pref)
            {
              if (meta->expand)
                child_box.y2 = position + child_nat + (int)extra_space;
              else
                child_box.y2 = position + child_nat;
            }
          else
            {
              child_box.y2 =
                position + MIN (child_nat,
                                child_min + (int)(extra_space / n_children));

              n_children --;
              if (extra_space >= (child_box.y2 - child_box.y1 - child_min))
                extra_space -= child_box.y2 - child_box.y1 - child_min;
              else
                extra_space = 0;
            }

          child_box.x1 = padding.left;
          child_box.x2 = avail_width + padding.left;
        }
      else
        {
          clutter_actor_get_preferred_width (child, avail_height,
                                             &child_min, &child_nat);

          child_box.x1 = position;

          if (allocate_pref)
            {
              if (meta->expand)
                child_box.x2 = position + child_nat + (int)extra_space;
              else
                child_box.x2 = position + child_nat;
            }
          else
            {
              child_box.x2 =
                position + MIN (child_nat,
                                child_min + (int)(extra_space / n_children));

              n_children --;
              if (extra_space >= (child_box.x2 - child_box.x1 - child_min))
                extra_space -= child_box.x2 - child_box.x1 - child_min;
              else
                extra_space = 0;
            }

          child_box.y1 = padding.top;
          child_box.y2 = avail_height + padding.top;
        }

      /* Adjust the box for alignment/fill */
      old_child_box = child_box;
      mx_allocate_align_fill (child, &child_box, meta->x_align, meta->y_align,
                              meta->x_fill, meta->y_fill);

      if (priv->is_animating)
        {
          ClutterActorBox *start, *end, now;
          gdouble alpha;

          ClutterActorBox *copy = g_new (ClutterActorBox, 1);

          *copy = child_box;

          start = g_hash_table_lookup (priv->start_allocations, child);
          end = &child_box;
          alpha = clutter_alpha_get_alpha (priv->alpha);

          if (!start)
            {
              /* don't know where this actor was from (possibly recently
               * added), so just allocate the end co-ordinates */
              clutter_actor_allocate (child, end, flags);
              goto next;
            }

          now.x1 = (int) (start->x1 + (end->x1 - start->x1) * alpha);
          now.x2 = (int) (start->x2 + (end->x2 - start->x2) * alpha);
          now.y1 = (int) (start->y1 + (end->y1 - start->y1) * alpha);
          now.y2 = (int) (start->y2 + (end->y2 - start->y2) * alpha);

          clutter_actor_allocate (child, &now, flags);
        }
      else
        {
          /* store the allocations in case an animation is needed soon */
          ClutterActorBox *copy;

          /* update the value in the hash table */
          if (priv->enable_animations)
            {
              copy = g_boxed_copy (CLUTTER_TYPE_ACTOR_BOX, &child_box);
              g_hash_table_insert (priv->start_allocations, child, copy);
            }
          else
            {
              copy = &child_box;
            }

          clutter_actor_allocate (child, copy, flags);
        }

next:
      if (priv->orientation == MX_ORIENTATION_VERTICAL)
        position += (old_child_box.y2 - old_child_box.y1) + priv->spacing;
      else
        position += (old_child_box.x2 - old_child_box.x1) + priv->spacing;
    }
}

static void
mx_box_layout_apply_transform (ClutterActor *a,
                               CoglMatrix   *m)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (a)->priv;
  gdouble x, y;

  CLUTTER_ACTOR_CLASS (mx_box_layout_parent_class)->apply_transform (a, m);

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  cogl_matrix_translate (m, (int) -x, (int) -y, 0);
}

static gboolean
mx_box_layout_get_paint_volume (ClutterActor       *actor,
                                ClutterPaintVolume *volume)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  ClutterVertex vertex;

  if (!clutter_paint_volume_set_from_allocation (volume, actor))
    return FALSE;

  clutter_paint_volume_get_origin (volume, &vertex);

  if (priv->hadjustment)
    vertex.x += mx_adjustment_get_value (priv->hadjustment);

  if (priv->vadjustment)
    vertex.y += mx_adjustment_get_value (priv->vadjustment);

  clutter_paint_volume_set_origin (volume, &vertex);

  return TRUE;
}

static void
mx_box_layout_paint (ClutterActor *actor)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  GList *l;
  gdouble x, y;
  ClutterActorBox child_b;
  ClutterActorBox box_b;

  CLUTTER_ACTOR_CLASS (mx_box_layout_parent_class)->paint (actor);

  if (priv->children == NULL)
    return;

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  clutter_actor_get_allocation_box (actor, &box_b);
  box_b.x2 = (box_b.x2 - box_b.x1) + x;
  box_b.x1 = x;
  box_b.y2 = (box_b.y2 - box_b.y1) + y;
  box_b.y1 = y;

  for (l = priv->children; l; l = g_list_next (l))
    {
      ClutterActor *child = (ClutterActor*) l->data;

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      clutter_actor_get_allocation_box (child, &child_b);

      if ((child_b.x1 < box_b.x2) &&
          (child_b.x2 > box_b.x1) &&
          (child_b.y1 < box_b.y2) &&
          (child_b.y2 > box_b.y1))
        {
          clutter_actor_paint (child);
        }
    }
}

static void
mx_box_layout_pick (ClutterActor       *actor,
                    const ClutterColor *color)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (actor)->priv;
  GList *l;
  gdouble x, y;
  ClutterActorBox child_b;
  ClutterActorBox box_b;

  CLUTTER_ACTOR_CLASS (mx_box_layout_parent_class)->pick (actor, color);

  if (priv->children == NULL)
    return;

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  clutter_actor_get_allocation_box (actor, &box_b);
  box_b.x2 = (box_b.x2 - box_b.x1) + x;
  box_b.x1 = x;
  box_b.y2 = (box_b.y2 - box_b.y1) + y;
  box_b.y1 = y;

  for (l = priv->children; l; l = g_list_next (l))
    {
      ClutterActor *child = (ClutterActor*) l->data;

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      clutter_actor_get_allocation_box (child, &child_b);

      if ((child_b.x1 < box_b.x2)
          && (child_b.x2 > box_b.x1)
          && (child_b.y1 < box_b.y2)
          && (child_b.y2 > box_b.y1))
        {
          clutter_actor_paint (child);
        }
    }
}

static void
mx_box_layout_class_init (MxBoxLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxBoxLayoutPrivate));

  object_class->get_property = mx_box_layout_get_property;
  object_class->set_property = mx_box_layout_set_property;
  object_class->dispose = mx_box_layout_dispose;
  object_class->finalize = mx_box_layout_finalize;

  actor_class->allocate = mx_box_layout_allocate;
  actor_class->get_preferred_width = mx_box_layout_get_preferred_width;
  actor_class->get_preferred_height = mx_box_layout_get_preferred_height;
  actor_class->apply_transform = mx_box_layout_apply_transform;
  actor_class->get_paint_volume = mx_box_layout_get_paint_volume;

  actor_class->paint = mx_box_layout_paint;
  actor_class->pick = mx_box_layout_pick;

  pspec = g_param_spec_enum ("orientation",
                             "Orientation",
                             "Orientation of the layout",
                             MX_TYPE_ORIENTATION,
                             MX_ORIENTATION_HORIZONTAL,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ORIENTATION, pspec);

  pspec = g_param_spec_uint ("spacing",
                             "Spacing",
                             "Spacing between children",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SPACING, pspec);

  pspec = g_param_spec_boolean ("enable-animations",
                                "Enable Animations",
                                "Enable animations between certain property"
                                " and child property changes",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ENABLE_ANIMATIONS, pspec);

  pspec = g_param_spec_boolean ("scroll-to-focused",
                                "Scroll to focused",
                                "Automatically scroll to the focused actor",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SCROLL_TO_FOCUSED, pspec);

  /* MxScrollable properties */
  g_object_class_override_property (object_class,
                                    PROP_HADJUST,
                                    "horizontal-adjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUST,
                                    "vertical-adjustment");

}

static void
mx_box_layout_free_allocation (ClutterActorBox *box)
{
  g_boxed_free (CLUTTER_TYPE_ACTOR_BOX, box);
}

static void
mx_box_layout_style_changed (MxWidget *widget,
                             gpointer  userdata)
{
  MxBoxLayout *layout = MX_BOX_LAYOUT (widget);
  MxBoxLayoutPrivate *priv = layout->priv;
  guint spacing;

  mx_stylable_get (MX_STYLABLE (widget),
                   "x-mx-spacing", &spacing,
                   NULL);

  if (!priv->ignore_css_spacing && (priv->spacing != spacing))
    {
      priv->spacing = spacing;
      clutter_actor_queue_relayout (CLUTTER_ACTOR (widget));
    }

  clutter_actor_queue_redraw (CLUTTER_ACTOR (widget));
}

static void
mx_box_layout_init (MxBoxLayout *self)
{
  self->priv = BOX_LAYOUT_PRIVATE (self);


  self->priv->start_allocations = g_hash_table_new_full (g_direct_hash,
                                                         g_direct_equal,
                                                         NULL,
                                                         (GDestroyNotify)
                                                         mx_box_layout_free_allocation);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_box_layout_style_changed), NULL);

  self->priv->scroll_to_focused = TRUE;
}

/**
 * mx_box_layout_new:
 *
 * Create a new #MxBoxLayout.
 *
 * Returns: a newly allocated #MxBoxLayout
 */
ClutterActor *
mx_box_layout_new (void)
{
  return g_object_new (MX_TYPE_BOX_LAYOUT, NULL);
}

/**
 * mx_box_layout_set_orientation:
 * @box: A #MxBoxLayout
 * @orientation: orientation value for the layout
 *
 * Set the orientation of the box layout.
 *
 */
void
mx_box_layout_set_orientation (MxBoxLayout *box,
                               MxOrientation orientation)
{
  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  if (box->priv->orientation != orientation)
    {
      box->priv->orientation = orientation;
      _mx_box_layout_start_animation (box);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (box));

      g_object_notify (G_OBJECT (box), "orientation");
    }
}

/**
 * mx_box_layout_get_orientation:
 * @box: A #MxBoxLayout
 *
 * Get the value of the #MxBoxLayout:orientation property.
 *
 * Returns: the orientation of the layout
 */
MxOrientation
mx_box_layout_get_orientation (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->orientation;
}

/**
 * mx_box_layout_set_spacing:
 * @box: A #MxBoxLayout
 * @spacing: the spacing value
 *
 * Set the amount of spacing between children in pixels
 *
 */
void
mx_box_layout_set_spacing (MxBoxLayout *box,
                           guint        spacing)
{
  MxBoxLayoutPrivate *priv;

  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  priv = box->priv;

  if (priv->spacing != spacing)
    {
      priv->spacing = spacing;
      priv->ignore_css_spacing = TRUE;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (box));

      g_object_notify (G_OBJECT (box), "spacing");
    }
}

/**
 * mx_box_layout_get_spacing:
 * @box: A #MxBoxLayout
 *
 * Get the spacing between children in pixels
 *
 * Returns: the spacing value
 */
guint
mx_box_layout_get_spacing (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), 0);

  return box->priv->spacing;
}

/**
 * mx_box_layout_set_enable_animations:
 * @box: A #MxBoxLayout
 * @enable_animations: #TRUE to enable animations
 *
 * Enable animations when certain properties change.
 *
 */
void
mx_box_layout_set_enable_animations (MxBoxLayout *box,
                                     gboolean     enable_animations)
{
  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  if (box->priv->enable_animations != enable_animations)
    {
      box->priv->enable_animations = enable_animations;
      clutter_actor_queue_relayout ((ClutterActor*) box);

      g_object_notify (G_OBJECT (box), "enable-animations");
    }
}

/**
 * mx_box_layout_get_enable_animations:
 * @box: A #MxBoxLayout
 *
 * Get the value of the #MxBoxLayout:enable-animations property.
 *
 * Returns: #TRUE if animations enabled
 */
gboolean
mx_box_layout_get_enable_animations (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->enable_animations;
}

static inline void
mx_box_layout_set_property_valist (MxBoxLayout  *box,
                                   ClutterActor *actor,
                                   const gchar  *first_property,
                                   va_list       var_args)
{
  ClutterContainer *container = CLUTTER_CONTAINER (box);
  ClutterChildMeta *meta;
  GObjectClass *klass;
  const gchar *pname;

  meta = clutter_container_get_child_meta (container, actor);
  g_assert (meta);

  klass = G_OBJECT_GET_CLASS (meta);

  pname = first_property;
  while (pname)
    {
      GValue value = { 0, };
      GParamSpec *pspec;
      gchar *error;

      pspec = g_object_class_find_property (klass, pname);
      if (pspec == NULL)
        {
          g_warning ("%s: the layout property '%s' for MxBoxLayout "
                     "(meta type '%s') does not exist",
                     G_STRLOC,
                     pname,
                     G_OBJECT_TYPE_NAME (meta));
          break;
        }

      if (!(pspec->flags & G_PARAM_WRITABLE))
        {
          g_warning ("%s: the layout property '%s' for MxBoxLayout "
                     "(meta type '%s') is not writable",
                     G_STRLOC,
                     pspec->name,
                     G_OBJECT_TYPE_NAME (meta));
          break;
        }

      g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
      G_VALUE_COLLECT (&value, var_args, 0, &error);
      if (error)
        {
          g_warning ("%s: %s", G_STRLOC, error);
          g_free (error);
          break;
        }

      clutter_container_child_set_property (container,
                                            actor,
                                            pspec->name, &value);
      g_value_unset (&value);

      pname = va_arg (var_args, gchar*);
    }
}

static void
mx_box_layout_create_child_meta (MxBoxLayout  *box,
                                 ClutterActor *actor)
{
  ClutterContainer *container = CLUTTER_CONTAINER (box);
  ClutterContainerIface *iface = CLUTTER_CONTAINER_GET_IFACE (container);

  g_assert (g_type_is_a (iface->child_meta_type, MX_TYPE_BOX_LAYOUT_CHILD));

  if (G_LIKELY (iface->create_child_meta))
      iface->create_child_meta (container, actor);
}

/**
 * mx_box_layout_add_actor:
 * @box: a #MxBoxLayout
 * @actor: the #ClutterActor actor to add to the box layout
 * @position: the position where to insert the actor
 *
 * Inserts @actor at @position in @box.
 */
void
mx_box_layout_add_actor (MxBoxLayout  *box,
                         ClutterActor *actor,
                         gint          position)
{
  MxBoxLayoutPrivate *priv;

  g_return_if_fail (MX_IS_BOX_LAYOUT (box));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));

  priv = box->priv;

  /* this is really mx_box_container_add_actor() with a different insert() */
  priv->children = g_list_insert (priv->children,
                                  actor,
                                  position);
  mx_box_layout_create_child_meta (box, actor);
  clutter_actor_set_parent (actor, (ClutterActor*) box);

  if (priv->enable_animations)
    {
      _mx_box_layout_start_animation (box);

      if (priv->timeline)
        {
          /* fade in the new actor when there is room */
          clutter_actor_set_opacity (actor, 0);
          g_signal_connect_swapped (priv->timeline, "completed",
                                    G_CALLBACK (fade_in_actor), actor);
        }
    }
  else
    {
      clutter_actor_queue_relayout ((ClutterActor *) box);
    }

  g_signal_emit_by_name (box, "actor-added", actor);

}

/**
 * mx_box_layout_add_actor_with_properties:
 * @box: a #MxBoxLayout
 * @actor: the #ClutterActor actor to add to the box layout
 * @position: the position where to insert the actor
 * @first_property: name of the first property to set
 * @...: value for the first property, followed optionally by more name/value
 *       pairs terminated with NULL.
 *
 * Inserts @actor at @position in the layout @box. You can set some layout
 * properties on the child at the same time.
 *
 * If @position is negative, or is larger than the number of actors in the
 * layout, the new actor is added on to the end of the list.
 */
void
mx_box_layout_add_actor_with_properties (MxBoxLayout  *box,
                                         ClutterActor *actor,
                                         gint          position,
                                         const char   *first_property,
                                         ...)
{
  va_list var_args;

  mx_box_layout_add_actor (box, actor, position);

  if (first_property == NULL || *first_property == '\0')
    return;

  va_start (var_args, first_property);
  mx_box_layout_set_property_valist (box, actor, first_property, var_args);
  va_end (var_args);
}

/**
 * mx_box_layout_set_scroll_to_focused:
 * @box: A #MxBoxLayout
 * @scroll_to_focused: #TRUE to enable automatically scrolling to the
 *   focused actor
 *
 * Enables or disables automatic scrolling to the focused actor.
 *
 * Since: 1.2
 */
void
mx_box_layout_set_scroll_to_focused (MxBoxLayout *box,
                                     gboolean     scroll_to_focused)
{
  MxBoxLayoutPrivate *priv;

  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  priv = box->priv;
  if (priv->scroll_to_focused != scroll_to_focused)
    {
      priv->scroll_to_focused = scroll_to_focused;
      g_object_notify (G_OBJECT (box), "scroll-to-focused");
    }
}

/**
 * mx_box_layout_get_scroll_to_focused:
 * @box: A #MxBoxLayout
 *
 * Get the value of the #MxBoxLayout:scroll-to-focused property.
 *
 * Returns: #TRUE if automatically scrolling to the focused actor is enabled
 *
 * Since: 1.2
 */
gboolean
mx_box_layout_get_scroll_to_focused (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), FALSE);
  return box->priv->scroll_to_focused;
}
