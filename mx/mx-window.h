/*
 * mx-window.h
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *             Chris Lord <chris@linux.intel.com>
 *
 */



#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_WINDOW_H
#define _MX_WINDOW_H

#include <glib-object.h>
#include <mx/mx-toolbar.h>

G_BEGIN_DECLS

#define MX_TYPE_WINDOW mx_window_get_type()

#define MX_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW, MxWindow))

#define MX_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW, MxWindowClass))

#define MX_IS_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW))

#define MX_IS_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW))

#define MX_WINDOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW, MxWindowClass))

/**
 * MxWindow:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct _MxWindow MxWindow;
typedef struct _MxWindowClass MxWindowClass;
typedef struct _MxWindowPrivate MxWindowPrivate;

struct _MxWindow
{
  GObject parent;

  MxWindowPrivate *priv;
};

struct _MxWindowClass
{
  GObjectClass parent_class;

  /* signals, not vfuncs */
  void (*destroy)             (MxWindow *window);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_window_get_type (void) G_GNUC_CONST;

MxWindow *mx_window_new (void);
MxWindow *mx_window_new_with_clutter_stage (ClutterStage *stage);

MxWindow *mx_window_get_for_stage (ClutterStage *stage);

ClutterActor* mx_window_get_child (MxWindow *window);
void          mx_window_set_child (MxWindow *window, ClutterActor *actor);


MxToolbar* mx_window_get_toolbar (MxWindow *window);
void       mx_window_set_toolbar (MxWindow *window, MxToolbar *toolbar);

gboolean   mx_window_get_has_toolbar (MxWindow *window);
void       mx_window_set_has_toolbar (MxWindow *window, gboolean  toolbar);

gboolean   mx_window_get_small_screen (MxWindow *window);
void       mx_window_set_small_screen (MxWindow *window, gboolean small_screen);

gboolean   mx_window_get_fullscreen   (MxWindow *window);
void       mx_window_set_fullscreen   (MxWindow *window, gboolean fullscreen);

void         mx_window_set_title (MxWindow *window, const gchar *title);
const gchar *mx_window_get_title (MxWindow *window);

void         mx_window_set_icon_name (MxWindow *window, const gchar *icon_name);
const gchar *mx_window_get_icon_name (MxWindow *window);

void         mx_window_set_icon_from_cogl_texture (MxWindow   *window,
                                                   CoglHandle  texture);

ClutterStage *mx_window_get_clutter_stage (MxWindow *window);

void       mx_window_get_window_position (MxWindow *window, gint *x, gint *y);
void       mx_window_set_window_position (MxWindow *window, gint  x, gint  y);

void       mx_window_get_window_size (MxWindow *window,
                                      gint *width, gint *height);
void       mx_window_set_window_size (MxWindow *window,
                                      gint  width, gint  height);

void       mx_window_present (MxWindow *window);

void             mx_window_set_window_rotation (MxWindow         *window,
                                                MxWindowRotation  rotation);
MxWindowRotation mx_window_get_window_rotation (MxWindow         *window);

void mx_window_show (MxWindow *window);
void mx_window_hide (MxWindow *window);

G_END_DECLS

#endif /* _MX_WINDOW_H */
