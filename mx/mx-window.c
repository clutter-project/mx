/*
 * mx-window.c: A top-level window class
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

/**
 * SECTION:mx-window
 * @short_description: an object that represents a platform-specific window
 *
 * #MxWindow is a platform-specific window, providing functions for moving,
 * resizing, icons and rotation. Every window has an associated #ClutterStage
 * in which its children are kept. This #ClutterStage can be used in the
 * normal way, but to take advantage of automatic resizing and rotation, the
 * #MxWindow functions should be used.
 *
 * When the #MxWindow loses its last reference, its contents are destroyed.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-window.h"
#include "mx-native-window.h"
#include "mx-toolbar.h"
#include "mx-focus-manager.h"
#include "mx-private.h"
#include "mx-marshal.h"

#ifdef HAVE_X11
#include "x11/mx-window-x11.h"
#endif

G_DEFINE_TYPE (MxWindow, mx_window, G_TYPE_OBJECT)

static GQuark window_quark = 0;

struct _MxWindowPrivate
{
  MxNativeWindow *native_window;

  guint has_toolbar   : 1;
  guint small_screen  : 1;
  guint fullscreen    : 1;
  guint rotate_size   : 1;

  gchar      *icon_name;
  CoglHandle  icon_texture;

  ClutterActor *stage;
  ClutterActor *toolbar;
  ClutterActor *child;
  ClutterActor *resize_grip;
  ClutterActor *debug_actor;

  MxWindowRotation  rotation;
  ClutterTimeline  *rotation_timeline;
  ClutterAlpha     *rotation_alpha;
  gfloat            start_angle;
  gfloat            end_angle;
  gfloat            angle;
};

#define WINDOW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW, MxWindowPrivate))

enum
{
  PROP_STYLE = 1,
  PROP_STYLE_CLASS,
  PROP_STYLE_PSEUDO_CLASS,
  PROP_HAS_TOOLBAR,
  PROP_TOOLBAR,
  PROP_SMALL_SCREEN,
  PROP_FULLSCREEN,
  PROP_TITLE,
  PROP_ICON_NAME,
  PROP_ICON_COGL_TEXTURE,
  PROP_CLUTTER_STAGE,
  PROP_CHILD,
  PROP_WINDOW_ROTATION,
  PROP_WINDOW_ROTATION_TIMELINE,
  PROP_WINDOW_ROTATION_ANGLE
};

enum
{
  DESTROY,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
mx_window_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxWindow *self = MX_WINDOW (object);
  MxWindowPrivate *priv = self->priv;

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

    case PROP_FULLSCREEN:
      g_value_set_boolean (value, priv->fullscreen);
      break;

    case PROP_TITLE:
      g_value_set_string (value, mx_window_get_title (self));
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

    case PROP_WINDOW_ROTATION:
      g_value_set_enum (value, priv->rotation);
      break;

    case PROP_WINDOW_ROTATION_TIMELINE:
      g_value_set_object (value, priv->rotation_timeline);
      break;

    case PROP_WINDOW_ROTATION_ANGLE:
      g_value_set_float (value, priv->angle);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxWindow *window = MX_WINDOW (object);

  switch (property_id)
    {
    case PROP_HAS_TOOLBAR:
      mx_window_set_has_toolbar (window,
                                 g_value_get_boolean (value));
      break;

    case PROP_TOOLBAR:
      mx_window_set_toolbar (window, g_value_get_object (value));
      break;

    case PROP_SMALL_SCREEN:
      mx_window_set_small_screen (window,
                                  g_value_get_boolean (value));
      break;

    case PROP_FULLSCREEN:
      mx_window_set_fullscreen (window, g_value_get_boolean (value));
      break;

    case PROP_TITLE:
      mx_window_set_title (window, g_value_get_string (value));
      break;

    case PROP_ICON_NAME:
      mx_window_set_icon_name (window,
                               g_value_get_string (value));
      break;

    case PROP_ICON_COGL_TEXTURE:
      mx_window_set_icon_from_cogl_texture (MX_WINDOW (object),
                                            g_value_get_pointer (value));
      break;

    case PROP_CLUTTER_STAGE:
      window->priv->stage = (ClutterActor *)g_value_get_object (value);
      break;

    case PROP_CHILD:
      mx_window_set_child (window, (ClutterActor *)g_value_get_object (value));
      break;

    case PROP_WINDOW_ROTATION:
      mx_window_set_window_rotation (window, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_dispose (GObject *object)
{
  MxWindow *self = MX_WINDOW (object);
  MxWindowPrivate *priv = self->priv;

  if (priv->icon_texture)
    {
      cogl_handle_unref (priv->icon_texture);
      priv->icon_texture = NULL;
    }

  if (priv->toolbar)
    mx_window_set_toolbar (self, NULL);

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

  if (priv->native_window)
    {
      g_object_unref (priv->native_window);
      priv->native_window = NULL;
    }

  if (priv->rotation_alpha)
    {
      g_object_unref (priv->rotation_alpha);
      priv->rotation_alpha = NULL;
    }

  if (priv->rotation_timeline)
    {
      g_object_unref (priv->rotation_timeline);
      priv->rotation_timeline = NULL;
    }

  G_OBJECT_CLASS (mx_window_parent_class)->dispose (object);
}

static void
mx_window_finalize (GObject *object)
{
  MxWindowPrivate *priv = MX_WINDOW (object)->priv;

  g_free (priv->icon_name);

  G_OBJECT_CLASS (mx_window_parent_class)->finalize (object);
}

static void
mx_window_get_size (MxWindow *window, gfloat *width, gfloat *height)
{
  gfloat stage_width, stage_height, scale, angle;
  MxWindowPrivate *priv = window->priv;

  clutter_actor_get_size (priv->stage, &stage_width, &stage_height);

  /* Normalise the angle */
  angle = priv->angle;
  while (angle < 0)
    angle += 360.f;
  while (angle >= 360.f)
    angle -= 360.f;

  /* Interpolate between width and height depending on rotation */
  if (!clutter_timeline_is_playing (priv->rotation_timeline) ||
      priv->rotate_size)
    {
      if (angle <= 90.f)
        scale = angle / 90.f;
      else if (angle <= 180.f)
        scale = 1.f - (angle - 90.f) / 90.f;
      else if (angle <= 270.f)
        scale = (angle - 180.f) / 90.f;
      else
        scale = 1.f - (angle - 270.f) / 90.f;
    }
  else if ((priv->rotation == MX_WINDOW_ROTATION_0) ||
           (priv->rotation == MX_WINDOW_ROTATION_180))
    scale = 0;
  else
    scale = 1;

  if (width)
    *width = (scale * stage_height) + ((1.f - scale) * stage_width);
  if (height)
    *height = (scale * stage_width) + ((1.f - scale) * stage_height);
}

static void
debug_paint (ClutterActor *stage,
             MxWindow     *window)
{
  MxPadding padding = { 0, };
  static gpointer current;
  ClutterActor *actor;
  gfloat width, height, x, y;
  CoglMatrix matrix;

  actor = window->priv->debug_actor;

  if (!actor)
    return;

  if (MX_IS_WIDGET (actor))
    mx_widget_get_padding (MX_WIDGET (actor), &padding);


  clutter_actor_get_size (actor, &width, &height);
  clutter_actor_get_transformed_position (actor, &x, &y);


    {
      ClutterActor *cur_actor;
      GSList *parents = NULL, *l;

      cogl_get_modelview_matrix (&matrix);

      /* Get the complete modelview matrix for light by applying all of
         its parent transformations as well as its own in reverse */
      for (cur_actor = actor;
           cur_actor != stage;
           cur_actor = clutter_actor_get_parent (cur_actor))
        parents = g_slist_prepend (parents, cur_actor);

      for (l = parents; l; l = l->next)
        {
          CoglMatrix actor_matrix;

          cogl_matrix_init_identity (&actor_matrix);
          clutter_actor_get_transformation_matrix (CLUTTER_ACTOR (l->data),
                                                   &actor_matrix);

          cogl_matrix_multiply (&matrix,
                                &matrix,
                                &actor_matrix);
        }

      g_slist_free (parents);
    }

  cogl_push_matrix ();

  cogl_set_modelview_matrix (&matrix);


  if (current != actor)
    {
      current = actor;

      const char *id = clutter_actor_get_name (current);
      const char *class = NULL;
      const char *type_name = G_OBJECT_TYPE_NAME (current);

      if (MX_IS_STYLABLE (current))
        class = mx_stylable_get_style_class (current);

      g_debug ("%s%s%s%s%s (%.1f, %.1f) (%.1f x %.1f)",
               (type_name) ? type_name : "",
               (class) ? "." : "",
               (class) ? class : "",
               (id) ? "#" : "",
               (id) ? id : "",
               x, y,
               width, height);
    }

  cogl_set_source_color4f (0, 0, 1, 0.5);
  cogl_rectangle (0, 0, width, height);

  cogl_set_source_color4f (0, 1, 0, 0.5);
  cogl_rectangle (padding.left, padding.top,
                  width - padding.right,
                  height - padding.bottom);

  cogl_set_source_color4f (0, 0, 0, 1);
  cogl_path_rectangle (0, 0, width, height);
  cogl_path_stroke ();

  cogl_set_source_color4f (1, 1, 1, 1);
  cogl_path_rectangle (1, 1, width - 1, height - 1);
  cogl_path_stroke ();

  cogl_pop_matrix ();
}

static void
debug_actor_notify (gpointer debug_actor_p,
                    GObject *actor)
{
  *((gpointer *)debug_actor_p) = NULL;
}

static gboolean
debug_captured_event (ClutterActor *actor,
                      ClutterEvent *event,
                      gpointer      data)
{
  MxWindowPrivate *priv = MX_WINDOW (data)->priv;

  if (event->type == CLUTTER_MOTION)
    {
      ClutterMotionEvent *m_event = (ClutterMotionEvent *) event;

      if (priv->debug_actor)
        g_object_weak_unref (G_OBJECT (priv->debug_actor), debug_actor_notify,
                             &priv->debug_actor);

      priv->debug_actor = clutter_stage_get_actor_at_pos (CLUTTER_STAGE (actor),
                                                          CLUTTER_PICK_ALL,
                                                          m_event->x,
                                                          m_event->y);
      g_object_weak_ref (G_OBJECT (priv->debug_actor), debug_actor_notify,
                         &priv->debug_actor);
    }

  return FALSE;
}


static void
mx_window_post_paint_cb (ClutterActor *actor, MxWindow *window)
{
  gfloat width, height, stage_width, stage_height;

  MxWindowPrivate *priv = window->priv;

  /* If we're in small-screen or fullscreen mode, or we don't have the toolbar,
   * we don't want a frame or a resize handle.
   */
  if (!priv->has_toolbar || priv->small_screen || priv->fullscreen)
    return;

  mx_window_get_size (window, &width, &height);
  clutter_actor_get_size (actor, &stage_width, &stage_height);

  cogl_push_matrix ();

  /* Adjust for rotation */
  cogl_translate ((stage_width - width) / 2.f,
                  (stage_height - height) / 2.f,
                  0);

  cogl_translate (width / 2.f, height / 2.f, 0);
  cogl_rotate (priv->angle, 0, 0, 1);
  cogl_translate (-width / 2.f, -height / 2.f, 0);

  /* paint frame */
  cogl_set_source_color4f (0.2, 0.2, 0.2, 1);

  cogl_rectangle (0, 0, width, 1);
  cogl_rectangle (0, height - 1, width, height);

  cogl_rectangle (0, 1, 1, height - 1);
  cogl_rectangle (width - 1, 1, width, height - 1);

  cogl_pop_matrix ();

  if (_mx_debug (MX_DEBUG_LAYOUT))
    debug_paint (actor, window);
}

static void
mx_window_allocation_changed_cb (ClutterActor           *actor,
                                 ClutterActorBox        *box,
                                 ClutterAllocationFlags  flags,
                                 MxWindow               *window)
{
  MxPadding padding;
  gboolean from_toolbar;
  MxWindowPrivate *priv;
  gfloat x, y, width, height, toolbar_height, stage_width, stage_height;

  /* Note, ideally this would happen just before allocate, but there's
   * no signal we can connect to for that without overriding an actor.
   *
   * Instead, we do this each time the allocation is changed on the stage
   * or the toolbar. This shouldn't be a frequent occurence, but
   * unfortunately can happen multiple times before the actual relayout
   * happens.
   */

  priv = window->priv;

  from_toolbar = (actor == priv->toolbar);
  actor = priv->stage;

  mx_window_get_size (window, &width, &height);
  clutter_actor_get_size (actor, &stage_width, &stage_height);

  x = (stage_width - width) / 2.f;
  y = (stage_height - height) / 2.f;

  if (!priv->has_toolbar || priv->small_screen || priv->fullscreen)
      padding.top = padding.right = padding.bottom = padding.left = 0;
  else
      padding.top = padding.right = padding.bottom = padding.left = 1;

  if (priv->has_toolbar && priv->toolbar)
    {
      clutter_actor_get_preferred_height (priv->toolbar,
                                          width - padding.left -
                                          padding.right,
                                          NULL, &toolbar_height);

      if (!from_toolbar)
        {
          clutter_actor_set_position (priv->toolbar,
                                      padding.left + x,
                                      padding.top + y);
          clutter_actor_set_rotation (priv->toolbar,
                                      CLUTTER_Z_AXIS,
                                      priv->angle,
                                      width / 2.f - padding.left,
                                      height / 2.f - padding.top, 0);
          g_object_set (G_OBJECT (priv->toolbar),
                        "natural-width",
                        width - padding.left - padding.right,
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
                    "natural-width", width - padding.left - padding.right,
                    "natural-height", height - toolbar_height -
                                      padding.top - padding.bottom,
                    "x", padding.left + x,
                    "y", toolbar_height + padding.top + y,
                    NULL);
      clutter_actor_set_rotation (priv->child,
                                  CLUTTER_Z_AXIS,
                                  priv->angle,
                                  width / 2.f - padding.left,
                                  height / 2.f - padding.top - toolbar_height,
                                  0);
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
mx_window_reallocate (MxWindow *self)
{
  ClutterActorBox box;
  MxWindowPrivate *priv = self->priv;

  clutter_actor_get_allocation_box (priv->stage, &box);
  mx_window_allocation_changed_cb (priv->stage, &box, 0, self);
}

static void
mx_window_fullscreen_set_cb (ClutterStage *stage,
                             GParamSpec   *pspec,
                             MxWindow     *self)
{
  MxWindowPrivate *priv = self->priv;

  /* Synchronise our local fullscreen-set property */
  if (priv->fullscreen != clutter_stage_get_fullscreen (stage))
    {
      priv->fullscreen = !priv->fullscreen;
      g_object_notify (G_OBJECT (self), "fullscreen");
    }

  /* If we're in small-screen mode, make sure the size gets reset
   * correctly.
   */
  if (!priv->fullscreen)
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

  mx_window_reallocate (self);
  clutter_actor_queue_relayout (CLUTTER_ACTOR (stage));
}

static void
mx_window_title_cb (ClutterStage *stage,
                    GParamSpec   *pspec,
                    MxWindow     *self)
{
  g_object_notify (G_OBJECT (self), "title");
}

static void
mx_window_destroy_cb (ClutterStage *stage, MxWindow *self)
{
  self->priv->stage = NULL;
  g_signal_emit (self, signals[DESTROY], 0);
}

static void
mx_window_actor_added_cb (ClutterContainer *container,
                          ClutterActor     *actor,
                          MxWindow         *self)
{
  MxWindowPrivate *priv = self->priv;
  if (priv->resize_grip && priv->child)
    clutter_actor_raise (priv->resize_grip, priv->child);
}

static void
mx_window_actor_removed_cb (ClutterContainer *container,
                            ClutterActor     *actor,
                            MxWindow         *self)
{
  MxWindowPrivate *priv = self->priv;

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
                             MxWindow     *self)
{
  MxWindowPrivate *priv = self->priv;

  if (!clutter_stage_get_user_resizable (stage))
    clutter_actor_hide (priv->resize_grip);
  else
    {
      if (!priv->fullscreen &&
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
mx_window_constructed (GObject *object)
{
  MxWindow *self = MX_WINDOW (object);
  MxWindowPrivate *priv = self->priv;

  if (!priv->stage)
    {
      priv->stage = g_object_new (CLUTTER_TYPE_STAGE, NULL);
      clutter_stage_set_user_resizable ((ClutterStage *)priv->stage, TRUE);
    }

  mx_focus_manager_get_for_stage ((ClutterStage *)priv->stage);

  g_object_add_weak_pointer (G_OBJECT (priv->stage),
                             (gpointer *)&priv->stage);
  g_object_set_qdata (G_OBJECT (priv->stage), window_quark, object);

  if (!priv->toolbar && priv->has_toolbar)
    mx_window_set_toolbar (self, MX_TOOLBAR (mx_toolbar_new ()));

  priv->resize_grip = mx_icon_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->resize_grip), "ResizeGrip");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
                               priv->resize_grip);
  if (priv->fullscreen ||
      !clutter_stage_get_user_resizable (CLUTTER_STAGE (priv->stage)) ||
      !priv->has_toolbar)
    clutter_actor_hide (priv->resize_grip);
  g_object_add_weak_pointer (G_OBJECT (priv->resize_grip),
                             (gpointer *)&priv->resize_grip);

  g_signal_connect_after (priv->stage, "paint",
                          G_CALLBACK (mx_window_post_paint_cb), object);
  g_signal_connect (priv->stage, "allocation-changed",
                    G_CALLBACK (mx_window_allocation_changed_cb), object);
  g_signal_connect (priv->stage, "notify::fullscreen-set",
                    G_CALLBACK (mx_window_fullscreen_set_cb), object);
  g_signal_connect (priv->stage, "notify::title",
                    G_CALLBACK (mx_window_title_cb), object);
  g_signal_connect (priv->stage, "destroy",
                    G_CALLBACK (mx_window_destroy_cb), object);
  g_signal_connect (priv->stage, "actor-added",
                    G_CALLBACK (mx_window_actor_added_cb), object);
  g_signal_connect (priv->stage, "actor-removed",
                    G_CALLBACK (mx_window_actor_removed_cb), object);
  g_signal_connect (priv->stage, "notify::user-resizable",
                    G_CALLBACK (mx_window_user_resizable_cb), object);

  if (_mx_debug (MX_DEBUG_LAYOUT))
    g_signal_connect (priv->stage, "captured-event",
                      G_CALLBACK (debug_captured_event), object);

  g_object_set (G_OBJECT (priv->stage), "use-alpha", TRUE, NULL);

#ifdef HAVE_X11
  priv->native_window = _mx_window_x11_new (self);
#endif
}

static void
mx_window_class_init (MxWindowClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWindowPrivate));

  object_class->get_property = mx_window_get_property;
  object_class->set_property = mx_window_set_property;
  object_class->dispose = mx_window_dispose;
  object_class->finalize = mx_window_finalize;
  object_class->constructed = mx_window_constructed;

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
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TOOLBAR, pspec);

  pspec = g_param_spec_boolean ("small-screen",
                                "Small screen",
                                "Window should occupy the entire screen "
                                "contents, without explicitly setting "
                                "itself fullscreen.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SMALL_SCREEN, pspec);

  pspec = g_param_spec_boolean ("fullscreen",
                                "Fullscreen",
                                "Window should be set to full-screen mode.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FULLSCREEN, pspec);

  pspec = g_param_spec_string ("title",
                               "Title",
                               "Title to use for the window.",
                               NULL,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TITLE, pspec);

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

  pspec = g_param_spec_enum ("window-rotation",
                             "Window rotation",
                             "The window's rotation.",
                             MX_TYPE_WINDOW_ROTATION,
                             MX_WINDOW_ROTATION_0,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_WINDOW_ROTATION, pspec);

  pspec = g_param_spec_object ("window-rotation-timeline",
                               "Window rotation timeline",
                               "The timeline used for the window rotation "
                               "transition animation.",
                               CLUTTER_TYPE_TIMELINE,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_WINDOW_ROTATION_TIMELINE,
                                   pspec);

  pspec = g_param_spec_float ("window-rotation-angle",
                              "Window rotation angle",
                              "The current angle of rotation about the z-axis "
                              "for the window.",
                              0.f, 360.f, 0.f,
                              MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_WINDOW_ROTATION_ANGLE,
                                   pspec);

  /**
   * MxWindow::destroy:
   * @window: the object that received the signal
   *
   * Emitted when the stage managed by the window is destroyed.
   */
  signals[DESTROY] = g_signal_new ("destroy",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_LAST,
                                   G_STRUCT_OFFSET (MxWindowClass, destroy),
                                   NULL, NULL,
                                   _mx_marshal_VOID__VOID,
                                   G_TYPE_NONE, 0);

  window_quark = g_quark_from_static_string ("mx-window");
}

static void
mx_window_rotation_new_frame_cb (ClutterTimeline *timeline,
                                 gint             msecs,
                                 MxWindow        *self)
{
  MxWindowPrivate *priv = self->priv;
  gfloat alpha = clutter_alpha_get_alpha (priv->rotation_alpha);

  priv->angle = (alpha * priv->end_angle) + ((1.f - alpha) * priv->start_angle);
  mx_window_reallocate (self);
  g_object_notify (G_OBJECT (self), "window-rotation-angle");
}

static void
mx_window_rotation_completed_cb (ClutterTimeline *timeline,
                                 MxWindow        *self)
{
  MxWindowPrivate *priv = self->priv;

  priv->angle = priv->end_angle;
  while (priv->angle >= 360.f)
    priv->angle -= 360.f;
  while (priv->angle < 0.f)
    priv->angle += 360.f;

  priv->rotate_size = FALSE;

  mx_window_reallocate (self);
  g_object_notify (G_OBJECT (self), "window-rotation-angle");
}

static void
mx_window_init (MxWindow *self)
{
  MxWindowPrivate *priv = self->priv = WINDOW_PRIVATE (self);

  priv->rotation_timeline = clutter_timeline_new (400);
  priv->rotation_alpha = clutter_alpha_new_full (priv->rotation_timeline,
                                                 CLUTTER_EASE_IN_OUT_QUAD);
  priv->has_toolbar = TRUE;

  g_signal_connect (priv->rotation_timeline, "new-frame",
                    G_CALLBACK (mx_window_rotation_new_frame_cb), self);
  g_signal_connect (priv->rotation_timeline, "completed",
                    G_CALLBACK (mx_window_rotation_completed_cb), self);
}

CoglHandle
_mx_window_get_icon_cogl_texture (MxWindow *window)
{
  CoglHandle texture;
  MxWindowPrivate *priv;

  g_return_val_if_fail (MX_IS_WINDOW (window), COGL_INVALID_HANDLE);

  priv = window->priv;
  texture = priv->icon_texture;
  priv->icon_texture = NULL;

  return texture;
}

ClutterActor *
_mx_window_get_resize_grip (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return window->priv->resize_grip;
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
 * Returns: (transfer none): A #MxWindow, or %NULL
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
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (actor == NULL || CLUTTER_IS_ACTOR (actor));

  priv = window->priv;
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
    }

  mx_window_reallocate (window);
  g_object_notify (G_OBJECT (window), "child");
}

/**
 * mx_window_get_child:
 * @window: A #MxWindow
 *
 * Get the primary child of the window. See mx_window_set_child().
 *
 * Returns: (transfer none): A #ClutterActor, or %NULL
 */
ClutterActor*
mx_window_get_child (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return window->priv->child;
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
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

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

      mx_window_reallocate (window);
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
  return window->priv->has_toolbar;
}

/**
 * mx_window_get_toolbar:
 * @window: A #MxWindow
 *
 * Retrieves the toolbar associated with the window.
 *
 * Returns: (transfer none): A #MxToolbar
 */
MxToolbar *
mx_window_get_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return MX_TOOLBAR (window->priv->toolbar);
}

/**
 * mx_window_set_toolbar:
 * @window: (allow-none): A #MxWindow
 *
 * Sets the toolbar associated with the window.
 *
 * Since: 1.2
 */
void
mx_window_set_toolbar (MxWindow  *window,
                       MxToolbar *toolbar)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (!toolbar || MX_IS_TOOLBAR (toolbar));

  priv = window->priv;

  if (priv->toolbar == (ClutterActor *)toolbar)
    return;

  /* Remove old toolbar */
  if (priv->toolbar)
    {
      g_signal_handlers_disconnect_by_func (priv->toolbar,
                                            mx_window_allocation_changed_cb,
                                            window);
      g_object_remove_weak_pointer (G_OBJECT (priv->toolbar),
                                    (gpointer *)&priv->toolbar);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage),
                                      priv->toolbar);
    }

  priv->toolbar = (ClutterActor *)toolbar;

  /* Add new toolbar */
  if (toolbar)
    {
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
                                   priv->toolbar);
      g_object_add_weak_pointer (G_OBJECT (priv->toolbar),
                                 (gpointer *)&priv->toolbar);
      g_signal_connect (priv->toolbar, "allocation-changed",
                        G_CALLBACK (mx_window_allocation_changed_cb), window);
    }

  priv->has_toolbar = priv->toolbar ? TRUE : FALSE;
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
  return window->priv->small_screen;
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
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->small_screen != small_screen)
    {
      priv->small_screen = small_screen;
      g_object_notify (G_OBJECT (window), "small-screen");
    }
}

/**
 * mx_window_get_fullscreen:
 * @window: A #MxWindow
 *
 * Determines if the window has been set to be in fullscreen mode.
 *
 * Returns: %TRUE if the window has been set to be in fullscreen mode,
 *   otherwise %FALSE
 *
 * Since: 1.2
 */
gboolean
mx_window_get_fullscreen (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);
  return window->priv->fullscreen;
}

/**
 * mx_window_set_fullscreen:
 * @window: A #MxWindow
 * @fullscreen: %TRUE to request fullscreen mode, %FALSE to disable
 *
 * Set the window to be in fullscreen mode or windowed mode.
 *
 * <note><para>
 * Setting fullscreen mode doesn't necessarily mean the window is actually
 * fullscreen. Setting this property is only a request to the underlying
 * window system.
 * </para></note>
 *
 * Since: 1.2
 */
void
mx_window_set_fullscreen (MxWindow *window,
                          gboolean  fullscreen)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->fullscreen != fullscreen)
    {
      priv->fullscreen = fullscreen;
      clutter_stage_set_fullscreen (CLUTTER_STAGE (priv->stage), fullscreen);
      g_object_notify (G_OBJECT (window), "fullscreen");
    }
}

/**
 * mx_window_set_title:
 * @window: A #MxWindow
 * @title: A string to use for the window title name
 *
 * Sets the title used for the window, the results of which are
 * window-system specific.
 *
 * Since: 1.2
 */
void
mx_window_set_title (MxWindow    *window,
                     const gchar *title)
{
  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (title != NULL);

  clutter_stage_set_title (CLUTTER_STAGE (window->priv->stage), title);
}

/**
 * mx_window_get_title:
 * @window: A #MxWindow
 *
 * Retrieves the title used for the window.
 *
 * Returns: The title used for the window
 *
 * Since: 1.2
 */
const gchar *
mx_window_get_title (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return clutter_stage_get_title (CLUTTER_STAGE (window->priv->stage));
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
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

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
  return window->priv->icon_name;
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
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (texture != NULL);

  priv = window->priv;

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
 * Returns: (transfer none): A #ClutterStage
 */
ClutterStage *
mx_window_get_clutter_stage (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);
  return CLUTTER_STAGE (window->priv->stage);
}

/**
 * mx_window_get_window_position:
 * @window: an #MxWindow
 * @x: (out): A pointer for the x-coordinate
 * @y: (out): A pointer for the y-coordinate
 *
 * Retrieves the absolute position of the window on the screen.
 */
void
mx_window_get_window_position (MxWindow *window,
                               gint     *x,
                               gint     *y)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->native_window)
    _mx_native_window_get_position (priv->native_window, x, y);
  else
    {
      if (x)
        *x = 0;
      if (y)
        *y = 0;
    }
}

/**
 * mx_window_set_window_position:
 * @window: A #MxWindow
 * @x: An x-coordinate
 * @y: A y-coordinate
 *
 * Sets the absolute position of the window on the screen.
 */
void
mx_window_set_window_position (MxWindow *window,
                               gint      x,
                               gint      y)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->native_window)
    _mx_native_window_set_position (priv->native_window, x, y);
}

/**
 * mx_window_get_window_size:
 * @window: A #MxWindow
 * @width: (out): A #gint pointer for the window's width
 * @height: (out): A #gint pointer for the window's height
 *
 * Retrieves the size of the display area of the window, taking into
 * account any window border. This includes the area occupied by the
 * window's toolbar, if it's enabled.
 *
 * Since: 1.2
 */
void
mx_window_get_window_size (MxWindow *window,
                           gint     *width,
                           gint     *height)
{
  MxWindowPrivate *priv;
  gfloat fwidth, fheight;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

  mx_window_get_size (window, &fwidth, &fheight);

  if (width)
    *width = (gint)fwidth;
  if (height)
    *height = (gint)fheight;

  if (priv->has_toolbar && !priv->small_screen && !priv->fullscreen)
    {
      if (width)
        *width = *width + 1;
      if (height)
        *height = *height + 1;
    }
}

/**
 * mx_window_set_window_size:
 * @window: A #MxWindow
 * @width: A width, in pixels
 * @height: A height, in pixels
 *
 * Sets the size of the window, taking into account any window border. This
 * corresponds to the window's available area for its child, minus the area
 * occupied by the window's toolbar, if it's enabled.
 *
 * <note><para>
 * Setting the window size may involve a request to the underlying windowing
 * system, and may not immediately be reflected.
 * </para></note>
 *
 * Since: 1.2
 */
void
mx_window_set_window_size (MxWindow *window,
                           gint      width,
                           gint      height)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

  if (priv->has_toolbar && !priv->small_screen && !priv->fullscreen)
    {
      width ++;
      height ++;
    }

  clutter_actor_set_size (priv->stage, width, height);
}

/**
 * mx_window_present:
 * @window: A #MxWindow
 *
 * Present the window. The actual behaviour is specific to the window system.
 *
 * Since: 1.2
 */
void
mx_window_present (MxWindow *window)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->native_window)
    _mx_native_window_present (priv->native_window);
}

/**
 * mx_window_set_window_rotation:
 * @window: A #MxWindow
 * @rotation: The #MxWindowRotation
 *
 * Set the rotation of the window.
 *
 * Since: 1.2
 */
void
mx_window_set_window_rotation (MxWindow         *window,
                               MxWindowRotation  rotation)
{
  guint msecs;
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;
  if (priv->rotation == rotation)
    return;

  if (((priv->rotation == MX_WINDOW_ROTATION_0) ||
       (priv->rotation == MX_WINDOW_ROTATION_180)) &&
      ((rotation == MX_WINDOW_ROTATION_90) ||
       (rotation == MX_WINDOW_ROTATION_270)))
    priv->rotate_size = TRUE;
  else if (((priv->rotation == MX_WINDOW_ROTATION_90) ||
            (priv->rotation == MX_WINDOW_ROTATION_270)) &&
           ((rotation == MX_WINDOW_ROTATION_0) ||
            (rotation == MX_WINDOW_ROTATION_180)))
    priv->rotate_size = TRUE;

  priv->rotation = rotation;

  priv->start_angle = priv->angle;
  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0 :
      priv->end_angle = 0.f;
      break;
    case MX_WINDOW_ROTATION_90 :
      priv->end_angle = 90.f;
      break;
    case MX_WINDOW_ROTATION_180 :
      priv->end_angle = 180.f;
      break;
    case MX_WINDOW_ROTATION_270 :
      priv->end_angle = 270.f;
      break;
    }

  if (priv->end_angle - priv->start_angle > 180.f)
    priv->end_angle -= 360.f;
  else if (priv->end_angle - priv->start_angle < -180.f)
    priv->end_angle += 360.f;

  msecs = (guint)((ABS (priv->end_angle - priv->start_angle) / 90.f) * 400.f);
  clutter_timeline_rewind (priv->rotation_timeline);
  clutter_timeline_set_duration (priv->rotation_timeline, msecs);
  clutter_timeline_start (priv->rotation_timeline);

  g_object_notify (G_OBJECT (window), "window-rotation");
}

/**
 * mx_window_get_window_rotation:
 * @window: A #MxWindow
 *
 * Retrieve the rotation of the window.
 *
 * Returns: An #MxWindowRotation
 *
 * Since: 1.2
 */
MxWindowRotation
mx_window_get_window_rotation (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), MX_WINDOW_ROTATION_0);
  return window->priv->rotation;
}

/**
 * mx_window_show:
 * @window: A #MxWindow
 *
 * Show the window
 *
 * Since: 1.2
 */
void
mx_window_show (MxWindow *window)
{
  g_return_if_fail (MX_IS_WINDOW (window));
  clutter_actor_show (window->priv->stage);
}


/**
 * mx_window_hide:
 * @window: A #MxWindow
 *
 * Hide the window
 *
 * Since: 1.2
 */
void
mx_window_hide (MxWindow *window)
{
  g_return_if_fail (MX_IS_WINDOW (window));
  clutter_actor_hide (window->priv->stage);
}

