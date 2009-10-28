/*
 * mx-popup.c: popup menu class
 *
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_POPUP_H
#define _MX_POPUP_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include <mx/mx.h>
#include "mx-action.h"

G_BEGIN_DECLS

#define MX_TYPE_POPUP mx_popup_get_type()

#define MX_POPUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_POPUP, MxPopup))

#define MX_POPUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_POPUP, MxPopupClass))

#define MX_IS_POPUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_POPUP))

#define MX_IS_POPUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_POPUP))

#define MX_POPUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_POPUP, MxPopupClass))

typedef struct _MxPopup MxPopup;
typedef struct _MxPopupClass MxPopupClass;
typedef struct _MxPopupPrivate MxPopupPrivate;

/**
 * MxPopup:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxPopup
{
  /*< private >*/
  MxWidget parent;

  MxPopupPrivate *priv;
};

struct _MxPopupClass
{
  MxWidgetClass parent_class;

  void (*action_activated) (MxPopup  *popup,
                            MxAction *action);
};

GType mx_popup_get_type (void);

ClutterActor *mx_popup_new           (void);

void          mx_popup_add_action    (MxPopup  *popup,
                                      MxAction *action);
void          mx_popup_remove_action (MxPopup  *popup,
                                      MxAction *action);
void          mx_popup_clear         (MxPopup  *popup);

G_END_DECLS

#endif /* _MX_POPUP_H */
