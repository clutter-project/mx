/*
 * nbtk-box-layout.h: box layout actor
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


#include "nbtk-box-layout.h"

#include "nbtk-private.h"
#include "nbtk-scrollable.h"




static void nbtk_box_container_iface_init (ClutterContainerIface *iface);
static void nbtk_box_scrollable_interface_init (NbtkScrollableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkBoxLayout, nbtk_box_layout, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                nbtk_box_container_iface_init)
                         G_IMPLEMENT_INTERFACE (NBTK_TYPE_SCROLLABLE,
                                                nbtk_box_scrollable_interface_init));

#define BOX_LAYOUT_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_BOX_LAYOUT, NbtkBoxLayoutPrivate))

enum {
  PROP_0,

  PROP_VERTICAL,
  PROP_PACK_START,

  PROP_HADJUST,
  PROP_VADJUST
};

struct _NbtkBoxLayoutPrivate
{
  GList *children;

  guint is_vertical : 1;
  guint is_pack_start : 1;

  NbtkAdjustment *hadjustment;
  NbtkAdjustment *vadjustment;
};

/*
 * NbtkScrollable Interface Implementation
 */
static void
adjustment_value_notify_cb (NbtkAdjustment *adjustment,
                            GParamSpec     *pspec,
                            NbtkBoxLayout  *box)
{
  clutter_actor_queue_redraw (CLUTTER_ACTOR (box));
}

static void
scrollable_set_adjustments (NbtkScrollable *scrollable,
                            NbtkAdjustment *hadjustment,
                            NbtkAdjustment *vadjustment)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (scrollable)->priv;

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
scrollable_get_adjustments (NbtkScrollable *scrollable,
                            NbtkAdjustment **hadjustment,
                            NbtkAdjustment **vadjustment)
{
  NbtkBoxLayoutPrivate *priv;
  ClutterActor *actor, *stage;

  priv = (NBTK_BOX_LAYOUT (scrollable))->priv;

  actor = CLUTTER_ACTOR (scrollable);
  stage = clutter_actor_get_stage (actor);
  if (G_UNLIKELY (stage == NULL))
    stage = clutter_stage_get_default ();

  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          NbtkAdjustment *adjustment;
          gdouble width, stage_width, increment;

          width = clutter_actor_get_width (actor);
          stage_width = clutter_actor_get_width (stage);
          increment = MAX (1.0, MIN (stage_width, width));

          adjustment = nbtk_adjustment_new (0,
                                            0,
                                            width,
                                            1.0,
                                            increment,
                                            increment);

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
          NbtkAdjustment *adjustment;
          gdouble height, stage_height, increment;

          height = clutter_actor_get_height (actor);
          stage_height = clutter_actor_get_height (stage);
          increment = MAX (1.0, MIN (stage_height, height));

          adjustment = nbtk_adjustment_new (0,
                                            0,
                                            height,
                                            1.0,
                                            increment,
                                            increment);

          scrollable_set_adjustments (scrollable,
                                      priv->hadjustment,
                                      adjustment);

          *vadjustment = adjustment;
        }
    }
}



static void
nbtk_box_scrollable_interface_init (NbtkScrollableInterface *iface)
{
  iface->set_adjustments = scrollable_set_adjustments;
  iface->get_adjustments = scrollable_get_adjustments;
}

/*
 * ClutterContainer Implementation
 */
static void
nbtk_box_container_add_actor (ClutterContainer *container,
                              ClutterActor     *actor)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  priv->children = g_list_append (priv->children, actor);

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
nbtk_box_container_remove_actor (ClutterContainer *container,
                                 ClutterActor     *actor)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (container)->priv;

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

  priv->children = g_list_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);

  clutter_actor_queue_relayout ((ClutterActor*) container);
}

static void
nbtk_box_container_foreach (ClutterContainer *container,
                            ClutterCallback   callback,
                            gpointer          callback_data)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (container)->priv;

  g_list_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
nbtk_box_container_lower (ClutterContainer *container,
                          ClutterActor     *actor,
                          ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_box_container_raise (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_box_container_sort_depth_order (ClutterContainer *container)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_box_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_box_container_add_actor;
  iface->remove = nbtk_box_container_remove_actor;
  iface->foreach = nbtk_box_container_foreach;
  iface->lower = nbtk_box_container_lower;
  iface->raise = nbtk_box_container_raise;
  iface->sort_depth_order = nbtk_box_container_sort_depth_order;
}


static void
nbtk_box_layout_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (object)->priv;
  NbtkAdjustment *adjustment;

  switch (property_id)
    {
    case PROP_VERTICAL:
      g_value_set_boolean (value, priv->is_vertical);
      break;

    case PROP_PACK_START:
      g_value_set_boolean (value, priv->is_pack_start);
      break;

    case PROP_HADJUST :
      scrollable_get_adjustments (NBTK_SCROLLABLE (object), &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST :
      scrollable_get_adjustments (NBTK_SCROLLABLE (object), NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_box_layout_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  NbtkBoxLayout *box = NBTK_BOX_LAYOUT (object);

  switch (property_id)
    {
    case PROP_VERTICAL:
      nbtk_box_layout_set_vertical (box, g_value_get_boolean (value));
      break;

    case PROP_PACK_START:
      nbtk_box_layout_set_pack_start (box, g_value_get_boolean (value));
      break;

    case PROP_HADJUST :
      scrollable_set_adjustments (NBTK_SCROLLABLE (object),
                                  g_value_get_object (value),
                                  box->priv->vadjustment);
      break;

    case PROP_VADJUST :
      scrollable_set_adjustments (NBTK_SCROLLABLE (object),
                                  box->priv->hadjustment,
                                  g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_box_layout_dispose (GObject *object)
{
  G_OBJECT_CLASS (nbtk_box_layout_parent_class)->dispose (object);
}

static void
nbtk_box_layout_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_box_layout_parent_class)->finalize (object);
}


static void
nbtk_box_layout_get_preferred_width (ClutterActor *actor,
                                     gfloat        for_height,
                                     gfloat       *min_width_p,
                                     gfloat       *natural_width_p)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (actor)->priv;
  GList *l;

  if (min_width_p)
    *min_width_p = 0;
  if (natural_width_p)
    *natural_width_p = 0;

  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;

      clutter_actor_get_preferred_width ((ClutterActor*) l->data,
                                         -1,
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

}

static void
nbtk_box_layout_get_preferred_height (ClutterActor *actor,
                                      gfloat        for_width,
                                      gfloat       *min_height_p,
                                      gfloat       *natural_height_p)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (actor)->priv;
  GList *l;

  if (min_height_p)
    *min_height_p = 0;
  if (natural_height_p)
    *natural_height_p = 0;

  for (l = priv->children; l; l = g_list_next (l))
    {
      gfloat child_min = 0, child_nat = 0;

      clutter_actor_get_preferred_height ((ClutterActor*) l->data,
                                          -1,
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
}

static void
nbtk_box_layout_allocate (ClutterActor          *actor,
                          const ClutterActorBox *box,
                          ClutterAllocationFlags flags)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (actor)->priv;
  GList *l;
  gfloat position = 0;

  CLUTTER_ACTOR_CLASS (nbtk_box_layout_parent_class)->allocate (actor, box,
                                                                flags);


  /* update adjustments for scrolling */
  if (priv->vadjustment)
    {
      gdouble prev_value;
      gfloat height;

      clutter_actor_get_preferred_height (actor, box->x2 - box->x1, NULL,
                                          &height);

      g_object_set (G_OBJECT (priv->vadjustment),
                   "lower", 0.0,
                   "upper", height,
                   "page-size", box->y2 - box->y1,
                   "step-increment", (box->y2 - box->y1) / 6,
                   "page-increment", box->y2 - box->y1,
                   NULL);

      prev_value = nbtk_adjustment_get_value (priv->vadjustment);
      nbtk_adjustment_set_value (priv->vadjustment, prev_value);
    }

  if (priv->hadjustment)
    {
      gdouble prev_value;
      gfloat width;

      /* get preferred width for this height */
      clutter_actor_get_preferred_width (actor, box->y2 - box->y1, NULL,
                                         &width);

      g_object_set (G_OBJECT (priv->hadjustment),
                   "lower", 0.0,
                   "upper", width,
                   "page-size", box->x2 - box->x1,
                   "step-increment", (box->x2 - box->x1) / 6,
                   "page-increment", box->x2 - box->x1,
                   NULL);

      prev_value = nbtk_adjustment_get_value (priv->hadjustment);
      nbtk_adjustment_set_value (priv->hadjustment, prev_value);
    }


  if (priv->is_pack_start)
    l = g_list_last (priv->children);
  else
    l = priv->children;

  while (l)
    {
      ClutterActor *child = (ClutterActor*) l->data;
      gfloat child_nat;
      ClutterActorBox child_box;

      if (priv->is_vertical)
        {
          clutter_actor_get_preferred_height (child, (box->x2 - box->x1),
                                              NULL, &child_nat);

          child_box.y1 = position;
          child_box.y2 = position + child_nat;
          child_box.x1 = 0;
          child_box.x2 = (box->x2 - box->x1);
          clutter_actor_allocate (child, &child_box, flags);
          position += child_nat;
        }
      else
        {

          clutter_actor_get_preferred_width (child, (box->y2 - box->y1),
                                             NULL, &child_nat);

          child_box.x1 = position;
          child_box.x2 = position + child_nat;
          child_box.y1 = 0;
          child_box.y2 = (box->y2 - box->y1);
          clutter_actor_allocate (child, &child_box, flags);
          position += child_nat;
        }

      if (priv->is_pack_start)
        l = g_list_previous (l);
      else
        l = g_list_next (l);
  }
}

static void
nbtk_box_layout_apply_transform (ClutterActor *a, CoglMatrix *m)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (a)->priv;
  gdouble x, y;

  CLUTTER_ACTOR_CLASS (nbtk_box_layout_parent_class)->apply_transform (a, m);

  if (priv->hadjustment)
    x = nbtk_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = nbtk_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  cogl_matrix_translate (m, (int) -x, (int) -y, 0);
}


static void
nbtk_box_layout_paint (ClutterActor *actor)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (actor)->priv;
  GList *l;
  gdouble x, y;
  ClutterActorBox child_b;
  ClutterActorBox box_b;

  if (priv->hadjustment)
    x = nbtk_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = nbtk_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  CLUTTER_ACTOR_CLASS (nbtk_box_layout_parent_class)->paint (actor);

  clutter_actor_get_allocation_box (actor, &box_b);
  box_b.x2 = (box_b.x2 - box_b.x1) + x;
  box_b.x1 = x;
  box_b.y2 = (box_b.y2 - box_b.y1) + y;
  box_b.y1 = y;

  for (l = priv->children; l; l = g_list_next (l))
    {
      ClutterActor *child = (ClutterActor*) l->data;

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
nbtk_box_layout_pick (ClutterActor *actor,
                      const ClutterColor *color)
{
  NbtkBoxLayoutPrivate *priv = NBTK_BOX_LAYOUT (actor)->priv;
  GList *l;
  gdouble x, y;
  ClutterActorBox child_b;
  ClutterActorBox box_b;

  if (priv->hadjustment)
    x = nbtk_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = nbtk_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  CLUTTER_ACTOR_CLASS (nbtk_box_layout_parent_class)->pick (actor, color);

  clutter_actor_get_allocation_box (actor, &box_b);
  box_b.x2 = (box_b.x2 - box_b.x1) + x;
  box_b.x1 = x;
  box_b.y2 = (box_b.y2 - box_b.y1) + y;
  box_b.y1 = y;

  for (l = priv->children; l; l = g_list_next (l))
    {
      ClutterActor *child = (ClutterActor*) l->data;

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
nbtk_box_layout_class_init (NbtkBoxLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkBoxLayoutPrivate));

  object_class->get_property = nbtk_box_layout_get_property;
  object_class->set_property = nbtk_box_layout_set_property;
  object_class->dispose = nbtk_box_layout_dispose;
  object_class->finalize = nbtk_box_layout_finalize;

  actor_class->allocate = nbtk_box_layout_allocate;
  actor_class->get_preferred_width = nbtk_box_layout_get_preferred_width;
  actor_class->get_preferred_height = nbtk_box_layout_get_preferred_height;
  actor_class->apply_transform = nbtk_box_layout_apply_transform;

  actor_class->paint = nbtk_box_layout_paint;
  actor_class->pick = nbtk_box_layout_pick;

  pspec = g_param_spec_boolean ("vertical",
                                "Vertical",
                                "Whether the layout should be vertical, rather"
                                "than horizontal",
                                FALSE,
                                NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_VERTICAL, pspec);

  pspec = g_param_spec_boolean ("pack-start",
                                "Pack Start",
                                "Whether to pack items at the start of the box",
                                FALSE,
                                NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PACK_START, pspec);


  /* NbtkScrollable properties */
  g_object_class_override_property (object_class,
                                    PROP_HADJUST,
                                    "hadjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUST,
                                    "vadjustment");

}

static void
nbtk_box_layout_init (NbtkBoxLayout *self)
{
  self->priv = BOX_LAYOUT_PRIVATE (self);
}

NbtkWidget *
nbtk_box_layout_new (void)
{
  return g_object_new (NBTK_TYPE_BOX_LAYOUT, NULL);
}

void
nbtk_box_layout_set_vertical (NbtkBoxLayout *box,
                              gboolean       vertical)
{
  g_return_if_fail (NBTK_IS_BOX_LAYOUT (box));

  if (box->priv->is_vertical != vertical)
    {
      box->priv->is_vertical = vertical;
      clutter_actor_queue_relayout ((ClutterActor*) box);
    }
}

gboolean
nbtk_box_layout_get_vertical (NbtkBoxLayout *box)
{
  g_return_val_if_fail (NBTK_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->is_vertical;
}

void
nbtk_box_layout_set_pack_start (NbtkBoxLayout *box,
                                gboolean       pack_start)
{
  g_return_if_fail (NBTK_IS_BOX_LAYOUT (box));

  if (box->priv->is_pack_start != pack_start)
    {
      box->priv->is_pack_start = pack_start;
      clutter_actor_queue_relayout ((ClutterActor*) box);
    }
}

gboolean
nbtk_box_layout_get_pack_start (NbtkBoxLayout *box)
{
  g_return_val_if_fail (NBTK_IS_BOX_LAYOUT (box), FALSE);

  return box->priv->is_pack_start;
}
