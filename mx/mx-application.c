/*
 * mx-application: application class
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
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>,
 *             Chris Lord  <chris@linux.intel.com>
 */

#include "config.h"

#include "mx-application.h"

#include "mx-private.h"

#include <glib/gi18n-lib.h>
#include <clutter/x11/clutter-x11.h>

#ifdef HAVE_STARTUP_NOTIFICATION
#  define SN_API_NOT_YET_FROZEN
#  include <libsn/sn.h>
#endif

#ifdef HAVE_DBUS
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#endif

G_DEFINE_TYPE (MxApplication, mx_application, G_TYPE_OBJECT)

#define APPLICATION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_APPLICATION, MxApplicationPrivate))

struct _MxApplicationPrivate
{
  GList              *windows;
  gchar              *name;
  MxApplicationFlags  flags;
  gboolean            is_proxy;

#ifdef HAVE_STARTUP_NOTIFICATION
  SnLauncheeContext  *sn_context;
#endif

#ifdef HAVE_DBUS
  gchar              *service_name;
  DBusGProxy         *proxy;
#endif
};

enum
{
  PROP_0,

  PROP_APP_NAME,
  PROP_FLAGS
};

static MxApplication *app_singleton = NULL;

static GObject*
mx_application_constructor (GType                  type,
                            guint                  n_construct_params,
                            GObjectConstructParam *construct_params)
{
  GObject *object;

  /* ensure MxApplication is a singleton */
  if (!app_singleton)
    {
      object =
        G_OBJECT_CLASS (mx_application_parent_class)->constructor (type,
                                                                   n_construct_params,
                                                                   construct_params);
      app_singleton = MX_APPLICATION (app_singleton);
    }
  else
    object = g_object_ref (G_OBJECT (app_singleton));

  return object;
}

static void
mx_application_constructed (GObject *object)
{
#ifdef HAVE_DBUS
  DBusGConnection *bus;
  guint32 request_status;

  GError *error = NULL;
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

  if (!(priv->flags & MX_APPLICATION_SINGLE_INSTANCE))
    return;

  if (!priv->service_name)
    {
      g_warning ("No service name, not registering DBus service");
      return;
    }

  if (!(bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error)))
    {
      g_warning ("%s", error->message);
      g_error_free (error);
      return;
    }

  priv->proxy = dbus_g_proxy_new_for_name (bus,
                                           DBUS_SERVICE_DBUS,
                                           DBUS_PATH_DBUS,
                                           DBUS_INTERFACE_DBUS);

  if (!org_freedesktop_DBus_request_name (priv->proxy,
                                          priv->service_name,
                                          DBUS_NAME_FLAG_DO_NOT_QUEUE,
                                          &request_status,
                                          &error))
    {
      g_warning ("Failed to request name: %s", error->message);
      g_error_free (error);
    }
  else if (request_status == DBUS_REQUEST_NAME_REPLY_EXISTS)
    {
      priv->is_proxy = TRUE;
    }
  else if (request_status != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
      g_warning ("Failed to request name");
    }
  else
    {
      gchar *path_from_name = g_strdelimit (priv->service_name, ".", '/');
      dbus_g_connection_register_g_object (bus,
                                           path_from_name,
                                           object);
      g_free (path_from_name);
    }

  dbus_g_connection_unref (bus);
#endif
}

static void
mx_application_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

  switch (property_id)
    {
  case PROP_APP_NAME:
    g_value_set_string (value, priv->name);
    break;

  case PROP_FLAGS:
    g_value_set_uint (value, priv->flags);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_application_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

  switch (property_id)
    {
  case PROP_APP_NAME:
    priv->name = g_value_dup_string (value);
    g_set_application_name (priv->name);

#ifdef HAVE_DBUS
    if (priv->name)
      {
        gint i;
        gboolean raise;
        gchar *name, *name_ptr, *camel;

        /* Create an ASCII CamelCase string from the program name */
        /* We prefer the program name over the application name,
         * as the application name may be localised.
         */
        name = g_get_prgname ();
        if (!name)
          name = g_strdup (priv->name);
        else
          name = g_filename_display_basename (name);

        camel = g_malloc (strlen (name) + 1);
        name_ptr = name;
        raise = TRUE;
        i = 0;

        while (*name_ptr)
          {
            /* Ignore non-ASCII */
            if (*name_ptr < 0x80)
              {
                /* Don't let the first character be a number and
                 * only accept alpha/number.
                 */
                if (g_ascii_isalpha (*name_ptr) ||
                    ((i != 0) && g_ascii_isalnum (*name_ptr)))
                  {
                    if (raise)
                      {
                        camel[i] = g_ascii_toupper (*name_ptr);
                        raise = FALSE;
                      }
                    else
                      camel[i] = *name_ptr;

                    i++;
                  }
                else if ((*name_ptr == '-') ||
                         (*name_ptr == '_') ||
                         (*name_ptr == ' '))
                  {
                    /* Use upper-case after dashes/underscores/spaces */
                    raise = TRUE;
                  }
              }

            name_ptr = g_utf8_find_next_char (name_ptr, NULL);
          }

        g_free (name);

        /* Make sure string is NULL-terminated */
        camel[i] = '\0';

        /* Use CamelCase name for service path */
        priv->service_name = g_strconcat ("org.moblin.", camel, NULL);
        g_free (camel);
      }
#endif
    break;

  case PROP_FLAGS:
    priv->flags = g_value_get_uint (value);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_application_dispose (GObject *object)
{
#ifdef HAVE_DBUS
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }
#endif

  G_OBJECT_CLASS (mx_application_parent_class)->dispose (object);
}

static void
mx_application_finalize (GObject *object)
{
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

#ifdef HAVE_STARTUP_NOTIFICATION
  if (priv->sn_context)
    sn_launchee_context_unref (priv->sn_context);
#endif

#ifdef HAVE_DBUS
  g_free (priv->service_name);
#endif

  g_free (priv->name);

  G_OBJECT_CLASS (mx_application_parent_class)->finalize (object);
}

static ClutterStage *
mx_application_default_create_window (MxApplication *application)
{
  ClutterStage *stage;

  stage = CLUTTER_STAGE (clutter_stage_new ());

  mx_application_add_window (application, stage);

  clutter_stage_set_title (stage, application->priv->name);

  return stage;
}

static void
mx_application_default_raise (MxApplication *application)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->flags & MX_APPLICATION_CLUTTER_GTK)
    {
      g_error ("ClutterGtk not yet supported");
      return;
    }

  g_debug ("Raise");
}


static void
mx_application_class_init (MxApplicationClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxApplicationPrivate));

  object_class->constructor = mx_application_constructor;
  object_class->constructed = mx_application_constructed;
  object_class->get_property = mx_application_get_property;
  object_class->set_property = mx_application_set_property;
  object_class->dispose = mx_application_dispose;
  object_class->finalize = mx_application_finalize;

  klass->create_window = mx_application_default_create_window;
  klass->raise = mx_application_default_raise;

  pspec = g_param_spec_string ("application-name",
                               "Application Name",
                               "Name of the application",
                               "",
                               G_PARAM_CONSTRUCT_ONLY | MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_APP_NAME, pspec);

  pspec = g_param_spec_uint ("flags",
                             "Flags",
                             "Application Flags",
                             0, G_MAXINT, 0,
                             G_PARAM_CONSTRUCT_ONLY | MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FLAGS, pspec);
}

static void
mx_application_init (MxApplication *self)
{
  self->priv = APPLICATION_PRIVATE (self);
}

static void
mx_application_window_destroy_cb (ClutterActor  *actor,
                                  MxApplication *application)
{
  mx_application_remove_window (application, CLUTTER_STAGE (actor));

  if (!(application->priv->flags & MX_APPLICATION_KEEP_ALIVE)
      && (g_list_length (application->priv->windows) < 1))
    {
      mx_application_quit (application);
    }
}

#ifdef HAVE_STARTUP_NOTIFICATION
static void
mx_application_window_map_cb (ClutterActor  *actor,
                              GParamSpec    *pspec,
                              MxApplication *application)
{
  Window xwindow;

  MxApplicationPrivate *priv = application->priv;

  if (CLUTTER_ACTOR_IS_MAPPED (actor))
    {
      xwindow = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
      sn_launchee_context_setup_window (priv->sn_context, xwindow);
      sn_launchee_context_complete (priv->sn_context);

      sn_launchee_context_unref (priv->sn_context);
      priv->sn_context = NULL;

      g_signal_handlers_disconnect_by_func (actor,
                                            mx_application_window_map_cb,
                                            application);
    }
}
#endif

MxApplication *
mx_application_new (gint                *argc,
                    gchar             ***argv,
                    const gchar         *name,
                    MxApplicationFlags   flags)
{
  MxApplication *app;

  /* initialise clutter and the type system */
  if (flags & MX_APPLICATION_CLUTTER_GTK)
    {
      g_error ("ClutterGtk not yet supported");
      return NULL;
    }
  else
    clutter_init (argc, argv);

  /* check single instance */
  if (flags & MX_APPLICATION_SINGLE_INSTANCE)
    {
      g_warning ("Single instance applications not yet supported");
    }

  mx_set_locale ();

  /* create the application singleton */
  app = g_object_new (MX_TYPE_APPLICATION,
                      "flags", flags,
                      "application-name", name,
                      NULL);

  return app;
}

void
mx_application_run (MxApplication *application)
{
  clutter_main ();
}

void
mx_application_quit (MxApplication *application)
{
  clutter_main_quit ();
}

void
mx_application_add_window (MxApplication *application,
                           ClutterStage  *window)
{
#ifdef HAVE_STARTUP_NOTIFICATION
  static gboolean first_window = TRUE;
#endif

  MxApplicationPrivate *priv = application->priv;

  g_return_if_fail (MX_IS_APPLICATION (application));
  g_return_if_fail (CLUTTER_IS_STAGE (window));

  priv->windows = g_list_prepend (application->priv->windows, window);
  g_signal_connect (window, "destroy",
                    G_CALLBACK (mx_application_window_destroy_cb), application);

#ifdef HAVE_STARTUP_NOTIFICATION
  /* Use the first window of the application for startup notification */
  if (first_window)
    {
      SnDisplay *display;
      Display *xdisplay;
      int screen;

      first_window = FALSE;

      if (priv->flags & MX_APPLICATION_CLUTTER_GTK)
        {
          g_error ("ClutterGtk not yet supported");
          return;
        }
      else
        {
          xdisplay = clutter_x11_get_default_display ();
          screen = clutter_x11_get_default_screen ();
        }

      if (g_getenv ("LIBSN_SYNC"))
        XSynchronize (xdisplay, True);

      display = sn_display_new (xdisplay, NULL, NULL);
      priv->sn_context = sn_launchee_context_new_from_environment (display,
                                                                   screen);

      if (priv->sn_context)
        {
          if (CLUTTER_ACTOR_IS_MAPPED (window))
            mx_application_window_map_cb (CLUTTER_ACTOR (window),
                                          NULL,
                                          application);
          else
            g_signal_connect (window, "notify::mapped",
                              G_CALLBACK (mx_application_window_map_cb),
                              application);
        }
    }
#endif
}

void
mx_application_remove_window (MxApplication *application,
                              ClutterStage  *window)
{
  GList *link;

  g_return_if_fail (MX_IS_APPLICATION (application));
  g_return_if_fail (CLUTTER_IS_STAGE (window));

  link = g_list_find (application->priv->windows, window);

  if (!link)
    {
      g_warning ("Could not remove window from application, the window was not"
                 " found in the application's window list");
      return;
    }

  clutter_actor_destroy (CLUTTER_ACTOR (link->data));

  application->priv->windows = g_list_delete_link (application->priv->windows,
                                                   link);
}


G_CONST_RETURN GList*
mx_application_get_windows (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), NULL);

  return application->priv->windows;
}

ClutterStage *
mx_application_create_window (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), NULL);

  return MX_APPLICATION_GET_CLASS (application)->create_window (application);
}
