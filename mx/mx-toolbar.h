/*
 * mx-toolbar.h: toolbar actor
 *
 * Copyright 2009 Intel Corporation
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
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_TOOLBAR_H
#define _MX_TOOLBAR_H

#include "mx-widget.h"
#include "mx-button.h"

G_BEGIN_DECLS

#define MX_TYPE_TOOLBAR mx_toolbar_get_type()

#define MX_TOOLBAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_TOOLBAR, MxToolbar))

#define MX_TOOLBAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_TOOLBAR, MxToolbarClass))

#define MX_IS_TOOLBAR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_TOOLBAR))

#define MX_IS_TOOLBAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_TOOLBAR))

#define MX_TOOLBAR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_TOOLBAR, MxToolbarClass))

/**
 * MxToolbar:
 *
 *The contents of this structure are private and should only be accessed
 * through the public API.
 */
typedef struct _MxToolbar MxToolbar;
typedef struct _MxToolbarClass MxToolbarClass;
typedef struct _MxToolbarPrivate MxToolbarPrivate;

struct _MxToolbar
{
  MxBin parent;

  MxToolbarPrivate *priv;
};

struct _MxToolbarClass
{
  MxBinClass parent_class;

  /* signals */
  gboolean (*close_button_clicked) (MxToolbar *toolbar);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_toolbar_get_type (void) G_GNUC_CONST;

ClutterActor *mx_toolbar_new (void);

void     mx_toolbar_set_has_close_button (MxToolbar *toolbar, gboolean has_close_button);
gboolean mx_toolbar_get_has_close_button (MxToolbar *toolbar);

G_END_DECLS

#endif /* _MX_TOOLBAR_H */
