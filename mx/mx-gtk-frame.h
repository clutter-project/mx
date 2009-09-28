/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
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
#ifndef _MX_GTK_FRAME
#define _MX_GTK_FRAME

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MX_GTK_TYPE_FRAME mx_gtk_frame_get_type ()

#define MX_GTK_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_GTK_TYPE_FRAME, MxGtkFrame))

#define MX_GTK_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_GTK_TYPE_FRAME, MxGtkFrameClass))

#define MX_GTK_IS_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_GTK_TYPE_FRAME))

#define MX_GTK_IS_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_GTK_TYPE_FRAME))

#define MX_GTK_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_GTK_TYPE_FRAME, MxGtkFrameClass))

/**
 * MxGtkFrame:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  GtkFrame parent;
  GdkColor border_color;
} MxGtkFrame;

typedef struct {
  GtkFrameClass parent_class;
} MxGtkFrameClass;

GType mx_gtk_frame_get_type (void);

GtkWidget* mx_gtk_frame_new (void);

G_END_DECLS

#endif
