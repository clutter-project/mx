/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-stack-child.c: stack child actor
 *
 * Copyright 2010 Intel Corporation
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
 *             Chris Lord <chris@linux.intel.com>
 */

/**
 * SECTION:mx-stack-child
 * @short_description: meta data associated with a #MxStack child.
 *
 * #MxStackChild is a #ClutterChildMeta implementation that stores the
 * child properties for children inside a #MxStack.
 *
 * Since: 1.2
 */

#include "mx-stack-child.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxStackChild, mx_stack_child, CLUTTER_TYPE_CHILD_META)

#define STACK_CHILD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_STACK_CHILD, MxStackChildPrivate))


enum
{
  PROP_0,

  PROP_X_FILL,
  PROP_Y_FILL,
  PROP_X_ALIGN,
  PROP_Y_ALIGN,
  PROP_FIT,
  PROP_CROP
};

static void
mx_stack_child_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MxStackChild *child = MX_STACK_CHILD (object);

  switch (property_id)
    {
    case PROP_X_FILL:
      g_value_set_boolean (value, child->x_fill);
      break;
    case PROP_Y_FILL:
      g_value_set_boolean (value, child->y_fill);
      break;
    case PROP_X_ALIGN:
      g_value_set_enum (value, child->x_align);
      break;
    case PROP_Y_ALIGN:
      g_value_set_enum (value, child->y_align);
      break;
    case PROP_FIT:
      g_value_set_boolean (value, child->fit);
      break;
    case PROP_CROP:
      g_value_set_boolean (value, child->crop);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_stack_child_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  MxStackChild *child = MX_STACK_CHILD (object);
  ClutterActor *stack = CLUTTER_ACTOR (CLUTTER_CHILD_META (object)->container);

  switch (property_id)
    {
    case PROP_X_FILL:
      child->x_fill = g_value_get_boolean (value);
      break;
    case PROP_Y_FILL:
      child->y_fill = g_value_get_boolean (value);
      break;
    case PROP_X_ALIGN:
      child->x_align = g_value_get_enum (value);
      break;
    case PROP_Y_ALIGN:
      child->y_align = g_value_get_enum (value);
      break;
    case PROP_FIT:
      child->fit = g_value_get_boolean (value);
      break;
    case PROP_CROP:
      child->crop = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }

  clutter_actor_queue_relayout (stack);
}

static void
mx_stack_child_class_init (MxStackChildClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  object_class->get_property = mx_stack_child_get_property;
  object_class->set_property = mx_stack_child_set_property;

  pspec = g_param_spec_boolean ("x-fill", "x-fill",
                                "Whether the child should receive priority "
                                "when the container is allocating spare space "
                                "on the horizontal axis",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_X_FILL, pspec);

  pspec = g_param_spec_boolean ("y-fill", "y-fill",
                                "Whether the child should receive priority "
                                "when the container is allocating spare space "
                                "on the vertical axis",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_Y_FILL, pspec);

  pspec = g_param_spec_enum ("x-align",
                             "X Alignment",
                             "X alignment of the widget within the cell",
                             MX_TYPE_ALIGN,
                             MX_ALIGN_MIDDLE,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_X_ALIGN, pspec);

  pspec = g_param_spec_enum ("y-align",
                             "Y Alignment",
                             "Y alignment of the widget within the cell",
                             MX_TYPE_ALIGN,
                             MX_ALIGN_MIDDLE,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_Y_ALIGN, pspec);


  pspec = g_param_spec_boolean ("fit", "Fit",
                                "Attempt to fit the actor into the available"
                                " space while respecting the actor's"
                                " width-for-height or height-for-width"
                                " constraints. The fill properties are ignored"
                                " when this property is enabled.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FIT, pspec);

  pspec = g_param_spec_boolean ("crop", "Fill space",
                                "Attempt to fill the parent's space by scaling"
                                " the child and keeping its aspect ratio as"
                                " well. The fit and fill properties are"
                                " ignored when this property is enabled.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CROP, pspec);
}

static void
mx_stack_child_init (MxStackChild *self)
{
  self->x_fill = TRUE;
  self->y_fill = TRUE;

  self->x_align = MX_ALIGN_MIDDLE;
  self->y_align = MX_ALIGN_MIDDLE;
}

static MxStackChild *
_get_child_meta (MxStack      *stack,
                 ClutterActor *child)
{
  MxStackChild *meta;

  meta = (MxStackChild*)
    clutter_container_get_child_meta (CLUTTER_CONTAINER (stack), child);

  return meta;
}

/**
 * mx_stack_child_get_x_fill:
 * @stack: A #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:x-fill property.
 *
 * Returns: the current value of the "x-fill" property.
 *
 * Since: 1.2
 */
gboolean
mx_stack_child_get_x_fill (MxStack      *stack,
                           ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  meta = _get_child_meta (stack, child);

  return meta->x_fill;
}

/**
 * mx_stack_child_set_x_fill:
 * @stack: A #MxStack
 * @child: A #ClutterActor
 * @x_fill: A #gboolean
 *
 * Set the value of the #MxStackChild:x-fill property.
 *
 * Since: 1.2
 */
void
mx_stack_child_set_x_fill (MxStack      *stack,
                           ClutterActor *child,
                           gboolean      x_fill)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->x_fill = x_fill;

  clutter_actor_queue_relayout (child);
}

/**
 * mx_stack_child_get_y_fill:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:y-fill property
 *
 * Returns: the current value of the "y-fill" property
 *
 * Since: 1.2
 */
gboolean
mx_stack_child_get_y_fill (MxStack      *stack,
                           ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  meta = _get_child_meta (stack, child);

  return meta->y_fill;
}

/**
 * mx_stack_child_set_y_fill:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 * @y_fill: A #gboolean
 *
 * Set the value of the #MxStackChild:y-fill property.
 *
 * Since: 1.2
 */
void
mx_stack_child_set_y_fill (MxStack      *stack,
                           ClutterActor *child,
                           gboolean      y_fill)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->y_fill = y_fill;

  clutter_actor_queue_relayout (child);
}

/**
 * mx_stack_child_get_x_align:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:x-align property
 *
 * Returns: the current value of the "x-align" property
 *
 * Since: 1.2
 */
MxAlign
mx_stack_child_get_x_align (MxStack      *stack,
                            ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), MX_ALIGN_START);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), MX_ALIGN_START);

  meta = _get_child_meta (stack, child);

  return meta->x_align;
}

/**
 * mx_stack_child_set_x_align:
 * @stack: A #MxStack
 * @child: A #ClutterActor
 * @x_align: An #MxAlign
 *
 * Set the value of the #MxStackChild:x-align property.
 *
 * Since: 1.2
 */
void
mx_stack_child_set_x_align (MxStack      *stack,
                            ClutterActor *child,
                            MxAlign       x_align)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->x_align = x_align;

  clutter_actor_queue_relayout (child);
}

/**
 * mx_stack_child_get_y_align:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:y-align property.
 *
 * Returns: the current value of the "y-align" property.
 *
 * Since: 1.2
 */
MxAlign
mx_stack_child_get_y_align (MxStack      *stack,
                            ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), MX_ALIGN_START);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), MX_ALIGN_START);

  meta = _get_child_meta (stack, child);

  return meta->y_align;
}

/**
 * mx_stack_child_set_y_align:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 * @y_align: An #MxAlign
 *
 * Set the value of the #MxStackChild:y-align property.
 *
 * Since: 1.2
 */
void
mx_stack_child_set_y_align (MxStack      *stack,
                            ClutterActor *child,
                            MxAlign       y_align)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->y_align = y_align;

  clutter_actor_queue_relayout (child);
}


/**
 * mx_stack_child_get_fit:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:fit property.
 *
 * Returns: the current value of the #MxStackChild:fit property
 *
 * Since: 1.2
 */
gboolean
mx_stack_child_get_fit (MxStack      *stack,
                        ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  meta = _get_child_meta (stack, child);

  return meta->fit;
}

/**
 * mx_stack_child_set_fit:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 * @fit: A #gboolean
 *
 * Set the value of the #MxStackChild:fit property.
 *
 * Since: 1.2
 */
void
mx_stack_child_set_fit (MxStack      *stack,
                        ClutterActor *child,
                        gboolean      fit)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->fit = fit;

  clutter_actor_queue_relayout (child);
}

/**
 * mx_stack_child_get_crop:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 *
 * Get the value of the #MxStackChild:fit property.
 *
 * Returns: the current value of the #MxStackChild:crop property
 *
 * Since: 1.4
 */
gboolean
mx_stack_child_get_crop (MxStack      *stack,
                         ClutterActor *child)
{
  MxStackChild *meta;

  g_return_val_if_fail (MX_IS_STACK (stack), FALSE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), FALSE);

  meta = _get_child_meta (stack, child);

  return meta->crop;
}

/**
 * mx_stack_child_set_crop:
 * @stack: An #MxStack
 * @child: A #ClutterActor
 * @crop: A #gboolean
 *
 * Set the value of the #MxStackChild:crop property.
 *
 * Since: 1.4
 */
void
mx_stack_child_set_crop (MxStack      *stack,
                         ClutterActor *child,
                         gboolean      crop)
{
  MxStackChild *meta;

  g_return_if_fail (MX_IS_STACK (stack));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = _get_child_meta (stack, child);

  meta->crop = crop;

  clutter_actor_queue_relayout (child);
}
