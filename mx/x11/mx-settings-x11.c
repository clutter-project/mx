/*
 * mx-settings-x11.c: Global settings (X11 backend)
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-private.h"
#include "mx-settings.h"

#include "xsettings-client.h"

#include <string.h>

#include <clutter/x11/clutter-x11.h>

G_DEFINE_TYPE (MxSettings, mx_settings, MX_TYPE_SETTINGS_BASE)

#define SETTINGS_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_SETTINGS, MxSettingsPrivate))

enum
{
  MX_ATOM_NET_SUPPORTING_WM_CHECK,
  MX_ATOM_MOBLIN
};

static gchar *mx_settings_atoms[] =
{
  "_NET_SUPPORTING_WM_CHECK",
  "_MOBLIN"
};

struct _MxSettingsPrivate
{
  gchar *icon_theme;
  gchar *font_name;
  guint  long_press_timeout;
  guint  small_screen : 1;
  guint  atoms_init   : 1;

  XSettingsClient *client;
  Atom             atoms[G_N_ELEMENTS(mx_settings_atoms)];
  Window          *wm_window;
};

enum
{
  PROP_ICON_THEME = 1,
  PROP_FONT_NAME,
  PROP_LONG_PRESS_TIMEOUT,
  PROP_SMALL_SCREEN
};

static void
xsettings_notify_func (const char       *name,
                       XSettingsAction   action,
                       XSettingsSetting *setting,
                       void             *cb_data);
static ClutterX11FilterReturn
mx_settings_event_filter (XEvent       *xev,
                          ClutterEvent *cev,
                          void         *data);


static void
mx_settings_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  switch (property_id)
    {
    case PROP_ICON_THEME:
      g_value_set_string (value, priv->icon_theme);
      break;

    case PROP_FONT_NAME:
      g_value_set_string (value, priv->font_name);
      break;

    case PROP_LONG_PRESS_TIMEOUT:
      g_value_set_uint (value, priv->long_press_timeout);
      break;

    case PROP_SMALL_SCREEN:
      g_value_set_boolean (value, priv->small_screen);
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
  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  switch (property_id)
    {
    case PROP_ICON_THEME:
      g_free (priv->icon_theme);
      priv->icon_theme = g_value_dup_string (value);
      break;

    case PROP_FONT_NAME:
      g_free (priv->font_name);
      priv->font_name = g_value_dup_string (value);
      break;

    case PROP_LONG_PRESS_TIMEOUT:
      priv->long_press_timeout = g_value_get_uint (value);
      break;

    case PROP_SMALL_SCREEN:
      priv->small_screen = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_settings_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_settings_parent_class)->dispose (object);
}

static void
mx_settings_finalize (GObject *object)
{
  MxSettingsPrivate *priv = MX_SETTINGS (object)->priv;

  clutter_x11_remove_filter (mx_settings_event_filter, object);

  g_free (priv->icon_theme);
  g_free (priv->font_name);

  if (priv->client)
    xsettings_client_destroy (priv->client);

  if (priv->wm_window)
    XFree (priv->wm_window);

  G_OBJECT_CLASS (mx_settings_parent_class)->finalize (object);
}

static void
mx_settings_constructed (GObject *object)
{
  Display *dpy;
  Window root_win;
  XWindowAttributes attr;

  MxSettings *self = (MxSettings*) object;

  /* setup xsettings client */
  /* This needs to be done after the construction of the object
   * to prevent recursion because creating a new xsettings client will
   * cause the notify function to be called, which in turn may cause the
   * style-changed signal to be emitted on MxStyle. Handlers of the
   * style-changed signal may need an MxSettings object.
   */
  dpy = clutter_x11_get_default_display ();
  self->priv->client = xsettings_client_new (dpy,
                                             clutter_x11_get_default_screen (),
                                             xsettings_notify_func,
                                             NULL,
                                             self);

  /* Add X property change notifications to the event mask */
  root_win = clutter_x11_get_root_window ();
  if (XGetWindowAttributes (dpy, root_win, &attr))
    XSelectInput (dpy, root_win, attr.your_event_mask | PropertyChangeMask);

  clutter_x11_add_filter (mx_settings_event_filter, self);
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

static void
mx_settings_class_init (MxSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxSettingsPrivate));

  object_class->get_property = mx_settings_get_property;
  object_class->set_property = mx_settings_set_property;
  object_class->dispose = mx_settings_dispose;
  object_class->finalize = mx_settings_finalize;
  object_class->constructed = mx_settings_constructed;
  object_class->constructor = mx_settings_constructor;

  g_object_class_override_property (object_class,
                                    PROP_ICON_THEME,
                                    "icon-theme");
  g_object_class_override_property (object_class,
                                    PROP_FONT_NAME,
                                    "font-name");
  g_object_class_override_property (object_class,
                                    PROP_LONG_PRESS_TIMEOUT,
                                    "long-press-timeout");
  g_object_class_override_property (object_class,
                                    PROP_SMALL_SCREEN,
                                    "small-screen");
}

static void
xsettings_notify_func (const char       *name,
                       XSettingsAction   action,
                       XSettingsSetting *setting,
                       void             *cb_data)
{
  MxSettingsPrivate *priv = MX_SETTINGS (cb_data)->priv;

  if (!name)
    return;

  if (!strcmp (name, "Net/IconThemeName"))
    {
      /* check icon theme name has changed */
      if (g_strcmp0 (priv->icon_theme, setting->data.v_string))
        {
          g_free (priv->icon_theme);
          priv->icon_theme = g_strdup (setting->data.v_string);

          g_object_notify (G_OBJECT (cb_data), "icon-theme");
        }
    }
  else if (!strcmp (name, "Gtk/FontName"))
    {
      /* check if the font name has changed */
      if (g_strcmp0 (priv->font_name, setting->data.v_string))
        {
          g_free (priv->font_name);
          priv->font_name = g_strdup (setting->data.v_string);

          g_object_notify (G_OBJECT (cb_data), "font-name");

          g_signal_emit_by_name (mx_style_get_default (), "changed", 0, NULL);
        }
    }

}

static void
mx_settings_refresh_wm_props (MxSettings *self)
{
  unsigned long n_items, bytes_left;
  unsigned char *return_string;
  int return_format;
  Atom return_type;
  Display *dpy;

  MxSettingsPrivate *priv = self->priv;

  if (!priv->atoms[MX_ATOM_MOBLIN] ||
      !priv->wm_window)
    return;

  dpy = clutter_x11_get_default_display ();

  /* Get the Moblin WM properties string */
  clutter_x11_trap_x_errors ();
  return_string = NULL;
  XGetWindowProperty (dpy, *priv->wm_window,
                      priv->atoms[MX_ATOM_MOBLIN],
                      0, 8192, False, XA_STRING,
                      &return_type, &return_format,
                      &n_items, &bytes_left, &return_string);
  clutter_x11_untrap_x_errors ();

  /* The _MOBLIN properties string is a list of 'key=value' pairs,
   * delimited by colons.
   */
  if (return_string)
    {
      gboolean small_screen = priv->small_screen;
      gchar *prop = g_strdelimit ((gchar *)return_string, ":", '\0');

      priv->small_screen = FALSE;
      while (*prop)
        {
          gchar *key = g_strdelimit (prop, "=", '\0');
          gchar *value = key + strlen (key) + 1;

          /* Check for session-type=small-screen - the only property
           * we support, currently.
           */
          if (g_str_equal (key, "session-type") &&
              g_str_equal (value, "small-screen"))
            priv->small_screen = TRUE;

          prop = value + strlen (value) + 1;
        }

      XFree (return_string);

      if (priv->small_screen != small_screen)
        g_object_notify (G_OBJECT (self), "small-screen");
    }
}

static void
mx_settings_refresh_wm_window (MxSettings *self)
{
  unsigned long n_items, bytes_left;
  int return_format;
  Atom return_type;
  Window root_win;
  Display *dpy;

  MxSettingsPrivate *priv = self->priv;

  if (!priv->atoms[MX_ATOM_NET_SUPPORTING_WM_CHECK])
    return;

  root_win = clutter_x11_get_root_window ();
  dpy = clutter_x11_get_default_display ();

  if (priv->wm_window)
    {
      XFree (priv->wm_window);
      priv->wm_window = NULL;
    }

  /* Get the WM window */
  clutter_x11_trap_x_errors ();
  XGetWindowProperty (dpy, root_win,
                      priv->atoms[MX_ATOM_NET_SUPPORTING_WM_CHECK],
                      0, 1, False, XA_WINDOW,
                      &return_type, &return_format,
                      &n_items, &bytes_left,
                      (unsigned char **)(&priv->wm_window));
  clutter_x11_untrap_x_errors ();

  if (priv->wm_window && (*priv->wm_window == None))
    {
      XFree (priv->wm_window);
      priv->wm_window = NULL;
    }

  if (priv->wm_window)
    {
      XWindowAttributes attr;

      /* Add property-change notification */
      if (XGetWindowAttributes (dpy, *priv->wm_window, &attr))
        XSelectInput (dpy, *priv->wm_window,
                      attr.your_event_mask | PropertyChangeMask);

      /* Refresh the WM properties */
      mx_settings_refresh_wm_props (self);
    }
}

static void
mx_settings_init_wm (MxSettings *self)
{
  Display *dpy;

  MxSettingsPrivate *priv = self->priv;

  priv->atoms_init = TRUE;

  dpy = clutter_x11_get_default_display ();

  /* Create the relevant Atoms for reading WM properties */
  XInternAtoms (dpy,
                mx_settings_atoms,
                G_N_ELEMENTS (mx_settings_atoms),
                False,
                priv->atoms);

  /* Read the current properties */
  mx_settings_refresh_wm_window (self);
}

static ClutterX11FilterReturn
mx_settings_event_filter (XEvent       *xev,
                          ClutterEvent *cev,
                          void         *data)
{
  Window root_win;

  MxSettings *self = MX_SETTINGS (data);
  MxSettingsPrivate *priv = self->priv;

  if (!priv->atoms_init)
    mx_settings_init_wm (self);

  root_win = clutter_x11_get_root_window ();

  switch (xev->type)
    {
    case PropertyNotify:
      if (xev->xproperty.window == root_win)
        {
          if (xev->xproperty.atom ==
              priv->atoms[MX_ATOM_NET_SUPPORTING_WM_CHECK])
            mx_settings_refresh_wm_window (self);
        }
      else if (priv->wm_window &&
               (xev->xproperty.window == *priv->wm_window))
        {
          if (xev->xproperty.atom == priv->atoms[MX_ATOM_MOBLIN])
            mx_settings_refresh_wm_props (self);
        }
      break;

    case DestroyNotify:
      if (priv->wm_window &&
          (xev->xdestroywindow.window == *priv->wm_window))
        {
          XFree (priv->wm_window);
          priv->wm_window = NULL;
        }
      break;
    }

  if (xsettings_client_process_event (priv->client, xev))
    return CLUTTER_X11_FILTER_REMOVE;
  else
    return CLUTTER_X11_FILTER_CONTINUE;
}

static void
mx_settings_init (MxSettings *self)
{
  self->priv = SETTINGS_PRIVATE (self);

  /* setup defaults */
  self->priv->long_press_timeout = 500;
  self->priv->icon_theme = g_strdup ("hicolor");
  self->priv->font_name = g_strdup ("Sans 10");
}

MxSettings *
mx_settings_get_default (void)
{
  return g_object_new (MX_TYPE_SETTINGS, NULL);
}
