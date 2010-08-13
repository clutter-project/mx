/*
 * mx-settings.c: Global settings
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

#include "mx-settings.h"
#include "mx-private.h"

G_DEFINE_ABSTRACT_TYPE (MxSettings, mx_settings, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_ICON_THEME,
  PROP_FONT_NAME,
  PROP_LONG_PRESS_TIMEOUT
};

static void
mx_settings_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (property_id)
    {
    case PROP_ICON_THEME:
      g_value_set_string (value, "hicolor");
      break;

    case PROP_FONT_NAME:
      g_value_set_string (value, "Sans 10");
      break;

    case PROP_LONG_PRESS_TIMEOUT:
      g_value_set_uint (value, 500);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_settings_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_ICON_THEME:
    case PROP_FONT_NAME:
    case PROP_LONG_PRESS_TIMEOUT:
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_settings_class_init (MxSettingsClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = mx_settings_get_property;
  object_class->set_property = mx_settings_set_property;

  pspec = g_param_spec_string ("icon-theme",
                               "Icon Theme",
                               "Name of the icon theme to use when loading"
                               " icons",
                               "hicolor",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ICON_THEME, pspec);

  pspec = g_param_spec_string ("font-name",
                               "Font Name",
                               "Default font name",
                               "Sans 10",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FONT_NAME, pspec);

  pspec = g_param_spec_uint ("long-press-timeout",
                             "Long Press Timeout",
                             "Long press timeout",
                             0, G_MAXUINT, 500,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_LONG_PRESS_TIMEOUT,
                                   pspec);
}

static void
mx_settings_init (MxSettings *self)
{
}

