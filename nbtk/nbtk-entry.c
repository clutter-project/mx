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

#include <math.h>

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

#define NBTK_ENTRY_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_ENTRY, NbtkEntryPrivate))

struct _NbtkEntryPrivate
{
  ClutterActor *entry;

  NbtkPadding padding;
};

static void nbtk_stylable_iface_init (NbtkStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkEntry, nbtk_entry, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (NBTK_TYPE_STYLABLE,
                                                nbtk_stylable_iface_init));

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
  NbtkEntryPrivate *priv = NBTK_ENTRY (self)->priv;
  NbtkPadding *padding = NULL;
  ClutterColor *color = NULL;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  if (NBTK_WIDGET_CLASS (nbtk_entry_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_entry_parent_class)->style_changed (self);

  nbtk_stylable_get (NBTK_STYLABLE (self),
                     "color", &color,
                     "font-family", &font_name,
                     "font-size", &font_size,
                     "padding", &padding,
                     NULL);

  if (color)
    {
      clutter_text_set_color (CLUTTER_TEXT (priv->entry), color);
      clutter_color_free (color);
    }

  if (padding)
    {
      priv->padding = *padding;
      g_boxed_free (NBTK_TYPE_PADDING, padding);
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
}

static void
nbtk_entry_get_preferred_width (ClutterActor *actor,
                                ClutterUnit   for_height,
                                ClutterUnit  *min_width_p,
                                ClutterUnit  *natural_width_p)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;

  clutter_actor_get_preferred_width (priv->entry, for_height,
                                     min_width_p,
                                     natural_width_p);

  if (min_width_p)
    *min_width_p += priv->padding.left + priv->padding.right;

  if (natural_width_p)
    *natural_width_p += priv->padding.left + priv->padding.right;
}

static void
nbtk_entry_get_preferred_height (ClutterActor *actor,
                                 ClutterUnit   for_width,
                                 ClutterUnit  *min_height_p,
                                 ClutterUnit  *natural_height_p)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;

  clutter_actor_get_preferred_height (priv->entry, for_width,
                                      min_height_p,
                                      natural_height_p);

  if (min_height_p)
    *min_height_p += priv->padding.top + priv->padding.bottom;

  if (natural_height_p)
    *natural_height_p += priv->padding.top + priv->padding.bottom;
}

static void
nbtk_entry_allocate (ClutterActor          *actor,
                     const ClutterActorBox *box,
                     gboolean               absolute_origin_changed)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;
  ClutterActorClass *parent_class;
  ClutterActorBox child_box;

  parent_class = CLUTTER_ACTOR_CLASS (nbtk_entry_parent_class);
  parent_class->allocate (actor, box, absolute_origin_changed);
  
  child_box.x1 = priv->padding.left;
  child_box.y1 = priv->padding.top;
  child_box.x2 = box->x2 - box->x1 - priv->padding.right;
  child_box.y2 = box->y2 - box->y1 - priv->padding.bottom;
  
  clutter_actor_allocate (priv->entry, &child_box, absolute_origin_changed);
}

static void
nbtk_entry_focus_in (ClutterActor *actor)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;

  clutter_actor_grab_key_focus (priv->entry);
}

static void
nbtk_entry_paint (ClutterActor *actor)
{
  NbtkEntryPrivate *priv = NBTK_ENTRY (actor)->priv;
  ClutterActorClass *parent_class;

  parent_class = CLUTTER_ACTOR_CLASS (nbtk_entry_parent_class);
  parent_class->paint (actor);

  clutter_actor_paint (priv->entry);
}

static void
nbtk_stylable_iface_init (NbtkStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_boxed ("padding",
                                  "Padding",
                                  "Padding between the widgets borders "
                                  "and its content",
                                  NBTK_TYPE_PADDING,
                                  G_PARAM_READWRITE);
      nbtk_stylable_iface_install_property (iface, NBTK_TYPE_ENTRY, pspec);
    }
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

  actor_class->get_preferred_width = nbtk_entry_get_preferred_width;
  actor_class->get_preferred_height = nbtk_entry_get_preferred_height;
  actor_class->allocate = nbtk_entry_allocate;
  actor_class->focus_in = nbtk_entry_focus_in;
  actor_class->paint = nbtk_entry_paint;

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
  NbtkEntryPrivate *priv;
  ClutterColor cursor = { 0x0, 0x9c, 0xcf, 0xff };

  priv = entry->priv = NBTK_ENTRY_GET_PRIVATE (entry);

  priv->entry = g_object_new (CLUTTER_TYPE_TEXT,
                              "line-alignment", PANGO_ALIGN_LEFT,
                              "activatable", TRUE,
                              "editable", TRUE,
                              "reactive", TRUE,
                              "cursor-color", &cursor,
                              NULL);

  clutter_actor_set_parent (priv->entry, CLUTTER_ACTOR (entry));
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
