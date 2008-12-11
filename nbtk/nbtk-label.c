/* nbtk-label.c: Plain label actor
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/clutter-container.h>

#include "nbtk-label.h"

#include "nbtk-widget.h"
#include "nbtk-stylable.h"
#include "nbtk-behaviour-bounce.h"

enum
{
  PROP_0,

  PROP_LABEL
};

#define NBTK_LABEL_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_LABEL, NbtkLabelPrivate))

struct _NbtkLabelPrivate
{
  ClutterActor *label;
  ClutterActor *widget;
};

G_DEFINE_TYPE (NbtkLabel, nbtk_label, NBTK_TYPE_WIDGET)

static void
nbtk_label_set_property (GObject      *gobject,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  NbtkLabel *label = NBTK_LABEL (gobject);

  switch (prop_id)
    {
    case PROP_LABEL:
      nbtk_label_set_text (label, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_label_get_property (GObject    *gobject,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  NbtkLabelPrivate *priv = NBTK_LABEL (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, clutter_label_get_text (CLUTTER_LABEL (priv->label)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_label_style_changed (NbtkWidget *self)
{
  ClutterColor *color = NULL;
  NbtkLabelPrivate *priv;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  priv = NBTK_LABEL (self)->priv;

  nbtk_stylable_get (NBTK_STYLABLE (self),
                     "color", &color,
                     "font-name", &font_name,
                     "font-size", &font_size,
                     NULL);

  if (color)
    {
      clutter_label_set_color (CLUTTER_LABEL (priv->label), color);
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

      clutter_label_set_font_name (CLUTTER_LABEL (priv->label), font_string);
      g_free (font_string);
    }


  if (NBTK_WIDGET_CLASS (nbtk_label_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_label_parent_class)->style_changed (self);
}


static void
nbtk_label_class_init (NbtkLabelClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkLabelPrivate));

  gobject_class->set_property = nbtk_label_set_property;
  gobject_class->get_property = nbtk_label_get_property;

  widget_class->style_changed = nbtk_label_style_changed;

  pspec = g_param_spec_string ("text",
                               "Text",
                               "Text of the label",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

}

static void
nbtk_label_init (NbtkLabel *label)
{
  label->priv = NBTK_LABEL_GET_PRIVATE (label);

  label->priv->label = g_object_new (CLUTTER_TYPE_LABEL,
                                       "alignment", PANGO_ALIGN_CENTER,
                                       "ellipsize", PANGO_ELLIPSIZE_MIDDLE,
                                       "use-markup", TRUE,
                                       "wrap", FALSE,
                                       NULL);

  clutter_container_add (CLUTTER_CONTAINER (label), label->priv->label, NULL);
}

/**
 * nbtk_label_new:
 * @widget: actor the label is attached to
 * @text: text to set the label to
 *
 * Create a new #NbtkLabel with the specified label
 *
 * Returns: a new #NbtkLabel
 */
NbtkWidget *
nbtk_label_new (const gchar *text)
{
  NbtkLabel  *label;

  /* add the label to the stage, but don't allow it to be visible */
  label = g_object_new (NBTK_TYPE_LABEL,
                          "text", text,
                          NULL);

  return NBTK_WIDGET (label);
}

/**
 * nbtk_label_get_label:
 * @label: a #NbtkLabel
 *
 * Get the text displayed on the label
 *
 * Returns: the text for the label. This must not be freed by the application
 */
G_CONST_RETURN gchar *
nbtk_label_get_text (NbtkLabel *label)
{
  g_return_val_if_fail (NBTK_IS_LABEL (label), NULL);

  return clutter_label_get_text (CLUTTER_LABEL (label->priv->label));
}

/**
 * nbtk_label_set_text:
 * @label: a #NbtkLabel
 * @text: text to set the label to
 *
 * Sets the text displayed on the label
 */
void
nbtk_label_set_text (NbtkLabel *label,
                     const gchar *text)
{
  NbtkLabelPrivate *priv;

  g_return_if_fail (NBTK_IS_LABEL (label));

  priv = label->priv;

  clutter_label_set_text (CLUTTER_LABEL (priv->label), text);

  g_object_notify (G_OBJECT (label), "text");
}

/**
 * nbtk_label_get_clutter_label:
 * @label: a #NbtkLabel
 *
 * Retrieve the internal #ClutterLabel so that extra parameters can be set
 */
ClutterActor*
nbtk_label_get_clutter_label (NbtkLabel *label)
{
  g_return_val_if_fail (NBTK_LABEL (label), NULL);

  return label->priv->label;
}
