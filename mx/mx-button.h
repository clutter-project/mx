/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-button.h: Plain button actor
 *
 * Copyright 2007 OpenedHand
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
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_BUTTON_H__
#define __MX_BUTTON_H__

G_BEGIN_DECLS

#include <mx/mx-bin.h>
#include <mx/mx-action.h>

#define MX_TYPE_BUTTON                (mx_button_get_type ())
#define MX_BUTTON(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_BUTTON, MxButton))
#define MX_IS_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_BUTTON))
#define MX_BUTTON_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_BUTTON, MxButtonClass))
#define MX_IS_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_BUTTON))
#define MX_BUTTON_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_BUTTON, MxButtonClass))

typedef struct _MxButton              MxButton;
typedef struct _MxButtonPrivate       MxButtonPrivate;
typedef struct _MxButtonClass         MxButtonClass;

/**
 * MxButton:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */

struct _MxButton
{
  /*< private >*/
  MxBin parent_instance;

  MxButtonPrivate *priv;
};

struct _MxButtonClass
{
  MxBinClass parent_class;

  /* signals */
  void     (* clicked)    (MxButton          *button);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_button_get_type (void) G_GNUC_CONST;


ClutterActor         *mx_button_new             (void);
ClutterActor         *mx_button_new_with_label  (const gchar *text);
const gchar          *mx_button_get_label       (MxButton    *button);
void                  mx_button_set_label       (MxButton    *button,
                                                 const gchar *text);
const gchar          *mx_button_get_icon_name   (MxButton    *button);
void                  mx_button_set_icon_name   (MxButton    *button,
                                                 const gchar *icon_name);
guint                 mx_button_get_icon_size   (MxButton    *button);
void                  mx_button_set_icon_size   (MxButton    *button,
                                                 guint        icon_size);
void                  mx_button_set_is_toggle   (MxButton    *button,
                                                 gboolean     toggle);
gboolean              mx_button_get_is_toggle   (MxButton    *button);
void                  mx_button_set_toggled     (MxButton    *button,
                                                 gboolean     toggled);
gboolean              mx_button_get_toggled     (MxButton    *button);

void       mx_button_set_action (MxButton *button,
                                 MxAction *action);
MxAction  *mx_button_get_action (MxButton *button);

void       mx_button_set_icon_position (MxButton   *button,
                                        MxPosition  position);
MxPosition mx_button_get_icon_position (MxButton   *button);

void       mx_button_set_icon_visible  (MxButton   *button,
                                        gboolean    visible);
gboolean   mx_button_get_icon_visible  (MxButton   *button);

void       mx_button_set_label_visible (MxButton   *button,
                                        gboolean    visible);
gboolean   mx_button_get_label_visible (MxButton   *button);

G_END_DECLS

#endif /* __MX_BUTTON_H__ */
