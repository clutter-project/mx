/*
 * nbtk-icon.h: icon widget
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

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef _NBTK_ICON
#define _NBTK_ICON

#include <glib-object.h>
#include "nbtk-widget.h"

G_BEGIN_DECLS

#define NBTK_TYPE_ICON nbtk_icon_get_type()

#define NBTK_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_ICON, NbtkIcon))

#define NBTK_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_ICON, NbtkIconClass))

#define NBTK_IS_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_ICON))

#define NBTK_IS_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_ICON))

#define NBTK_ICON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_ICON, NbtkIconClass))

/**
 * NbtkIcon:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  NbtkWidget parent;
} NbtkIcon;

typedef struct {
  NbtkWidgetClass parent_class;
} NbtkIconClass;

GType nbtk_icon_get_type (void);

NbtkIcon* nbtk_icon_new (void);

G_END_DECLS

#endif /* _NBTK_ICON */

