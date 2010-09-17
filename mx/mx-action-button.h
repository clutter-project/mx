/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-action-button.h: MxAction object
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
#error "Only <mx/mx.h> can be included directly"
#endif

#ifndef _MX_ACTION_BUTTON_H
#define _MX_ACTION_BUTTON_H

G_BEGIN_DECLS

#include <mx/mx-action.h>
#include <mx/mx-button.h>

#define MX_TYPE_ACTION_BUTTON                   \
  (mx_action_button_get_type())
#define MX_ACTION_BUTTON(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               MX_TYPE_ACTION_BUTTON,   \
                               MxActionButton))
#define MX_ACTION_BUTTON_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            MX_TYPE_ACTION_BUTTON,      \
                            MxActionButtonClass))
#define MX_IS_ACTION_BUTTON(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                  \
                               MX_TYPE_ACTION_BUTTON))
#define MX_IS_ACTION_BUTTON_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                   \
                            MX_TYPE_ACTION_BUTTON))
#define MX_ACTION_BUTTON_GET_CLASS(obj)                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              MX_TYPE_ACTION_BUTTON,    \
                              MxActionButtonClass))

typedef struct _MxActionButtonPrivate MxActionButtonPrivate;
typedef struct _MxActionButton      MxActionButton;
typedef struct _MxActionButtonClass MxActionButtonClass;

/**
 * MxActionButton:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxActionButton
{
  MxButton parent;

  MxActionButtonPrivate *priv;
};

struct _MxActionButtonClass
{
  MxButtonClass parent_class;
};

GType mx_action_button_get_type (void) G_GNUC_CONST;
ClutterActor *mx_action_button_new (MxAction *action);
void mx_action_button_set_action (MxActionButton *button,
                                  MxAction        *action);
MxAction *mx_action_button_get_action (MxActionButton *button);

G_END_DECLS

#endif /* _MX_ACTION_BUTTON_H */
