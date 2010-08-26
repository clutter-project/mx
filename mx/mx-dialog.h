/*
 * mx-dialog.h
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *             Iain Holmes <iain@linux.intel.com>
 *
 */

#ifndef _MX_DIALOG_H
#define _MX_DIALOG_H

#include <glib-object.h>
#include <mx/mx-bin.h>
#include <mx/mx-action.h>

G_BEGIN_DECLS

#define MX_TYPE_DIALOG mx_dialog_get_type()

#define MX_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DIALOG, MxDialog))

#define MX_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DIALOG, MxDialogClass))

#define MX_IS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DIALOG))

#define MX_IS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DIALOG))

#define MX_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DIALOG, MxDialogClass))

/**
 * MxDialog:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct _MxDialog MxDialog;
typedef struct _MxDialogClass MxDialogClass;
typedef struct _MxDialogPrivate MxDialogPrivate;

struct _MxDialog
{
  /*< private >*/
  MxBin parent;

  MxDialogPrivate *priv;
};

struct _MxDialogClass
{
  MxBinClass parent_class;
};

GType mx_dialog_get_type (void) G_GNUC_CONST;

ClutterActor *mx_dialog_new (void);

void mx_dialog_set_transient_parent (MxDialog     *dialog,
                                     ClutterActor *actor);

void   mx_dialog_add_action    (MxDialog *dialog,
                                MxAction *action);
void   mx_dialog_remove_action (MxDialog *dialog,
                                MxAction *action);
GList *mx_dialog_get_actions   (MxDialog *dialog);

G_END_DECLS

#endif /* _MX_DIALOG_H */
