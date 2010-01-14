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
  gboolean            is_running;

#ifdef HAVE_STARTUP_NOTIFICATION
  SnLauncheeContext  *sn_context;
#endif

#ifdef HAVE_DBUS
  gchar              *service_name;
  DBusGProxy         *proxy;
  DBusGObjectInfo     object_info;
#endif

  GHashTable         *actions;
};

enum
{
  PROP_0,

  PROP_APP_NAME,
  PROP_FLAGS
};

enum
{
  ACTIONS_CHANGED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

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
mx_application_raise_activated_cb (MxAction *action, MxApplication *application)
{
  MX_APPLICATION_GET_CLASS (application)->raise (application);
}

#ifdef HAVE_DBUS
static gchar *
mx_application_get_service_path (MxApplication *application)
{
  gint i, length;
  gchar *path_from_name;

  MxApplicationPrivate *priv = application->priv;

  if (!priv->service_name)
    return NULL;

  length = strlen (priv->service_name);
  path_from_name = g_malloc (length + 2);

  path_from_name[0] = '/';
  for (i = 0; i < length; i++)
    path_from_name[i+1] = (priv->service_name[i] == '.') ?
      '/' : priv->service_name[i];
  path_from_name[i+1] = '\0';

  return path_from_name;
}

static void
mx_application_register_dbus (MxApplication *application,
                              gboolean       unregister)
{
  DBusGConnection *bus;

  GError *error = NULL;

  if (!(bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error)))
    {
      g_warning (G_STRLOC "%s", error->message);
      g_error_free (error);
      return;
    }

  if (unregister)
    {
      static gboolean first_run = TRUE;

      if (first_run)
        first_run = FALSE;
      else
        dbus_g_connection_unregister_g_object (bus, G_OBJECT (application));
    }
  else
    {
      gchar *path_from_name = mx_application_get_service_path (application);

      dbus_g_connection_register_g_object (bus,
                                           path_from_name,
                                           G_OBJECT (application));

      g_free (path_from_name);
    }
}
#endif

static void
mx_application_constructed (GObject *object)
{
  MxAction *raise;
#ifdef HAVE_DBUS
  DBusGConnection *bus;
  guint32 request_status;
  gboolean unique, success;

  GError *error = NULL;
  MxApplication *self = MX_APPLICATION (object);
  MxApplicationPrivate *priv = self->priv;

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

  unique = (priv->flags & MX_APPLICATION_SINGLE_INSTANCE) ? TRUE : FALSE;
  success = FALSE;

  if (!org_freedesktop_DBus_request_name (priv->proxy,
                                          priv->service_name,
                                          unique ?
                                            DBUS_NAME_FLAG_DO_NOT_QUEUE : 0,
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
  else if (request_status == DBUS_REQUEST_NAME_REPLY_IN_QUEUE)
    {
      if (!unique)
        g_warning ("Single-instance application in queue.");
      else
        success = TRUE;
    }
  else if (request_status != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
      g_warning ("Failed to request name");
    }
  else
    {
      success = TRUE;
    }

  /*if (success)
    mx_application_register_dbus (self, FALSE);*/

  dbus_g_connection_unref (bus);
#endif

  /* Add default 'raise' action */
  raise = mx_action_new_full ("Raise",
                              _("Raise application"),
                              G_CALLBACK (mx_application_raise_activated_cb),
                              self);
  mx_application_add_action (self, raise);
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

#ifdef HAVE_DBUS
static gchar *
mx_application_get_safe_name (const gchar *name)
{
  gint i;
  gchar *camel;
  gboolean raise;
  const gchar *name_ptr;

  /* Create an ASCII CamelCase string from arbitrary UTF-8 */
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

  /* Make sure string is NULL-terminated */
  camel[i] = '\0';

  return camel;
}
#endif

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
        gchar *name, *camel;

        /* We prefer the program name over the application name,
         * as the application name may be localised.
         */
        name = g_get_prgname ();
        if (!name)
          name = g_strdup (priv->name);
        else
          name = g_filename_display_basename (name);

        camel = mx_application_get_safe_name (name);
        g_free (name);

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
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

#ifdef HAVE_DBUS
  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }
#endif

  if (priv->actions)
    {
      g_hash_table_destroy (priv->actions);
      priv->actions = NULL;
    }

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
  g_free ((gpointer)priv->object_info.method_infos);
  g_free ((gpointer)priv->object_info.data);
  g_free ((gpointer)priv->object_info.exported_signals);
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
  ClutterStage *stage;

  MxApplicationPrivate *priv = application->priv;

  if (!priv->windows)
    return;

  stage = (ClutterStage *)g_list_last (priv->windows)->data;

  if (priv->flags & MX_APPLICATION_CLUTTER_GTK)
    {
      g_error ("ClutterGtk not yet supported");
      return;
    }
  else
    {
      Window window;
      Display *display;
      guint32 timestamp;
      XClientMessageEvent xclient;

      /* As with all these arcane, poorly documented X11 things, learnt
       * how to do this from reading GTK/GDK code.
       */
      display = clutter_x11_get_default_display ();
      window = clutter_x11_get_stage_window (stage);
      XRaiseWindow (display, window);

      /* These two calls may not be necessary */
      timestamp = 0x7FFFFFFF;
      XChangeProperty (display,
                       window,
                       XInternAtom (display, "_NET_WM_USER_TIME", False),
                       XA_CARDINAL,
                       32,
                       PropModeReplace,
                       (guchar *)&timestamp,
                       1);
      XMapWindow (display, window);

      memset (&xclient, 0, sizeof (xclient));
      xclient.type = ClientMessage;
      xclient.window = window;
      xclient.message_type = XInternAtom (display, "_NET_ACTIVE_WINDOW", False);
      xclient.format = 32;
      xclient.data.l[0] = 1;
      xclient.data.l[1] = timestamp;
      xclient.data.l[2] = None;
      xclient.data.l[3] = 0;
      xclient.data.l[4] = 0;

      XSendEvent (display,
                  clutter_x11_get_root_window (),
                  False,
                  SubstructureRedirectMask | SubstructureNotifyMask,
                  (XEvent *)&xclient);
    }
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

  signals[ACTIONS_CHANGED] =
    g_signal_new ("actions-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxApplicationClass, actions_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
mx_application_init (MxApplication *self)
{
  MxApplicationPrivate *priv = self->priv = APPLICATION_PRIVATE (self);

  priv->actions = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         g_object_unref);
}

static void
mx_application_window_destroy_cb (ClutterActor  *actor,
                                  MxApplication *application)
{
  mx_application_remove_window (application, CLUTTER_STAGE (actor));

  if (!(application->priv->flags & MX_APPLICATION_KEEP_ALIVE)
      && !(application->priv->windows))
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
  MxApplicationPrivate *priv;

  g_return_if_fail (MX_IS_APPLICATION (application));

  priv = application->priv;

  if (!priv->is_proxy)
    {
      priv->is_running = TRUE;
      clutter_main ();
    }
  else
    {
      /* Raise the running instance and fall through */
      mx_application_invoke_action (application, "Raise");
    }

  priv->is_running = FALSE;
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

#ifdef HAVE_DBUS
/* The following function/define are derived from generated code from
 * dbus-binding-tool.
 */
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
static void
dbus_glib_marshal_mx_application_BOOLEAN_POINTER (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     data2);
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  register MxAction *action;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }

  /*application = (MxApplication *)data1;*/
  /*GError **error = g_marshal_value_peek_pointer (param_values + 1);*/
  action = (MxAction *)(marshal_data ? marshal_data : cc->callback);
  g_signal_emit_by_name (action, "activated", NULL);

  g_value_set_boolean (return_value, TRUE);
}
#endif

static void
mx_application_register_actions (MxApplication *application)
{
#ifdef HAVE_DBUS
  GString *data;
  GHashTableIter iter;
  DBusGObjectInfo *info;
  gint i, n_actions, offset;

  MxApplicationPrivate *priv = application->priv;

  /* Unregister the object first as we'll be messing with the
   * DBusGObjectInfo that backs the object on the bus.
   */
  mx_application_register_dbus (application, TRUE);

  info = &priv->object_info;
  n_actions = g_hash_table_size (priv->actions);

  /* Fill in the basic information of the object info struct */
  info->format_version = 0;
  info->n_method_infos = n_actions;

  /* Generate the method info to map actions on the bus object */
  g_free ((gpointer)info->method_infos);
  info->method_infos = g_new (DBusGMethodInfo, n_actions);

  data = g_string_new ("");
  g_hash_table_iter_init (&iter, priv->actions);
  for (i = 0, offset = 0; i < n_actions; i++)
    {
      MxAction *action;
      gchar *name;
      DBusGMethodInfo *method_info = (DBusGMethodInfo *)&info->method_infos[i];

      /* It shouldn't be possible for this to fail, unless there's
       * memory corruption between here and the call to hash_table_size
       * above.
       */
      if (!g_hash_table_iter_next (&iter,
                                   (gpointer *)(&name),
                                   (gpointer *)(&action)))
        g_error ("Action hash-table size mismatch");

      /* Get a safe name to put on the bus */
      name = mx_application_get_safe_name (name);

      /* Generate introspection data */
      g_string_append (data, priv->service_name); /* Iface */
      g_string_append_c (data, '\0');
      g_string_append (data, name);               /* Name */
      g_string_append_c (data, '\0');
      g_string_append_c (data, 'S');              /* A/S (Synchronous) */
      g_string_append_c (data, '\0');
      /* Arguments would go here */
      g_string_append_c (data, '\0');

      g_free (name);

      /* Fill in method info */
      method_info->function = (GCallback)action;
      method_info->marshaller =
        dbus_glib_marshal_mx_application_BOOLEAN_POINTER;
      method_info->data_offset = offset;

      /* Update offset to point to the beginning of the next string */
      offset = data->len;
    }

  g_free ((gpointer)info->data);
  info->data = data->str;
  g_string_free (data, FALSE);

  /* Generate signal info (currently just ActionsChanged)
   * NOTE: If this string changes location, bad things happen -
   *       i.e. this shouldn't change
   */
  if (!info->exported_signals)
    {
      data = g_string_new (priv->service_name);
      g_string_append_c (data, '\0');
      g_string_append (data, "ActionsChanged");
      g_string_append_c (data, '\0');

      info->exported_signals = data->str;
      g_string_free (data, FALSE);
    }

  /* Generate property info (currently none) */
  info->exported_properties = "\0";

  /* Install info */
  dbus_g_object_type_install_info (MX_TYPE_APPLICATION, info);

  mx_application_register_dbus (application, FALSE);
#endif

  g_signal_emit (application, signals[ACTIONS_CHANGED], 0);
}

void
mx_application_add_action (MxApplication *application,
                           MxAction      *action)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->is_proxy)
    {
      g_warning ("Can't add actions to remote applications");
      return;
    }

  g_hash_table_insert (priv->actions,
                       g_strdup (mx_action_get_name (action)),
                       g_object_ref (action));

  mx_application_register_actions (application);
}

void
mx_application_remove_action (MxApplication *application,
                              const gchar   *name)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->is_proxy)
    {
      g_warning ("Can't remove actions on remote applications");
      return;
    }

  g_hash_table_remove (priv->actions, name);

  mx_application_register_actions (application);
}

void
mx_application_invoke_action (MxApplication *application,
                              const gchar   *name)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->is_proxy)
    {
#ifdef HAVE_DBUS
      gchar *path_from_name;
      DBusGConnection *bus;
      DBusGProxy *proxy;

      GError *error = NULL;

      if (!(bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error)))
        {
          g_warning (G_STRLOC "%s", error->message);
          g_error_free (error);
          return;
        }

      path_from_name = mx_application_get_service_path (application);
      proxy = dbus_g_proxy_new_for_name (bus,
                                         priv->service_name,
                                         path_from_name,
                                         priv->service_name);
      g_free (path_from_name);

      if (proxy)
        {
          gchar *safe_name = mx_application_get_safe_name (name);
          if (!dbus_g_proxy_call (proxy,
                                  safe_name,
                                  &error,
                                  G_TYPE_INVALID,
                                  G_TYPE_INVALID))
            {
              g_warning (G_STRLOC "%s", error->message);
              g_error_free (error);
            }
          g_free (safe_name);
        }
#endif
    }
  else
    {
      MxAction *action = g_hash_table_lookup (priv->actions, name);
      if (action)
        g_signal_emit_by_name (action, "activated", NULL);
    }
}

gboolean
mx_application_is_running (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), FALSE);

  return (application->priv->is_proxy || application->priv->is_running);
}

