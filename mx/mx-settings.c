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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-settings.h"
#include "mx-private.h"
#include "mx-settings-provider.h"

#ifdef HAVE_X11
#include "x11/mx-settings-x11.h"
#endif

G_DEFINE_TYPE (MxSettings, mx_settings, G_TYPE_OBJECT)

#define SETTINGS_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_SETTINGS, MxSettingsPrivate))

struct _MxSettingsPrivate
{
  MxSettingsProvider *provider;

  gchar *icon_theme;
  gchar *font_name;
  guint  long_press_timeout;
  guint  drag_threshold;
  guint  small_screen : 1;
};

static void
mx_settings_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  guint uint_value;
  gchar *string_value;
  gboolean boolean_value;

  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  /* Check if the settings provider has these settings first */
  if (priv->provider)
    {
      switch (property_id)
        {
        case MX_SETTINGS_ICON_THEME:
        case MX_SETTINGS_FONT_NAME:
          if (_mx_settings_provider_get_setting (priv->provider, property_id,
                                                 &string_value))
            {
              g_value_set_string (value, string_value);
              return;
            }
          break;

        case MX_SETTINGS_LONG_PRESS_TIMEOUT:
          if (_mx_settings_provider_get_setting (priv->provider, property_id,
                                                 &uint_value))
            {
              g_value_set_uint (value, uint_value);
              return;
            }
          break;

        case MX_SETTINGS_SMALL_SCREEN:
          if (_mx_settings_provider_get_setting (priv->provider, property_id,
                                                 &boolean_value))
            {
              g_value_set_boolean (value, boolean_value);
              return;
            }
          break;

        case MX_SETTINGS_DRAG_THRESHOLD:
          if (_mx_settings_provider_get_setting (priv->provider, property_id,
                                                 &uint_value))
            {
              g_value_set_uint (value, uint_value);
              return;
            }
        }
    }

  /* Check the internal settings */
  switch (property_id)
    {
    case MX_SETTINGS_ICON_THEME:
      g_value_set_string (value, priv->icon_theme);
      break;

    case MX_SETTINGS_FONT_NAME:
      g_value_set_string (value, priv->font_name);
      break;

    case MX_SETTINGS_LONG_PRESS_TIMEOUT:
      g_value_set_uint (value, priv->long_press_timeout);
      break;

    case MX_SETTINGS_SMALL_SCREEN:
      g_value_set_boolean (value, priv->small_screen);
      break;

    case MX_SETTINGS_DRAG_THRESHOLD:
      g_value_set_uint (value, priv->drag_threshold);
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
  guint uint_value;
  gboolean boolean_value;
  const gchar *string_value;

  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  /* Check if the settings provider can set these settings first */
  if (priv->provider)
    {
      switch (property_id)
        {
        case MX_SETTINGS_ICON_THEME:
        case MX_SETTINGS_FONT_NAME:
          string_value = g_value_get_string (value);
          if (_mx_settings_provider_set_setting (priv->provider, property_id,
                                                 &string_value))
            return;
          break;

        case MX_SETTINGS_LONG_PRESS_TIMEOUT:
          uint_value = g_value_get_uint (value);
          if (_mx_settings_provider_set_setting (priv->provider, property_id,
                                                 &uint_value))
            return;
          break;

        case MX_SETTINGS_SMALL_SCREEN:
          boolean_value = g_value_get_boolean (value);
          if (_mx_settings_provider_set_setting (priv->provider, property_id,
                                                 &boolean_value))
            return;
          break;

        case MX_SETTINGS_DRAG_THRESHOLD:
          uint_value = g_value_get_uint (value);
          if (_mx_settings_provider_set_setting (priv->provider, property_id,
                                                 &uint_value))
            return;
          break;
        }
    }

  switch (property_id)
    {
    case MX_SETTINGS_ICON_THEME:
      g_free (priv->icon_theme);
      priv->icon_theme = g_value_dup_string (value);
      break;

    case MX_SETTINGS_FONT_NAME:
      g_free (priv->font_name);
      priv->font_name = g_value_dup_string (value);
      break;

    case MX_SETTINGS_LONG_PRESS_TIMEOUT:
      priv->long_press_timeout = g_value_get_uint (value);
      break;

    case MX_SETTINGS_SMALL_SCREEN:
      priv->small_screen = g_value_get_boolean (value);
      break;

    case MX_SETTINGS_DRAG_THRESHOLD:
      priv->drag_threshold = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_settings_dispose (GObject *object)
{
  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  if (priv->provider)
    {
      g_object_unref (priv->provider);
      priv->provider = NULL;
    }

  G_OBJECT_CLASS (mx_settings_parent_class)->dispose (object);
}

static void
mx_settings_finalize (GObject *object)
{
  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  g_free (priv->icon_theme);
  g_free (priv->font_name);

  G_OBJECT_CLASS (mx_settings_parent_class)->finalize (object);
}

static GObject *
mx_settings_constructor (GType                  type,
                         guint                  n_construct_params,
                         GObjectConstructParam *construct_params)
{
  static MxSettings *the_singleton = NULL;
  GObject *object;

  if (!the_singleton)
    {
      object = G_OBJECT_CLASS (mx_settings_parent_class)->constructor (type,
                                                          n_construct_params,
                                                          construct_params);
      the_singleton = MX_SETTINGS (object);
    }
  else
    object = (G_OBJECT (the_singleton));

  return object;
}

#if defined(HAVE_X11)
static void
mx_settings_changed_cb (MxSettingsProvider *provider,
                        MxSettingsProperty  id,
                        MxSettings         *self)
{
  switch (id)
    {
    case MX_SETTINGS_ICON_THEME:
      g_object_notify (G_OBJECT (self), "icon-theme");
      return;

    case MX_SETTINGS_FONT_NAME:
      g_object_notify (G_OBJECT (self), "font-name");
      return;

    case MX_SETTINGS_LONG_PRESS_TIMEOUT:
      g_object_notify (G_OBJECT (self), "long-press-timeout");
      return;

    case MX_SETTINGS_SMALL_SCREEN:
      g_object_notify (G_OBJECT (self), "small-screen");
      return;

    case MX_SETTINGS_DRAG_THRESHOLD:
      g_object_notify (G_OBJECT (self), "drag-threshold");
      return;
    }
}
#endif

static void
mx_settings_constructed (GObject *self)
{
#ifdef HAVE_X11
  MxSettings *settings = MX_SETTINGS (self);
  MxSettingsPrivate *priv = settings->priv;

  priv->provider = _mx_settings_x11_new (settings);
  g_signal_connect (priv->provider, "setting-changed",
                    G_CALLBACK (mx_settings_changed_cb), settings);
#endif
}

static void
mx_settings_class_init (MxSettingsClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxSettingsPrivate));

  object_class->get_property = mx_settings_get_property;
  object_class->set_property = mx_settings_set_property;
  object_class->dispose = mx_settings_dispose;
  object_class->finalize = mx_settings_finalize;
  object_class->constructor = mx_settings_constructor;
  object_class->constructed = mx_settings_constructed;

  pspec = g_param_spec_string ("icon-theme",
                               "Icon Theme",
                               "Name of the icon theme to use when loading"
                               " icons",
                               "hicolor",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, MX_SETTINGS_ICON_THEME, pspec);

  pspec = g_param_spec_string ("font-name",
                               "Font Name",
                               "Default font name",
                               "Sans 10",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, MX_SETTINGS_FONT_NAME, pspec);

  pspec = g_param_spec_uint ("long-press-timeout",
                             "Long Press Timeout",
                             "Long press timeout",
                             0, G_MAXUINT, 500,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, MX_SETTINGS_LONG_PRESS_TIMEOUT,
                                   pspec);

  pspec = g_param_spec_boolean ("small-screen",
                                "Small screen",
                                "MeeGo small-screen mode is active",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, MX_SETTINGS_SMALL_SCREEN,
                                   pspec);

  pspec = g_param_spec_uint ("drag-threshold",
                             "Drag threshold",
                             "Pixel threshold to exceed before initiating "
                             "a drag event",
                             0, G_MAXUINT, 8,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, MX_SETTINGS_DRAG_THRESHOLD,
                                   pspec);
}

static void
mx_settings_init (MxSettings *self)
{
  MxSettingsPrivate *priv = self->priv = SETTINGS_PRIVATE (self);

  /* Setup defaults */
  priv->icon_theme = g_strdup ("hicolor");
  priv->font_name = g_strdup ("Sans 10");
  priv->long_press_timeout = 500;
  priv->small_screen = FALSE;
  priv->drag_threshold = 8;
}

/**
 * mx_settings_get_default:
 *
 * Get the default #MxSettings object.
 *
 * Returns: the #MxSettings singleton.
 *
 * Since: 1.2
 */
MxSettings *
mx_settings_get_default (void)
{
  return g_object_new (MX_TYPE_SETTINGS, NULL);
}
