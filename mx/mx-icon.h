/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-icon.h: icon widget
 *
 * Copyright 2009, 2010 Intel Corporation.
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

#ifndef _MX_ICON
#define _MX_ICON

#include <glib-object.h>
#include "mx-widget.h"

G_BEGIN_DECLS

#define MX_TYPE_ICON mx_icon_get_type()

#define MX_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_ICON, MxIcon))

#define MX_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_ICON, MxIconClass))

#define MX_IS_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_ICON))

#define MX_IS_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_ICON))

#define MX_ICON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_ICON, MxIconClass))

typedef struct _MxIconPrivate       MxIconPrivate;

/**
 * MxIcon:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  MxWidget parent;

  MxIconPrivate *priv;
} MxIcon;

typedef struct {
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
} MxIconClass;

GType mx_icon_get_type (void);

ClutterActor* mx_icon_new (void);



const gchar *mx_icon_get_icon_name (MxIcon *icon);
void         mx_icon_set_icon_name (MxIcon *icon, const gchar *icon_name);

gint         mx_icon_get_icon_size (MxIcon *icon);
void         mx_icon_set_icon_size (MxIcon *icon, gint size);


G_END_DECLS

#endif /* _MX_ICON */

