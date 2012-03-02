/*
 * mx-window-wayland: MxNativeWindow implementation for Wayland
 *
 * Copyright 2012 Intel Corporation.
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
 * Written by: Rob Bradford <rob@linux.intel.com>
 */

#include <wayland-client.h>
#include <clutter/wayland/clutter-wayland.h>

#include "mx-window-wayland.h"
#include "mx-private.h"

static void mx_native_window_iface_init (MxNativeWindowIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxWindowWayland, mx_window_wayland, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_NATIVE_WINDOW, mx_native_window_iface_init))

#define WINDOW_WAYLAND_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW_WAYLAND, MxWindowWaylandPrivate))

struct _MxWindowWaylandPrivate
{
  MxWindow *window;
};

enum
{
  PROP_0,

  PROP_WINDOW
};

static void
mx_native_window_iface_init (MxNativeWindowIface *iface)
{

}


static void
mx_window_wayland_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  switch (property_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, MX_WINDOW_WAYLAND (object)->priv->window);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_wayland_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  switch (property_id)
    {
      case PROP_WINDOW:
        MX_WINDOW_WAYLAND (object)->priv->window = g_value_get_object (value);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_wayland_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_window_wayland_parent_class)->dispose (object);
}

static void
mx_window_wayland_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_window_wayland_parent_class)->finalize (object);
}

struct wl_shell_surface *
clutter_stage_wayland_get_shell_surface (ClutterStage *stage);

struct wl_input_device *
clutter_input_device_wayland_get_input_device (ClutterInputDevice *device);


static gboolean
_resize_grip_button_press_event_cb (ClutterActor    *actor,
                                    ClutterEvent    *event,
                                    MxWindowWayland *window)
{
  MxWindowWaylandPrivate *priv = window->priv;
  ClutterStage *stage = mx_window_get_clutter_stage (priv->window);
  struct wl_shell_surface *shell_surface;
  struct wl_input_device *input_device;

  shell_surface = clutter_wayland_stage_get_wl_shell_surface (stage);
  input_device =
    clutter_wayland_input_device_get_wl_input_device (event->button.device);

  wl_shell_surface_resize (shell_surface,
                           input_device,
                           event->button.time,
                           WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT);

  return FALSE;
}

static gboolean
_toolbar_button_press_event_cb (ClutterActor    *actor,
                                ClutterEvent    *event,
                                MxWindowWayland *window)
{
  MxWindowWaylandPrivate *priv = window->priv;
  ClutterStage *stage = mx_window_get_clutter_stage (priv->window);
  struct wl_shell_surface *shell_surface;
  struct wl_input_device *input_device;

  shell_surface = clutter_wayland_stage_get_wl_shell_surface (stage);
  input_device =
    clutter_wayland_input_device_get_wl_input_device (event->button.device);

  wl_shell_surface_move (shell_surface, input_device, event->button.time);

  return FALSE;
}

static void
mx_window_wayland_constructed (GObject *object)
{
  MxWindowWayland *window_wayland = MX_WINDOW_WAYLAND (object);
  MxWindowWaylandPrivate *priv = window_wayland->priv;
  ClutterActor *toolbar, *resize_grip;

  toolbar = (ClutterActor *)mx_window_get_toolbar (priv->window);
  clutter_actor_set_reactive (toolbar, TRUE);
  g_signal_connect (toolbar,
                    "button-press-event",
                    (GCallback)_toolbar_button_press_event_cb,
                    window_wayland);

  resize_grip = _mx_window_get_resize_grip (priv->window);
  clutter_actor_set_reactive (resize_grip, TRUE);
  g_signal_connect (resize_grip,
                    "button-press-event",
                    (GCallback)_resize_grip_button_press_event_cb,
                    window_wayland);
}

static void
mx_window_wayland_class_init (MxWindowWaylandClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWindowWaylandPrivate));

  object_class->get_property = mx_window_wayland_get_property;
  object_class->set_property = mx_window_wayland_set_property;
  object_class->dispose = mx_window_wayland_dispose;
  object_class->finalize = mx_window_wayland_finalize;
  object_class->constructed = mx_window_wayland_constructed;

  g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
mx_window_wayland_init (MxWindowWayland *self)
{
  self->priv = WINDOW_WAYLAND_PRIVATE (self);
}

MxNativeWindow *
_mx_window_wayland_new (MxWindow *window)
{
  return g_object_new (MX_TYPE_WINDOW_WAYLAND,
                       "window", window,
                       NULL);
}


