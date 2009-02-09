/* nbtk-entry.c: Plain entry actor
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
 * SECTION:nbtk-entry
 * @short_description: Widget for displaying text
 *
 * #NbtkEntry is a simple widget for displaying text. It derives from
 * #NbtkWidget to add extra style and placement functionality over
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

#include "nbtk-entry.h"

#include "nbtk-widget.h"
#include "nbtk-stylable.h"

enum
{
  PROP_0,

  PROP_ENTRY
};

#define NBTK_ENTRY_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_ENTRY, NbtkEntryPrivate))

struct _NbtkEntryPrivate
{
  ClutterActor *entry;
};

G_DEFINE_TYPE (NbtkEntry, nbtk_entry, NBTK_TYPE_BIN)

static void
nbtk_entry_set_property (GObject      *gobject,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  NbtkEntry *entry = NBTK_ENTRY (gobject);

  switch (prop_id)
    {
    case PROP_ENTRY:
      nbtk_entry_set_text (entry, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_entry_get_property (GObject    *gobject,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (gobject)->priv;

  switch (prop_id)
    {
    case PROP_ENTRY:
      g_value_set_string (value, clutter_text_get_text (CLUTTER_TEXT (priv->entry)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_entry_style_changed (NbtkWidget *self)
{
  ClutterColor *color = NULL;
  NbtkEntryPrivate *priv;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  priv = NBTK_ENTRY (self)->priv;

  nbtk_stylable_get (NBTK_STYLABLE (self),
                     "color", &color,
                     "font-family", &font_name,
                     "font-size", &font_size,
                     NULL);

  if (color)
    {
      clutter_text_set_color (CLUTTER_TEXT (priv->entry), color);
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

      clutter_text_set_font_name (CLUTTER_TEXT (priv->entry), font_string);
      g_free (font_string);
    }


  if (NBTK_WIDGET_CLASS (nbtk_entry_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_entry_parent_class)->style_changed (self);
}

static void
nbtk_entry_allocate (ClutterActor          *actor,
                     const ClutterActorBox *box,
                     gboolean               absolute_origin_changed)
{
  NbtkPadding padding;
  ClutterActorBox child_box;

  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_entry_parent_class)->
    allocate (actor, box, absolute_origin_changed);
  
  nbtk_bin_get_padding (NBTK_BIN (actor), &padding);
  
  child_box.x1 = CLUTTER_UNITS_FROM_DEVICE (
                   CLUTTER_UNITS_TO_DEVICE (padding.left));
  child_box.y1 = CLUTTER_UNITS_FROM_DEVICE (
                   CLUTTER_UNITS_TO_DEVICE (padding.top));
  child_box.x2 = box->x2 - box->x1 - padding.right;
  child_box.y2 = box->y2 - box->y1 - padding.bottom;
  
  clutter_actor_allocate (priv->entry, &child_box, absolute_origin_changed);
}

static void
nbtk_entry_class_init (NbtkEntryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkEntryPrivate));

  gobject_class->set_property = nbtk_entry_set_property;
  gobject_class->get_property = nbtk_entry_get_property;
  
  actor_class->allocate = nbtk_entry_allocate;

  widget_class->style_changed = nbtk_entry_style_changed;

  pspec = g_param_spec_string ("text",
                               "Text",
                               "Text of the entry",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ENTRY, pspec);

}

static void
nbtk_entry_init (NbtkEntry *entry)
{
  entry->priv = NBTK_ENTRY_GET_PRIVATE (entry);

  entry->priv->entry = g_object_new (CLUTTER_TYPE_TEXT,
                                     "line-alignment", PANGO_ALIGN_LEFT,
                                     "editable", TRUE,
                                     "reactive", TRUE,
                                     NULL);

  clutter_container_add (CLUTTER_CONTAINER (entry), entry->priv->entry, NULL);
}

/**
 * nbtk_entry_new:
 * @text: text to set the entry to
 *
 * Create a new #NbtkEntry with the specified entry
 *
 * Returns: a new #NbtkEntry
 */
NbtkWidget *
nbtk_entry_new (const gchar *text)
{
  NbtkEntry  *entry;

  /* add the entry to the stage, but don't allow it to be visible */
  entry = g_object_new (NBTK_TYPE_ENTRY,
                          "text", text,
                          NULL);

  return NBTK_WIDGET (entry);
}

/**
 * nbtk_entry_get_text:
 * @entry: a #NbtkEntry
 *
 * Get the text displayed on the entry
 *
 * Returns: the text for the entry. This must not be freed by the application
 */
G_CONST_RETURN gchar *
nbtk_entry_get_text (NbtkEntry *entry)
{
  g_return_val_if_fail (NBTK_IS_ENTRY (entry), NULL);

  return clutter_text_get_text (CLUTTER_TEXT (entry->priv->entry));
}

/**
 * nbtk_entry_set_text:
 * @entry: a #NbtkEntry
 * @text: text to set the entry to
 *
 * Sets the text displayed on the entry
 */
void
nbtk_entry_set_text (NbtkEntry *entry,
                     const gchar *text)
{
  NbtkEntryPrivate *priv;

  g_return_if_fail (NBTK_IS_ENTRY (entry));

  priv = entry->priv;

  clutter_text_set_text (CLUTTER_TEXT (priv->entry), text);

  g_object_notify (G_OBJECT (entry), "text");
}

/**
 * nbtk_entry_get_clutter_text:
 * @entry: a #NbtkEntry
 *
 * Retrieve the internal #ClutterText so that extra parameters can be set
 *
 * Returns: the #ClutterText used by #NbtkEntry. The entry is owned by the
 * #NbtkEntry and should not be unref'ed by the application.
 */
ClutterActor*
nbtk_entry_get_clutter_text (NbtkEntry *entry)
{
  g_return_val_if_fail (NBTK_ENTRY (entry), NULL);

  return entry->priv->entry;
}
