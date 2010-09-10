/*
 * mx-settings-provider.c: An object that provides settings
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
 */

#include "mx-settings-provider.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"

enum
{
  SETTING_CHANGED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
mx_settings_provider_base_init (gpointer g_iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;
      GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);

      is_initialized = TRUE;

      pspec = g_param_spec_object ("settings",
                                   "Settings",
                                   "The parent MxSettings",
                                   MX_TYPE_SETTINGS,
                                   MX_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
      g_object_interface_install_property (g_iface, pspec);

      signals[SETTING_CHANGED] =
        g_signal_new (g_intern_static_string ("setting-changed"),
                      iface_type,
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MxSettingsProviderIface,
                                       setting_changed),
                      NULL, NULL,
                      _mx_marshal_VOID__UINT,
                      G_TYPE_NONE, 1, G_TYPE_UINT);
    }
}

GType
_mx_settings_provider_get_type (void)
{
  static GType settings_provider_type = 0;

  if (G_UNLIKELY (settings_provider_type == 0))
    {
      const GTypeInfo settings_provider_info = {
        sizeof (MxSettingsProviderIface),
        mx_settings_provider_base_init,
        NULL
      };

      settings_provider_type =
        g_type_register_static (G_TYPE_INTERFACE,
                                g_intern_static_string ("MxSettingsProvider"),
                                &settings_provider_info, 0);

      g_type_interface_add_prerequisite (settings_provider_type,
                                         G_TYPE_OBJECT);
    }

  return settings_provider_type;
}

gboolean
_mx_settings_provider_get_setting (MxSettingsProvider *provider,
                                   MxSettingsProperty  id,
                                   gpointer            value)
{
  MxSettingsProviderIface *iface;

  g_return_val_if_fail (MX_IS_SETTINGS_PROVIDER (provider), FALSE);

  iface = MX_SETTINGS_PROVIDER_GET_IFACE (provider);
  if (iface->get_setting)
    return iface->get_setting (provider, id, value);

  return FALSE;
}

gboolean
_mx_settings_provider_set_setting (MxSettingsProvider *provider,
                                   MxSettingsProperty  id,
                                   gpointer            value)
{
  MxSettingsProviderIface *iface;

  g_return_val_if_fail (MX_IS_SETTINGS_PROVIDER (provider), FALSE);

  iface = MX_SETTINGS_PROVIDER_GET_IFACE (provider);
  if (iface->set_setting)
    return iface->set_setting (provider, id, value);

  return FALSE;
}

void
_mx_settings_provider_setting_changed (MxSettingsProvider *provider,
                                       MxSettingsProperty  id)
{
  g_return_if_fail (MX_IS_SETTINGS_PROVIDER (provider));
  g_signal_emit (provider, signals[SETTING_CHANGED], 0, id);
}
