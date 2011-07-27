/*
 * mx-settings-provider.h: An object that provides settings
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

#ifndef _MX_SETTINGS_PROVIDER_H
#define _MX_SETTINGS_PROVIDER_H

#include <glib-object.h>
#include <mx/mx-settings.h>
#include "mx-private.h"

G_BEGIN_DECLS

#define MX_TYPE_SETTINGS_PROVIDER _mx_settings_provider_get_type()

#define MX_SETTINGS_PROVIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_SETTINGS_PROVIDER, MxSettingsProvider))

#define MX_IS_SETTINGS_PROVIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_SETTINGS_PROVIDER))

#define MX_SETTINGS_PROVIDER_GET_IFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
  MX_TYPE_SETTINGS_PROVIDER, MxSettingsProviderIface))

typedef struct _MxSettingsProvider MxSettingsProvider; /* dummy */
typedef struct _MxSettingsProviderIface MxSettingsProviderIface;

struct _MxSettingsProviderIface
{
  /*< private >*/
  GTypeInterface parent_iface;

  /*< public >*/
  /* signals, not vfuncs */
  void (* setting_changed) (MxSettingsProvider *provider,
                            MxSettingsProperty  id);

  /* vfuncs */
  gboolean (* get_setting) (MxSettingsProvider *provider,
                            MxSettingsProperty  id,
                            gpointer            value);
  gboolean (* set_setting) (MxSettingsProvider *provider,
                            MxSettingsProperty  id,
                            gpointer            value);
};

GType _mx_settings_provider_get_type (void) G_GNUC_CONST;

gboolean _mx_settings_provider_get_setting (MxSettingsProvider *provider,
                                            MxSettingsProperty  id,
                                            gpointer            value);
gboolean _mx_settings_provider_set_setting (MxSettingsProvider *provider,
                                            MxSettingsProperty  id,
                                            gpointer            value);

void     _mx_settings_provider_setting_changed (MxSettingsProvider *provider,
                                                MxSettingsProperty  id);

G_END_DECLS

#endif /* _MX_SETTINGS_PROVIDER_H */
