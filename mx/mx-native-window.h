/*
 * mx-native-window.h: An interface for interacting with native windows
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#ifndef _MX_NATIVE_WINDOW_H
#define _MX_NATIVE_WINDOW_H

#include <glib-object.h>
#include <mx/mx-window.h>

G_BEGIN_DECLS

#define MX_TYPE_NATIVE_WINDOW _mx_native_window_get_type()

#define MX_NATIVE_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_NATIVE_WINDOW, MxNativeWindow))

#define MX_IS_NATIVE_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_NATIVE_WINDOW))

#define MX_NATIVE_WINDOW_GET_IFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
  MX_TYPE_NATIVE_WINDOW, MxNativeWindowIface))

typedef struct _MxNativeWindow MxNativeWindow; /* dummy */
typedef struct _MxNativeWindowIface MxNativeWindowIface;


struct _MxNativeWindowIface
{
  GTypeInterface parent_iface;

  void (* get_position) (MxNativeWindow *window, gint *x, gint *y);
  void (* set_position) (MxNativeWindow *window, gint  x, gint  y);
  void (* present)      (MxNativeWindow *window);
};

GType _mx_native_window_get_type (void) G_GNUC_CONST;

void _mx_native_window_get_position (MxNativeWindow *window, gint *x, gint *y);
void _mx_native_window_set_position (MxNativeWindow *window, gint  x, gint  y);
void _mx_native_window_present      (MxNativeWindow *window);

G_END_DECLS

#endif /* _MX_NATIVE_WINDOW_H */
