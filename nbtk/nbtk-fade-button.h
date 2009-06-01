/*
 * nbtk-fade-button.h
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
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef _NBTK_FADE_BUTTON
#define _NBTK_FADE_BUTTON

#include <glib-object.h>
#include <nbtk/nbtk.h>

G_BEGIN_DECLS

#define NBTK_TYPE_FADE_BUTTON nbtk_fade_button_get_type()

#define NBTK_FADE_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_FADE_BUTTON, NbtkFadeButton))

#define NBTK_FADE_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_FADE_BUTTON, NbtkFadeButtonClass))

#define NBTK_IS_FADE_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_FADE_BUTTON))

#define NBTK_IS_FADE_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_FADE_BUTTON))

#define NBTK_FADE_BUTTON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_FADE_BUTTON, NbtkFadeButtonClass))

typedef struct _NbtkFadeButtonPrivate NbtkFadeButtonPrivate;

/**
 * NbtkFadeButton:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct {
  /*< private >*/
  NbtkButton parent;
  NbtkFadeButtonPrivate *priv;
} NbtkFadeButton;

typedef struct {
  NbtkButtonClass parent_class;
} NbtkFadeButtonClass;

GType nbtk_fade_button_get_type (void);

NbtkWidget* nbtk_fade_button_new (void);
NbtkWidget* nbtk_fade_button_new_with_label (const gchar *text);

G_END_DECLS

#endif /* _NBTK_FADE_BUTTON */


