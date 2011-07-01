/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-combo-box.h: combo box
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_COMBO_BOX_H
#define _MX_COMBO_BOX_H

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_COMBO_BOX mx_combo_box_get_type()

#define MX_COMBO_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_COMBO_BOX, MxComboBox))

#define MX_COMBO_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_COMBO_BOX, MxComboBoxClass))

#define MX_IS_COMBO_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_COMBO_BOX))

#define MX_IS_COMBO_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_COMBO_BOX))

#define MX_COMBO_BOX_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_COMBO_BOX, MxComboBoxClass))

typedef struct _MxComboBox MxComboBox;
typedef struct _MxComboBoxClass MxComboBoxClass;
typedef struct _MxComboBoxPrivate MxComboBoxPrivate;

/**
 * MxComboBox:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxComboBox
{
  /*< private >*/
  MxWidget parent;

  MxComboBoxPrivate *priv;
};

struct _MxComboBoxClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_combo_box_get_type (void);

ClutterActor *mx_combo_box_new (void);


void mx_combo_box_insert_text  (MxComboBox  *box,
                                gint         position,
                                const gchar *text);

void mx_combo_box_insert_text_with_icon (MxComboBox  *box,
                                         gint         position,
                                         const gchar *text,
                                         const gchar *icon);

void mx_combo_box_append_text  (MxComboBox  *box,
                                const gchar *text);
void mx_combo_box_prepend_text (MxComboBox  *box,
                                const gchar *text);
void mx_combo_box_remove_text  (MxComboBox  *box,
                                gint         position);
void mx_combo_box_remove_all   (MxComboBox *box);

void         mx_combo_box_set_active_text (MxComboBox  *box,
                                           const gchar *text);
const gchar* mx_combo_box_get_active_text (MxComboBox  *box);

void         mx_combo_box_set_active_icon_name (MxComboBox  *box,
                                                const gchar *icon_name);

const gchar* mx_combo_box_get_active_icon_name (MxComboBox  *box);

void         mx_combo_box_set_index       (MxComboBox *box,
                                           gint        index);
gint         mx_combo_box_get_index       (MxComboBox *box);

G_END_DECLS

#endif /* _MX_COMBO_BOX_H */
