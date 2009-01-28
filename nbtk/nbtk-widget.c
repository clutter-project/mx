/* nbtk-widget.c: Base class for Nbtk actors
 *
 * Copyright (C) 2007 OpenedHand
 * Copyright (C) 2008 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <clutter/clutter.h>

#include "nbtk-widget.h"

#include "nbtk-marshal.h"
#include "nbtk-private.h"
#include "nbtk-stylable.h"
#include "nbtk-texture-cache.h"
#include "nbtk-texture-frame.h"

/*
 * ClutterChildMeta Implementation
 */

enum {
  CHILD_PROP_0,

  CHILD_PROP_DND_DISABLED,
};

G_DEFINE_TYPE (NbtkWidgetChild, nbtk_widget_child, CLUTTER_TYPE_CHILD_META);

static void
widget_child_set_property (GObject      *gobject,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
  NbtkWidgetChild *child = NBTK_WIDGET_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_DND_DISABLED:
      child->dnd_disabled = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
widget_child_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkWidgetChild *child = NBTK_WIDGET_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_DND_DISABLED:
      g_value_set_boolean (value, child->dnd_disabled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_widget_child_class_init (NbtkWidgetChildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->set_property = widget_child_set_property;
  gobject_class->get_property = widget_child_get_property;

  pspec = g_param_spec_boolean ("dnd-disabled",
                                "DND is disabled",
                                "Indicates that this actor cannot participate "
                                "in drag and drop.",
                                FALSE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_DND_DISABLED,
				   pspec);
}

static void
nbtk_widget_child_init (NbtkWidgetChild *self)
{
}

/**
 * SECTION:nbtk-widget
 * @short_description: Base class for stylable actors
 *
 * #NbtkWidget is a simple abstract class on top of #ClutterActor. It
 * provides basic themeing properties, support for padding and alignment.
 *
 * Actors in the Nbtk library should subclass #NbtkWidget if they plan
 * to obey to a certain #NbtkStyle or if they implement #ClutterContainer
 * and want to offer basic layout capabilities.
 */

enum
{
  PROP_0,

  PROP_STYLE,
  PROP_PADDING,
  PROP_X_ALIGN,
  PROP_Y_ALIGN,
  PROP_PSEUDO_CLASS,
  PROP_STYLE_CLASS,
  PROP_DND_THRESHOLD,
  PROP_DND_ICON,
};

enum
{
  STYLE_CHANGED,
  DND_BEGIN,
  DND_MOTION,
  DND_END,
  DND_DROPPED,
  DND_ENTER,
  DND_LEAVE,

  LAST_SIGNAL
};

static guint actor_signals[LAST_SIGNAL] = { 0, };

static void nbtk_stylable_iface_init (NbtkStylableIface *iface);
static void nbtk_container_iface_init (ClutterContainerIface *iface);


G_DEFINE_ABSTRACT_TYPE_WITH_CODE (NbtkWidget, nbtk_widget, CLUTTER_TYPE_ACTOR,
                                  G_IMPLEMENT_INTERFACE (NBTK_TYPE_STYLABLE,
                                                         nbtk_stylable_iface_init)
                                  G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                         nbtk_container_iface_init));

#define NBTK_WIDGET_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_WIDGET, NbtkWidgetPrivate))

struct _NbtkWidgetPrivate
{
  ClutterActor *child;

  NbtkPadding border;
  NbtkPadding padding;
  gboolean override_css_padding;

  ClutterFixed x_align;
  ClutterFixed y_align;

  NbtkStyle *style;
  gchar *pseudo_class;
  gchar *style_class;

  ClutterActor *bg_image;
  ClutterColor *bg_color;

  ClutterActor *dnd_last_dest;
  ClutterActor *dnd_clone;
  ClutterActor *dnd_dragged;
  ClutterActor *dnd_icon;

  guint dnd_threshold;
  gint  dnd_x;
  gint  dnd_y;

  guint dnd_enter_cb_id;

  gboolean dnd_motion : 1;
  gboolean dnd_grab   : 1;
};

static void
nbtk_widget_add_actor (ClutterContainer *container,
                       ClutterActor     *actor)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (container)->priv;

  if (priv->child)
    clutter_actor_unparent (priv->child);

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
  priv->child = actor;

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
nbtk_widget_remove_actor (ClutterContainer *container,
                          ClutterActor     *actor)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (container)->priv;

  if (priv->child == actor)
    {
      g_object_ref (priv->child);

      clutter_actor_unparent (priv->child);

      clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

      g_signal_emit_by_name (container, "actor-removed", priv->child);

      g_object_unref (priv->child);
      priv->child = NULL;
    }
}

static void
nbtk_widget_foreach (ClutterContainer *container,
                     ClutterCallback   callback,
                     gpointer          callback_data)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (container)->priv;

  if (priv->child)
    callback (priv->child, callback_data);
}

static void
nbtk_widget_lower (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* single child */
}

static void
nbtk_widget_raise (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* single child */
}

static void
nbtk_widget_sort_depth_order (ClutterContainer *container)
{
  /* single child */
}

static void
nbtk_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_widget_add_actor;
  iface->remove = nbtk_widget_remove_actor;
  iface->foreach = nbtk_widget_foreach;
  iface->lower = nbtk_widget_lower;
  iface->raise = nbtk_widget_raise;
  iface->sort_depth_order = nbtk_widget_sort_depth_order;
}


static void
nbtk_widget_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkWidget *actor = NBTK_WIDGET (gobject);

  switch (prop_id)
    {
    case PROP_PADDING:
      nbtk_widget_set_padding (actor, g_value_get_boxed (value));
      break;

    case PROP_X_ALIGN:
      actor->priv->x_align =
        CLUTTER_FIXED_TO_FLOAT (g_value_get_double (value));
      break;

    case PROP_Y_ALIGN:
      actor->priv->y_align =
        CLUTTER_FIXED_TO_FLOAT (g_value_get_double (value));
      break;

    case PROP_STYLE:
      nbtk_stylable_set_style (NBTK_STYLABLE (actor),
                               g_value_get_object (value));
      break;

    case PROP_PSEUDO_CLASS:
      nbtk_widget_set_style_pseudo_class (actor, g_value_get_string (value));
      break;

    case PROP_STYLE_CLASS:
      nbtk_widget_set_style_class_name (actor, g_value_get_string (value));
      break;

    case PROP_DND_THRESHOLD:
      actor->priv->dnd_threshold = g_value_get_uint (value);
      break;

    case PROP_DND_ICON:
      actor->priv->dnd_icon = g_value_get_object (value);
      g_object_ref (actor->priv->dnd_icon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_widget_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkWidget *actor = NBTK_WIDGET (gobject);
  NbtkWidgetPrivate *priv = actor->priv;

  switch (prop_id)
    {
    case PROP_PADDING:
      {
        NbtkPadding padding = { 0, };

        nbtk_widget_get_padding (actor, &padding);
        g_value_set_boxed (value, &padding);
      }
      break;

    case PROP_X_ALIGN:
      g_value_set_double (value, CLUTTER_FIXED_TO_FLOAT (priv->x_align));
      break;

    case PROP_Y_ALIGN:
      g_value_set_double (value, CLUTTER_FIXED_TO_FLOAT (priv->y_align));
      break;

    case PROP_STYLE:
      g_value_set_object (value, priv->style);
      break;

    case PROP_PSEUDO_CLASS:
      g_value_set_string (value, priv->pseudo_class);
      break;

    case PROP_STYLE_CLASS:
      g_value_set_string (value, priv->style_class);
      break;

    case PROP_DND_THRESHOLD:
      g_value_set_uint (value, priv->dnd_threshold);
      break;

    case PROP_DND_ICON:
      g_value_set_object (value, priv->dnd_icon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_widget_dispose (GObject *gobject)
{
  NbtkWidget *actor = NBTK_WIDGET (gobject);
  NbtkWidgetPrivate *priv = NBTK_WIDGET (actor)->priv;

  if (priv->style)
    {
      g_object_unref (priv->style);
      priv->style = NULL;
    }

  if (priv->child)
    {
      clutter_actor_unparent (priv->child);
      priv->child = NULL;
    }

  /*
   * If we are using custom dnd icon, release the extra reference we
   * have for it.
   *
   * NB: dnd_icon will == dnd_clone, so the extra reference should be released
   *     before we deal with dnd_clone.
   */
  if (priv->dnd_icon)
    g_object_unref (priv->dnd_icon);

  if (priv->dnd_clone)
    {
      ClutterActor *clone = priv->dnd_clone;
      priv->dnd_clone = NULL;
      clutter_actor_destroy (clone);
    }

  G_OBJECT_CLASS (nbtk_widget_parent_class)->dispose (gobject);
}


static void
nbtk_widget_allocate (ClutterActor          *actor,
                      const ClutterActorBox *box,
                      gboolean               origin_changed)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (actor)->priv;
  ClutterActorClass *klass;

  klass = CLUTTER_ACTOR_CLASS (nbtk_widget_parent_class);
  klass->allocate (actor, box, origin_changed);

  if (priv->bg_image)
    {
      ClutterActorBox frame_box = {
          0, 0, box->x2 - box->x1, box->y2 - box->y1
      };

      clutter_actor_allocate (CLUTTER_ACTOR (priv->bg_image),
                              &frame_box,
                              origin_changed);
    }

  if (priv->child)
    {
      ClutterFixed x_align, y_align;
      ClutterUnit available_width, available_height;
      ClutterUnit child_width, child_height;
      ClutterActorBox child_box = { 0, };

      nbtk_widget_get_alignmentx (NBTK_WIDGET (actor), &x_align, &y_align);

      available_width  = box->x2 - box->x1
                       - priv->padding.left - priv->padding.right
                       - priv->border.left - priv->border.right;
      available_height = box->y2 - box->y1
                       - priv->padding.top - priv->padding.bottom
                       - priv->border.top - priv->border.bottom;

      if (available_width < 0)
        available_width = 0;

      if (available_height < 0)
        available_height = 0;

      clutter_actor_get_preferred_size (priv->child,
                                        NULL, NULL,
                                        &child_width,
                                        &child_height);

      if (child_width > available_width)
        child_width = available_width;

      if (child_height > available_height)
        child_height = available_height;
      child_box.x1 = CLUTTER_FIXED_MUL ((available_width - child_width),
                                        x_align)
                   + priv->padding.left + priv->border.left;
      child_box.y1 = CLUTTER_FIXED_MUL ((available_height - child_height),
                                        y_align)
                   + priv->padding.top + priv->border.top;

      /* align the co-ordinates to device units to prevent allocation on sub-pixels */
      child_box.x1 = CLUTTER_UNITS_FROM_DEVICE ((CLUTTER_UNITS_TO_DEVICE (child_box.x1)));
      child_box.y1 = CLUTTER_UNITS_FROM_DEVICE ((CLUTTER_UNITS_TO_DEVICE (child_box.y1)));

      child_box.x2 = child_box.x1 + child_width;
      child_box.y2 = child_box.y1 + child_height;

      clutter_actor_allocate (priv->child, &child_box, origin_changed);
    }
}

static void
nbtk_widget_pick (ClutterActor       *actor,
                  const ClutterColor *pick_color)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (actor)->priv;

  /* chain up, so we get a box with our coordinates */
  CLUTTER_ACTOR_CLASS (nbtk_widget_parent_class)->pick (actor, pick_color);

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
}

static void
nbtk_widget_paint (ClutterActor *self)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (self)->priv;

  if (priv->bg_color)
    {
      ClutterActorBox allocation = { 0, };
      ClutterColor bg_color = *priv->bg_color;
      guint w, h;

      bg_color.alpha = clutter_actor_get_paint_opacity (self)
                       * bg_color.alpha / 255;

      clutter_actor_get_allocation_box (self, &allocation);

      w = CLUTTER_UNITS_TO_DEVICE (allocation.x2 - allocation.x1);
      h = CLUTTER_UNITS_TO_DEVICE (allocation.y2 - allocation.y1);

      cogl_color (&bg_color);
      cogl_rectangle (0, 0, w, h);
    }

  if (priv->bg_image)
    clutter_actor_paint (CLUTTER_ACTOR (priv->bg_image));

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
}

static void
nbtk_widget_get_preferred_width (ClutterActor *actor,
                                 ClutterUnit   for_height,
                                 ClutterUnit  *min_width_p,
                                 ClutterUnit  *natural_width_p)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (actor)->priv;
  ClutterUnit min_width, natural_width;

  min_width = 0;
  natural_width = priv->padding.left + priv->padding.right
                + priv->border.left + priv->border.right;

  if (priv->child)
    {
      ClutterUnit child_min, child_natural;

      clutter_actor_get_preferred_width (priv->child, for_height,
                                         &child_min,
                                         &child_natural);

      min_width += child_min;
      natural_width += child_natural;
    }

  if (min_width_p)
    *min_width_p = min_width;

  if (natural_width_p)
    *natural_width_p = natural_width;
}

static void
nbtk_widget_get_preferred_height (ClutterActor *actor,
                                  ClutterUnit   for_width,
                                  ClutterUnit  *min_height_p,
                                  ClutterUnit  *natural_height_p)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (actor)->priv;
  ClutterUnit min_height, natural_height;

  min_height = 0;
  natural_height = priv->padding.top + priv->padding.bottom
                 + priv->border.top + priv->border.bottom;

  if (priv->child)
    {
      ClutterUnit child_min, child_natural;

      clutter_actor_get_preferred_height (priv->child, for_width,
                                          &child_min,
                                          &child_natural);

      min_height += child_min;
      natural_height += child_natural;
    }

  if (min_height_p)
    *min_height_p = min_height;

  if (natural_height_p)
    *natural_height_p = natural_height;
}

static void
nbtk_widget_parent_set (ClutterActor *widget,
                        ClutterActor *old_parent)
{
  ClutterActor *parent;

  parent = clutter_actor_get_parent (widget);

  /* don't send the style changed signal if we no longer have a parent actor */
  if (parent)
    {
      g_signal_emit (widget, actor_signals[STYLE_CHANGED], 0);
    }

  if (CLUTTER_ACTOR_CLASS (nbtk_widget_parent_class)->parent_set)
    CLUTTER_ACTOR_CLASS (nbtk_widget_parent_class)->parent_set (widget, old_parent);
}

static void
nbtk_widget_style_changed (NbtkWidget *self)
{
  NbtkWidgetPrivate *priv = self->priv;
  NbtkPadding *padding = NULL;
  gchar *bg_file;
  gint border_left;
  gint border_right;
  gint border_top;
  gint border_bottom;

  /* cache these values for use in the paint function */
  nbtk_stylable_get (NBTK_STYLABLE (self),
                    "background-color", &priv->bg_color,
                    "background-image", &bg_file,
                    "border-top-width", &border_top,
                    "border-bottom-width", &border_bottom,
                    "border-right-width", &border_right,
                    "border-left-width", &border_left,
                    "padding", &padding,
                    NULL);

  if (padding && !priv->override_css_padding)
    priv->padding = *padding;

  priv->border.left = CLUTTER_UNITS_FROM_INT (border_left);
  priv->border.right = CLUTTER_UNITS_FROM_INT (border_right);
  priv->border.top = CLUTTER_UNITS_FROM_INT (border_top);
  priv->border.bottom = CLUTTER_UNITS_FROM_INT (border_bottom);

  if (priv->bg_image)
    {
       clutter_actor_unparent (CLUTTER_ACTOR (priv->bg_image));
       priv->bg_image = NULL;
    }

  if (bg_file)
    {
      NbtkTextureCache *texture_cache;
      ClutterActor *texture;


      texture_cache = nbtk_texture_cache_get_default ();
      texture = nbtk_texture_cache_get_texture (texture_cache,
                                                bg_file,
                                                FALSE);
      priv->bg_image = nbtk_texture_frame_new (CLUTTER_TEXTURE (texture),
                                               border_left,
                                               border_top,
                                               border_right,
                                               border_bottom);
      clutter_actor_set_parent (CLUTTER_ACTOR (priv->bg_image), CLUTTER_ACTOR (self));

      g_free (bg_file);
    }

  clutter_actor_queue_relayout ((ClutterActor *)self);
}

static void
nbtk_widget_dnd_dropped (NbtkWidget   *actor,
			 ClutterActor *dragged,
			 ClutterActor *icon,
			 gint          x,
			 gint          y)
{
  ClutterActor *parent;

  /*
   * NbtkWidget as such does not support DND, so if the default handler is
   * called, we try to propagate the signal down the ancestry chain.
   */

  parent = clutter_actor_get_parent (CLUTTER_ACTOR (actor));
  while (parent && !NBTK_IS_WIDGET (parent))
    parent = clutter_actor_get_parent (parent);

  if (parent)
    {
      /*
       * We found a parent that is a NbtkWidget; call its handler instead.
       */
      NbtkWidgetClass *klass = NBTK_WIDGET_GET_CLASS (parent);

      if (klass->dnd_dropped)
	klass->dnd_dropped (NBTK_WIDGET (parent), dragged, icon, x, y);
    }
}

static void
nbtk_widget_class_init (NbtkWidgetClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkWidgetPrivate));

  gobject_class->set_property = nbtk_widget_set_property;
  gobject_class->get_property = nbtk_widget_get_property;
  gobject_class->dispose = nbtk_widget_dispose;

  actor_class->allocate = nbtk_widget_allocate;
  actor_class->pick = nbtk_widget_pick;
  actor_class->paint = nbtk_widget_paint;
  actor_class->get_preferred_height = nbtk_widget_get_preferred_height;
  actor_class->get_preferred_width = nbtk_widget_get_preferred_width;
  actor_class->parent_set = nbtk_widget_parent_set;

  klass->style_changed = nbtk_widget_style_changed;
  klass->dnd_dropped = nbtk_widget_dnd_dropped;

  /**
   * NbtkWidget:padding:
   *
   * Padding around an actor, expressed in #ClutterUnit<!-- -->s. Padding
   * is the internal space between an actors bounding box and its internal
   * children.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_PADDING,
                                   g_param_spec_boxed ("padding",
                                                       "Padding",
                                                       "Units of padding around an actor",
                                                       NBTK_TYPE_PADDING,
                                                       NBTK_PARAM_READWRITE));
  /**
   * NbtkWidget:x-align:
   *
   * Alignment of internal children along the X axis, relative to the
   * actor's bounding box origin, and in relative units (1.0 is the
   * current width of the actor).
   *
   * A value of 0.0 will left-align the children; 0.5 will align them at
   * the middle of the actor's width; 1.0 will right align the children.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_X_ALIGN,
                                   g_param_spec_double ("x-align",
                                                        "X Alignment",
                                                        "Alignment (between 0.0 and 1.0) on the X axis",
                                                        0.0, 1.0, 0.5,
                                                        NBTK_PARAM_READWRITE));
  /**
   * NbtkWidget:y-align:
   *
   * Alignment of internal children along the Y axis, relative to the
   * actor's bounding box origin, and in relative units (1.0 is the
   * current height of the actor).
   *
   * A value of 0.0 will top-align the children; 0.5 will align them at
   * the middle of the actor's height; 1.0 will bottom align the children.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_Y_ALIGN,
                                   g_param_spec_double ("y-align",
                                                        "Y Alignement",
                                                        "Alignment (between 0.0 and 1.0) on the Y axis",
                                                        0.0, 1.0, 0.5,
                                                        NBTK_PARAM_READWRITE));

  /**
   * NbtkWidget:pseudo-class:
   *
   * The pseudo-class of the actor. Typical values include "hover", "active",
   * "focus".
   */
  g_object_class_install_property (gobject_class,
                                   PROP_PSEUDO_CLASS,
                                   g_param_spec_string ("pseudo-class",
                                                        "Pseudo Class",
                                                        "Pseudo class for styling",
                                                        "",
                                                        NBTK_PARAM_READWRITE));
  /**
   * NbtkWidget:style-class:
   *
   * The style-class of the actor for use in styling.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_STYLE_CLASS,
                                   g_param_spec_string ("style-class",
                                                        "Style Class",
                                                        "Style class for styling",
                                                        "",
                                                        NBTK_PARAM_READWRITE));

  g_object_class_override_property (gobject_class, PROP_STYLE, "style");

  /**
   * NbtkWidget:dnd-threshold:
   *
   * The threshold which needs to be exceeded before motion is treated as
   * drag. Value of 0 indicates that drag and drop is disabled.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DND_THRESHOLD,
                                   g_param_spec_uint ("dnd-threshold",
						      "DND threshold",
						      "DND threshold",
                                                       0, G_MAXUINT, 0,
						      NBTK_PARAM_READWRITE));


  /**
   * NbtkWidget:dnd-icon:
   *
   * An #ClutterActor icon to use during drag and drop. When this property is
   * not set, a clone of the dragged item is used.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DND_ICON,
                                   g_param_spec_object ("dnd-icon",
							"DND Icon",
							"Icon to use for DND",
							CLUTTER_TYPE_ACTOR,
							NBTK_PARAM_READWRITE));

  /**
   * NbtkWidget::style-changed:
   * @actor: the actor that received the signal
   *
   * The ::style-changed signal will be emitted each time the style for the
   * object changes. This includes when any of the properties linked to the
   * style change, including #pseudo-class and #style-class
   */
  actor_signals[STYLE_CHANGED] =
    g_signal_new (I_("style-changed"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, style_changed),
                  NULL, NULL,
                  _nbtk_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * NbtkWidget::dnd-begin:
   * @actor: the #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-begin signal will be emitted each time a child of the container
   * enters into drag and drop state. When the drag and drop state ends,
   * the ::dnd-end signal is issued.
   */
  actor_signals[DND_BEGIN] =
    g_signal_new (I_("dnd-begin"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_begin),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);

  /**
   * NbtkWidget::dnd-motion:
   * @actor: #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-motion signal will be emitted each time a child of the container
   * changes is position due to dragging.
   */
  actor_signals[DND_MOTION] =
    g_signal_new (I_("dnd-motion"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_motion),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);

  /**
   * NbtkWidget::dnd-end:
   * @actor: #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-end signal will be emitted each time a child of the container
   * exists a drag and drop state. If the signal is issued due to successful
   * drop, the ::dnd-drop signal will be issued before the ::dnd-end signal.
   */
  actor_signals[DND_END] =
    g_signal_new (I_("dnd-end"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_end),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);

  /**
   * NbtkWidget::dnd-dropped:
   * @actor: #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-drop signal will be emitted before the ::dnd-end signal if
   * the drag resulted in a successful drop.
   */
  actor_signals[DND_DROPPED] =
    g_signal_new (I_("dnd-dropped"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_dropped),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);

  /**
   * NbtkWidget::dnd-enter:
   * @actor: #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-enter signal will be emitted each time the dragged child enters
   * the bounding box of a dnd-enabled NbtkWidget.
   */
  actor_signals[DND_ENTER] =
    g_signal_new (I_("dnd-enter"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_enter),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);

  /**
   * NbtkWidget::dnd-leave:
   * @actor: #ClutterActor that received the signal
   * @dragged: #ClutterActor that is being dragged.
   * @icon: #ClutterActor representing the actor being dragged.
   * @x:     x coordinate of the event
   * @y:     y coordinate of the event
   *
   * The ::dnd-leave signal will be emitted each time a dragged child of the
   * container leaves the bounding box of a dnd-enabled NbtkWidget.
   */
  actor_signals[DND_LEAVE] =
    g_signal_new (I_("dnd-leave"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkWidgetClass, dnd_leave),
                  NULL, NULL,
                  _nbtk_marshal_VOID__OBJECT_OBJECT_INT_INT,
                  G_TYPE_NONE, 4,
		  CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR,
		  G_TYPE_INT, G_TYPE_INT);
}

static NbtkStyle *
nbtk_widget_get_style (NbtkStylable *stylable)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (stylable)->priv;

  if (!priv->style)
    priv->style = g_object_ref (nbtk_style_get_default ());

  return priv->style;
}

static void
nbtk_widget_set_style (NbtkStylable *stylable,
                       NbtkStyle    *style)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (stylable)->priv;

  if (priv->style)
    g_object_unref (priv->style);

  priv->style = g_object_ref_sink (style);
}

static NbtkStylable*
nbtk_widget_get_container (NbtkStylable *stylable)
{
  ClutterActor *parent;

  g_return_val_if_fail (NBTK_IS_WIDGET (stylable), NULL);

  parent = clutter_actor_get_parent (CLUTTER_ACTOR (stylable));

  if (NBTK_IS_STYLABLE (parent))
    {
      return NBTK_STYLABLE (parent);
    }
  else
    {
      return NULL;
    }
}

static NbtkStylable*
nbtk_widget_get_base_style (NbtkStylable *stylable)
{
  return NULL;
}

static const gchar*
nbtk_widget_get_style_id (NbtkStylable *stylable)
{
  g_return_val_if_fail (NBTK_IS_WIDGET (stylable), NULL);

  return clutter_actor_get_name (CLUTTER_ACTOR (stylable));
}

static const gchar*
nbtk_widget_get_style_type (NbtkStylable *stylable)
{
  return G_OBJECT_TYPE_NAME (stylable);
}

static const gchar*
nbtk_widget_get_style_class (NbtkStylable *stylable)
{
  g_return_val_if_fail (NBTK_IS_WIDGET (stylable), NULL);

  return NBTK_WIDGET (stylable)->priv->style_class;
}

static const gchar*
nbtk_widget_get_pseudo_class (NbtkStylable *stylable)
{
  g_return_val_if_fail (NBTK_IS_WIDGET (stylable), NULL);

  return NBTK_WIDGET (stylable)->priv->pseudo_class;
}

static gboolean
nbtk_widget_get_viewport (NbtkStylable *stylable,
                          gint *x,
                          gint *y,
                          gint *width,
                          gint *height)
{
  g_return_val_if_fail (NBTK_IS_WIDGET (stylable), FALSE);

  *x = 0;
  *y = 0;

  *width = clutter_actor_get_width (CLUTTER_ACTOR (stylable));
  *height = clutter_actor_get_height (CLUTTER_ACTOR (stylable));

  return TRUE;
}

/**
 * nbtk_widget_set_style_class_name:
 * @actor: a #NbtkWidget
 * @pseudo_class: a new pseudo class string
 *
 * Set the style class name
 */
void
nbtk_widget_set_style_class_name (NbtkWidget  *actor,
                                  const gchar *style_class)
{
  g_return_if_fail (NBTK_WIDGET (actor));

  if (g_strcmp0 (style_class, actor->priv->style_class))
    {
      g_free (actor->priv->style_class);
      actor->priv->style_class = g_strdup (style_class);
      g_signal_emit (actor, actor_signals[STYLE_CHANGED], 0);
    }
}


/**
 * nbtk_widget_get_style_class_name:
 * @actor: a #NbtkWidget
 *
 * Get the current style class name
 *
 * Returns: the class name string. The string is owned by the #NbtkWidget and
 * should not be modified or freed.
 */
const gchar*
nbtk_widget_get_style_class_name (NbtkWidget *actor)
{
  g_return_val_if_fail (NBTK_WIDGET (actor), NULL);

  return actor->priv->style_class;
}

/**
 * nbtk_widget_get_style_pseudo_class:
 * @actor: a #NbtkWidget
 *
 * Get the current style pseudo class
 *
 * Returns: the pseudo class string. The string is owned by the #NbtkWidget and
 * should not be modified or freed.
 */
const gchar*
nbtk_widget_get_style_pseudo_class (NbtkWidget *actor)
{
  g_return_val_if_fail (NBTK_WIDGET (actor), NULL);

  return actor->priv->pseudo_class;
}

/**
 * nbtk_widget_set_style_pseudo_class:
 * @actor: a #NbtkWidget
 * @pseudo_class: a new pseudo class string
 *
 * Set the style pseudo class
 */
void
nbtk_widget_set_style_pseudo_class (NbtkWidget  *actor,
                                    const gchar *pseudo_class)
{
  g_return_if_fail (NBTK_WIDGET (actor));

  if (g_strcmp0 (pseudo_class, actor->priv->pseudo_class))
    {
      g_free (actor->priv->pseudo_class);
      actor->priv->pseudo_class = g_strdup (pseudo_class);
      g_signal_emit (actor, actor_signals[STYLE_CHANGED], 0);
    }
}


static void
nbtk_stylable_iface_init (NbtkStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;
      ClutterColor color = { 0x00, 0x00, 0x00, 0xff };
      ClutterColor bg_color = { 0xff, 0xff, 0xff, 0x00 };

      pspec = clutter_param_spec_color ("background-color",
                                  "Background Color",
                                  "The background color of an actor",
                                  &bg_color,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = clutter_param_spec_color ("color",
                                  "Text Color",
                                  "The color of the text of an actor",
                                  &color,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_string ("background-image",
                                   "Background Image",
                                   "Background image filename",
                                   NULL,
                                   G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_string ("font-family",
                                   "Font Family",
                                   "Name of the font to use",
                                   "Sans",
                                   G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("font-size",
                                "Font Size",
                                "Size of the font to use in pixels",
                                0, G_MAXINT, 12,
                                G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("border-left-width",
                                "Border Left Width",
                                "Left border of the image",
                                0, G_MAXINT, 0,
                                G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("border-right-width",
                                "Border Right Width",
                                "Right border of the image",
                                0, G_MAXINT, 0,
                                G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("border-top-width",
                                "Border Top Width",
                                "Top border of the image",
                                0, G_MAXINT, 0,
                                G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("border-bottom-width",
                                "Border Bottom Width",
                                "Bottom border of the image",
                                0, G_MAXINT, 0,
                                G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      pspec = g_param_spec_boxed ("padding",
                                  "Padding",
                                  "Padding between the widgets borders and its content",
                                  NBTK_TYPE_PADDING,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_WIDGET, pspec);

      iface->get_style = nbtk_widget_get_style;
      iface->set_style = nbtk_widget_set_style;
      iface->get_base_style = nbtk_widget_get_base_style;
      iface->get_container = nbtk_widget_get_container;
      iface->get_style_id = nbtk_widget_get_style_id;
      iface->get_style_type = nbtk_widget_get_style_type;
      iface->get_style_class = nbtk_widget_get_style_class;
      iface->get_pseudo_class = nbtk_widget_get_pseudo_class;
      /* iface->get_attribute = nbtk_widget_get_attribute; */
      iface->get_viewport = nbtk_widget_get_viewport;
    }
}


static void
nbtk_widget_name_notify (NbtkWidget *widget,
                         GParamSpec *pspec,
                         gpointer data)
{
  g_signal_emit (widget, actor_signals[STYLE_CHANGED], 0);
}

static void
nbtk_widget_init (NbtkWidget *actor)
{
  NbtkWidgetPrivate *priv;

  actor->priv = priv = NBTK_WIDGET_GET_PRIVATE (actor);

  /* no padding */
  priv->padding.top = priv->padding.bottom = 0;
  priv->padding.right = priv->padding.left = 0;
  priv->override_css_padding = FALSE;

  /* middle align */
  priv->x_align = priv->y_align = CLUTTER_FLOAT_TO_FIXED (0.5);

  clutter_actor_set_reactive (CLUTTER_ACTOR (actor), TRUE);

  /* connect style changed */
  g_signal_connect (actor, "notify::name", G_CALLBACK (nbtk_widget_name_notify), NULL);

}

/**
 * nbtk_widget_set_padding:
 * @actor: a #NbtkWidget
 * @padding: padding for internal children or %NULL to clear previously set padding.
 *
 * Sets @padding around @actor.
 */
void
nbtk_widget_set_padding (NbtkWidget        *actor,
                         const NbtkPadding *padding)
{
  g_return_if_fail (NBTK_IS_WIDGET (actor));

  actor->priv->override_css_padding = (gboolean) padding;

  if (padding)
    actor->priv->padding = *padding;
  else
    {
      /* Reset back to CSS-provided padding. */
      NbtkPadding *css_padding = NULL;
      nbtk_stylable_get (NBTK_STYLABLE (actor),
                         "padding", &css_padding,
                         NULL);
      if (css_padding)
        {
          actor->priv->padding = *css_padding;
          g_boxed_free (NBTK_TYPE_PADDING, css_padding);
        }
    }

  g_object_notify (G_OBJECT (actor), "padding");

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_relayout (CLUTTER_ACTOR (actor));
}

/**
 * nbtk_widget_get_padding:
 * @actor: a #NbtkWidget
 * @padding: return location for the padding
 *
 * Retrieves the padding aound @actor.
 */
void
nbtk_widget_get_padding (NbtkWidget  *actor,
                         NbtkPadding *padding)
{
  g_return_if_fail (NBTK_IS_WIDGET (actor));
  g_return_if_fail (padding != NULL);

  *padding = actor->priv->padding;
}

/**
 * nbtk_widget_set_alignment:
 * @actor: a #NbtkWidget
 * @x_align: relative alignment on the X axis
 * @y_align: relative alignment on the Y axis
 *
 * Sets the alignment, relative to the @actor's width and height, of
 * the internal children.
 */
void
nbtk_widget_set_alignment (NbtkWidget *actor,
                           gdouble     x_align,
                           gdouble     y_align)
{
  NbtkWidgetPrivate *priv;

  g_return_if_fail (NBTK_IS_WIDGET (actor));

  g_object_ref (actor);
  g_object_freeze_notify (G_OBJECT (actor));

  priv = actor->priv;

  x_align = CLAMP (x_align, 0.0, 1.0);
  y_align = CLAMP (y_align, 0.0, 1.0);

  priv->x_align = CLUTTER_FLOAT_TO_FIXED (x_align);
  g_object_notify (G_OBJECT (actor), "x-align");

  priv->y_align = CLUTTER_FLOAT_TO_FIXED (y_align);
  g_object_notify (G_OBJECT (actor), "y-align");

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (actor));

  g_object_thaw_notify (G_OBJECT (actor));
  g_object_unref (actor);
}

/**
 * nbtk_widget_get_alignment:
 * @actor: a #NbtkWidget
 * @x_align: return location for the relative alignment on the X axis,
 *   or %NULL
 * @y_align: return location for the relative alignment on the Y axis,
 *   or %NULL
 *
 * Retrieves the alignment, relative to the @actor's width and height, of
 * the internal children.
 */
void
nbtk_widget_get_alignment (NbtkWidget *actor,
                           gdouble    *x_align,
                           gdouble    *y_align)
{
  NbtkWidgetPrivate *priv;

  g_return_if_fail (NBTK_IS_WIDGET (actor));

  priv = actor->priv;

  if (x_align)
    *x_align = CLUTTER_FIXED_TO_FLOAT (priv->x_align);

  if (y_align)
    *y_align = CLUTTER_FIXED_TO_FLOAT (priv->y_align);
}

/**
 * nbtk_widget_set_alignmentx:
 * @actor: a #NbtkWidget
 * @x_align: relative alignment on the X axis
 * @y_align: relative alignment on the Y axis
 *
 * Fixed point version of nbtk_widget_set_alignment().
 *
 * Sets the alignment, relative to the @actor's width and height, of
 * the internal children.
 */
void
nbtk_widget_set_alignmentx (NbtkWidget   *actor,
                            ClutterFixed  x_align,
                            ClutterFixed  y_align)
{
  NbtkWidgetPrivate *priv;

  g_return_if_fail (NBTK_IS_WIDGET (actor));

  g_object_ref (actor);
  g_object_freeze_notify (G_OBJECT (actor));

  priv = actor->priv;

  x_align = CLAMP (x_align, 0, CFX_ONE);
  y_align = CLAMP (y_align, 0, CFX_ONE);

  if (priv->x_align != x_align)
    {
      priv->x_align = x_align;
      g_object_notify (G_OBJECT (actor), "x-align");
    }

  if (priv->y_align != y_align)
    {
      priv->y_align = y_align;
      g_object_notify (G_OBJECT (actor), "y-align");
    }

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (actor));

  g_object_thaw_notify (G_OBJECT (actor));
  g_object_unref (actor);
}

/**
 * nbtk_widget_get_alignmentx:
 * @actor: a #NbtkWidget
 * @x_align: return location for the relative alignment on the X axis,
 *   or %NULL
 * @y_align: return location for the relative alignment on the Y axis,
 *   or %NULL
 *
 * Fixed point version of nbtk_widget_get_alignment().
 *
 * Retrieves the alignment, relative to the @actor's width and height, of
 * the internal children.
 */
void
nbtk_widget_get_alignmentx (NbtkWidget   *actor,
                            ClutterFixed *x_align,
                            ClutterFixed *y_align)
{
  NbtkWidgetPrivate *priv;

  g_return_if_fail (NBTK_IS_WIDGET (actor));

  priv = actor->priv;

  if (x_align)
    *x_align = priv->x_align;

  if (y_align)
    *y_align = priv->y_align;
}

/**
 * nbtk_widget_get_dnd_threshold:
 * @actor: a #NbtkWidget
 *
 * Returns: the current threshold.
 * Retrieves the drag and drop threshold.
 */
guint
nbtk_widget_get_dnd_threshold (NbtkWidget *actor)
{
  g_return_val_if_fail (NBTK_IS_WIDGET (actor), 0);

  return actor->priv->dnd_threshold;
}

/**
 * nbtk_widget_set_dnd_threshold:
 * @actor: a #NbtkWidget
 * @threshold: the threshold.
 *
 * Sets the drag and drop threshold.
 */
void
nbtk_widget_set_dnd_threshold (NbtkWidget *actor, guint threshold)
{
  NbtkWidgetPrivate *priv;

  g_return_if_fail (NBTK_IS_WIDGET (actor));

  priv = actor->priv;

  if (priv->dnd_threshold != threshold)
    {
      priv->dnd_threshold = threshold;

      g_object_notify (G_OBJECT (actor), "dnd-threshold");
    }
}

static gboolean
nbtk_widget_dnd_enter_event_cb (ClutterActor *actor,
				ClutterCrossingEvent *event,
				gpointer data)
{
  ClutterActor *dest = event->source;
  NbtkWidgetPrivate *priv = NBTK_WIDGET (data)->priv;

  while (dest && (!NBTK_IS_WIDGET (dest) ||
		  !NBTK_WIDGET (dest)->priv->dnd_threshold))
    dest = clutter_actor_get_parent (dest);

  if (dest && NBTK_IS_WIDGET (dest) && NBTK_WIDGET (dest)->priv->dnd_threshold)
    {
      if (dest != priv->dnd_last_dest)
	{
	  g_debug ("Enter event on %p (%s)\n", dest, G_OBJECT_TYPE_NAME (dest));

	  if (priv->dnd_last_dest)
	    {
	      g_object_ref (priv->dnd_last_dest);

	      g_signal_emit (priv->dnd_last_dest, actor_signals[DND_LEAVE], 0,
			     priv->dnd_dragged,
			     priv->dnd_clone, event->x, event->y);

	      g_object_unref (priv->dnd_last_dest);
	    }

	  g_object_ref (dest);

	  g_signal_emit (dest, actor_signals[DND_ENTER], 0,
			 priv->dnd_dragged,
			 priv->dnd_clone, event->x, event->y);

	  g_object_unref (dest);

	  priv->dnd_last_dest = dest;
	}
    }
  else if (priv->dnd_last_dest)
    {
      g_object_ref (priv->dnd_last_dest);

      g_signal_emit (priv->dnd_last_dest, actor_signals[DND_LEAVE], 0,
		     priv->dnd_dragged,
		     priv->dnd_clone, event->x, event->y);

      g_object_unref (priv->dnd_last_dest);

      priv->dnd_last_dest = NULL;
    }

  return TRUE;
}

static gboolean
nbtk_widget_child_dnd_motion_cb (ClutterActor *child,
				 ClutterEvent *event,
				 gpointer      data);

static gboolean
nbtk_widget_child_dnd_release_cb (ClutterActor *child,
				  ClutterEvent *event,
				  gpointer      data)
{
  NbtkWidget *widget = data;
  NbtkWidgetPrivate *priv;
  gboolean retval = FALSE;

  if (event->type != CLUTTER_BUTTON_RELEASE)
    return FALSE;

  priv = NBTK_WIDGET (widget)->priv;

  if (priv->dnd_motion)
    {
      ClutterActor *dest;
      ClutterStage *stage;
      ClutterActor *clone;

      gint x = event->motion.x;
      gint y = event->motion.y;

      clone = priv->dnd_clone;

      /*
       * Hide the clone, so it does not interfer with picking.
       */
      clutter_actor_hide (clone);

      stage = CLUTTER_STAGE (clutter_actor_get_stage (child));

      dest = clutter_stage_get_actor_at_pos (stage, x, y);

      if (dest)
	{
	  /*
	   * If the target is not NbtkWidget, or the widget does not have dnd
	   * enabled, we try to propagate the signal down the ancestry chain.
	   */
	  if (!NBTK_IS_WIDGET (dest) ||
	      !NBTK_WIDGET (dest)->priv->dnd_threshold)
	    {
	      dest = clutter_actor_get_parent (dest);

	      while (dest && (!NBTK_IS_WIDGET (dest) ||
			      !NBTK_WIDGET (dest)->priv->dnd_threshold))
		dest = clutter_actor_get_parent (dest);
	    }

	  if (dest)
	    {
	      g_object_ref (dest);

	      g_signal_emit (dest, actor_signals[DND_DROPPED], 0, child, clone,
			     x, y);

	      g_object_unref (dest);
	    }
	}

      if (priv->dnd_enter_cb_id)
	{
	  g_signal_handler_disconnect (priv->dnd_dragged,
				       priv->dnd_enter_cb_id);
	  priv->dnd_enter_cb_id = 0;
	}

      g_object_ref (widget);

      g_signal_emit (widget, actor_signals[DND_END], 0,
		     child, clone, x, y);

      g_object_unref (widget);

      priv->dnd_clone = NULL;
      g_object_unref (clone);

      if (clone != priv->dnd_icon)
	clutter_actor_destroy (clone);
      else
	clutter_actor_unparent (clone);

      retval = TRUE;
    }

  if (priv->dnd_grab)
    clutter_ungrab_pointer ();

  priv->dnd_motion = FALSE;
  priv->dnd_grab = FALSE;

  g_signal_handlers_disconnect_by_func (child,
                                        nbtk_widget_child_dnd_motion_cb,
                                        data);

  if (priv->dnd_dragged)
    {
      g_object_unref (priv->dnd_dragged);
      priv->dnd_dragged = NULL;
    }

  return retval;
}

static gboolean
nbtk_widget_child_dnd_motion_cb (ClutterActor *child,
				 ClutterEvent *event,
				 gpointer      data)
{
  NbtkWidget *widget = data;
  NbtkWidgetPrivate *priv = NBTK_WIDGET (widget)->priv;
  gint x = event->motion.x;
  gint y = event->motion.y;
  gint dx = x - priv->dnd_x;
  gint dy = y - priv->dnd_y;
  guint threshold = priv->dnd_threshold;
  ClutterStage *stage;

  if (dx < threshold && dy < threshold)
    return FALSE;

  stage = CLUTTER_STAGE (clutter_actor_get_stage (CLUTTER_ACTOR (widget)));

  if (!priv->dnd_motion)
    {
      ClutterActor *clone;
      gdouble scale_x, scale_y;
      guint child_w, child_h;

      priv->dnd_last_dest = CLUTTER_ACTOR (widget);

      if (priv->dnd_icon)
	{
	  clone = priv->dnd_icon;
	}
      else
	{
	  ClutterTexture *parent_tx;

	  parent_tx = clutter_clone_texture_get_parent_texture (
						 CLUTTER_CLONE_TEXTURE (child));

	  clone = clutter_clone_texture_new (parent_tx);

	  clutter_actor_get_scale (child, &scale_x, &scale_y);
	  clutter_actor_set_scale (clone, scale_x, scale_y);

	  clutter_actor_get_size  (child, &child_w, &child_h);
	  clutter_actor_set_size  (clone, child_w, child_h);
	}

      g_signal_connect (child, "button-release-event",
			G_CALLBACK (nbtk_widget_child_dnd_release_cb), widget);

      priv->dnd_clone = clone;

      clutter_actor_set_position (clone, x, y);

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), clone);
      g_object_ref (clone);

      clutter_actor_show (clone);

      g_object_ref (widget);

      g_signal_emit (widget, actor_signals[DND_BEGIN], 0, child, clone, x, y);

      g_object_unref (widget);

      priv->dnd_motion = TRUE;
    }
  else
    {
      clutter_actor_move_by (priv->dnd_clone, dx, dy);
      clutter_actor_queue_redraw (priv->dnd_clone);
    }

  g_object_ref (widget);

  g_signal_emit (widget, actor_signals[DND_MOTION], 0,
		 child, priv->dnd_clone, x, y);

  g_object_unref (widget);

  priv->dnd_x = x;
  priv->dnd_y = y;

  return TRUE;
}

static gboolean
nbtk_widget_child_dnd_press_cb (ClutterActor *child,
				ClutterEvent *event,
				gpointer      data)
{
  NbtkWidget *widget = data;
  NbtkWidgetPrivate *priv = NBTK_WIDGET (widget)->priv;
  guint threshold = priv->dnd_threshold;

  if (!threshold)
    return FALSE;

  priv->dnd_x = event->button.x;
  priv->dnd_y = event->button.y;

  priv->dnd_motion = FALSE;
  priv->dnd_grab = TRUE;

  priv->dnd_dragged = g_object_ref (child);

  clutter_grab_pointer (child);

  g_signal_connect (child, "motion-event",
                    G_CALLBACK (nbtk_widget_child_dnd_motion_cb), data);

  priv->dnd_enter_cb_id =
    g_signal_connect (child, "enter-event",
		      G_CALLBACK (nbtk_widget_dnd_enter_event_cb), data);

  return TRUE;
}

ClutterActor *
_nbtk_widget_get_dnd_clone (NbtkWidget *widget)
{
  NbtkWidgetPrivate *priv = NBTK_WIDGET (widget)->priv;

  return priv->dnd_clone;
}


/**
 * nbtk_widget_setup_child_dnd:
 * @actor: a #NbtkWidget
 * @child: a #ClutterActor
 *
 * Sets up child for drag and drop; this function is intended
 * for use by NbtkWidget subclasses that wish to support DND.
 */
void
nbtk_widget_setup_child_dnd (NbtkWidget *actor, ClutterActor *child)
{
  g_signal_connect (child, "button-press-event",
		    G_CALLBACK (nbtk_widget_child_dnd_press_cb), actor);
}

/**
 * nbtk_widget_undo_child_dnd:
 * @actor: a #NbtkWidget
 * @child: a #ClutterActor
 *
 * Removes the DND machinery from the child; this function is intended
 * for use by NbtkWidget subclasses that wish to support DND.
 */
void
nbtk_widget_undo_child_dnd (NbtkWidget *actor, ClutterActor *child)
{
  g_signal_handlers_disconnect_by_func (child,
                                        nbtk_widget_child_dnd_press_cb,
                                        actor);
  g_signal_handlers_disconnect_by_func (child,
                                        nbtk_widget_child_dnd_release_cb,
                                        actor);
}

static NbtkPadding *
nbtk_padding_copy (const NbtkPadding *padding)
{
  NbtkPadding *copy;

  g_return_val_if_fail (padding != NULL, NULL);

  copy = g_slice_new (NbtkPadding);
  *copy = *padding;

  return copy;
}

static void
nbtk_padding_free (NbtkPadding *padding)
{
  if (G_LIKELY (padding))
    g_slice_free (NbtkPadding, padding);
}

GType
nbtk_padding_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    our_type =
      g_boxed_type_register_static (I_("NbtkPadding"),
                                    (GBoxedCopyFunc) nbtk_padding_copy,
                                    (GBoxedFreeFunc) nbtk_padding_free);

  return our_type;
}
