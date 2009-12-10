/*
 * mx-box-layout-child.c: box layout child actor
 *
 * Copyright 2009 Intel Corporation
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
 */

/**
 * SECTION:mx-box-layout-child
 * @short_description: meta data associated with a #MxBoxLayout child.
 *
 * #MxBoxLayoutChild is a #ClutterChildMeta implementation that stores the
 * child properties for children inside a #MxBoxLayout.
 */

#include "mx-box-layout-child.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxBoxLayoutChild, mx_box_layout_child, CLUTTER_TYPE_CHILD_META)

#define BOX_LAYOUT_CHILD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_BOX_LAYOUT_CHILD, MxBoxLayoutChildPrivate))


enum
{
  PROP_0,

  PROP_EXPAND,
  PROP_X_FILL,
  PROP_Y_FILL,
  PROP_X_ALIGN,
  PROP_Y_ALIGN
};

static void
mx_box_layout_child_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  MxBoxLayoutChild *child = MX_BOX_LAYOUT_CHILD (object);

  switch (property_id)
    {
    case PROP_EXPAND:
      g_value_set_boolean (value, child->expand);
      break;
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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_box_layout_child_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  MxBoxLayoutChild *child = MX_BOX_LAYOUT_CHILD (object);
  MxBoxLayout *box = MX_BOX_LAYOUT (CLUTTER_CHILD_META (object)->container);

  _mx_box_layout_start_animation (box);


  switch (property_id)
    {
    case PROP_EXPAND:
      child->expand = g_value_get_boolean (value);
      break;
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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }

  clutter_actor_queue_relayout ((ClutterActor*) box);
}

static void
mx_box_layout_child_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_box_layout_child_parent_class)->dispose (object);
}

static void
mx_box_layout_child_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_box_layout_child_parent_class)->finalize (object);
}

static void
mx_box_layout_child_class_init (MxBoxLayoutChildClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  object_class->get_property = mx_box_layout_child_get_property;
  object_class->set_property = mx_box_layout_child_set_property;
  object_class->dispose = mx_box_layout_child_dispose;
  object_class->finalize = mx_box_layout_child_finalize;


  pspec = g_param_spec_boolean ("expand", "Expand",
                                "Allocate the child extra space",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_EXPAND, pspec);

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
}

static void
mx_box_layout_child_init (MxBoxLayoutChild *self)
{
  self->expand = FALSE;

  self->x_fill = TRUE;
  self->y_fill = TRUE;

  self->x_align = MX_ALIGN_MIDDLE;
  self->y_align = MX_ALIGN_MIDDLE;
}

