/*
 * mx-window-x11.h: A native, top-level window (X11 backend)
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

#ifndef _MX_WINDOW_X11_H
#define _MX_WINDOW_X11_H

#include <glib-object.h>
#include "mx-window.h"
#include "mx-native-window.h"

G_BEGIN_DECLS

#define MX_TYPE_WINDOW_X11 _mx_window_x11_get_type()

#define MX_WINDOW_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW_X11, MxWindowX11))

#define MX_WINDOW_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW_X11, MxWindowX11Class))

#define MX_IS_WINDOW_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW_X11))

#define MX_IS_WINDOW_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW_X11))

#define MX_WINDOW_X11_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW_X11, MxWindowX11Class))

typedef struct _MxWindowX11 MxWindowX11;
typedef struct _MxWindowX11Class MxWindowX11Class;
typedef struct _MxWindowX11Private MxWindowX11Private;

struct _MxWindowX11
{
  GObject parent;

  MxWindowX11Private *priv;
};

struct _MxWindowX11Class
{
  GObjectClass parent_class;
};

GType _mx_window_x11_get_type (void) G_GNUC_CONST;

MxNativeWindow *_mx_window_x11_new (MxWindow *window);

G_END_DECLS

#endif /* _MX_WINDOW_X11_H */
