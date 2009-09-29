/*
 * mx-label.c: Plain label actor
 *
 * Copyright 2008,2009 Intel Corporation
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

/**
 * SECTION:mx-label
 * @short_description: Widget for displaying text
 *
 * #MxLabel is a simple widget for displaying text. It derives from
 * #MxWidget to add extra style and placement functionality over
 * #ClutterText. The internal #ClutterText is publicly accessibly to allow
 * applications to set further properties.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include "mx-label.h"

#include "mx-widget.h"
#include "mx-stylable.h"

enum
{
  PROP_0,

  PROP_LABEL
};

#define MX_LABEL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_LABEL, MxLabelPrivate))

struct _MxLabelPrivate
{
  ClutterActor *label;
};

G_DEFINE_TYPE (MxLabel, mx_label, MX_TYPE_WIDGET);

static void
mx_label_set_property (GObject      *gobject,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  MxLabel *label = MX_LABEL (gobject);

  switch (prop_id)
    {
    case PROP_LABEL:
      mx_label_set_text (label, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_label_get_property (GObject    *gobject,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  MxLabelPrivate *priv = MX_LABEL (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, clutter_text_get_text (CLUTTER_TEXT (priv->label)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_label_style_changed (MxWidget *self)
{
  MxLabelPrivate *priv = MX_LABEL (self)->priv;
  ClutterColor *color = NULL;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  mx_stylable_get (MX_STYLABLE (self),
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
        {
          if (font_size)
            font_string = g_strdup_printf ("%dpx", font_size);
          else
            font_string = font_name;
        }

      clutter_text_set_font_name (CLUTTER_TEXT (priv->label), font_string);
      g_free (font_string);
    }

}

static void
mx_label_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  clutter_actor_get_preferred_width (priv->label, for_height,
                                     min_width_p,
                                     natural_width_p);

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
mx_label_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  clutter_actor_get_preferred_height (priv->label, for_width,
                                      min_height_p,
                                      natural_height_p);

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;
}

static void
mx_label_allocate (ClutterActor          *actor,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  ClutterActorClass *parent_class;
  ClutterActorBox child_box;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  parent_class = CLUTTER_ACTOR_CLASS (mx_label_parent_class);
  parent_class->allocate (actor, box, flags);

  child_box.x1 = padding.left;
  child_box.y1 = padding.top;
  child_box.x2 = box->x2 - box->x1 - padding.right;
  child_box.y2 = box->y2 - box->y1 - padding.bottom;

  clutter_actor_allocate (priv->label, &child_box, flags);
}

static void
mx_label_paint (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  ClutterActorClass *parent_class;

  parent_class = CLUTTER_ACTOR_CLASS (mx_label_parent_class);
  parent_class->paint (actor);

  clutter_actor_paint (priv->label);
}

static void
mx_label_map (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_label_parent_class)->map (actor);

  clutter_actor_map (priv->label);
}

static void
mx_label_unmap (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_label_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->label);
}

static void
mx_label_class_init (MxLabelClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxLabelPrivate));

  gobject_class->set_property = mx_label_set_property;
  gobject_class->get_property = mx_label_get_property;

  actor_class->paint = mx_label_paint;
  actor_class->allocate = mx_label_allocate;
  actor_class->get_preferred_width = mx_label_get_preferred_width;
  actor_class->get_preferred_height = mx_label_get_preferred_height;
  actor_class->map = mx_label_map;
  actor_class->unmap = mx_label_unmap;

  pspec = g_param_spec_string ("text",
                               "Text",
                               "Text of the label",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

}

static void
mx_label_init (MxLabel *label)
{
  MxLabelPrivate *priv;

  label->priv = priv = MX_LABEL_GET_PRIVATE (label);

  label->priv->label = g_object_new (CLUTTER_TYPE_TEXT,
                                     "ellipsize", PANGO_ELLIPSIZE_END,
                                     NULL);

  clutter_actor_set_parent (priv->label, CLUTTER_ACTOR (label));

  g_signal_connect (label, "style-changed",
                    G_CALLBACK (mx_label_style_changed), NULL);
}

/**
 * mx_label_new:
 * @text: text to set the label to
 *
 * Create a new #MxLabel with the specified label
 *
 * Returns: a new #MxLabel
 */
MxWidget *
mx_label_new (const gchar *text)
{
  if (text == NULL || *text == '\0')
    return g_object_new (MX_TYPE_LABEL, NULL);
  else
    return g_object_new (MX_TYPE_LABEL,
                         "text", text,
                         NULL);
}

/**
 * mx_label_get_text:
 * @label: a #MxLabel
 *
 * Get the text displayed on the label
 *
 * Returns: the text for the label. This must not be freed by the application
 */
G_CONST_RETURN gchar *
mx_label_get_text (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), NULL);

  return clutter_text_get_text (CLUTTER_TEXT (label->priv->label));
}

/**
 * mx_label_set_text:
 * @label: a #MxLabel
 * @text: text to set the label to
 *
 * Sets the text displayed on the label
 */
void
mx_label_set_text (MxLabel     *label,
                   const gchar *text)
{
  MxLabelPrivate *priv;

  g_return_if_fail (MX_IS_LABEL (label));
  g_return_if_fail (text != NULL);

  priv = label->priv;

  clutter_text_set_text (CLUTTER_TEXT (priv->label), text);

  g_object_notify (G_OBJECT (label), "text");
}

/**
 * mx_label_get_clutter_text:
 * @label: a #MxLabel
 *
 * Retrieve the internal #ClutterText so that extra parameters can be set
 *
 * Returns: the #ClutterText used by #MxLabel. The label is owned by the
 * #MxLabel and should not be unref'ed by the application.
 */
ClutterActor*
mx_label_get_clutter_text (MxLabel *label)
{
  g_return_val_if_fail (MX_LABEL (label), NULL);

  return label->priv->label;
}
