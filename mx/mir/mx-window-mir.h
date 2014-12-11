/*
 * mx-window-mir: MxNativeWindow implementation for Mir
 *
 * Copyright 2014 Canonical Ltd.
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
 * Written by: Marco Trevisan <marco.trevisan@canonical.com>
 */

#ifndef _MX_WINDOW_MIR_H
#define _MX_WINDOW_MIR_H

#include <glib-object.h>
#include "mx-window.h"
#include "mx-native-window.h"

G_BEGIN_DECLS

#define MX_TYPE_WINDOW_MIR mx_window_mir_get_type()

#define MX_WINDOW_MIR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW_MIR, MxWindowMir))

#define MX_WINDOW_MIR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW_MIR, MxWindowMirClass))

#define MX_IS_WINDOW_MIR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW_MIR))

#define MX_IS_WINDOW_MIR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW_MIR))

#define MX_WINDOW_MIR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW_MIR, MxWindowMirClass))

typedef struct _MxWindowMir MxWindowMir;
typedef struct _MxWindowMirClass MxWindowMirClass;
typedef struct _MxWindowMirPrivate MxWindowMirPrivate;

struct _MxWindowMir
{
  GObject parent;

  MxWindowMirPrivate *priv;
};

struct _MxWindowMirClass
{
  GObjectClass parent_class;
};

GType mx_window_mir_get_type (void) G_GNUC_CONST;

MxNativeWindow *_mx_window_mir_new (MxWindow *window);

G_END_DECLS

#endif /* _MX_WINDOW_MIR_H */
