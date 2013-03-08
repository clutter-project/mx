/*
 * mx-application: application class
 *
 * Copyright 2010, 2012 Intel Corporation.
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

G_DEFINE_TYPE (MxApplication, mx_application, G_TYPE_APPLICATION)

#define APPLICATION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_APPLICATION, MxApplicationPrivate))

struct _MxApplicationPrivate
{
  GList              *windows;
  gchar              *name;

#ifdef HAVE_STARTUP_NOTIFICATION
  SnLauncheeContext  *sn_context;
#endif
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
mx_application_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  switch (property_id)
    {
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
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_application_dispose (GObject *object)
{
#if 0
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;
#endif

  G_OBJECT_CLASS (mx_application_parent_class)->dispose (object);
}

static void
mx_application_finalize (GObject *object)
{
#ifdef HAVE_STARTUP_NOTIFICATION
  MxApplicationPrivate *priv = MX_APPLICATION (object)->priv;

  if (priv->sn_context)
    {
      sn_launchee_context_complete (priv->sn_context);
      sn_launchee_context_unref (priv->sn_context);
    }
#endif

  G_OBJECT_CLASS (mx_application_parent_class)->finalize (object);
}

/**
 * mx_application_startup:
 *
 * Initialise Clutter, MxSettings and set the locale after the application has
 * been registered. Create the startup notification context if startup
 * notification support has been enabled.
 */
static void
mx_application_startup (GApplication *application)
{
  ClutterInitError error;
  MxSettings *settings;

  MxApplication *self = MX_APPLICATION (application);


  /* chain up */
  G_APPLICATION_CLASS (mx_application_parent_class)->startup (application);


  error = clutter_init (0, 0);

  if (error != CLUTTER_INIT_SUCCESS)
    return;

  mx_set_locale ();


  settings = mx_settings_get_default ();
  if (settings)
    g_signal_connect (settings, "notify::small-screen",
                      G_CALLBACK (mx_application_notify_small_screen_cb), self);


#if defined (HAVE_STARTUP_NOTIFICATION) && defined (HAVE_X11)
    {
      MxApplicationPrivate *priv = self->priv;
      SnDisplay *display;
      Display *xdisplay;
      int screen;

      xdisplay = clutter_x11_get_default_display ();
      screen = clutter_x11_get_default_screen ();

      if (g_getenv ("LIBSN_SYNC"))
        XSynchronize (xdisplay, True);

      display = sn_display_new (xdisplay, NULL, NULL);
      priv->sn_context = sn_launchee_context_new_from_environment (display,
                                                                   screen);
    }
#endif
}

/**
 * mx_application_activate:
 *
 * Present the windows associated with the application when it is activated
 */
static void
mx_application_activate (GApplication *application)
{
  MxApplicationPrivate *priv = MX_APPLICATION (application)->priv;
  GList *l;

  for (l = priv->windows; l; l = g_list_next (l))
    mx_window_present (l->data);
}

static void
mx_application_class_init (MxApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxApplicationPrivate));

  object_class->get_property = mx_application_get_property;
  object_class->set_property = mx_application_set_property;
  object_class->dispose = mx_application_dispose;
  object_class->finalize = mx_application_finalize;

  app_class->startup = mx_application_startup;
  app_class->activate = mx_application_activate;
}

static void
mx_application_init (MxApplication *self)
{
  self->priv = APPLICATION_PRIVATE (self);
}

static void
mx_application_window_destroy_cb (MxWindow      *window,
                                  MxApplication *application)
{
  mx_application_remove_window (application, window);
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
 * @application_id: Unique application name.
 * @flags: Application flags.
 *
 * Intialises everything needed to operate Clutter and use #MxApplication.
 * See clutter_init().
 *
 * Return value: the #MxApplication singleton.
 */
MxApplication *
mx_application_new (const gchar *application_id,
                    GApplicationFlags flags)
{
  MxApplication *app;

  g_return_val_if_fail (g_application_id_is_valid (application_id), NULL);

  app = g_object_new (MX_TYPE_APPLICATION,
                      "application-id", application_id,
                      "flags", flags,
                      NULL);

  return app;
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

  /* add a hold to the application, and release it when the window is
   * destroyed */
  g_application_hold (G_APPLICATION (application));
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

  g_application_release (G_APPLICATION (application));

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
 * @application: An #MxApplication
 * @window_title: Title for the new window
 *
 * Creates a new #MxWindow and adds it to MxApplication. The application must be
 * registered before this function is run.
 *
 * Returns: (transfer full): The newly created MxWindow
 */
MxWindow*
mx_application_create_window (MxApplication *application,
                              const gchar   *window_title)
{
  MxWindow *window;

  g_return_val_if_fail (g_application_get_is_registered (G_APPLICATION (application)),
                        NULL);

  window = mx_window_new ();

  mx_window_set_title (window, window_title);

  mx_application_add_window (application, window);

  return window;
}
