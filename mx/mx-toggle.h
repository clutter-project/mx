/*
 * mx-toggle: toggle switch actor
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

#ifndef _MX_TOGGLE_H
#define _MX_TOGGLE_H

#include "mx-widget.h"

G_BEGIN_DECLS

#define MX_TYPE_TOGGLE mx_toggle_get_type()

#define MX_TOGGLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_TOGGLE, MxToggle))

#define MX_TOGGLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_TOGGLE, MxToggleClass))

#define MX_IS_TOGGLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_TOGGLE))

#define MX_IS_TOGGLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_TOGGLE))

#define MX_TOGGLE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_TOGGLE, MxToggleClass))

typedef struct _MxToggle MxToggle;
typedef struct _MxToggleClass MxToggleClass;
typedef struct _MxTogglePrivate MxTogglePrivate;

/**
 * MxToggle:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxToggle
{
  MxWidget parent;

  MxTogglePrivate *priv;
};

struct _MxToggleClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_toggle_get_type (void) G_GNUC_CONST;

ClutterActor *mx_toggle_new (void);

void      mx_toggle_set_active (MxToggle *toggle, gboolean active);
gboolean  mx_toggle_get_active (MxToggle *toggle);

G_END_DECLS

#endif /* _MX_TOGGLE_H */
