/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-clipboard-default.c: The default clipboard object
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
 *             Chris Lord <chris@linux.intel.com>
 *
 */


#include "mx-clipboard.h"

G_DEFINE_TYPE (MxClipboard, mx_clipboard, G_TYPE_OBJECT)

#define CLIPBOARD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_CLIPBOARD, MxClipboardPrivate))

struct _MxClipboardPrivate
{
  gchar *text;
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
  MxClipboardPrivate *priv = MX_CLIPBOARD (object)->priv;

  g_free (priv->text);

  G_OBJECT_CLASS (mx_clipboard_parent_class)->finalize (object);
}

static void
mx_clipboard_class_init (MxClipboardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxClipboardPrivate));

  object_class->get_property = mx_clipboard_get_property;
  object_class->set_property = mx_clipboard_set_property;
  object_class->dispose = mx_clipboard_dispose;
  object_class->finalize = mx_clipboard_finalize;
}

static void
mx_clipboard_init (MxClipboard *self)
{
  self->priv = CLIPBOARD_PRIVATE (self);
}

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

typedef struct
{
  MxClipboard             *clipboard;
  MxClipboardCallbackFunc  callback;
  gpointer                 user_data;
} MxClipboardClosure;

static gboolean
mx_clipboard_get_text_cb (MxClipboardClosure *closure)
{
  if (closure->clipboard)
    {
      MxClipboardPrivate *priv = closure->clipboard->priv;
      g_object_remove_weak_pointer (G_OBJECT (closure->clipboard),
                                    (gpointer *)&closure->clipboard);
      closure->callback (closure->clipboard, priv->text, closure->user_data);
    }

  g_slice_free (MxClipboardClosure, closure);

  return FALSE;
}

void
mx_clipboard_get_text (MxClipboard            *clipboard,
                       MxClipboardCallbackFunc callback,
                       gpointer                user_data)
{
  MxClipboardClosure *closure;

  g_return_if_fail (MX_IS_CLIPBOARD (clipboard));
  g_return_if_fail (callback != NULL);

  closure = g_slice_new (MxClipboardClosure);
  closure->clipboard = clipboard;
  closure->callback = callback;
  closure->user_data = user_data;

  g_object_add_weak_pointer (G_OBJECT (clipboard),
                             (gpointer *)&closure->clipboard);
  g_idle_add ((GSourceFunc)mx_clipboard_get_text_cb, closure);
}

void
mx_clipboard_set_text (MxClipboard *clipboard,
                       const gchar *text)
{
  MxClipboardPrivate *priv;

  g_return_if_fail (MX_IS_CLIPBOARD (clipboard));
  g_return_if_fail (text != NULL);

  priv = clipboard->priv;
  g_free (priv->text);
  priv->text = g_strdup (text);
}
