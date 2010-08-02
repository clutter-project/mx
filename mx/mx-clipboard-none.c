/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-clipboard.c: clipboard object
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Damien Lespiau <damien.lespiau@intel.com>
 *
 */

/**
 * SECTION:mx-clipboard
 * @short_description: a simple representation clipboard
 *
 * #MxClipboard is a very simple object representation of the clipboard
 * available to applications. Text is always assumed to be UTF-8 and non-text
 * items are not handled.
 */


#include "mx-clipboard.h"

G_DEFINE_TYPE (MxClipboard, mx_clipboard, G_TYPE_OBJECT)

#define CLIPBOARD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_CLIPBOARD, MxClipboardPrivate))

struct _MxClipboardPrivate
{
};

static void
mx_clipboard_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_clipboard_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_clipboard_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_clipboard_parent_class)->dispose (object);
}

static void
mx_clipboard_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_clipboard_parent_class)->finalize (object);
}

static void
mx_clipboard_class_init (MxClipboardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /* g_type_class_add_private (klass, sizeof (MxClipboardPrivate)); */

  object_class->get_property = mx_clipboard_get_property;
  object_class->set_property = mx_clipboard_set_property;
  object_class->dispose = mx_clipboard_dispose;
  object_class->finalize = mx_clipboard_finalize;
}

static void
mx_clipboard_init (MxClipboard *self)
{
  /* self->priv = CLIPBOARD_PRIVATE (self); */
}

/**
 * mx_clipboard_get_default:
 *
 * Get the global #MxClipboard object that represents the clipboard.
 *
 * Returns: (transfer none): a #MxClipboard owned by Mx and must not be
 * unrefferenced or freed.
 */
MxClipboard*
mx_clipboard_get_default (void)
{
  static MxClipboard *default_clipboard = NULL;

  if (!default_clipboard)
    {
      default_clipboard = g_object_new (MX_TYPE_CLIPBOARD, NULL);
    }

  return default_clipboard;
}

/**
 * mx_clipboard_get_text:
 * @clipboard: A #MxClipboard
 * @callback: function to be called when the text is retreived
 * @user_data: data to be passed to the callback
 *
 * Request the data from the clipboard in text form. @callback is executed
 * when the data is retreived.
 *
 */
void
mx_clipboard_get_text (MxClipboard            *clipboard,
                       MxClipboardCallbackFunc callback,
                       gpointer                user_data)
{
  g_return_if_fail (MX_IS_CLIPBOARD (clipboard));
  g_return_if_fail (callback != NULL);

  g_warning ("Write me");
}

/**
 * mx_clipboard_set_text:
 * @clipboard: A #MxClipboard
 * @text: text to copy to the clipboard
 *
 * Sets text as the current contents of the clipboard.
 *
 */
void
mx_clipboard_set_text (MxClipboard *clipboard,
                       const gchar *text)
{
  g_return_if_fail (MX_IS_CLIPBOARD (clipboard));
  g_return_if_fail (text != NULL);

  g_warning ("Write me");
}
