/*
 * mx-settings.h: Global settings
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

#ifndef _MX_SETTINGS_H
#define _MX_SETTINGS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_SETTINGS mx_settings_get_type()

#define MX_SETTINGS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_SETTINGS, MxSettings))

#define MX_SETTINGS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_SETTINGS, MxSettingsClass))

#define MX_IS_SETTINGS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_SETTINGS))

#define MX_IS_SETTINGS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_SETTINGS))

#define MX_SETTINGS_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_SETTINGS, MxSettingsClass))

typedef struct _MxSettings MxSettings;
typedef struct _MxSettingsClass MxSettingsClass;
typedef struct _MxSettingsPrivate MxSettingsPrivate;


struct _MxSettings
{
  GObject parent;

  MxSettingsPrivate *priv;
};

struct _MxSettingsClass
{
  GObjectClass parent_class;

  /*< private >*/
  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_settings_get_type (void) G_GNUC_CONST;

/**
 * mx_settings_get_default:
 *
 * Get the global MxSettings object.
 *
 * Returns: (transfer none): an #MxSettings object
 */
MxSettings *mx_settings_get_default (void);

G_END_DECLS

#endif /* _MX_SETTINGS_H */
