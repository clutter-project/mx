/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-clipboard.h: clipboard object
 *
 * Copyright 2009 Intel Corporation.
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_CLIPBOARD_H
#define _MX_CLIPBOARD_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_CLIPBOARD mx_clipboard_get_type()

#define MX_CLIPBOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_CLIPBOARD, MxClipboard))

#define MX_CLIPBOARD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_CLIPBOARD, MxClipboardClass))

#define MX_IS_CLIPBOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_CLIPBOARD))

#define MX_IS_CLIPBOARD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_CLIPBOARD))

#define MX_CLIPBOARD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_CLIPBOARD, MxClipboardClass))

typedef struct _MxClipboard MxClipboard;
typedef struct _MxClipboardClass MxClipboardClass;
typedef struct _MxClipboardPrivate MxClipboardPrivate;

/**
 * MxClipboard:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxClipboard
{
  /*< private >*/
  GObject parent;
  MxClipboardPrivate *priv;
};

struct _MxClipboardClass
{
  GObjectClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

/**
 * MxClipboardCallbackFunc:
 * @clipboard: A #MxClipboard
 * @text: text from the clipboard
 * @user_data: user data
 *
 * Callback function called when text is retrieved from the clipboard.
 */
typedef void (*MxClipboardCallbackFunc) (MxClipboard *clipboard,
                                         const gchar *text,
                                         gpointer     user_data);

GType mx_clipboard_get_type (void);

/**
 * mx_clipboard_get_default:
 *
 * Get the global #MxClipboard object that represents the clipboard.
 *
 * Returns: (transfer none): a #MxClipboard owned by Mx and must not be
 * unrefferenced or freed.
 */
MxClipboard* mx_clipboard_get_default (void);

/**
 * mx_clipboard_get_text:
 * @clipboard: A #MxClipboard
 * @callback: (scope async): function to be called when the text is retreived
 * @user_data: data to be passed to the callback
 *
 * Request the data from the clipboard in text form. @callback is executed
 * when the data is retreived.
 *
 */
void mx_clipboard_get_text (MxClipboard             *clipboard,
                            MxClipboardCallbackFunc  callback,
                            gpointer                 user_data);

/**
 * mx_clipboard_set_text:
 * @clipboard: A #MxClipboard
 * @text: text to copy to the clipboard
 *
 * Sets text as the current contents of the clipboard.
 *
 */
void mx_clipboard_set_text (MxClipboard             *clipboard,
                            const gchar             *text);

G_END_DECLS

#endif /* _MX_CLIPBOARD_H */
