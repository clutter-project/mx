/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-bin.c: Basic container actor
 *
 * Copyright (c) 2009 Intel Corporation.
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
 * Written by: Emmanuele Bassi <ebassi@linux.intel.com>
 *
 */

/**
 * SECTION:mx-bin
 * @short_description: a simple container with one actor.
 *
 * #MxBin is a simple abstract container capable of having only one
 * #ClutterActor as a child. #MxBin does not allocate the child itself,
 * therefore any subclasses are required to implement the
 * #ClutterActorClass.allocate function.
 * #mx_bin_allocate_child() can be used if no special allocation requirements
 * are needed.
 *
 * #MxFrame is a simple implementation of #MxBin that can be used as a single
 * actor container that implements alignment and padding.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

#include "mx-bin.h"
#include "mx-enum-types.h"
#include "mx-private.h"
#include "mx-focusable.h"


#define MX_BIN_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_BIN, MxBinPrivate))

struct _MxBinPrivate
{
  ClutterActor *child;
  gboolean      child_has_space;

  MxAlign       x_align;
  MxAlign       y_align;

  guint         x_fill : 1;
  guint         y_fill : 1;
};

enum
{
  PROP_0,

  PROP_CHILD,
  PROP_X_ALIGN,
  PROP_Y_ALIGN,
  PROP_X_FILL,
  PROP_Y_FILL
};

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void mx_bin_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (MxBin, mx_bin, MX_TYPE_WIDGET,
                                  G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                         clutter_container_iface_init)
                                  G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                         mx_bin_focusable_iface_init));

void
_mx_bin_get_align_factors (MxBin   *bin,
                           gdouble *x_align,
                           gdouble *y_align)
{
  MxBinPrivate *priv = bin->priv;
  gdouble factor;

  switch (priv->x_align)
    {
    case MX_ALIGN_START:
      factor = 0.0;
      break;

    case MX_ALIGN_MIDDLE:
      factor = 0.5;
      break;

    case MX_ALIGN_END:
      factor = 1.0;
      break;

    default:
      factor = 0.0;
      break;
    }

  if (x_align)
    *x_align = factor;

  switch (priv->y_align)
    {
    case MX_ALIGN_START:
      factor = 0.0;
      break;

    case MX_ALIGN_MIDDLE:
      factor = 0.5;
      break;

    case MX_ALIGN_END:
      factor = 1.0;
      break;

    default:
      factor = 0.0;
      break;
    }

  if (y_align)
    *y_align = factor;
}

static void
mx_bin_add (ClutterContainer *container,
            ClutterActor     *actor)
{
  mx_bin_set_child (MX_BIN (container), actor);
}

static void
mx_bin_remove (ClutterContainer *container,
               ClutterActor     *actor)
{
  MxBinPrivate *priv = MX_BIN (container)->priv;

  if (priv->child == actor)
    mx_bin_set_child (MX_BIN (container), NULL);
}

static void
mx_bin_foreach (ClutterContainer *container,
                ClutterCallback   callback,
                gpointer          user_data)
{
  MxBinPrivate *priv = MX_BIN (container)->priv;

  if (priv->child)
    callback (priv->child, user_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_bin_add;
  iface->remove = mx_bin_remove;
  iface->foreach = mx_bin_foreach;
}

static MxFocusable*
mx_bin_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxBinPrivate *priv = MX_BIN (focusable)->priv;

  if (MX_IS_FOCUSABLE (priv->child))
    return mx_focusable_accept_focus (MX_FOCUSABLE (priv->child), hint);
  else
    return NULL;
}

static void
mx_bin_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_bin_accept_focus;
}


static void
mx_bin_paint (ClutterActor *self)
{
  MxBinPrivate *priv = MX_BIN (self)->priv;

  /* allow MxWidget to paint the background */
  CLUTTER_ACTOR_CLASS (mx_bin_parent_class)->paint (self);

  /* then paint our child */
  if (priv->child && priv->child_has_space)
    clutter_actor_paint (priv->child);
}

static void
mx_bin_pick (ClutterActor       *self,
             const ClutterColor *pick_color)
{
  MxBinPrivate *priv = MX_BIN (self)->priv;

  /* get the default pick implementation */
  CLUTTER_ACTOR_CLASS (mx_bin_parent_class)->pick (self, pick_color);

  if (priv->child)
    clutter_actor_paint (priv->child);
}

/**
 * mx_bin_allocate_child:
 * @bin: An #MxBin
 * @box: The allocation box of the parent actor.
 * @flags: #ClutterAllocationFlags, usually provided by the.
 * clutter_actor_allocate function.
 *
 * Allocates the child of an #MxBin using the width and height from @box.
 * This function should usually only be called by subclasses of #MxBin.
 *
 * This function can be used to allocate the child of an #MxBin if no special
 * allocation requirements are needed. It is similar to
 * #mx_allocate_align_fill, except that it reads the alignment, padding and
 * fill values from the #MxBin, and will call #clutter_actor_allocate on the
 * child.
 *
 */
void
mx_bin_allocate_child (MxBin                  *bin,
                       const ClutterActorBox  *box,
                       ClutterAllocationFlags  flags)
{
  MxBinPrivate *priv;

  g_return_if_fail (MX_IS_BIN (bin));

  priv = bin->priv;

  if (priv->child)
    {
      MxPadding padding;
      ClutterActorBox allocation = { 0, };

      mx_widget_get_padding (MX_WIDGET (bin), &padding);

      allocation.x1 = padding.left;
      allocation.x2 = box->x2 - box->x1 - padding.right;
      allocation.y1 = padding.top;
      allocation.y2 = box->y2 - box->y1 - padding.bottom;

      mx_allocate_align_fill (priv->child,
                              &allocation,
                              priv->x_align,
                              priv->y_align,
                              priv->x_fill,
                              priv->y_fill);

      clutter_actor_allocate (priv->child, &allocation, flags);
    }
}

static void
mx_bin_get_preferred_width (ClutterActor *self,
                            gfloat        for_height,
                            gfloat       *min_width_p,
                            gfloat       *natural_width_p)
{
  MxBinPrivate *priv = MX_BIN (self)->priv;
  gfloat min_width, natural_width;
  gfloat available_height;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  available_height = for_height - padding.top - padding.bottom;

  min_width = natural_width = padding.left + padding.right;

  if (priv->child == NULL || !CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    {
      if (min_width_p)
        *min_width_p = min_width;

      if (natural_width_p)
        *natural_width_p = natural_width;
    }
  else
    {
      clutter_actor_get_preferred_width (priv->child, available_height,
                                         min_width_p,
                                         natural_width_p);

      if (min_width_p)
        *min_width_p += min_width;

      if (natural_width_p)
        *natural_width_p += natural_width;
    }
}

static void
mx_bin_get_preferred_height (ClutterActor *self,
                             gfloat        for_width,
                             gfloat       *min_height_p,
                             gfloat       *natural_height_p)
{
  MxBinPrivate *priv = MX_BIN (self)->priv;
  gfloat min_height, natural_height;
  gfloat available_width;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  available_width = for_width - padding.left - padding.right;

  min_height = natural_height = padding.top + padding.bottom;

  if (priv->child == NULL || !CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    {
      if (min_height_p)
        *min_height_p = min_height;

      if (natural_height_p)
        *natural_height_p = natural_height;
    }
  else
    {
      clutter_actor_get_preferred_height (priv->child, available_width,
                                          min_height_p,
                                          natural_height_p);

      if (min_height_p)
        *min_height_p += min_height;

      if (natural_height_p)
        *natural_height_p += natural_height;
    }
}

static void
mx_bin_dispose (GObject *gobject)
{
  MxBinPrivate *priv = MX_BIN (gobject)->priv;

  if (priv->child)
    {
      clutter_actor_destroy (priv->child);
      priv->child = NULL;
    }

  G_OBJECT_CLASS (mx_bin_parent_class)->dispose (gobject);
}

static void
mx_bin_set_property (GObject      *gobject,
                     guint         prop_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  MxBin *bin = MX_BIN (gobject);

  switch (prop_id)
    {
    case PROP_CHILD:
      mx_bin_set_child (bin, g_value_get_object (value));
      break;

    case PROP_X_ALIGN:
      mx_bin_set_alignment (bin,
                            g_value_get_enum (value),
                            bin->priv->y_align);
      break;

    case PROP_Y_ALIGN:
      mx_bin_set_alignment (bin,
                            bin->priv->x_align,
                            g_value_get_enum (value));
      break;

    case PROP_X_FILL:
      mx_bin_set_fill (bin,
                       g_value_get_boolean (value),
                       bin->priv->y_fill);
      break;

    case PROP_Y_FILL:
      mx_bin_set_fill (bin,
                       bin->priv->x_fill,
                       g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
mx_bin_get_property (GObject    *gobject,
                     guint       prop_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
  MxBinPrivate *priv = MX_BIN (gobject)->priv;

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, priv->child);
      break;

    case PROP_X_FILL:
      g_value_set_boolean (value, priv->x_fill);
      break;

    case PROP_Y_FILL:
      g_value_set_boolean (value, priv->y_fill);
      break;

    case PROP_X_ALIGN:
      g_value_set_enum (value, priv->x_align);
      break;

    case PROP_Y_ALIGN:
      g_value_set_enum (value, priv->y_align);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
mx_bin_class_init (MxBinClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxBinPrivate));

  gobject_class->set_property = mx_bin_set_property;
  gobject_class->get_property = mx_bin_get_property;
  gobject_class->dispose = mx_bin_dispose;

  actor_class->get_preferred_width = mx_bin_get_preferred_width;
  actor_class->get_preferred_height = mx_bin_get_preferred_height;
  actor_class->paint = mx_bin_paint;
  actor_class->pick = mx_bin_pick;

  /**
   * MxBin:child:
   *
   * The child #ClutterActor of the #MxBin container.
   */
  pspec = g_param_spec_object ("child",
                               "Child",
                               "The child of the Bin",
                               CLUTTER_TYPE_ACTOR,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_CHILD, pspec);

  /**
   * MxBin:x-align:
   *
   * The horizontal alignment of the #MxBin child.
   */
  pspec = g_param_spec_enum ("x-align",
                             "X Align",
                             "The horizontal alignment",
                             MX_TYPE_ALIGN,
                             MX_ALIGN_MIDDLE,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_X_ALIGN, pspec);

  /**
   * MxBin:y-align:
   *
   * The vertical alignment of the #MxBin child.
   */
  pspec = g_param_spec_enum ("y-align",
                             "Y Align",
                             "The vertical alignment",
                             MX_TYPE_ALIGN,
                             MX_ALIGN_MIDDLE,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Y_ALIGN, pspec);

  /**
   * MxBin:x-fill:
   *
   * Whether the child should fill the horizontal allocation
   */
  pspec = g_param_spec_boolean ("x-fill",
                                "X Fill",
                                "Whether the child should fill the "
                                "horizontal allocation",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_X_FILL, pspec);

  /**
   * MxBin:y-fill:
   *
   * Whether the child should fill the vertical allocation
   */
  pspec = g_param_spec_boolean ("y-fill",
                                "Y Fill",
                                "Whether the child should fill the "
                                "vertical allocation",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Y_FILL, pspec);
}

static void
mx_bin_init (MxBin *bin)
{
  bin->priv = MX_BIN_GET_PRIVATE (bin);

  bin->priv->x_align = MX_ALIGN_MIDDLE;
  bin->priv->y_align = MX_ALIGN_MIDDLE;
  bin->priv->child_has_space = TRUE;
}

/**
 * mx_bin_set_child:
 * @bin: a #MxBin
 * @child: a #ClutterActor, or %NULL
 *
 * Sets @child as the child of @bin.
 *
 * If @bin already has a child, the previous child is removed.
 */
void
mx_bin_set_child (MxBin        *bin,
                  ClutterActor *child)
{
  MxBinPrivate *priv;

  g_return_if_fail (MX_IS_BIN (bin));
  g_return_if_fail (child == NULL || CLUTTER_IS_ACTOR (child));

  priv = bin->priv;

  if (priv->child == child)
    return;

  if (priv->child)
    {
      ClutterActor *old_child = priv->child;

      g_object_ref (old_child);

      priv->child = NULL;
      clutter_actor_unparent (old_child);

      g_signal_emit_by_name (bin, "actor-removed", old_child);

      g_object_unref (old_child);
    }

  if (child)
    {
      priv->child = child;
      clutter_actor_set_parent (child, CLUTTER_ACTOR (bin));

      g_signal_emit_by_name (bin, "actor-added", priv->child);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (bin));

  g_object_notify (G_OBJECT (bin), "child");
}

/**
 * mx_bin_get_child:
 * @bin: a #MxBin
 *
 * Retrieves a pointer to the child of @bin.
 *
 * Return value: (transfer none): a #ClutterActor, or %NULL
 */
ClutterActor *
mx_bin_get_child (MxBin *bin)
{
  g_return_val_if_fail (MX_IS_BIN (bin), NULL);

  return bin->priv->child;
}

/**
 * mx_bin_set_alignment:
 * @bin: a #MxBin
 * @x_align: horizontal alignment
 * @y_align: vertical alignment
 *
 * Sets the horizontal and vertical alignment of the child
 * inside a #MxBin.
 */
void
mx_bin_set_alignment (MxBin  *bin,
                      MxAlign x_align,
                      MxAlign y_align)
{
  MxBinPrivate *priv;
  gboolean changed = FALSE;

  g_return_if_fail (MX_IS_BIN (bin));

  priv = bin->priv;

  g_object_freeze_notify (G_OBJECT (bin));

  if (priv->x_align != x_align)
    {
      priv->x_align = x_align;
      g_object_notify (G_OBJECT (bin), "x-align");
      changed = TRUE;
    }

  if (priv->y_align != y_align)
    {
      priv->y_align = y_align;
      g_object_notify (G_OBJECT (bin), "y-align");
      changed = TRUE;
    }

  if (changed)
    clutter_actor_queue_relayout (CLUTTER_ACTOR (bin));

  g_object_thaw_notify (G_OBJECT (bin));
}

/**
 * mx_bin_get_alignment:
 * @bin: a #MxBin
 * @x_align: return location for the horizontal alignment, or %NULL
 * @y_align: return location for the vertical alignment, or %NULL
 *
 * Retrieves the horizontal and vertical alignment of the child
 * inside a #MxBin, as set by mx_bin_set_alignment().
 */
void
mx_bin_get_alignment (MxBin   *bin,
                      MxAlign *x_align,
                      MxAlign *y_align)
{
  MxBinPrivate *priv;

  g_return_if_fail (MX_IS_BIN (bin));

  priv = bin->priv;

  if (x_align)
    *x_align = priv->x_align;

  if (y_align)
    *y_align = priv->y_align;
}

/**
 * mx_bin_set_fill:
 * @bin: a #MxBin
 * @x_fill: %TRUE if the child should fill horizontally the @bin
 * @y_fill: %TRUE if the child should fill vertically the @bin
 *
 * Sets whether the child of @bin should fill out the horizontal
 * and/or vertical allocation of the parent
 */
void
mx_bin_set_fill (MxBin   *bin,
                 gboolean x_fill,
                 gboolean y_fill)
{
  MxBinPrivate *priv;
  gboolean changed = FALSE;

  g_return_if_fail (MX_IS_BIN (bin));

  priv = bin->priv;

  g_object_freeze_notify (G_OBJECT (bin));

  if (priv->x_fill != x_fill)
    {
      priv->x_fill = x_fill;
      changed = TRUE;

      g_object_notify (G_OBJECT (bin), "x-fill");
    }

  if (priv->y_fill != y_fill)
    {
      priv->y_fill = y_fill;
      changed = TRUE;

      g_object_notify (G_OBJECT (bin), "y-fill");
    }

  if (changed)
    clutter_actor_queue_relayout (CLUTTER_ACTOR (bin));

  g_object_thaw_notify (G_OBJECT (bin));
}

/**
 * mx_bin_get_fill:
 * @bin: a #MxBin
 * @x_fill: (out): return location for the horizontal fill, or %NULL
 * @y_fill: (out): return location for the vertical fill, or %NULL
 *
 * Retrieves the horizontal and vertical fill settings
 */
void
mx_bin_get_fill (MxBin    *bin,
                 gboolean *x_fill,
                 gboolean *y_fill)
{
  g_return_if_fail (MX_IS_BIN (bin));

  if (x_fill)
    *x_fill = bin->priv->x_fill;

  if (y_fill)
    *y_fill = bin->priv->y_fill;
}
