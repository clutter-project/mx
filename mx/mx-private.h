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
#include "mx-table-child.h"

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
  MX_SETTINGS_SMALL_SCREEN,
  MX_SETTINGS_DRAG_THRESHOLD,
  MX_SETTINGS_TOUCH_MODE
} MxSettingsProperty;


ClutterActor *_mx_widget_get_dnd_clone (MxWidget *widget);

void _mx_box_layout_start_animation (MxBoxLayout *box);

void _mx_bin_get_align_factors (MxBin   *bin,
                                gdouble *x_align,
                                gdouble *y_align);

/* used by MxTableChild to update row/column count */
void _mx_table_update_row_col (MxTable      *table,
                               MxTableChild *meta);

CoglHandle _mx_window_get_icon_cogl_texture (MxWindow *window);

ClutterActor * _mx_window_get_resize_grip (MxWindow *window);

void _mx_style_invalidate_cache (MxStylable *stylable);

gchar * _mx_stylable_get_style_string (MxStylable *stylable);

const gchar * _mx_enum_to_string (GType type,
                                  gint  value);
gboolean
_mx_string_to_enum (GType        type,
                    const gchar *nick,
                    gint        *value);

void     _mx_fade_effect_set_freeze_update (MxFadeEffect *effect,
                                            gboolean      freeze);
gboolean _mx_fade_effect_get_freeze_update (MxFadeEffect *effect);


void _mx_paint_texture_with_opacity (CoglHandle texture,
                                     guint8     opacity,
                                     gfloat     x,
                                     gfloat     y,
                                     gfloat     width,
                                     gfloat     height);

gboolean _mx_settings_get_touch_mode (MxSettings *settings);


typedef enum
{
  MX_DEBUG_LAYOUT      = 1 << 0,
  MX_DEBUG_INSPECTOR   = 1 << 1,
  MX_DEBUG_FOCUS       = 1 << 2,
  MX_DEBUG_CSS         = 1 << 3,
  MX_DEBUG_STYLE_CACHE = 1 << 4
} MxDebugTopic;

gboolean _mx_debug (gint debug);

#define _MX_STYLE_DEFINE_STYLE_CLASS_DEF(name) \
  const gchar* _mx_style_common_style_class_##name(void)

#define _MX_STYLE_GET_STYLE_CLASS(name) \
  _mx_style_common_style_class_##name()

_MX_STYLE_DEFINE_STYLE_CLASS_DEF (active);
_MX_STYLE_DEFINE_STYLE_CLASS_DEF (disabled);
_MX_STYLE_DEFINE_STYLE_CLASS_DEF (focus);
_MX_STYLE_DEFINE_STYLE_CLASS_DEF (hover);

#ifdef G_HAVE_ISO_VARARGS

#define MX_NOTE(topic,...)                         G_STMT_START { \
    if (G_UNLIKELY (_mx_debug(MX_DEBUG_##topic)))                 \
          g_message ("[" #topic "] " G_STRLOC ": " __VA_ARGS__);  \
                                                   } G_STMT_END

#elif G_HAVE_GNUC_VARARGS

#define MX_NOTE(topic, fmt, args...)               G_STMT_START { \
    if (G_UNLIKELY (_mx_debug(MX_DEBUG_##topic)))                 \
          g_message ("[" #topic "] " G_STRLOC ": " fmt, ##args);  \
                                                   } G_STMT_END

#else /* no variadic macros, your compiler sucks at C */

#warning "Can't use variadic macros, MX_NOTE() disabled."

static inline void
MX_NOTE (gint         topic,
         const gchar *fmt,
         ...)
{

}

#endif /* G_HAVE_ISO_VARARGS */

G_END_DECLS

#endif /* __MX_PRIVATE_H__ */
