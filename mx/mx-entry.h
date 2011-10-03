/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-entry.h: Plain entry actor
 *
 * Copyright 2008, 2009 Intel Corporation.
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_ENTRY_H__
#define __MX_ENTRY_H__

G_BEGIN_DECLS

#include <mx/mx-widget.h>

#define MX_TYPE_ENTRY                (mx_entry_get_type ())
#define MX_ENTRY(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_ENTRY, MxEntry))
#define MX_IS_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_ENTRY))
#define MX_ENTRY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_ENTRY, MxEntryClass))
#define MX_IS_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_ENTRY))
#define MX_ENTRY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_ENTRY, MxEntryClass))

typedef struct _MxEntry              MxEntry;
typedef struct _MxEntryPrivate       MxEntryPrivate;
typedef struct _MxEntryClass         MxEntryClass;

/**
 * MxEntry:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxEntry
{
  /*< private >*/
  MxWidget parent_instance;

  MxEntryPrivate *priv;
};

struct _MxEntryClass
{
  MxWidgetClass parent_class;

  /* signals */
  void (*primary_icon_clicked)   (MxEntry *entry);
  void (*secondary_icon_clicked) (MxEntry *entry);


  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_entry_get_type (void) G_GNUC_CONST;

ClutterActor         *mx_entry_new              (void);
ClutterActor         *mx_entry_new_with_text    (const gchar *text);
const gchar          *mx_entry_get_text         (MxEntry     *entry);
void                  mx_entry_set_text         (MxEntry     *entry,
                                                 const gchar *text);
ClutterActor*         mx_entry_get_clutter_text (MxEntry     *entry);

void                  mx_entry_set_hint_text    (MxEntry     *entry,
                                                 const gchar *text);
const gchar          *mx_entry_get_hint_text    (MxEntry     *entry);

void                  mx_entry_set_password_char (MxEntry  *entry,
                                                  gunichar  password_char);
gunichar              mx_entry_get_password_char (MxEntry  *entry);

void mx_entry_set_primary_icon_from_file   (MxEntry     *entry,
                                            const gchar *filename);
void mx_entry_set_primary_icon_tooltip_text (MxEntry     *entry,
                                             const gchar *text);

void mx_entry_set_secondary_icon_from_file (MxEntry     *entry,
                                            const gchar *filename);
void mx_entry_set_secondary_icon_tooltip_text (MxEntry     *entry,
                                               const gchar *text);


void mx_entry_set_icon_highlight_suffix (MxEntry     *entry,
                                         const gchar *suffix);

const gchar *mx_entry_get_icon_highlight_suffix (MxEntry     *entry);

G_END_DECLS

#endif /* __MX_ENTRY_H__ */
