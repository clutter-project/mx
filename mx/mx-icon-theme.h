/*
 * mx-icon-theme.h: A freedesktop icon theme object
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
 *
 * Author: Chris Lord <chris@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_ICON_THEME_H
#define _MX_ICON_THEME_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include <cogl/cogl.h>

G_BEGIN_DECLS

#define MX_TYPE_ICON_THEME mx_icon_theme_get_type()

#define MX_ICON_THEME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_ICON_THEME, MxIconTheme))

#define MX_ICON_THEME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_ICON_THEME, MxIconThemeClass))

#define MX_IS_ICON_THEME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_ICON_THEME))

#define MX_IS_ICON_THEME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_ICON_THEME))

#define MX_ICON_THEME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_ICON_THEME, MxIconThemeClass))

typedef struct _MxIconTheme MxIconTheme;
typedef struct _MxIconThemeClass MxIconThemeClass;
typedef struct _MxIconThemePrivate MxIconThemePrivate;

/**
 * MxIconTheme:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxIconTheme
{
  GObject parent;

  MxIconThemePrivate *priv;
};

struct _MxIconThemeClass
{
  GObjectClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_icon_theme_get_type (void) G_GNUC_CONST;

MxIconTheme    *mx_icon_theme_new (void);

MxIconTheme    *mx_icon_theme_get_default (void);

const gchar    *mx_icon_theme_get_theme_name (MxIconTheme *theme);

void            mx_icon_theme_set_theme_name (MxIconTheme *theme,
                                              const gchar *theme_name);

CoglHandle      mx_icon_theme_lookup (MxIconTheme *theme,
                                      const gchar *icon_name,
                                      gint         size);

ClutterTexture *mx_icon_theme_lookup_texture (MxIconTheme *theme,
                                              const gchar *icon_name,
                                              gint         size);

gboolean        mx_icon_theme_has_icon (MxIconTheme *theme,
                                        const gchar *icon_name);

const GList    *mx_icon_theme_get_search_paths (MxIconTheme *theme);

void            mx_icon_theme_set_search_paths (MxIconTheme *theme,
                                                const GList *paths);

G_END_DECLS

#endif /* _MX_ICON_THEME_H */
