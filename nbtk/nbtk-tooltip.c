/* nbtk-tooltip.c: Plain tooltip actor
 *
 * Copyright 2008 Intel Corporation
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 */

/**
 * SECTION:nbtk-tooltip
 * @short_description: A tooltip widget
 *
 * #NbtkTooltip implements a single tooltip. It should not normally be created
 * by the application but by the widget implementing tooltip capabilities, for
 * example, #nbtk_button_set_tooltip().
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include "nbtk-tooltip.h"

#include "nbtk-widget.h"
#include "nbtk-stylable.h"

enum
{
  PROP_0,

  PROP_LABEL,
  PROP_WIDGET,
};

#define NBTK_TOOLTIP_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_TOOLTIP, NbtkTooltipPrivate))

struct _NbtkTooltipPrivate
{
  ClutterActor    *label;
  NbtkWidget      *widget;
};

G_DEFINE_TYPE (NbtkTooltip, nbtk_tooltip, NBTK_TYPE_BIN)

static void
nbtk_tooltip_set_property (GObject      *gobject,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  NbtkTooltip *tooltip = NBTK_TOOLTIP (gobject);

  switch (prop_id)
    {
    case PROP_LABEL:
      nbtk_tooltip_set_label (tooltip, g_value_get_string (value));
      break;

    case PROP_WIDGET:
      nbtk_tooltip_set_widget (tooltip, g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_tooltip_get_property (GObject    *gobject,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  NbtkTooltipPrivate *priv = NBTK_TOOLTIP (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, clutter_text_get_text (CLUTTER_TEXT (priv->label)));
      break;

    case PROP_WIDGET:
      g_value_set_pointer (value, priv->widget);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_tooltip_style_changed (NbtkWidget *self)
{
  ClutterColor *color = NULL;
  NbtkTooltipPrivate *priv;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  priv = NBTK_TOOLTIP (self)->priv;

  nbtk_stylable_get (NBTK_STYLABLE (self),
                     "color", &color,
                     "font-family", &font_name,
                     "font-size", &font_size,
                     NULL);

  if (color)
    {
      clutter_text_set_color (CLUTTER_TEXT (priv->label), color);
      clutter_color_free (color);
    }

  if (font_name || font_size)
    {
      if (font_name && font_size)
        {
          font_string = g_strdup_printf ("%s %dpx", font_name, font_size);
          g_free (font_name);
        }
      else
        if (font_size)
          font_string = g_strdup_printf ("%dpx", font_size);
        else
          font_string = font_name;

      clutter_text_set_font_name (CLUTTER_TEXT (priv->label), font_string);

      g_free (font_string);
    }

  if (NBTK_WIDGET_CLASS (nbtk_tooltip_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_tooltip_parent_class)->style_changed (self);
}

static void
nbtk_tooltip_class_init (NbtkTooltipClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkTooltipPrivate));

  gobject_class->set_property = nbtk_tooltip_set_property;
  gobject_class->get_property = nbtk_tooltip_get_property;

  widget_class->style_changed = nbtk_tooltip_style_changed;

  pspec = g_param_spec_string ("label",
                               "Label",
                               "Label of the tooltip",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

  pspec = g_param_spec_pointer ("widget",
                                "Widget",
                                "Widget the tooltip is associated with",
                                G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_WIDGET, pspec);
}


static void
nbtk_tooltip_weak_ref_notify (gpointer tooltip, GObject *obj)
{
  NbtkTooltipPrivate *priv = NBTK_TOOLTIP (tooltip)->priv;

  priv->widget = NULL;

  if (!clutter_actor_get_parent (CLUTTER_ACTOR (tooltip)))
    {
      g_object_ref_sink (G_OBJECT (tooltip));
      g_object_unref (G_OBJECT (tooltip));
    }
  else
    {
      ClutterActor *actor = CLUTTER_ACTOR (tooltip);
      ClutterActor *parent = clutter_actor_get_parent (actor);

      if (CLUTTER_IS_CONTAINER (parent))
        clutter_container_remove_actor (CLUTTER_CONTAINER (parent), actor);
      else
        clutter_actor_unparent (actor);
    }
}

static void
nbtk_tooltip_init (NbtkTooltip *tooltip)
{
  tooltip->priv = NBTK_TOOLTIP_GET_PRIVATE (tooltip);

  tooltip->priv->label = g_object_new (CLUTTER_TYPE_TEXT,
                                       "line-alignment", PANGO_ALIGN_CENTER,
                                       "ellipsize", PANGO_ELLIPSIZE_MIDDLE,
                                       "use-markup", TRUE,
                                       NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (tooltip),
                               CLUTTER_ACTOR (tooltip->priv->label));

  g_object_set (tooltip, "show-on-set-parent", FALSE, NULL);

  clutter_actor_set_reactive (CLUTTER_ACTOR (tooltip), FALSE);
}


/**
 * nbtk_tooltip_get_label:
 * @tooltip: a #NbtkTooltip
 *
 * Get the text displayed on the tooltip
 *
 * Returns: the text for the tooltip. This must not be freed by the application
 */
G_CONST_RETURN gchar *
nbtk_tooltip_get_label (NbtkTooltip *tooltip)
{
  g_return_val_if_fail (NBTK_IS_TOOLTIP (tooltip), NULL);

  return clutter_text_get_text (CLUTTER_TEXT (tooltip->priv->label));
}

/**
 * nbtk_tooltip_set_label:
 * @tooltip: a #NbtkTooltip
 * @text: text to set the label to
 *
 * Sets the text displayed on the tooltip
 */
void
nbtk_tooltip_set_label (NbtkTooltip *tooltip,
                        const gchar *text)
{
  NbtkTooltipPrivate *priv;

  g_return_if_fail (NBTK_IS_TOOLTIP (tooltip));

  priv = tooltip->priv;

  clutter_text_set_text (CLUTTER_TEXT (priv->label), text);

  g_object_notify (G_OBJECT (tooltip), "label");
}

/**
 * nbtk_tooltip_get_widget:
 * @tooltip: a #NbtkTooltip
 *
 * Get the widget associated with the tooltip
 *
 * Returns: the associated tooltip
 */
NbtkWidget*
nbtk_tooltip_get_widget (NbtkTooltip *tooltip)
{
  g_return_val_if_fail (NBTK_IS_TOOLTIP (tooltip), NULL);

  return tooltip->priv->widget;
}

/**
 * nbtk_tooltip_set_widget:
 * @tooltip: a #NbtkTooltip
 * @widget: text to set the widget to
 *
 * Sets the text displayed on the tooltip
 */
void
nbtk_tooltip_set_widget (NbtkTooltip *tooltip,
                         NbtkWidget  *widget)
{
  NbtkTooltipPrivate *priv;

  g_return_if_fail (NBTK_IS_TOOLTIP (tooltip));

  priv = tooltip->priv;

  if (G_UNLIKELY (priv->widget))
    {
      /* remove the weak ref from the old widget */
      g_object_weak_unref (G_OBJECT (priv->widget),
                           nbtk_tooltip_weak_ref_notify,
                           tooltip);
    }

  priv->widget = widget;

  g_object_weak_ref (G_OBJECT (widget), nbtk_tooltip_weak_ref_notify, tooltip);
}


/**
 * nbtk_tooltip_show:
 * @tooltip: a #NbtkTooltip
 *
 * Show the tooltip relative to the associated widget.
 */
void
nbtk_tooltip_show (NbtkTooltip *tooltip)
{
  NbtkTooltipPrivate *priv;
  ClutterActor *parent;
  ClutterActor *stage;
  ClutterActor *widget = CLUTTER_ACTOR (tooltip->priv->widget);
  ClutterActor *self = CLUTTER_ACTOR (tooltip);
  gint x, y;
  guint w, h;

  if (!widget)
    return;

  g_return_if_fail (NBTK_TOOLTIP (tooltip));

  priv = tooltip->priv;
  parent = clutter_actor_get_parent (self);
  stage = clutter_actor_get_stage (widget);

  if (!stage)
    {
      g_warning ("NbtkTooltip associated widget is not on any stage.");
      return;
    }

  /* make sure we're parented on the stage */
  if (G_UNLIKELY (parent != stage))
    {
      if (parent)
        {
          g_warning ("NbtkTooltip must be parented directly on the stage");
        }
      clutter_actor_reparent (self, stage);
    }

  /* raise the tooltip to the top */
  clutter_container_raise_child (CLUTTER_CONTAINER (stage),
                                 CLUTTER_ACTOR (tooltip),
                                 NULL);

  /* place the tooltip under the associated actor */
  clutter_actor_get_transformed_position (widget, &x, &y);
  clutter_actor_get_transformed_size (widget, &w, &h);


  clutter_actor_move_anchor_point_from_gravity (self, CLUTTER_GRAVITY_NORTH);

  clutter_actor_set_position (self,
                              (x + w / 2),
                              y + h);

  clutter_actor_move_anchor_point_from_gravity (self,
                                                CLUTTER_GRAVITY_NONE);

  /* finally show the tooltip... */
  clutter_actor_show (self);

  /* and give it some bounce! */
  g_object_set (G_OBJECT (self),
                "scale-gravity", CLUTTER_GRAVITY_CENTER,
                NULL);
  clutter_actor_set_scale (self, 0.0, 0.0);
  clutter_actor_animate (self, CLUTTER_EASE_OUT_ELASTIC,
                         500,
                         "scale-x", 1.0,
                         "scale-y", 1.0,
                         NULL);
}

/**
 * nbtk_tooltip_hide:
 * @tooltip: a #NbtkTooltip
 *
 * Hide the tooltip
 */
void
nbtk_tooltip_hide (NbtkTooltip *tooltip)
{
  ClutterAnimation *animation;

  g_return_if_fail (NBTK_TOOLTIP (tooltip));

  g_object_set (G_OBJECT (tooltip),
                "scale-gravity", CLUTTER_GRAVITY_CENTER,
                NULL);
  animation =
    clutter_actor_animate (CLUTTER_ACTOR (tooltip), CLUTTER_EASE_IN_SINE,
                           150,
                           "scale-x", 0.0,
                           "scale-y", 0.0,
                           NULL);
  g_signal_connect_swapped (animation, "completed",
                            G_CALLBACK (clutter_actor_hide), tooltip);
}
