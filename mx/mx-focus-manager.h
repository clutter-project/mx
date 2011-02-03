/*
 * mx-focus-manager: Keyboard focus manager object
 *
 * Copyright 2010 Intel Corporation
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

#ifndef _MX_FOCUS_MANAGER_H
#define _MX_FOCUS_MANAGER_H

#include <glib-object.h>

#include <clutter/clutter.h>
#include "mx-focusable.h"

G_BEGIN_DECLS

#define MX_TYPE_FOCUS_MANAGER mx_focus_manager_get_type()

#define MX_FOCUS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_FOCUS_MANAGER, MxFocusManager))

#define MX_FOCUS_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_FOCUS_MANAGER, MxFocusManagerClass))

#define MX_IS_FOCUS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_FOCUS_MANAGER))

#define MX_IS_FOCUS_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_FOCUS_MANAGER))

#define MX_FOCUS_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_FOCUS_MANAGER, MxFocusManagerClass))

typedef struct _MxFocusManager MxFocusManager;
typedef struct _MxFocusManagerClass MxFocusManagerClass;
typedef struct _MxFocusManagerPrivate MxFocusManagerPrivate;

/**
 * MxFocusManager:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxFocusManager
{
  GObject parent;

  MxFocusManagerPrivate *priv;
};

struct _MxFocusManagerClass
{
  GObjectClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_focus_manager_get_type (void) G_GNUC_CONST;

MxFocusManager *mx_focus_manager_get_for_stage (ClutterStage *stage);


ClutterStage* mx_focus_manager_get_stage (MxFocusManager *manager);

MxFocusable* mx_focus_manager_get_focused (MxFocusManager *manager);

void         mx_focus_manager_push_focus  (MxFocusManager *manager,
                                           MxFocusable    *focusable);
void         mx_focus_manager_push_focus_with_hint  (MxFocusManager *manager,
                                                     MxFocusable    *focusable,
                                                     MxFocusHint     hint);

void         mx_focus_manager_move_focus  (MxFocusManager   *manager,
                                           MxFocusDirection  direction);

G_END_DECLS

#endif /* _MX_FOCUS_MANAGER_H */
