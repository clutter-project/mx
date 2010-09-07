/*
 * mx-stack.c: A container that allows the stacking of multiple widgets
 *
 * Copyright 2010 Intel Corporation.
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:mx-stack
 * @short_description: A container allow stacking of children over each other
 *
 * The #MxStack arranges its children in a stack, where each child can be
 * allocated its preferred size or larger, if the fill option is set. If
 * the fill option isn't set, a child's position will be determined by its
 * alignment properties.
 */

#include "mx-stack.h"
#include "mx-stack-child.h"
#include "mx-focusable.h"
#include "mx-utils.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxStack, mx_stack, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init))

#define STACK_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_STACK, MxStackPrivate))

struct _MxStackPrivate
{
  GList        *children;
  ClutterActor *current_focus;
};

/* ClutterContainerIface */

static void
mx_stack_add_actor (ClutterContainer *container,
                    ClutterActor     *actor)
{
  MxStackPrivate *priv = MX_STACK (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
  priv->children = g_list_append (priv->children, actor);

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
mx_stack_remove_actor (ClutterContainer *container,
                       ClutterActor     *actor)
{
  GList *actor_link;

  MxStackPrivate *priv = MX_STACK (container)->priv;

  actor_link = g_list_find (priv->children, actor);

  if (!actor_link)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  g_object_ref (actor);

  if (priv->current_focus == actor)
    priv->current_focus = NULL;

  priv->children = g_list_delete_link (priv->children, actor_link);
  clutter_actor_unparent (actor);

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);
}

static void
mx_stack_foreach (ClutterContainer *container,
                  ClutterCallback   callback,
                  gpointer          callback_data)
{
  MxStackPrivate *priv = MX_STACK (container)->priv;
  g_list_foreach (priv->children, (GFunc)callback, callback_data);
}

static void
mx_stack_lower (ClutterContainer *container,
                ClutterActor     *actor,
                ClutterActor     *sibling)
{
  gint i;
  GList *c, *position, *actor_link = NULL;

  MxStackPrivate *priv = MX_STACK (container)->priv;

  if (priv->children && (priv->children->data == actor))
    return;

  position = priv->children;
  for (c = priv->children, i = 0; c; c = c->next, i++)
    {
      if (c->data == actor)
        actor_link = c;
      if (c->data == sibling)
        position = c;
    }

  if (!actor_link)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  priv->children = g_list_delete_link (priv->children, actor_link);
  priv->children = g_list_insert_before (priv->children, position, actor);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
mx_stack_raise (ClutterContainer *container,
                ClutterActor     *actor,
                ClutterActor     *sibling)
{
  gint i;
  GList *c, *actor_link = NULL;

  gint position = -1;
  MxStackPrivate *priv = MX_STACK (container)->priv;

  for (c = priv->children, i = 0; c; c = c->next, i++)
    {
      if (c->data == actor)
        actor_link = c;
      if (c->data == sibling)
        position = i;
    }

  if (!actor_link)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  if (!actor_link->next)
    return;

  priv->children = g_list_delete_link (priv->children, actor_link);
  priv->children = g_list_insert (priv->children, actor, position);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static gint
mx_stack_depth_sort_cb (gconstpointer a,
                        gconstpointer b)
{
  gfloat depth_a = clutter_actor_get_depth ((ClutterActor *)a);
  gfloat depth_b = clutter_actor_get_depth ((ClutterActor *)a);

  if (depth_a < depth_b)
    return -1;
  else if (depth_a > depth_b)
    return 1;
  else
    return 0;
}

static void
mx_stack_sort_depth_order (ClutterContainer *container)
{
  MxStackPrivate *priv = MX_STACK (container)->priv;

  priv->children = g_list_sort (priv->children, mx_stack_depth_sort_cb);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_stack_add_actor;
  iface->remove = mx_stack_remove_actor;
  iface->foreach = mx_stack_foreach;
  iface->lower = mx_stack_lower;
  iface->raise = mx_stack_raise;
  iface->sort_depth_order = mx_stack_sort_depth_order;

  iface->child_meta_type = MX_TYPE_STACK_CHILD;
}

/* MxFocusableIface */

static MxFocusable *
mx_stack_move_focus (MxFocusable      *focusable,
                    MxFocusDirection  direction,
                    MxFocusable      *from)
{
  GList *c;

  MxStackPrivate *priv = MX_STACK (focusable)->priv;

  focusable = NULL;

  c = g_list_find (priv->children, from);
  while (c && !focusable)
    {
      MxFocusHint hint;
      ClutterActor *child;

      switch (direction)
        {
        case MX_FOCUS_DIRECTION_PREVIOUS :
        case MX_FOCUS_DIRECTION_LEFT :
        case MX_FOCUS_DIRECTION_UP :
          c = c->prev;
          hint = MX_FOCUS_HINT_LAST;
          break;
        default:
          c = c->next;
          hint = MX_FOCUS_HINT_FIRST;
          break;
        }

      if (!c)
        continue;

      child = c->data;
      if (!MX_IS_FOCUSABLE (child))
        continue;

      focusable = mx_focusable_accept_focus (MX_FOCUSABLE (child), hint);
      if (focusable)
        priv->current_focus = child;
    }

  return focusable;
}

static MxFocusable *
mx_stack_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  GList *c, *children;
  MxStackPrivate *priv = MX_STACK (focusable)->priv;
  ClutterContainer *container = CLUTTER_CONTAINER (focusable);

  focusable = NULL;

  switch (hint)
    {
    case MX_FOCUS_HINT_PRIOR:
      if (priv->current_focus &&
          (!MX_IS_WIDGET (priv->current_focus) ||
           !mx_widget_get_disabled ((MxWidget *)priv->current_focus)))
        {
          focusable =
            mx_focusable_accept_focus (MX_FOCUSABLE (priv->current_focus),
                                       hint);
          if (focusable)
            break;
        }
      /* This purposefully runs into the next case statement */

    case MX_FOCUS_HINT_FIRST:
    case MX_FOCUS_HINT_LAST:
      children = clutter_container_get_children (container);
      if (hint == MX_FOCUS_HINT_LAST)
        children = g_list_reverse (children);

      if (children)
        {
          c = children;
          while (c && !focusable)
            {
              ClutterActor *child = c->data;
              c = c->next;

              if (!MX_IS_FOCUSABLE (child))
                continue;

              if (MX_IS_WIDGET (child) &&
                  mx_widget_get_disabled ((MxWidget *)child))
                continue;

              priv->current_focus = child;
              focusable = mx_focusable_accept_focus (MX_FOCUSABLE (child),
                                                     hint);
            }
          g_list_free (children);
        }
      break;
    }

  return focusable;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->move_focus = mx_stack_move_focus;
  iface->accept_focus = mx_stack_accept_focus;
}

/* Actor implementation */

static void
mx_stack_dispose (GObject *object)
{
  MxStackPrivate *priv = MX_STACK (object)->priv;

  while (priv->children)
    clutter_actor_destroy (CLUTTER_ACTOR (priv->children->data));

  G_OBJECT_CLASS (mx_stack_parent_class)->dispose (object);
}

static void
mx_stack_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *nat_width_p)
{
  GList *c;
  MxPadding padding;
  gfloat min_width, nat_width, child_min_width, child_nat_width;

  MxStackPrivate *priv = MX_STACK (actor)->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  if (for_height >= 0)
    for_height = MAX (0, for_height - padding.top - padding.bottom);

  min_width = nat_width = 0;
  for (c = priv->children; c; c = c->next)
    {
      ClutterActor *child = c->data;
      clutter_actor_get_preferred_width (child, for_height,
                                         &child_min_width, &child_nat_width);
      if (child_min_width > min_width)
        min_width = child_min_width;
      if (child_nat_width > nat_width)
        nat_width = child_nat_width;
    }

  min_width += padding.left + padding.right;
  nat_width += padding.left + padding.right;

  if (min_width_p)
    *min_width_p = min_width;
  if (nat_width_p)
    *nat_width_p = nat_width;
}

static void
mx_stack_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *nat_height_p)
{
  GList *c;
  MxPadding padding;
  gfloat min_height, nat_height, child_min_height, child_nat_height;

  MxStackPrivate *priv = MX_STACK (actor)->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  if (for_width >= 0)
    for_width = MAX (0, for_width - padding.left - padding.right);

  min_height = nat_height = 0;
  for (c = priv->children; c; c = c->next)
    {
      ClutterActor *child = c->data;
      clutter_actor_get_preferred_height (child, for_width,
                                          &child_min_height, &child_nat_height);
      if (child_min_height > min_height)
        min_height = child_min_height;
      if (child_nat_height > nat_height)
        nat_height = child_nat_height;
    }

  min_height += padding.top + padding.bottom;
  nat_height += padding.top + padding.bottom;

  if (min_height_p)
    *min_height_p = min_height;
  if (nat_height_p)
    *nat_height_p = nat_height;
}

static void
mx_stack_allocate (ClutterActor           *actor,
                   const ClutterActorBox  *box,
                   ClutterAllocationFlags  flags)
{
  GList *c;
  ClutterActorBox avail_space;

  MxStackPrivate *priv = MX_STACK (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_stack_parent_class)->allocate (actor, box, flags);

  mx_widget_get_available_area (MX_WIDGET (actor), box, &avail_space);

  for (c = priv->children; c; c = c->next)
    {
      gboolean x_fill, y_fill;
      MxAlign x_align, y_align;

      ClutterActor *child = c->data;
      ClutterActorBox child_box = avail_space;

      clutter_container_child_get (CLUTTER_CONTAINER (actor),
                                   child,
                                   "x-fill", &x_fill,
                                   "y-fill", &y_fill,
                                   "x-align", &x_align,
                                   "y-align", &y_align,
                                   NULL);

      mx_allocate_align_fill (child,
                              &child_box,
                              x_align,
                              y_align,
                              x_fill,
                              y_fill);

      clutter_actor_allocate (child, &child_box, flags);
    }
}

static void
mx_stack_paint (ClutterActor *actor)
{
  GList *c;

  MxStackPrivate *priv = MX_STACK (actor)->priv;

  for (c = priv->children; c; c = c->next)
    clutter_actor_paint (c->data);
}

static void
mx_stack_pick (ClutterActor       *actor,
               const ClutterColor *color)
{
  mx_stack_paint (actor);
}

static void
mx_stack_class_init (MxStackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxStackPrivate));

  object_class->dispose = mx_stack_dispose;

  actor_class->get_preferred_width = mx_stack_get_preferred_width;
  actor_class->get_preferred_height = mx_stack_get_preferred_height;
  actor_class->allocate = mx_stack_allocate;
  actor_class->paint = mx_stack_paint;
  actor_class->pick = mx_stack_pick;
}

static void
mx_stack_init (MxStack *self)
{
  self->priv = STACK_PRIVATE (self);
}

/**
 * mx_stack_new:
 *
 * Create a new #MxStack.
 *
 * Returns: a newly allocated #MxStack
 */
ClutterActor *
mx_stack_new ()
{
  return g_object_new (MX_TYPE_STACK, NULL);
}
