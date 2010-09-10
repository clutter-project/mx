/*
 * mx-native-window.c: An interface for interacting with native windows
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-native-window.h"
#include "mx-private.h"

static void
mx_native_window_base_init (gpointer g_iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_object ("window",
                                   "Window",
                                   "The parent MxWindow",
                                   MX_TYPE_WINDOW,
                                   MX_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
      g_object_interface_install_property (g_iface, pspec);
    }
}

GType
_mx_native_window_get_type (void)
{
  static GType native_window_type = 0;

  if (G_UNLIKELY (native_window_type == 0))
    {
      const GTypeInfo native_window_info = {
        sizeof (MxNativeWindowIface),
        mx_native_window_base_init,
        NULL
      };

      native_window_type =
        g_type_register_static (G_TYPE_INTERFACE,
                                g_intern_static_string ("MxNativeWindow"),
                                &native_window_info, 0);

      g_type_interface_add_prerequisite (native_window_type,
                                         G_TYPE_OBJECT);
    }

  return native_window_type;
}

void
_mx_native_window_get_position (MxNativeWindow *window,
                                gint           *x,
                                gint           *y)
{
  MxNativeWindowIface *iface;

  g_return_if_fail (MX_IS_NATIVE_WINDOW (window));

  iface = MX_NATIVE_WINDOW_GET_IFACE (window);
  if (iface->get_position)
    iface->get_position (window, x, y);
  else
    {
      *x = 0;
      *y = 0;
    }
}

void
_mx_native_window_set_position (MxNativeWindow *window,
                                gint            x,
                                gint            y)
{
  MxNativeWindowIface *iface;

  g_return_if_fail (MX_IS_NATIVE_WINDOW (window));

  iface = MX_NATIVE_WINDOW_GET_IFACE (window);
  if (iface->set_position)
    iface->set_position (window, x, y);
}

void
_mx_native_window_present (MxNativeWindow *window)
{
  MxNativeWindowIface *iface;

  g_return_if_fail (MX_IS_NATIVE_WINDOW (window));

  iface = MX_NATIVE_WINDOW_GET_IFACE (window);
  if (iface->present)
    iface->present (window);
}
