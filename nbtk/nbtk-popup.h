/*
 * nbtk-popup.c: popup menu class
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

#ifndef _NBTK_POPUP_H
#define _NBTK_POPUP_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include <nbtk/nbtk.h>
#include "nbtk-action.h"

G_BEGIN_DECLS

#define NBTK_TYPE_POPUP nbtk_popup_get_type()

#define NBTK_POPUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_POPUP, NbtkPopup))

#define NBTK_POPUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_POPUP, NbtkPopupClass))

#define NBTK_IS_POPUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_POPUP))

#define NBTK_IS_POPUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_POPUP))

#define NBTK_POPUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_POPUP, NbtkPopupClass))

typedef struct _NbtkPopup NbtkPopup;
typedef struct _NbtkPopupClass NbtkPopupClass;
typedef struct _NbtkPopupPrivate NbtkPopupPrivate;

/**
 * NbtkPopup:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _NbtkPopup
{
  /*< private >*/
  NbtkWidget parent;

  NbtkPopupPrivate *priv;
};

struct _NbtkPopupClass
{
  NbtkWidgetClass parent_class;

  void (*action_activated) (NbtkPopup *popup, NbtkAction *action);
};

GType nbtk_popup_get_type (void);

NbtkWidget *nbtk_popup_new (void);

void nbtk_popup_add_action (NbtkPopup  *popup,
                           NbtkAction *action);

void nbtk_popup_remove_action (NbtkPopup  *popup,
                              NbtkAction *action);

void nbtk_popup_clear (NbtkPopup *popup);

G_END_DECLS

#endif /* _NBTK_POPUP_H */
