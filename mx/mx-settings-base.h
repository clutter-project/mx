/*
 * mx-settings-base.h: Global settings base class
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

#ifndef _MX_SETTINGS_BASE_H
#define _MX_SETTINGS_BASE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_SETTINGS_BASE mx_settings_base_get_type()

#define MX_SETTINGS_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_SETTINGS_BASE, MxSettingsBase))

#define MX_SETTINGS_BASE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_SETTINGS_BASE, MxSettingsBaseClass))

#define MX_IS_SETTINGS_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_SETTINGS_BASE))

#define MX_IS_SETTINGS_BASE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_SETTINGS_BASE))

#define MX_SETTINGS_BASE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_SETTINGS_BASE, MxSettingsBaseClass))

typedef struct _MxSettingsBase MxSettingsBase;
typedef struct _MxSettingsBaseClass MxSettingsBaseClass;

struct _MxSettingsBase
{
  GObject parent;
};

struct _MxSettingsBaseClass
{
  GObjectClass parent_class;
};


GType mx_settings_base_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _MX_SETTINGS_BASE_H */
