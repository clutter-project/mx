/*
 * nbtk-bin.c: Basic container actor
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
 * SECTION:nbtk-bin
 * @short_description: a simple container with one actor
 *
 * #NbtkBin is a simple container capable of having only one
 * #ClutterActor as a child.
 *
 * #NbtkBin inherits from #NbtkWidget, so it is fully themable.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

#include "nbtk-bin.h"
#include "nbtk-enum-types.h"
#include "nbtk-private.h"
#include "nbtk-stylable.h"

#define NBTK_BIN_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_BIN, NbtkBinPrivate))

struct _NbtkBinPrivate
{
  ClutterActor *child;

  NbtkAlignment x_align;
  NbtkAlignment y_align;
};

enum
{
  PROP_0,

  PROP_CHILD,
  PROP_PADDING,
  PROP_PADDING_SET,
  PROP_X_ALIGN,
  PROP_Y_ALIGN
};

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkBin, nbtk_bin, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init));

void
_nbtk_bin_get_align_factors (NbtkBin *bin,
                             gdouble *x_align,
                             gdouble *y_align)
{
  NbtkBinPrivate *priv = bin->priv;
  gdouble factor;

  switch (priv->x_align)
    {
    case NBTK_ALIGN_LEFT:
      factor = 0.0;
      break;

    case NBTK_ALIGN_CENTER:
      factor = 0.5;
      break;

    case NBTK_ALIGN_RIGHT:
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
    case NBTK_ALIGN_TOP:
      factor = 0.0;
      break;

    case NBTK_ALIGN_CENTER:
      factor = 0.5;
      break;

    case NBTK_ALIGN_BOTTOM:
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
nbtk_bin_add (ClutterContainer *container,
              ClutterActor     *actor)
{
  nbtk_bin_set_child (NBTK_BIN (container), actor);
}

static void
nbtk_bin_remove (ClutterContainer *container,
                 ClutterActor     *actor)
{
  NbtkBinPrivate *priv = NBTK_BIN (container)->priv;

  if (priv->child == actor)
    nbtk_bin_set_child (NBTK_BIN (container), NULL);
}

static void
nbtk_bin_foreach (ClutterContainer *container,
                  ClutterCallback   callback,
                  gpointer          user_data)
{
  NbtkBinPrivate *priv = NBTK_BIN (container)->priv;

  if (priv->child)
    callback (priv->child, user_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_bin_add;
  iface->remove = nbtk_bin_remove;
  iface->foreach = nbtk_bin_foreach;
}

static void
nbtk_bin_paint (ClutterActor *self)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;

  /* allow NbtkWidget to paint the background */
  CLUTTER_ACTOR_CLASS (nbtk_bin_parent_class)->paint (self);

  /* the pain our child */
  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
}

static void
nbtk_bin_pick (ClutterActor       *self,
               const ClutterColor *pick_color)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;

  /* get the default pick implementation */
  CLUTTER_ACTOR_CLASS (nbtk_bin_parent_class)->pick (self, pick_color);

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
}

static void
nbtk_bin_allocate (ClutterActor          *self,
                   const ClutterActorBox *box,
                   gboolean               origin_changed)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_bin_parent_class)->allocate (self, box,
                                                         origin_changed);

  if (priv->child)
    {
      ClutterUnit natural_width, natural_height;
      ClutterUnit min_width, min_height;
      ClutterUnit child_width, child_height;
      ClutterUnit available_width, available_height;
      ClutterRequestMode request;
      ClutterActorBox allocation = { 0, };
      NbtkPadding padding = { 0, };
      gdouble x_align, y_align;

      _nbtk_bin_get_align_factors (NBTK_BIN (self), &x_align, &y_align);

      nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

      available_width  = box->x2 - box->x1
                       - padding.left - padding.right;
      available_height = box->y2 - box->y1
                       - padding.top - padding.bottom;

      if (available_width < 0)
        available_width = 0;

      if (available_height < 0)
        available_height = 0;

      request = CLUTTER_REQUEST_HEIGHT_FOR_WIDTH;
      g_object_get (G_OBJECT (priv->child), "request-mode", &request, NULL);

      if (request == CLUTTER_REQUEST_HEIGHT_FOR_WIDTH)
        {
          clutter_actor_get_preferred_width (priv->child, available_height,
                                             &min_width,
                                             &natural_width);

          child_width = CLAMP (natural_width, min_width, available_width);

          clutter_actor_get_preferred_height (priv->child, child_width,
                                              &min_height,
                                              &natural_height);

          child_height = CLAMP (natural_height, min_height, available_height);
        }
      else if (request == CLUTTER_REQUEST_WIDTH_FOR_HEIGHT)
        {
          clutter_actor_get_preferred_height (priv->child, available_width,
                                              &min_height,
                                              &natural_height);

          child_height = CLAMP (natural_height, min_height, available_height);

          clutter_actor_get_preferred_width (priv->child, child_height,
                                             &min_width,
                                             &natural_width);

          child_width = CLAMP (natural_width, min_width, available_width);
        }

      allocation.x1 = (int) ((available_width - child_width) * x_align
                    + padding.left);
      allocation.y1 = (int) ((available_height - child_height) * y_align
                    + padding.top);
      allocation.x2 = allocation.x1 + child_width;
      allocation.y2 = allocation.y1 + child_height;

      clutter_actor_allocate (priv->child, &allocation, origin_changed);
    }
}

static void
nbtk_bin_get_preferred_width (ClutterActor *self,
                              ClutterUnit   for_height,
                              ClutterUnit  *min_width_p,
                              ClutterUnit  *natural_width_p)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;
  ClutterUnit min_width, natural_width;
  NbtkPadding padding = { 0, };

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  min_width     = 0;
  natural_width = padding.left + padding.right;

  if (priv->child == NULL)
    {
      if (min_width_p)
        *min_width_p = min_width;

      if (natural_width_p)
        *natural_width_p = natural_width;
    }
  else
    {
      clutter_actor_get_preferred_width (priv->child, for_height,
                                         min_width_p,
                                         natural_width_p);

      if (min_width_p)
        *min_width_p += min_width;

      if (natural_width_p)
        *natural_width_p += natural_width;
    }
}

static void
nbtk_bin_get_preferred_height (ClutterActor *self,
                               ClutterUnit   for_width,
                               ClutterUnit  *min_height_p,
                               ClutterUnit  *natural_height_p)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;
  ClutterUnit min_height, natural_height;
  NbtkPadding padding = { 0, };

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  min_height     = 0;
  natural_height = padding.top + padding.bottom;

  if (priv->child == NULL)
    {
      if (min_height_p)
        *min_height_p = min_height;

      if (natural_height_p)
        *natural_height_p = natural_height;
    }
  else
    {
      clutter_actor_get_preferred_height (priv->child, for_width,
                                          min_height_p,
                                          natural_height_p);

      if (min_height_p)
        *min_height_p += min_height;

      if (natural_height_p)
        *natural_height_p += natural_height;
    }
}

static void
nbtk_bin_dispose (GObject *gobject)
{
  NbtkBinPrivate *priv = NBTK_BIN (gobject)->priv;

  if (priv->child)
    {
      clutter_actor_unparent (priv->child);
      priv->child = NULL;
    }

  G_OBJECT_CLASS (nbtk_bin_parent_class)->dispose (gobject);
}

static void
nbtk_bin_set_property (GObject      *gobject,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  NbtkBin *bin = NBTK_BIN (gobject);

  switch (prop_id)
    {
    case PROP_CHILD:
      nbtk_bin_set_child (bin, g_value_get_object (value));
      break;

    case PROP_PADDING:
      nbtk_bin_set_padding (bin, g_value_get_boxed (value));
      break;

    case PROP_X_ALIGN:
      nbtk_bin_set_alignment (bin,
                              g_value_get_enum (value),
                              bin->priv->y_align);
      break;

    case PROP_Y_ALIGN:
      nbtk_bin_set_alignment (bin,
                              bin->priv->x_align,
                              g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
nbtk_bin_get_property (GObject    *gobject,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  NbtkBinPrivate *priv = NBTK_BIN (gobject)->priv;
  NbtkPadding padding = { 0, };

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, priv->child);
      break;

    case PROP_PADDING:
      g_warning ("The padding property on NbtkBin is deprecated, please use"                          " nbtk_widget_get_padding() instead.");
      nbtk_widget_get_padding (NBTK_WIDGET (gobject), &padding);
      g_value_set_boxed (value, &padding);
      break;

    case PROP_PADDING_SET:
      g_warning ("The padding-set property on NbtkBin is deprecated, it should not"
                 " be used.");
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
nbtk_bin_style_changed (NbtkWidget *self)
{
  NbtkBinPrivate *priv = NBTK_BIN (self)->priv;

  NBTK_WIDGET_CLASS (nbtk_bin_parent_class)->style_changed (self);

  if (priv->child)
    g_signal_emit_by_name (priv->child, "style-changed", 0);
}

static void
nbtk_bin_class_init (NbtkBinClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkBinPrivate));

  gobject_class->set_property = nbtk_bin_set_property;
  gobject_class->get_property = nbtk_bin_get_property;
  gobject_class->dispose = nbtk_bin_dispose;

  actor_class->get_preferred_width = nbtk_bin_get_preferred_width;
  actor_class->get_preferred_height = nbtk_bin_get_preferred_height;
  actor_class->allocate = nbtk_bin_allocate;
  actor_class->paint = nbtk_bin_paint;
  actor_class->pick = nbtk_bin_pick;

  widget_class->style_changed = nbtk_bin_style_changed;

  /**
   * NbtkBin:child:
   *
   * The child #ClutterActor of the #NbtkBin container.
   */
  pspec = g_param_spec_object ("child",
                               "Child",
                               "The child of the Bin",
                               CLUTTER_TYPE_ACTOR,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_CHILD, pspec);

  /**
   * NbtkBin:padding:
   *
   * The padding of the #NbtkBin child from the internal border
   * of the #NbtkBin actor.
   */
  pspec = g_param_spec_boxed ("padding",
                              "Padding",
                              "The padding of the child",
                              NBTK_TYPE_PADDING,
                              NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_PADDING, pspec);

  /**
   * NbtkBin:padding-set:
   *
   * Whether the #NbtkBin:padding has been set using
   * nbtk_bin_set_padding() or by the CSS theme.
   */
  pspec = g_param_spec_boolean ("padding-set",
                                "Padding Set",
                                "Whether the padding has been set",
                                FALSE,
                                NBTK_PARAM_READABLE);
  g_object_class_install_property (gobject_class, PROP_PADDING_SET, pspec);

  /**
   * NbtkBin:x-align:
   *
   * The horizontal alignment of the #NbtkBin child.
   */
  pspec = g_param_spec_enum ("x-align",
                             "X Align",
                             "The horizontal alignment",
                             NBTK_TYPE_ALIGNMENT,
                             NBTK_ALIGN_CENTER,
                             NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_X_ALIGN, pspec);

  /**
   * NbtkBin:y-align:
   *
   * The vertical alignment of the #NbtkBin child.
   */
  pspec = g_param_spec_enum ("y-align",
                             "Y Align",
                             "The vertical alignment",
                             NBTK_TYPE_ALIGNMENT,
                             NBTK_ALIGN_CENTER,
                             NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Y_ALIGN, pspec);
}

static void
nbtk_bin_init (NbtkBin *bin)
{
  bin->priv = NBTK_BIN_GET_PRIVATE (bin);

  bin->priv->x_align = NBTK_ALIGN_CENTER;
  bin->priv->y_align = NBTK_ALIGN_CENTER;
}

/**
 * nbtk_bin_new:
 *
 * Creates a new #NbtkBin, a simple container for one child.
 *
 * Return value: the newly created #NbtkBin actor
 */
NbtkWidget *
nbtk_bin_new (void)
{
  return g_object_new (NBTK_TYPE_BIN, NULL);
}

/**
 * nbtk_bin_set_child:
 * @bin: a #NbtkBin
 * @child: a #ClutterActor, or %NULL
 *
 * Sets @child as the child of @bin.
 *
 * If @bin already has a child, the previous child is removed.
 */
void
nbtk_bin_set_child (NbtkBin *bin,
                    ClutterActor *child)
{
  NbtkBinPrivate *priv;

  g_return_if_fail (NBTK_IS_BIN (bin));
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

      g_signal_emit_by_name (bin, "actor-removed", priv->child);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (bin));

  g_object_notify (G_OBJECT (bin), "child");
}

/**
 * nbtk_bin_get_child:
 * @bin: a #NbtkBin
 *
 * Retrieves a pointer to the child of @bin.
 *
 * Return value: a #ClutterActor, or %NULL
 */
ClutterActor *
nbtk_bin_get_child (NbtkBin *bin)
{
  g_return_val_if_fail (NBTK_IS_BIN (bin), NULL);

  return bin->priv->child;
}

/**
 * nbtk_bin_set_padding:
 * @bin: a #NbtkBin
 * @padding: the padding of the bin, or %NULL
 *
 * Sets the padding of #NbtkBin. The padding is the space from
 * the internal border of the #NbtkBin used when allocating the
 * child.
 *
 * If @padding is %NULL the default padding provided by the
 * CSS theme will be used instead.
 *
 * Deprecated: please set padding using a stylesheet
 */
G_GNUC_DEPRECATED void
nbtk_bin_set_padding (NbtkBin           *bin,
                      const NbtkPadding *padding)
{
  g_warning ("%s is deprecated, please set padding using a stylesheet",
             __FUNCTION__);
}

/**
 * nbtk_bin_get_padding:
 * @bin: a #NbtkBin
 * @padding: return location for a #NbtkPadding
 *
 * Retrieves the padding set using nbtk_bin_set_padding().
 *
 * Deprecated: Please use nbtk_widget_get_padding instead
 */
G_GNUC_DEPRECATED void
nbtk_bin_get_padding (NbtkBin     *bin,
                      NbtkPadding *padding)
{
  g_return_if_fail (NBTK_IS_BIN (bin));
  g_return_if_fail (padding != NULL);

  g_warning ("%s is deprecated. Please use nbtk_widget_get_padding instead",
             __FUNCTION__);
  nbtk_widget_get_padding (NBTK_WIDGET (bin), padding);
}

/**
 * nbtk_bin_set_alignment:
 * @bin: a #NbtkBin
 * @x_align: horizontal alignment
 * @y_align: vertical alignment
 *
 * Sets the horizontal and vertical alignment of the child
 * inside a #NbtkBin.
 */
void
nbtk_bin_set_alignment (NbtkBin       *bin,
                        NbtkAlignment  x_align,
                        NbtkAlignment  y_align)
{
  NbtkBinPrivate *priv;
  gboolean changed = FALSE;

  g_return_if_fail (NBTK_IS_BIN (bin));

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
 * nbtk_bin_get_alignment:
 * @bin: a #NbtkBin
 * @x_align: return location for the horizontal alignment, or %NULL
 * @y_align: return location for the vertical alignment, or %NULL
 *
 * Retrieves the horizontal and vertical alignment of the child
 * inside a #NbtkBin, as set by nbtk_bin_set_alignment().
 */
void
nbtk_bin_get_alignment (NbtkBin       *bin,
                        NbtkAlignment *x_align,
                        NbtkAlignment *y_align)
{
  NbtkBinPrivate *priv;

  g_return_if_fail (NBTK_IS_BIN (bin));

  priv = bin->priv;

  if (x_align)
    *x_align = priv->x_align;

  if (y_align)
    *y_align = priv->y_align;
}

static gpointer
nbtk_padding_copy (gpointer data)
{
  return g_slice_dup (NbtkPadding, data);
}

static void
nbtk_padding_free (gpointer data)
{
  if (G_LIKELY (data))
    g_slice_free (NbtkPadding, data);
}

GType
nbtk_padding_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    our_type = g_boxed_type_register_static (I_("NbtkPadding"),
                                             nbtk_padding_copy,
                                             nbtk_padding_free);

  return our_type;
}
