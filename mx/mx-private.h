/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-private.h: Private declarations
 *
 * Copyright 2007 OpenedHand
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

#ifndef __MX_PRIVATE_H__
#define __MX_PRIVATE_H__

#include <glib.h>
#include "mx.h"

G_BEGIN_DECLS

#define MX_PARAM_READABLE     \
        (G_PARAM_READABLE |     \
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB)

#define MX_PARAM_WRITABLE     \
        (G_PARAM_WRITABLE |     \
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB)

#define MX_PARAM_READWRITE    \
        (G_PARAM_READABLE | G_PARAM_WRITABLE | \
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB)

#define MX_ALIGN_TO_FLOAT(x) ((x == MX_ALIGN_START) ? 0.0 : (x == MX_ALIGN_MIDDLE) ? 0.5 : 1.0)

#if !CLUTTER_CHECK_VERSION (1, 3, 13)
#define CLUTTER_KEY_Return              CLUTTER_Return
#define CLUTTER_KEY_KP_Enter            CLUTTER_KP_Enter
#define CLUTTER_KEY_ISO_Enter           CLUTTER_ISO_Enter
#define CLUTTER_KEY_space               CLUTTER_space
#define CLUTTER_KEY_Tab                 CLUTTER_Tab
#define CLUTTER_KEY_ISO_Left_Tab        CLUTTER_ISO_Left_Tab
#define CLUTTER_KEY_Up                  CLUTTER_Up
#define CLUTTER_KEY_Down                CLUTTER_Down
#define CLUTTER_KEY_Left                CLUTTER_Left
#define CLUTTER_KEY_Right               CLUTTER_Right
#define CLUTTER_KEY_c                   CLUTTER_c
#define CLUTTER_KEY_v                   CLUTTER_v
#define CLUTTER_KEY_x                   CLUTTER_x
#define CLUTTER_KEY_z                   CLUTTER_z
#endif

struct _MxTableChild
{
  ClutterChildMeta parent_instance;

  gint col;
  gint row;
  gint col_span;
  gint row_span;
  gdouble x_align;
  gdouble y_align;
  guint x_expand : 1;
  guint y_expand : 1;
  guint x_fill : 1;
  guint y_fill : 1;
};

typedef enum
{
  MX_SETTINGS_ICON_THEME = 1,
  MX_SETTINGS_FONT_NAME,
  MX_SETTINGS_LONG_PRESS_TIMEOUT,
  MX_SETTINGS_SMALL_SCREEN
} MxSettingsProperty;


ClutterActor *_mx_widget_get_dnd_clone (MxWidget *widget);

void _mx_box_layout_start_animation (MxBoxLayout *box);

void _mx_bin_get_align_factors (MxBin   *bin,
                                gdouble *x_align,
                                gdouble *y_align);

/* used by MxTableChild to update row/column count */
void _mx_table_update_row_col (MxTable *table,
                               gint     row,
                               gint     col);

CoglHandle _mx_window_get_icon_cogl_texture (MxWindow *window);

ClutterActor * _mx_window_get_resize_grip (MxWindow *window);

void _mx_style_invalidate_cache (MxStylable *stylable);

enum
{
  MX_DEBUG_LAYOUT = 1,
  MX_DEBUG_INSPECTOR = 2
};

gboolean _mx_debug (gint debug);

G_END_DECLS


#endif /* __MX_PRIVATE_H__ */
