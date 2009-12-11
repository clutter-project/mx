/*
 * mx-box-layout.h: box layout actor
 *
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
 *   #MxBoxLayout:vertical property set to
 *   #FALSE.</para>
 *   <graphic fileref="MxBoxLayout-horizontal.png" format="PNG"/>
 * </figure>
 *
 * <figure id="mx-box-layout-vertical">
 *   <title>Box layout with vertical flow</title>
 *   <para>The image shows an #MxBoxLayout with the
 *   #MxBoxLayout:vertical property set to
 *   #TRUE.</para>
 *   <graphic fileref="MxBoxLayout-vertical.png" format="PNG"/>
 * </figure>
 */

#include "mx-box-layout.h"

#include "mx-private.h"
#include "mx-scrollable.h"
#include "mx-box-layout-child.h"



static void mx_box_container_iface_init (ClutterContainerIface *iface);
static void mx_box_scrollable_interface_init (MxScrollableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (MxBoxLayout, mx_box_layout, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                mx_box_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                mx_box_scrollable_interface_init));

#define BOX_LAYOUT_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_BOX_LAYOUT, MxBoxLayoutPrivate))

enum {
  PROP_0,

  PROP_VERTICAL,
  PROP_PACK_START,
  PROP_SPACING,

  PROP_HADJUST,
  PROP_VADJUST
};

struct _MxBoxLayoutPrivate
{
  GList        *children;

  guint         spacing;

  guint         is_vertical : 1;
  guint         is_pack_start : 1;

  MxAdjustment *hadjustment;
  MxAdjustment *vadjustment;

  GHashTable      *start_allocations;
  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
  guint            is_animating : 1;
  guint            animation_enabled : 1;
};

void _mx_box_layout_finish_animation (MxBoxLayout *box);

void
_mx_box_layout_start_animation (MxBoxLayout *box)
{
  MxBoxLayoutPrivate *priv = box->priv;

  if (priv->is_animating)
      return;

  if (!CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (box)))
    return;

  priv->is_animating = TRUE;

  priv->timeline = clutter_timeline_new (500);
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

          adjustment = mx_adjustment_new (0.0,
                                          0.0,
                                          1.0,
                                          1.0,
                                          1.0,
                                          1.0);

          scrollable_set_adjustments (scrollable,
                                      adjustment,
                                      priv->vadjustment);

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

          adjustment = mx_adjustment_new (0.0,
                                          0.0,
                                          1.0,
                                          1.0,
                                          1.0,
                                          1.0);

          scrollable_set_adjustments (scrollable,
                                      priv->hadjustment,
                                      adjustment);

          *vadjustment = adjustment;
        }
    }
}



static void
mx_box_scrollable_interface_init (MxScrollableInterface *iface)
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

  _mx_box_layout_start_animation (MX_BOX_LAYOUT (container));

  if (priv->timeline)
    {
      /* fade in the new actor when there is room */
      clutter_actor_set_opacity (actor, 0);
      g_signal_connect_swapped (priv->timeline, "completed",
                                G_CALLBACK (fade_in_actor), actor);
    }

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
real_remove_actor (ClutterActor    *actor)
{
  ClutterContainer *container;
  MxBoxLayoutPrivate *priv;
  GList *item = NULL;

  container = CLUTTER_CONTAINER (clutter_actor_get_parent (actor));


  if (!container)
    return;

  priv = MX_BOX_LAYOUT (container)->priv;


  item = g_list_find (priv->children, actor);

  priv->children = g_list_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  _mx_box_layout_start_animation (MX_BOX_LAYOUT (container));

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);
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

#if 0 /* disable animation, since clutter destroys the child meta */
  if (CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (container)))
    {
      clutter_actor_animate (actor, CLUTTER_LINEAR, 300,
                             "opacity", 0x00,
                             "signal-swapped::completed",
                             real_remove_actor, actor,
                             NULL);
    }
  else
    {
#endif
      real_remove_actor (actor);
#if 0
    }
#endif

}

static void
mx_box_container_foreach (ClutterContainer *container,
                          ClutterCallback   callback,
                          gpointer          callback_data)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (container)->priv;

  g_list_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
mx_box_container_lower (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
mx_box_container_raise (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
mx_box_container_sort_depth_order (ClutterContainer *container)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
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
    case PROP_VERTICAL:
      g_value_set_boolean (value, priv->is_vertical);
      break;

    case PROP_PACK_START:
      g_value_set_boolean (value, priv->is_pack_start);
      break;

    case PROP_SPACING:
      g_value_set_uint (value, priv->spacing);
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
    case PROP_VERTICAL:
      mx_box_layout_set_vertical (box, g_value_get_boolean (value));
      break;

    case PROP_PACK_START:
      mx_box_layout_set_pack_start (box, g_value_get_boolean (value));
      break;

    case PROP_SPACING:
      mx_box_layout_set_spacing (box, g_value_get_uint (value));
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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_box_layout_dispose (GObject *object)
{
  MxBoxLayoutPrivate *priv = MX_BOX_LAYOUT (object)->priv;

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

  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;

      if (!CLUTTER_ACTOR_IS_VISIBLE ((ClutterActor*) l->data))
        continue;

      n_children++;

      clutter_actor_get_preferred_width ((ClutterActor*) l->data,
                                         (!priv->is_vertical) ? for_height : -1,
                                         &child_min,
                                         &child_nat);

      if (priv->is_vertical)
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


  if (!priv->is_vertical && n_children > 1)
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


  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;

      if (!CLUTTER_ACTOR_IS_VISIBLE ((ClutterActor*) l->data))
        continue;

      n_children++;

      clutter_actor_get_preferred_height ((ClutterActor*) l->data,
                                          (priv->is_vertical) ? for_width : -1,
                                          &child_min,
                                          &child_nat);

      if (!priv->is_vertical)
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

  if (priv->is_vertical && n_children > 1)
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
  gfloat position = 0;
  GList *l;
  gint n_expand_children, extra_space;

  CLUTTER_ACTOR_CLASS (mx_box_layout_parent_class)->allocate (actor, box,
                                                              flags);

  if (priv->children == NULL)
    return;


  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  avail_width  = box->x2 - box->x1
                 - padding.left
                 - padding.right;
  avail_height = box->y2 - box->y1
                 - padding.top
                 - padding.bottom;

  if (priv->is_vertical)
    {
      mx_box_layout_get_preferred_height (actor, avail_width, NULL,
                                          &pref_height);
      pref_width = avail_width;
    }
  else
    {
      mx_box_layout_get_preferred_width (actor, avail_height, NULL,
                                         &pref_width);
      pref_height = avail_height;
    }

  /* update adjustments for scrolling */
  if (priv->vadjustment)
    {
      gdouble prev_value;

      g_object_set (G_OBJECT (priv->vadjustment),
                    "lower", 0.0,
                    "upper", pref_height,
                    "page-size", avail_height,
                    "step-increment", avail_height / 6,
                    "page-increment", avail_height,
                    NULL);

      prev_value = mx_adjustment_get_value (priv->vadjustment);
      mx_adjustment_set_value (priv->vadjustment, prev_value);
    }

  if (priv->hadjustment)
    {
      gdouble prev_value;

      g_object_set (G_OBJECT (priv->hadjustment),
                    "lower", 0.0,
                    "upper", pref_width,
                    "page-size", avail_width,
                    "step-increment", avail_width / 6,
                    "page-increment", avail_width,
                    NULL);

      prev_value = mx_adjustment_get_value (priv->hadjustment);
      mx_adjustment_set_value (priv->hadjustment, prev_value);
    }

  /* count the number of children with expand set to TRUE */
  n_expand_children = 0;
  for (l = priv->children; l; l = l->next)
    {
      gboolean expand;
      clutter_container_child_get ((ClutterContainer *) actor,
                                   (ClutterActor*) l->data,
                                   "expand", &expand,
                                   NULL);
      if (expand)
        n_expand_children++;
    }

  if (n_expand_children == 0)
    {
      extra_space = 0;
      n_expand_children = 1;
    }
  else
    {
      if (priv->is_vertical)
        extra_space = (avail_height - pref_height) / n_expand_children;
      else
        extra_space = (avail_width - pref_width) / n_expand_children;

      /* don't shrink anything */
      if (extra_space < 0)
        extra_space = 0;
    }

  if (priv->is_vertical)
    position = padding.top;
  else
    position = padding.left;

  if (priv->is_pack_start)
    l = g_list_last (priv->children);
  else
    l = priv->children;

  for (l = (priv->is_pack_start) ? g_list_last (priv->children) : priv->children;
       l;
       l = (priv->is_pack_start) ? l->prev : l->next)
    {
      ClutterActor *child = (ClutterActor*) l->data;
      ClutterActorBox child_box;
      gfloat child_nat;
      gboolean xfill, yfill, expand;
      MxAlign xalign, yalign;

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      clutter_container_child_get ((ClutterContainer*) actor, child,
                                   "x-fill", &xfill,
                                   "y-fill", &yfill,
                                   "x-align", &xalign,
                                   "y-align", &yalign,
                                   "expand", &expand,
                                   NULL);

      if (priv->is_vertical)
        {
          clutter_actor_get_preferred_height (child, avail_width,
                                              NULL, &child_nat);

          child_box.y1 = position;
          if (expand)
            child_box.y2 = position + child_nat + extra_space;
          else
            child_box.y2 = position + child_nat;
          child_box.x1 = padding.left;
          child_box.x2 = avail_width + padding.left;
        }
      else
        {

          clutter_actor_get_preferred_width (child, avail_height,
                                             NULL, &child_nat);

          child_box.x1 = position;

          if (expand)
            child_box.x2 = position + child_nat + extra_space;
          else
            child_box.x2 = position + child_nat;

          child_box.y1 = padding.top;
          child_box.y2 = avail_height + padding.top;
        }

      _mx_allocate_fill (child, &child_box, xalign, yalign, xfill, yfill);

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
              continue;
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
          copy = g_boxed_copy (CLUTTER_TYPE_ACTOR_BOX, &child_box);
          g_hash_table_insert (priv->start_allocations, child, copy);


          clutter_actor_allocate (child, copy, flags);
        }

      if (expand)
        position += (child_nat + priv->spacing + extra_space);
      else
        position += (child_nat + priv->spacing);

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

  actor_class->paint = mx_box_layout_paint;
  actor_class->pick = mx_box_layout_pick;

  pspec = g_param_spec_boolean ("vertical",
                                "Vertical",
                                "Whether the layout should be vertical, rather "
                                "than horizontal",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_VERTICAL, pspec);

  pspec = g_param_spec_boolean ("pack-start",
                                "Pack Start",
                                "Whether to pack items at the start of the box",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PACK_START, pspec);

  pspec = g_param_spec_uint ("spacing",
                             "Spacing",
                             "Spacing between children",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SPACING, pspec);

  /* MxScrollable properties */
  g_object_class_override_property (object_class,
                                    PROP_HADJUST,
                                    "hadjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUST,
                                    "vadjustment");

}

static void
mx_box_layout_free_allocation (ClutterActorBox *box)
{
  g_boxed_free (CLUTTER_TYPE_ACTOR_BOX, box);
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
 * mx_box_layout_set_vertical:
 * @box: A #MxBoxLayout
 * @vertical: #TRUE if the layout should be vertical
 *
 * Set the value of the #MxBoxLayout::vertical property
 *
 */
void
mx_box_layout_set_vertical (MxBoxLayout *box,
                            gboolean     vertical)
{
  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  if (box->priv->is_vertical != vertical)
    {
      box->priv->is_vertical = vertical;
      _mx_box_layout_start_animation (box);
      clutter_actor_queue_relayout ((ClutterActor*) box);

      g_object_notify (G_OBJECT (box), "vertical");
    }
}

/**
 * mx_box_layout_get_vertical:
 * @box: A #MxBoxLayout
 *
 * Get the value of the #MxBoxLayout:vertical property.
 *
 * Returns: #TRUE if the layout is vertical
 */
gboolean
mx_box_layout_get_vertical (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->is_vertical;
}

/**
 * mx_box_layout_set_pack_start:
 * @box: A #MxBoxLayout
 * @pack_start: #TRUE if the layout should use pack-start
 *
 * Set the value of the #MxBoxLayout:pack-start property.
 *
 */
void
mx_box_layout_set_pack_start (MxBoxLayout *box,
                              gboolean     pack_start)
{
  g_return_if_fail (MX_IS_BOX_LAYOUT (box));

  if (box->priv->is_pack_start != pack_start)
    {
      box->priv->is_pack_start = pack_start;
      clutter_actor_queue_relayout ((ClutterActor*) box);

      g_object_notify (G_OBJECT (box), "pack-start");
    }
}

/**
 * mx_box_layout_get_pack_start:
 * @box: A #MxBoxLayout
 *
 * Get the value of the #MxBoxLayout:pack-start property.
 *
 * Returns: #TRUE if pack-start is enabled
 */
gboolean
mx_box_layout_get_pack_start (MxBoxLayout *box)
{
  g_return_val_if_fail (MX_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->is_pack_start;
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
