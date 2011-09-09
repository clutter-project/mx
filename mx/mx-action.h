/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-action.h: MxAction object
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
 * Boston, MA 02111-1307, USA.
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_ACTION_H
#define _MX_ACTION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_ACTION mx_action_get_type()

#define MX_ACTION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_ACTION, MxAction))

#define MX_ACTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_ACTION, MxActionClass))

#define MX_IS_ACTION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_ACTION))

#define MX_IS_ACTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_ACTION))

#define MX_ACTION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_ACTION, MxActionClass))

typedef struct _MxAction MxAction;
typedef struct _MxActionClass MxActionClass;
typedef struct _MxActionPrivate MxActionPrivate;

/**
 * MxAction:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxAction
{
  /*< private >*/
  GInitiallyUnowned parent;

  MxActionPrivate *priv;
};

struct _MxActionClass
{
  GInitiallyUnownedClass parent_class;

  void (*activated) (MxAction *action);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_action_get_type (void);

/**
 * MxActionCallbackFunc:
 * @action: An #MxAction
 * @user_data: user data
 *
 * Callback function called when action is activated.
 */
typedef void (*MxActionCallbackFunc) (MxAction *action,
                                      gpointer  user_data);

MxAction *    mx_action_new                (void);

MxAction *    mx_action_new_with_parameter (const gchar        *name,
                                            const GVariantType *parameter_type);
MxAction *    mx_action_new_stateful       (const gchar        *name,
                                            const GVariantType *parameter_type,
                                            GVariant           *state);

MxAction *   mx_action_new_full   (const gchar *name,
                                   const gchar *display_name,
                                   GCallback    activated_cb,
                                   gpointer     user_data);

const gchar *mx_action_get_name   (MxAction    *action);
void         mx_action_set_name   (MxAction    *action,
                                   const gchar *name);

const gchar *mx_action_get_display_name (MxAction    *action);
void         mx_action_set_display_name (MxAction    *action,
                                         const gchar *name);

const gchar *mx_action_get_icon   (MxAction    *action);
void         mx_action_set_icon   (MxAction    *action,
                                   const gchar *name);

gboolean     mx_action_get_active (MxAction    *action);
void         mx_action_set_active (MxAction    *action,
                                   gboolean     active);

G_END_DECLS

#endif /* _MX_ACTION_H */
