/*
 * mx-settings-x11.h: X11 settings provider
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
 *
 */

#ifndef _MX_SETTINGS_X11_H
#define _MX_SETTINGS_X11_H

#include <glib-object.h>
#include "mx-settings-provider.h"

G_BEGIN_DECLS

#define MX_TYPE_SETTINGS_X11 _mx_settings_x11_get_type()

#define MX_SETTINGS_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_SETTINGS_X11, MxSettingsX11))

#define MX_SETTINGS_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_SETTINGS_X11, MxSettingsX11Class))

#define MX_IS_SETTINGS_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_SETTINGS_X11))

#define MX_IS_SETTINGS_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_SETTINGS_X11))

#define MX_SETTINGS_X11_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_SETTINGS_X11, MxSettingsX11Class))

typedef struct _MxSettingsX11 MxSettingsX11;
typedef struct _MxSettingsX11Class MxSettingsX11Class;
typedef struct _MxSettingsX11Private MxSettingsX11Private;

struct _MxSettingsX11
{
  GObject parent;

  MxSettingsX11Private *priv;
};

struct _MxSettingsX11Class
{
  GObjectClass parent_class;
};

GType _mx_settings_x11_get_type (void) G_GNUC_CONST;

MxSettingsProvider *_mx_settings_x11_new (MxSettings *settings);

G_END_DECLS

#endif /* _MX_SETTINGS_X11_H */
