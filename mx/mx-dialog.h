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

#ifndef __MX_DIALOG_H__
#define __MX_DIALOG_H__

#include <mx/mx-modal-frame.h>
#include <mx/mx-action.h>

G_BEGIN_DECLS

#define MX_TYPE_DIALOG                                                 \
   (mx_dialog_get_type())
#define MX_DIALOG(obj)                                                 \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                MX_TYPE_DIALOG,                        \
                                MxDialog))
#define MX_DIALOG_CLASS(klass)                                         \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             MX_TYPE_DIALOG,                           \
                             MxDialogClass))
#define MX_IS_DIALOG(obj)                                              \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                MX_TYPE_DIALOG))
#define MX_IS_DIALOG_CLASS(klass)                                      \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             MX_TYPE_DIALOG))
#define MX_DIALOG_GET_CLASS(obj)                                       \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               MX_TYPE_DIALOG,                         \
                               MxDialogClass))

typedef struct _MxDialogPrivate MxDialogPrivate;
typedef struct _MxDialog      MxDialog;
typedef struct _MxDialogClass MxDialogClass;

struct _MxDialog
{
    MxModalFrame parent;

    MxDialogPrivate *priv;
};

struct _MxDialogClass
{
    MxModalFrameClass parent_class;
};

GType mx_dialog_get_type (void) G_GNUC_CONST;
ClutterActor *mx_dialog_new (void);
void mx_dialog_set_content (MxDialog     *self,
                            ClutterActor *content);
void mx_dialog_add_action (MxDialog *self,
                            MxAction  *action);

G_END_DECLS

#endif /* __MX_DIALOG_H__ */
