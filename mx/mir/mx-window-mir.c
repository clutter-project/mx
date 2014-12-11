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

#include <clutter/mir/clutter-mir.h>

#include "mx-window-mir.h"
#include "mx-private.h"

static void mx_native_window_iface_init (MxNativeWindowIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxWindowMir, mx_window_mir, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_NATIVE_WINDOW, mx_native_window_iface_init))

#define WINDOW_MIR_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW_MIR, MxWindowMirPrivate))

struct _MxWindowMirPrivate
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
mx_window_mir_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  switch (property_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, MX_WINDOW_MIR (object)->priv->window);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_mir_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  switch (property_id)
    {
      case PROP_WINDOW:
        MX_WINDOW_MIR (object)->priv->window = g_value_get_object (value);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_mir_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_window_mir_parent_class)->dispose (object);
}

static void
mx_window_mir_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_window_mir_parent_class)->finalize (object);
}

static void
mx_window_mir_class_init (MxWindowMirClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWindowMirPrivate));

  object_class->get_property = mx_window_mir_get_property;
  object_class->set_property = mx_window_mir_set_property;
  object_class->dispose = mx_window_mir_dispose;
  object_class->finalize = mx_window_mir_finalize;

  g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
mx_window_mir_init (MxWindowMir *self)
{
  self->priv = WINDOW_MIR_PRIVATE (self);
}

MxNativeWindow *
_mx_window_mir_new (MxWindow *window)
{
  return g_object_new (MX_TYPE_WINDOW_MIR,
                       "window", window,
                       NULL);
}
