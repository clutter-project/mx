/*
 * mx-window-base.c: A top-level window base class
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
 *             Chris Lord <chris@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-window-base.h"
#include "mx-toolbar.h"
#include "mx-focus-manager.h"
#include "mx-private.h"
#include "mx-marshal.h"

G_DEFINE_ABSTRACT_TYPE (MxWindowBase, mx_window_base, G_TYPE_OBJECT)

static GQuark window_quark = 0;

typedef struct
{
  guint has_toolbar   : 1;
  guint small_screen  : 1;

  gchar      *icon_name;
  CoglHandle  icon_texture;

  ClutterActor *stage;
  ClutterActor *toolbar;
  ClutterActor *child;
  ClutterActor *resize_grip;
} MxWindowBasePrivate;

#define WINDOW_BASE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW_BASE, MxWindowBasePrivate))

enum
{
  PROP_STYLE = 1,
  PROP_STYLE_CLASS,
  PROP_STYLE_PSEUDO_CLASS,
  PROP_HAS_TOOLBAR,
  PROP_TOOLBAR,
  PROP_SMALL_SCREEN,
  PROP_ICON_NAME,
  PROP_ICON_COGL_TEXTURE,
  PROP_CLUTTER_STAGE,
  PROP_CHILD
};

static void
mx_window_base_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (object);

  switch (property_id)
    {
    case PROP_HAS_TOOLBAR:
      g_value_set_boolean (value, priv->has_toolbar);
      break;

    case PROP_TOOLBAR:
      g_value_set_object (value, priv->toolbar);
      break;

    case PROP_SMALL_SCREEN:
      g_value_set_boolean (value, priv->small_screen);
      break;

    case PROP_ICON_NAME:
      g_value_set_string (value, priv->icon_name);
      break;

    case PROP_ICON_COGL_TEXTURE:
      g_value_set_pointer (value, priv->icon_texture);
      break;

    case PROP_CLUTTER_STAGE:
      g_value_set_object (value, priv->stage);
      break;

    case PROP_CHILD:
      g_value_set_object (value, priv->child);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_base_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_HAS_TOOLBAR:
      mx_window_set_has_toolbar (MX_WINDOW (object),
                                 g_value_get_boolean (value));
      break;

    case PROP_SMALL_SCREEN:
      mx_window_set_small_screen (MX_WINDOW (object),
                                  g_value_get_boolean (value));
      break;

    case PROP_ICON_NAME:
      mx_window_set_icon_name (MX_WINDOW (object),
                               g_value_get_string (value));
      break;

    case PROP_ICON_COGL_TEXTURE:
      mx_window_set_icon_from_cogl_texture (MX_WINDOW (object),
                                            g_value_get_pointer (value));
      break;

    case PROP_CLUTTER_STAGE:
      WINDOW_BASE_PRIVATE (object)->stage =
        (ClutterActor *)g_value_get_object (value);
      break;

    case PROP_CHILD:
      mx_window_set_child (MX_WINDOW (object),
                           (ClutterActor *)g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_base_dispose (GObject *object)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (object);

  if (priv->icon_texture)
    {
      cogl_handle_unref (priv->icon_texture);
      priv->icon_texture = NULL;
    }

  if (priv->toolbar)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->toolbar),
                                    (gpointer *)&priv->toolbar);
      priv->toolbar = NULL;
    }

  if (priv->resize_grip)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->resize_grip),
                                    (gpointer *)&priv->resize_grip);
      priv->resize_grip = NULL;
    }

  if (priv->stage)
    {
      g_object_set_qdata (G_OBJECT (priv->stage), window_quark, NULL);

      /* Destroying the stage will destroy all the actors inside it */
      g_object_remove_weak_pointer (G_OBJECT (priv->stage),
                                    (gpointer *)&priv->stage);
      ClutterActor *stage = priv->stage;
      priv->stage = NULL;
      clutter_actor_destroy (stage);
    }

  G_OBJECT_CLASS (mx_window_base_parent_class)->dispose (object);
}

static void
mx_window_base_finalize (GObject *object)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (object);

  g_free (priv->icon_name);

  G_OBJECT_CLASS (mx_window_base_parent_class)->finalize (object);
}

static void
mx_window_post_paint_cb (ClutterActor *actor, MxWindow *window)
{
  gfloat width, height;

  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (window);

  /* If we're in small-screen or fullscreen mode, or we don't have the toolbar,
   * we don't want a frame or a resize handle.
   */
  if (!priv->has_toolbar || priv->small_screen ||
      clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
    return;

  /* paint frame */

  clutter_actor_get_size (actor, &width, &height);
  cogl_set_source_color4f (0.2, 0.2, 0.2, 1);

  cogl_rectangle (0, 0, width, 1);
  cogl_rectangle (0, height - 1, width, height);

  cogl_rectangle (0, 1, 1, height - 1);
  cogl_rectangle (width - 1, 1, width, height - 1);
}

static void
mx_window_allocation_changed_cb (ClutterActor           *actor,
                                 ClutterActorBox        *box,
                                 ClutterAllocationFlags  flags,
                                 MxWindow               *window)
{
  MxPadding padding;
  gboolean from_toolbar;
  MxWindowBasePrivate *priv;
  gfloat width, height, toolbar_height, stage_width, stage_height;

  /* Note, ideally this would happen just before allocate, but there's
   * no signal we can connect to for that without overriding an actor.
   *
   * Instead, we do this each time the allocation is changed on the stage
   * or the toolbar. This shouldn't be a frequent occurence, but
   * unfortunately can happen multiple times before the actual relayout
   * happens.
   */

  priv = WINDOW_BASE_PRIVATE (window);

  from_toolbar = (actor == priv->toolbar);
  actor = clutter_actor_get_stage (actor);

  clutter_actor_get_size (actor, &stage_width, &stage_height);

  if (!priv->has_toolbar || priv->small_screen ||
      clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
      padding.top = padding.right = padding.bottom = padding.left = 0;
  else
      padding.top = padding.right = padding.bottom = padding.left = 1;

  if (priv->has_toolbar && priv->toolbar)
    {
      clutter_actor_get_preferred_height (priv->toolbar,
                                          stage_width - padding.left -
                                          padding.right,
                                          NULL, &toolbar_height);

      if (!from_toolbar)
        {
          clutter_actor_set_position (priv->toolbar, padding.left, padding.top);
          g_object_set (G_OBJECT (priv->toolbar),
                        "natural-width",
                        stage_width - padding.left - padding.right,
                        NULL);
        }
    }
  else
    {
      toolbar_height = 0;
    }

  if (priv->child)
    {
      g_object_set (G_OBJECT (priv->child),
                    "natural-width", stage_width - padding.left - padding.right,
                    "natural-height", stage_height - toolbar_height -
                                      padding.top - padding.bottom,
                    "x", padding.left,
                    "y", toolbar_height + padding.top,
                    NULL);
    }

  if (priv->resize_grip)
    {
      clutter_actor_get_preferred_size (priv->resize_grip,
                                        NULL, NULL,
                                        &width, &height);
      clutter_actor_set_position (priv->resize_grip,
                                  stage_width - width - padding.right,
                                  stage_height - height - padding.bottom);
    }
}

static void
mx_window_fullscreen_set_cb (ClutterStage *stage,
                             GParamSpec   *pspec,
                             MxWindow     *self)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (self);

  /* If we're in small-screen mode, make sure the size gets reset
   * correctly.
   */
  if (!clutter_stage_get_fullscreen (stage))
    {
      if (!priv->small_screen && priv->resize_grip && priv->has_toolbar &&
          clutter_stage_get_user_resizable (stage))
        {
          clutter_actor_show (priv->resize_grip);
          if (priv->child)
            clutter_actor_raise (priv->resize_grip, priv->child);
        }
    }
  else if (priv->resize_grip)
    clutter_actor_hide (priv->resize_grip);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (stage));
}

static void
mx_window_destroy_cb (ClutterStage *stage, MxWindowBase *self)
{
  WINDOW_BASE_PRIVATE (self)->stage = NULL;
  g_signal_emit_by_name (self, "destroy");
}

static void
mx_window_actor_added_cb (ClutterContainer *container,
                          ClutterActor     *actor,
                          MxWindowBase     *self)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (self);
  if (priv->resize_grip && priv->child)
    clutter_actor_raise (priv->resize_grip, priv->child);
}

static void
mx_window_actor_removed_cb (ClutterContainer *container,
                            ClutterActor     *actor,
                            MxWindowBase     *self)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (self);

  if (actor == priv->child)
    {
      g_object_set (G_OBJECT (priv->child),
                    "natural-width-set", FALSE,
                    "natural-height-set", FALSE,
                    NULL);
      priv->child = NULL;
      g_object_notify (G_OBJECT (self), "child");
    }
}

static void
mx_window_user_resizable_cb (ClutterStage *stage,
                             GParamSpec   *pspec,
                             MxWindowBase *self)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (self);

  if (!clutter_stage_get_user_resizable (stage))
    clutter_actor_hide (priv->resize_grip);
  else
    {
      if (!clutter_stage_get_fullscreen (stage) &&
          !priv->small_screen &&
          priv->has_toolbar)
        {
          clutter_actor_show (priv->resize_grip);
          if (priv->child)
            clutter_actor_raise (priv->resize_grip, priv->child);
        }
    }
}

static void
mx_window_base_constructed (GObject *object)
{
  MxWindowBasePrivate *priv = WINDOW_BASE_PRIVATE (object);

  if (!priv->stage)
    {
      priv->stage = clutter_stage_new ();
      clutter_stage_set_user_resizable ((ClutterStage *)priv->stage, TRUE);
    }
  g_object_add_weak_pointer (G_OBJECT (priv->stage),
                             (gpointer *)&priv->stage);
  g_object_set_qdata (G_OBJECT (priv->stage), window_quark, object);

  priv->has_toolbar = TRUE;
  priv->toolbar = mx_toolbar_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage), priv->toolbar);
  g_object_add_weak_pointer (G_OBJECT (priv->toolbar),
                             (gpointer *)&priv->toolbar);

  priv->resize_grip = mx_icon_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->resize_grip), "ResizeGrip");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
                               priv->resize_grip);
  if (clutter_stage_get_fullscreen (CLUTTER_STAGE (priv->stage)) ||
      !clutter_stage_get_user_resizable (CLUTTER_STAGE (priv->stage)) ||
      !priv->has_toolbar)
    clutter_actor_hide (priv->resize_grip);
  g_object_add_weak_pointer (G_OBJECT (priv->resize_grip),
                             (gpointer *)&priv->resize_grip);

  g_signal_connect_after (priv->stage, "paint",
                          G_CALLBACK (mx_window_post_paint_cb), object);
  g_signal_connect (priv->stage, "allocation-changed",
                    G_CALLBACK (mx_window_allocation_changed_cb), object);
  g_signal_connect (priv->toolbar, "allocation-changed",
                    G_CALLBACK (mx_window_allocation_changed_cb), object);
  g_signal_connect (priv->stage, "notify::fullscreen-set",
                    G_CALLBACK (mx_window_fullscreen_set_cb), object);
  g_signal_connect (priv->stage, "destroy",
                    G_CALLBACK (mx_window_destroy_cb), object);
  g_signal_connect (priv->stage, "actor-added",
                    G_CALLBACK (mx_window_actor_added_cb), object);
  g_signal_connect (priv->stage, "actor-removed",
                    G_CALLBACK (mx_window_actor_removed_cb), object);
  g_signal_connect (priv->stage, "notify::user-resizable",
                    G_CALLBACK (mx_window_user_resizable_cb), object);

  g_object_set (G_OBJECT (priv->stage), "use-alpha", TRUE, NULL);
}

static void
mx_window_base_class_init (MxWindowBaseClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWindowBasePrivate));

  object_class->get_property = mx_window_base_get_property;
  object_class->set_property = mx_window_base_set_property;
  object_class->dispose = mx_window_base_dispose;
  object_class->finalize = mx_window_base_finalize;
  object_class->constructed = mx_window_base_constructed;

  pspec = g_param_spec_boolean ("has-toolbar",
                                "Has toolbar",
                                "Window should have a toolbar.",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_HAS_TOOLBAR, pspec);

  pspec = g_param_spec_object ("toolbar",
                               "Toolbar",
                               "The MxToolbar associated with the window.",
                               MX_TYPE_TOOLBAR,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_TOOLBAR, pspec);

  pspec = g_param_spec_boolean ("small-screen",
                                "Small screen",
                                "Window should occupy the entire screen "
                                "contents, without explicitly setting "
                                "itself fullscreen.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SMALL_SCREEN, pspec);

  pspec = g_param_spec_string ("icon-name",
                               "Icon name",
                               "Icon name to use for the window icon.",
                               NULL,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ICON_NAME, pspec);

  pspec = g_param_spec_string ("icon-cogl-texture",
                               "Icon CoglTexture",
                               "CoglTexture to use for the window icon.",
                               NULL,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ICON_COGL_TEXTURE, pspec);

  pspec = g_param_spec_object ("clutter-stage",
                               "Clutter stage",
                               "ClutterStage to use as the window.",
                               CLUTTER_TYPE_STAGE,
                               MX_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CLUTTER_STAGE, pspec);

  pspec = g_param_spec_object ("child",
                               "Child",
                               "ClutterActor used as the window child.",
                               CLUTTER_TYPE_ACTOR,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CHILD, pspec);

  window_quark = g_quark_from_static_string ("mx-window");
}

static void
mx_window_base_init (MxWindowBase *self)
{
}

CoglHandle
_mx_window_get_icon_cogl_texture (MxWindow *window)
{
  CoglHandle texture;
  MxWindowBasePrivate *priv;

  g_return_val_if_fail (MX_IS_WINDOW (window), COGL_INVALID_HANDLE);

  priv = WINDOW_BASE_PRIVATE (window);
  texture = priv->icon_texture;
  priv->icon_texture = NULL;

  return texture;
}

ClutterActor *
_mx_window_get_resize_grip (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return WINDOW_BASE_PRIVATE (window)->resize_grip;
}

/**
 * mx_window_new:
 *
 * Creates a new #MxWindow.
 *
 * Returns: A #MxWindow
 */
MxWindow *
mx_window_new (void)
{
  return g_object_new (MX_TYPE_WINDOW, NULL);
}

/**
 * mx_window_new_with_clutter_stage:
 * @stage: A #ClutterStage
 *
 * Creates a new #MxWindow, using @stage as the backing #ClutterStage. This
 * function is meant for use primarily for embedding a #MxWindow into
 * a foreign stage when using a Clutter toolkit integration library.
 *
 * Returns: A #MxWindow
 */
MxWindow *
mx_window_new_with_clutter_stage (ClutterStage *stage)
{
  return g_object_new (MX_TYPE_WINDOW, "clutter-stage", stage, NULL);
}

/**
 * mx_window_get_for_stage:
 * @stage: A #ClutterStage
 *
 * Gets the #MxWindow parent of the #ClutterStage, if it exists.
 *
 * Returns: A #MxWindow, or %NULL
 */
MxWindow *
mx_window_get_for_stage (ClutterStage *stage)
{
  g_return_val_if_fail (CLUTTER_IS_STAGE (stage), NULL);
  return (MxWindow *)g_object_get_qdata (G_OBJECT (stage), window_quark);
}

/**
 * mx_window_set_child:
 * @window: A #MxWindow
 * @actor: A #ClutterActor
 *
 * Adds @actor to the window and sets it as the primary child. When the
 * stage managed in the window changes size, the child will be resized
 * to match it.
 */
void
mx_window_set_child (MxWindow     *window,
                     ClutterActor *actor)
{
  MxWindowBasePrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (actor == NULL || CLUTTER_IS_ACTOR (actor));

  priv = WINDOW_BASE_PRIVATE (window);
  if (!priv->stage)
    return;

  if (priv->child == actor)
    return;

  if (priv->child)
    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage),
                                    priv->child);

  if (actor)
    {
      priv->child = actor;
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
                                   priv->child);
      /*if (CLUTTER_ACTOR_IS_MAPPED (priv->stage))
        mx_window_pre_paint_cb (priv->stage, window);*/
    }

  g_object_notify (G_OBJECT (window), "child");
}

/**
 * mx_window_get_child:
 * @window: A #MxWindow
 *
 * Get the primary child of the window. See mx_window_set_child().
 *
 * Returns: A #ClutterActor, or %NULL
 */
ClutterActor*
mx_window_get_child (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return WINDOW_BASE_PRIVATE (window)->child;
}

/**
 * mx_window_set_has_toolbar:
 * @window: A #MxWindow
 * @toolbar: %TRUE if the toolbar should be displayed
 *
 * Sets whether the window has a toolbar or not. If the window has a toolbar,
 * client-side window decorations will be enabled.
 */
void
mx_window_set_has_toolbar (MxWindow *window,
                           gboolean  toolbar)
{
  MxWindowBasePrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = WINDOW_BASE_PRIVATE (window);

  if (priv->has_toolbar != toolbar)
    {
      priv->has_toolbar = toolbar;

      if (!toolbar)
        {
          clutter_actor_hide (priv->toolbar);
          clutter_actor_hide (priv->resize_grip);
        }
      else
        {
          clutter_actor_show (priv->toolbar);
          if (clutter_stage_get_user_resizable ((ClutterStage *)priv->stage))
            clutter_actor_show (priv->resize_grip);
        }

      g_object_notify (G_OBJECT (window), "has-toolbar");
    }
}

/**
 * mx_window_get_has_toolbar:
 * @window: A #MxWindow
 *
 * Determines whether the window has a toolbar or not.
 * See mx_window_set_has_toolbar().
 *
 * Returns: %TRUE if the window has a toolbar, otherwise %FALSE
 */
gboolean
mx_window_get_has_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);

  return WINDOW_BASE_PRIVATE (window)->has_toolbar;
}

/**
 * mx_window_get_toolbar:
 * @window: A #MxWindow
 *
 * Retrieves the toolbar associated with the window.
 *
 * Returns: A #MxToolbar
 */
MxToolbar *
mx_window_get_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return (MxToolbar*) WINDOW_BASE_PRIVATE (window)->toolbar;
}

/**
 * mx_window_get_small_screen:
 * @window: A #MxWindow
 *
 * Determines if the window is in small-screen mode.
 * See mx_window_set_small_screen().
 *
 * Returns: %TRUE if the window is in small-screen mode, otherwise %FALSE
 */
gboolean
mx_window_get_small_screen (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);
  return WINDOW_BASE_PRIVATE (window)->small_screen;
}

/**
 * mx_window_set_small_screen:
 * @window: A #MxWindow
 * @small_screen: %TRUE if small-screen mode should be enabled
 *
 * Enables or disables small-screen mode. This mode is meant primarily
 * for platforms with limited screen-space, such as netbooks. When enabled,
 * the window will take up all available room and will disable moving and
 * resizing.
 */
void
mx_window_set_small_screen (MxWindow *window, gboolean small_screen)
{
  MxWindowBasePrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = WINDOW_BASE_PRIVATE (window);
  if (priv->small_screen != small_screen)
    {
      priv->small_screen = small_screen;
      g_object_notify (G_OBJECT (window), "small-screen");
    }
}

/**
 * mx_window_set_icon_name:
 * @window: A #MxWindow
 * @icon_name: (allow-none): An icon name, or %NULL
 *
 * Set an icon-name to use for the window icon. The icon will be looked up
 * from the default theme.
 */
void
mx_window_set_icon_name (MxWindow *window, const gchar *icon_name)
{
  MxWindowBasePrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = WINDOW_BASE_PRIVATE (window);

  if (priv->icon_name && icon_name && g_str_equal (priv->icon_name, icon_name))
    return;
  if (!priv->icon_name && !icon_name)
    return;

  g_free (priv->icon_name);
  priv->icon_name = g_strdup (icon_name);

  g_object_notify (G_OBJECT (window), "icon-name");
}

/**
 * mx_window_get_icon_name:
 * @window: A #MxWindow
 *
 * Gets the currently set window icon name. This will be %NULL if there is none
 * set, or the icon was set with mx_window_set_icon_from_cogl_texture().
 *
 * Returns: The window icon name, or %NULL
 */
const gchar *
mx_window_get_icon_name (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return WINDOW_BASE_PRIVATE (window)->icon_name;
}

/**
 * mx_window_set_icon_from_cogl_texture:
 * @window: A #MxWindow
 * @texture: A #CoglHandle for a texture
 *
 * Sets the window icon from a texture. This will take precedence over
 * any currently set icon-name.
 */
void
mx_window_set_icon_from_cogl_texture (MxWindow   *window,
                                      CoglHandle  texture)
{
  MxWindowBasePrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (texture != NULL);

  priv = WINDOW_BASE_PRIVATE (window);

  if (priv->icon_name)
    {
      g_free (priv->icon_name);
      priv->icon_name = NULL;
      g_object_notify (G_OBJECT (window), "icon-name");
    }

  if (priv->icon_texture)
    {
      cogl_handle_unref (priv->icon_texture);
      priv->icon_texture = NULL;
    }

  if (texture)
    priv->icon_texture = cogl_handle_ref (texture);

  g_object_notify (G_OBJECT (window), "icon-cogl-texture");
}

/**
 * mx_window_get_clutter_stage:
 * @window: A #MxWindow
 *
 * Gets the #ClutterStage managed by the window.
 *
 * Returns: A #ClutterStage
 */
ClutterStage *
mx_window_get_clutter_stage (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return (ClutterStage *)WINDOW_BASE_PRIVATE (window)->stage;
}

