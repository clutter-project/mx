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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-application.h"

#include "mx-private.h"
#include "mx-settings.h"
#include "mx-window.h"

#include <glib/gi18n-lib.h>

#ifdef HAVE_STARTUP_NOTIFICATION

#  define SN_API_NOT_YET_FROZEN
#  include <libsn/sn.h>

#  ifdef HAVE_X11
#    include <clutter/x11/clutter-x11.h>
#  endif

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
  gboolean            actions_valid;
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
      app_singleton = MX_APPLICATION (object);
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

static void
mx_application_actions_changed_cb (DBusGProxy *proxy,
                                   gpointer    data)
{
  MxApplicationPrivate *priv = MX_APPLICATION (data)->priv;

  /* Mark that our actions table is outdated */
  priv->actions_valid = FALSE;

  g_signal_emit (data, signals[ACTIONS_CHANGED], 0);
}
#endif

static void
mx_application_notify_small_screen_cb (MxSettings    *settings,
                                       GParamSpec    *pspec,
                                       MxApplication *self)
{
  MxApplicationPrivate *priv = self->priv;

  /* Reflect small-screen mode in the first added window of the
   * application.
   *
   * FIXME: This should probably be optional.
   */
  if (priv->windows)
    {
      gboolean small_screen = FALSE;
      MxWindow *window = g_list_last (priv->windows)->data;

      g_object_get (G_OBJECT (settings), "small-screen", &small_screen, NULL);
      mx_window_set_small_screen (window, small_screen);
    }
}

static void
mx_application_constructed (GObject *object)
{
  MxSettings *settings;

  MxApplication *self = MX_APPLICATION (object);
  MxApplicationPrivate *priv = self->priv;
  gboolean success = FALSE;

#if defined (HAVE_STARTUP_NOTIFICATION) && defined (HAVE_X11)
  SnDisplay *display;
  Display *xdisplay;
  int screen;
#endif

#ifdef HAVE_DBUS
  GError *error = NULL;

  DBusGConnection *bus;
  guint32 request_status;
  gboolean unique;

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
      DBusGProxy *proxy;
      gchar *path_from_name;

      priv->is_proxy = TRUE;

      path_from_name = mx_application_get_service_path (self);
      proxy = dbus_g_proxy_new_for_name (bus,
                                         priv->service_name,
                                         path_from_name,
                                         priv->service_name);
      g_free (path_from_name);

      if (proxy)
        {
          dbus_g_proxy_add_signal (proxy, "ActionsChanged", G_TYPE_INVALID);

          dbus_g_proxy_connect_signal (proxy, "ActionsChanged",
                                       G_CALLBACK (
                                         mx_application_actions_changed_cb),
                                       self, NULL);
          g_object_unref (proxy);
        }
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

  /* No need to register here, adding an action will cause us to
   * re-register anyway (and we add raise below).
   */
  /*if (success)
    mx_application_register_dbus (self, FALSE);*/

  dbus_g_connection_unref (bus);
#endif

  /* Add default 'raise' action */
  if (!priv->is_proxy && success)
    {
      MxAction *raise_action =
        mx_action_new_full ("Raise",
                            _("Raise application"),
                            G_CALLBACK (mx_application_raise_activated_cb),
                            self);
      mx_application_add_action (self, raise_action);
    }

  settings = mx_settings_get_default ();
  if (settings)
    g_signal_connect (settings, "notify::small-screen",
                      G_CALLBACK (mx_application_notify_small_screen_cb), self);


#if defined (HAVE_STARTUP_NOTIFICATION) && defined (HAVE_X11)
  xdisplay = clutter_x11_get_default_display ();
  screen = clutter_x11_get_default_screen ();

  if (g_getenv ("LIBSN_SYNC"))
    XSynchronize (xdisplay, True);

  display = sn_display_new (xdisplay, NULL, NULL);
  priv->sn_context = sn_launchee_context_new_from_environment (display,
                                                               screen);
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

#ifdef HAVE_DBUS
static gchar *
mx_application_get_safe_name (const gchar *name)
{
  gint i;
  gchar *camel;
  gboolean raise_case;
  const gchar *name_ptr;

  /* Create an ASCII CamelCase string from arbitrary UTF-8 */
  camel = g_malloc (strlen (name) + 1);
  name_ptr = name;
  raise_case = TRUE;
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
              if (raise_case)
                {
                  camel[i] = g_ascii_toupper (*name_ptr);
                  raise_case = FALSE;
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
              raise_case = TRUE;
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
        /* Use CamelCase name for service name */
        gchar *camel = mx_application_get_safe_name (priv->name);
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
    {
      sn_launchee_context_complete (priv->sn_context);
      sn_launchee_context_unref (priv->sn_context);
    }
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

static MxWindow *
mx_application_default_create_window (MxApplication *application)
{
  MxWindow *window;
  ClutterStage *stage;

  window = mx_window_new ();
  stage = mx_window_get_clutter_stage (window);

  mx_application_add_window (application, window);

  clutter_stage_set_title (stage, application->priv->name);

  return window;
}

static void
mx_application_default_raise (MxApplication *application)
{
  MxApplicationPrivate *priv = application->priv;

  if (!priv->windows)
    return;

  mx_window_present (MX_WINDOW (g_list_last (priv->windows)->data));
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

  /**
   * MxApplication::actions-changed
   *
   * Emitted when an action has been added or removed from the MxApplication.
   */
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
mx_application_window_destroy_cb (MxWindow      *window,
                                  MxApplication *application)
{
  mx_application_remove_window (application, window);

  if (!(application->priv->flags & MX_APPLICATION_KEEP_ALIVE)
      && !(application->priv->windows))
    {
      mx_application_quit (application);
    }
}

#if defined (HAVE_STARTUP_NOTIFICATION) && defined (HAVE_X11)
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

/**
 * mx_application_new:
 * @argc: (inout): The number of arguments in argv.
 * @argv: (array length=argc) (inout) (allow-none): A pointer to an array of
 *   arguments
 * @name: Unique application name.
 * @flags: Application flags.
 *
 * Intialises everything needed to operate Clutter and use #MxApplication.
 * See clutter_init().
 *
 * Return value: the #MxApplication singleton.
 */
MxApplication *
mx_application_new (gint                *argc,
                    gchar             ***argv,
                    const gchar         *name,
                    MxApplicationFlags   flags)
{
  MxApplication *app;
  ClutterInitError result;
  GError *error = NULL;

  /* initialise clutter and the type system */
  result = clutter_init_with_args (argc, argv, name, NULL, NULL, &error);

  if (result != CLUTTER_INIT_SUCCESS)
    {
      /* abort the application if Clutter failed to initialise */
      g_error ("Failed to initialise Clutter: %s", error->message);
    }


  mx_set_locale ();

  /* create the application singleton */
  app = g_object_new (MX_TYPE_APPLICATION,
                      "flags", flags,
                      "application-name", name,
                      NULL);

  return app;
}

/**
 * mx_application_run:
 * @application: an MxApplication
 *
 * Run the main loop of the application and start processing events. This
 * function will not return until the application is quit. If the application
 * is single instance and an existing instance is already running, this will
 * cause the existing instance to be raised and the function will complete
 * immediately.
 */
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

/**
 * mx_application_quit:
 * @application: an #MxApplication
 *
 * Stop the application from running and quit the main loop. This will cause
 * the call to mx_application_run() to complete.
 */
void
mx_application_quit (MxApplication *application)
{
  clutter_main_quit ();
}

/**
 * mx_application_get_flags:
 * @application: an #MxApplication
 *
 * Get the application flags that where set on @application when created.
 *
 * Returns: the application flags
 */
MxApplicationFlags
mx_application_get_flags (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), 0);
  return application->priv->flags;
}

/**
 * mx_application_add_window:
 * @application: The #MxApplication
 * @window: (transfer full): The #MxWindow to add to the application
 *
 * Adds a window to the list of windows associated with @application. If this
 * is the first window, it will be treated as the primary window and used for
 * startup notification.
 *
 * This function does not take a reference on @window.
 */
void
mx_application_add_window (MxApplication *application,
                           MxWindow      *window)
{
  static gboolean first_window = TRUE;

  MxApplicationPrivate *priv = application->priv;

  g_return_if_fail (MX_IS_APPLICATION (application));
  g_return_if_fail (MX_IS_WINDOW (window));

  priv->windows = g_list_prepend (application->priv->windows, window);
  g_signal_connect (window, "destroy",
                    G_CALLBACK (mx_application_window_destroy_cb), application);

  /* Use the first window of the application for startup notification and
   * mirroring small-screen mode.
   */
  if (first_window)
    {
      gboolean small_screen;
#if defined (HAVE_X11) && defined (HAVE_STARTUP_NOTIFICATION)
      ClutterStage *stage;
#endif

      first_window = FALSE;

#if defined (HAVE_X11) && defined (HAVE_STARTUP_NOTIFICATION)
      stage = mx_window_get_clutter_stage (window);

      if (priv->sn_context)
        {
          if (CLUTTER_ACTOR_IS_MAPPED (stage))
            mx_application_window_map_cb (CLUTTER_ACTOR (stage),
                                          NULL,
                                          application);
          else
            g_signal_connect (stage, "notify::mapped",
                              G_CALLBACK (mx_application_window_map_cb),
                              application);
        }
#endif

      g_object_get (G_OBJECT (mx_settings_get_default ()),
                    "small-screen", &small_screen, NULL);
      mx_window_set_small_screen (window, small_screen);
    }
  else
    {
      /* FIXME: Other windows in the application should probably be marked
       *        as tool windows, or something special/clever like that.
       */
    }
}

/**
 * mx_application_remove_window:
 * @application: an #MxApplication
 * @window: an #MxWindow
 *
 * Remove the specified window from the application. This will cause the window
 * to be unreferenced and destroyed unless another reference is held on it.
 */
void
mx_application_remove_window (MxApplication *application,
                              MxWindow      *window)
{
  GList *list;

  g_return_if_fail (MX_IS_APPLICATION (application));
  g_return_if_fail (MX_IS_WINDOW (window));

  list = g_list_find (application->priv->windows, window);

  if (!list)
    {
      g_warning ("Could not remove window from application, the window was not"
                 " found in the application's window list");
      return;
    }

  g_object_unref (G_OBJECT (list->data));

  application->priv->windows = g_list_delete_link (application->priv->windows,
                                                   list);
}


/**
 * mx_application_get_windows:
 * @application: an #MxApplication
 *
 * Retrieves all windows added to @application.
 *
 * Return value: (element-type MxWindow) (transfer none): a list
 *   of #MxWindow<!-- -->s. The returned list is owned by
 *   @application and must not be altered.
 */
const GList*
mx_application_get_windows (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), NULL);

  return application->priv->windows;
}

/**
 * mx_application_create_window:
 * @application: The #MxApplication
 *
 * Creates a window and associates it with the application.
 *
 * Return value: (transfer none): An #MxWindow.
 */
MxWindow *
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
dbus_glib_marshal_mx_application_action (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data)
{
  GCClosure *cc = (GCClosure*) closure;
  MxAction *action;
  GVariant *variant = NULL;

  g_return_if_fail (return_value != NULL);

  action = (MxAction *)(marshal_data ? marshal_data : cc->callback);

  /* collect the action parameter */
  if (n_param_values == 3)
    {
      GArray *array;
      array = g_value_get_boxed (&param_values[1]);
      variant = g_variant_new_from_data (g_action_get_parameter_type (G_ACTION (action)),
                                         array->data, array->len,
                                         FALSE, NULL, NULL);
    }

  if (variant)
    g_action_activate (G_ACTION (action), variant);
  else
    g_signal_emit_by_name (action, "activated", NULL);

  g_value_set_boolean (return_value, TRUE);
}

/* The following marshaller was generated with dbus-binding-tool */
static void
dbus_glib_marshal_mx_application_BOOLEAN__POINTER_POINTER (
                                                  GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER_POINTER)(gpointer     data1,
                                                            gpointer     arg_1,
                                                            gpointer     arg_2,
                                                            gpointer     data2);
  register GMarshalFunc_BOOLEAN__POINTER_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

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
  callback = (GMarshalFunc_BOOLEAN__POINTER_POINTER) (marshal_data ?
                                                      marshal_data :
                                                      cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

static gboolean
mx_application_get_actions_cb (MxApplication  *application,
                               GPtrArray     **actions,
                               GError        **error)
{
  GHashTableIter iter;
  gpointer key, value;

  MxApplicationPrivate *priv = application->priv;
  GValue string_value = { 0 };
  GValue bool_value = { 0 };

  *actions = g_ptr_array_new ();
  g_value_init (&string_value, G_TYPE_STRING);
  g_value_init (&bool_value, G_TYPE_BOOLEAN);

  g_hash_table_iter_init (&iter, priv->actions);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      const gchar *name, *display_name;

      MxAction *action = value;
      GValueArray *value_array = g_value_array_new (3);

      name = mx_action_get_name (action);
      display_name = mx_action_get_display_name (action);
      if (!name)
        name = "";
      if (!display_name)
        display_name = name;

      g_value_set_string (&string_value, name);
      g_value_array_append (value_array, &string_value);

      g_value_set_string (&string_value, display_name);
      g_value_array_append (value_array, &string_value);

      g_value_set_boolean (&bool_value, mx_action_get_active (action));
      g_value_array_append (value_array, &bool_value);

      g_ptr_array_add (*actions, value_array);
    }

  g_value_unset (&string_value);
  g_value_unset (&bool_value);

  return TRUE;
}
#endif

static void
mx_application_register_actions (MxApplication *application)
{
#ifdef HAVE_DBUS
  GString *data;
  GHashTableIter iter;
  DBusGObjectInfo *info;
  gint i, n_methods, offset;
  DBusGMethodInfo *method_info;

  MxApplicationPrivate *priv = application->priv;
  const gchar get_actions_data[] =
    "\0GetActions\0S\0actions\0O\0F\0N\0a(ssb)\0";

  /* Unregister the object first as we'll be messing with the
   * DBusGObjectInfo that backs the object on the bus.
   */
  mx_application_register_dbus (application, TRUE);

  info = &priv->object_info;
  n_methods = g_hash_table_size (priv->actions) + 1;

  /* Fill in the basic information of the object info struct */
  info->format_version = 0;
  info->n_method_infos = n_methods;

  /* Generate the method info to map actions on the bus object */
  g_free ((gpointer)info->method_infos);
  info->method_infos = g_new (DBusGMethodInfo, n_methods);

  /* First add the GetActions function */
  data = g_string_new (priv->service_name);
  g_string_append_len (data, get_actions_data, sizeof (get_actions_data));
  method_info = (DBusGMethodInfo *)&info->method_infos[0];
  method_info->function = (GCallback)mx_application_get_actions_cb;
  method_info->marshaller =
    dbus_glib_marshal_mx_application_BOOLEAN__POINTER_POINTER;
  method_info->data_offset = 0;

  /* Now the methods for the actions */
  g_hash_table_iter_init (&iter, priv->actions);
  for (i = 1, offset = data->len; i < n_methods; i++)
    {
      MxAction *action;
      gchar *name;

      method_info = (DBusGMethodInfo *)&info->method_infos[i];

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
      if (g_action_get_parameter_type (G_ACTION (action)))
        {
          g_string_append (data, "action-parameter");
          g_string_append_c (data, '\0');
          g_string_append_c (data, 'I');
          g_string_append_c (data, '\0');
          g_string_append (data, "ay");
          g_string_append_c (data, '\0');
        }
      else
        {
          g_string_append_c (data, '\0');
        }
      g_string_append_c (data, '\0');

      g_free (name);

      /* Fill in method info */
      method_info->function = (GCallback)action;
      method_info->marshaller =
        dbus_glib_marshal_mx_application_action;
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

/**
 * mx_application_add_action:
 * @application: an #MxApplication
 * @action: an #MxAction
 *
 * Add an action to the application.
 */
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

/**
 * mx_application_remove_action:
 * @application: an #MxApplication
 * @name: name of the action to remove
 *
 * Remove the action with the specified name from the application.
 */
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

#ifdef HAVE_DBUS
static DBusGProxy *
mx_application_get_dbus_proxy (MxApplication *application)
{
  gchar *path_from_name;
  DBusGConnection *bus;
  DBusGProxy *proxy;

  GError *error = NULL;
  MxApplicationPrivate *priv = application->priv;

  if (!(bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error)))
    {
      g_warning (G_STRLOC "%s", error->message);
      g_error_free (error);
      return NULL;
    }

  path_from_name = mx_application_get_service_path (application);
  proxy = dbus_g_proxy_new_for_name (bus,
                                     priv->service_name,
                                     path_from_name,
                                     priv->service_name);
  g_free (path_from_name);
  dbus_g_connection_unref (bus);

  return proxy;
}

static void
mx_application_proxy_action_cb (MxAction      *action,
                                MxApplication *application)
{
  mx_application_invoke_action (application, mx_action_get_name (action));
}
#endif

/**
 * mx_application_get_actions:
 * @application: an #MxApplication
 *
 * Retrieves all actions registered on @application.
 *
 * Return value: (element-type Mx.Action) (transfer container): a list
 *   of #MxAction<!-- -->s. Use g_list_free() on the returned list
 *   when done.
 */
GList *
mx_application_get_actions (MxApplication *application)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->is_proxy)
    {
#ifdef HAVE_DBUS
      /* Refresh our list of actions if necessary */
      if (!priv->actions_valid)
        {
          DBusGProxy *proxy = mx_application_get_dbus_proxy (application);

          if (proxy)
            {
              GType array_type, struct_type;
              GPtrArray *actions;

              GError *error = NULL;

              /* All this code is just for calling a dbus method with the
               * signiature 'a(ssb)' (the GetActions method) and creating
               * corresponding MxAction objects.
               */
              struct_type = dbus_g_type_get_struct ("GValueArray",
                                                    G_TYPE_STRING,
                                                    G_TYPE_STRING,
                                                    G_TYPE_BOOLEAN,
                                                    G_TYPE_INVALID);
              array_type = dbus_g_type_get_collection ("GPtrArray",
                                                       struct_type);

              if (!dbus_g_proxy_call (proxy,
                                      "GetActions",
                                      &error,
                                      G_TYPE_INVALID,
                                      array_type,
                                      &actions,
                                      G_TYPE_INVALID))
                {
                  g_warning (G_STRLOC "%s", error->message);
                  g_error_free (error);
                }
              else
                {
                  guint i;

                  g_hash_table_remove_all (priv->actions);
                  for (i = 0; i < actions->len; i++)
                    {
                      GValue *name, *display_name, *active;
                      MxAction *action;

                      GValueArray *value_array = g_ptr_array_index (actions, i);

                      name = g_value_array_get_nth (value_array, 0);
                      display_name = g_value_array_get_nth (value_array, 1);
                      active = g_value_array_get_nth (value_array, 2);

                      action =
                        mx_action_new_full (g_value_get_string (name),
                                            g_value_get_string (display_name),
                                            G_CALLBACK (
                                              mx_application_proxy_action_cb),
                                            application);
                      mx_action_set_active (action,
                                            g_value_get_boolean (active));

                      g_hash_table_insert (priv->actions,
                                           g_value_dup_string (name),
                                           action);

                      g_value_array_free (value_array);
                    }
                  g_ptr_array_free (actions, TRUE);
                }
              g_object_unref (proxy);
            }
        }
#endif
    }

  return g_hash_table_get_values (priv->actions);
}

/**
 * mx_application_invoke_action:
 * @application: an #MxApplication
 * @name: name of the action to invoke
 *
 * Run the named action for the application.
 */
void
mx_application_invoke_action (MxApplication *application,
                              const gchar   *name)
{
  mx_application_invoke_action_with_parameter (application, name, NULL);
}

/**
 * mx_application_invoke_action_with_parameter:
 * @application: an #MxApplication
 * @name: name of the action to invoke
 * @variant: parameter for the action
 *
 * Run the named action for the application, passing @variant as the parameter
 * for the action.
 *
 * Since: 1.4
 */
void
mx_application_invoke_action_with_parameter (MxApplication *application,
                                             const gchar   *name,
                                             GVariant      *variant)
{
  MxApplicationPrivate *priv = application->priv;

  if (priv->is_proxy)
    {
#ifdef HAVE_DBUS
      DBusGProxy *proxy = mx_application_get_dbus_proxy (application);
      GArray data = { 0, };

      if (variant)
        {
          data.data = g_new0 (gchar, g_variant_get_size (variant));
          g_variant_store (variant, data.data);
          data.len = g_variant_get_size (variant);
        }


      if (proxy)
        {
          GError *error = NULL;
          gchar *safe_name = mx_application_get_safe_name (name);
          if (!dbus_g_proxy_call (proxy,
                                  safe_name,
                                  &error,
                                  (variant) ? DBUS_TYPE_G_UCHAR_ARRAY : G_TYPE_INVALID,
                                  (variant) ? &data : G_TYPE_INVALID,
                                  G_TYPE_INVALID,
                                  G_TYPE_INVALID))
            {
              g_warning (G_STRLOC "%s", error->message);
              g_error_free (error);
            }
          g_free (safe_name);
          g_object_unref (proxy);

          if (variant)
            {
              g_free (data.data);
            }
        }
#endif
    }
  else
    {
      MxAction *action = g_hash_table_lookup (priv->actions, name);
      if (action)
        {
          if (variant)
            g_action_activate (G_ACTION (action), variant);
          else
            g_signal_emit_by_name (action, "activated", NULL);
        }
    }
}

/**
 * mx_application_is_running:
 * @application: an #MxApplication
 *
 * Query whether #MxApplication is running. This will also return #TRUE if the
 * given #MxApplication is single instance and there is an instance already
 * running.
 *
 * Returns: #TRUE if the application is running
 */
gboolean
mx_application_is_running (MxApplication *application)
{
  g_return_val_if_fail (MX_IS_APPLICATION (application), FALSE);

  return (application->priv->is_proxy || application->priv->is_running);
}

