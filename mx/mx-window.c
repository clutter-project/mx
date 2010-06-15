/*
 * mx-window.c
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

#include "mx-window.h"
#include "mx-toolbar.h"
#include "mx-focus-manager.h"
#include "mx-private.h"
#include "mx-marshal.h"
#include <clutter/x11/clutter-x11.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h>

G_DEFINE_TYPE (MxWindow, mx_window, G_TYPE_OBJECT)

#define WINDOW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW, MxWindowPrivate))

static GQuark window_quark = 0;

struct _MxWindowPrivate
{
  guint is_fullscreen : 1;
  guint has_toolbar   : 1;
  guint is_resizing   : 1;
  guint small_screen  : 1;
  guint has_mapped    : 1;
  guint width_set     : 1;
  guint height_set    : 1;
  guint icon_changed  : 1;

  gint  is_moving;

  gfloat     last_width;
  gfloat     last_height;
  gfloat     natural_width;
  gfloat     natural_height;

  gchar      *icon_name;
  CoglHandle  icon_texture;

  ClutterActor *stage;
  ClutterActor *toolbar;
  ClutterActor *child;
  ClutterActor *resize_grip;

  gint  drag_x_start;
  gint  drag_y_start;
  gint  drag_win_x_start;
  gint  drag_win_y_start;
  guint drag_width_start;
  guint drag_height_start;

  MxFocusManager *focus_manager;
};

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
  MxWindowPrivate *priv = MX_WINDOW (object)->priv;

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
mx_window_set_property (GObject      *object,
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
      MX_WINDOW (object)->priv->stage =
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
mx_window_get_size (MxWindow *self,
                    gfloat   *width_p,
                    gfloat   *height_p,
                    gfloat   *pref_width_p,
                    gfloat   *pref_height_p)
{
  gboolean has_border;
  gfloat width, pref_width;
  MxWindowPrivate *priv = self->priv;

  pref_width = width = 0;

  if (!priv->stage)
    return;

  has_border = priv->has_toolbar &&
               !(priv->small_screen ||
                 clutter_stage_get_fullscreen (CLUTTER_STAGE (priv->stage)));

  if (priv->toolbar)
    clutter_actor_get_preferred_width (priv->toolbar,
                                       -1,
                                       &width,
                                       &pref_width);

  if (priv->child)
    {
      gfloat child_min_width, child_pref_width;
      clutter_actor_get_preferred_width (priv->child,
                                         -1,
                                         &child_min_width,
                                         &child_pref_width);
      if (child_min_width > width)
        width = child_min_width;
      if (child_pref_width > pref_width)
        pref_width = child_pref_width;
    }

  if (width_p)
    *width_p = width + (has_border ? 2 : 0);
  if (pref_width_p)
    *pref_width_p = pref_width + (has_border ? 2 : 0);

  if (!height_p && !pref_height_p)
    return;

  if (height_p)
    *height_p = 0;
  if (pref_height_p)
    *pref_height_p = 0;

  if (priv->toolbar)
    clutter_actor_get_preferred_height (priv->toolbar,
                                        width,
                                        height_p,
                                        pref_height_p);

  if (priv->child)
    {
      gfloat child_min_height, child_pref_height;
      clutter_actor_get_preferred_height (priv->child,
                                          width,
                                          &child_min_height,
                                          &child_pref_height);
      if (height_p)
        *height_p += child_min_height;
      if (pref_height_p)
        *pref_height_p += child_pref_height;
    }

  if (has_border)
    {
      if (height_p)
        *height_p += 2;
      if (pref_height_p)
        *pref_height_p += 2;
    }
}

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long inputMode;
    unsigned long status;
} PropMotifWmHints;

static void
mx_window_set_wm_hints (MxWindow *window)
{
  const gchar *icon_name;
  Display *dpy;
  Window win;

  static Atom motif_wm_hints_atom = None;
  static Atom net_wm_icon = None;

  MxWindowPrivate *priv = window->priv;

  if (!priv->stage)
    return;

  dpy = clutter_x11_get_default_display ();
  win = clutter_x11_get_stage_window (CLUTTER_STAGE (priv->stage));

  if (win == None)
    return;

  if (!motif_wm_hints_atom)
    motif_wm_hints_atom = XInternAtom (dpy, "_MOTIF_WM_HINTS", False);

  /* Remove/add the window decorations */
  if (motif_wm_hints_atom)
    {
      PropMotifWmHints new_hints = {0,};
      PropMotifWmHints *hints;

      hints = &new_hints;

      hints->flags = 0x2;
      hints->functions = 0x0;
      hints->decorations = priv->has_toolbar ? 0x0 : 0x1;

      XChangeProperty (dpy, win, motif_wm_hints_atom, motif_wm_hints_atom,
                       32, PropModeReplace, (guchar*) hints,
                       sizeof(PropMotifWmHints)/ sizeof (long));
    }

  if (!net_wm_icon)
    net_wm_icon = XInternAtom (dpy, "_NET_WM_ICON", False);

  /* Set the window icon */
  if (!priv->icon_changed)
    return;

  priv->icon_changed = FALSE;

  icon_name = priv->icon_name ? priv->icon_name : g_get_prgname ();
  if ((priv->icon_texture || icon_name) && net_wm_icon)
    {
      guint width, height;
      CoglHandle texture;
      guchar *data;
      gint size;

      /* Lookup icon for program name if there's no texture set */
      if (priv->icon_texture)
        {
          texture = priv->icon_texture;
          priv->icon_texture = NULL;
        }
      else
        {
          texture = mx_icon_theme_lookup (mx_icon_theme_get_default (),
                                          icon_name, 32);
          if (!texture)
            {
              /* Remove the window icon */
              clutter_x11_trap_x_errors ();
              XDeleteProperty (dpy, win, net_wm_icon);
              clutter_x11_untrap_x_errors ();

              return;
            }
        }

      /* Get window icon size */
      width = cogl_texture_get_width (texture);
      height = cogl_texture_get_height (texture);
      size = cogl_texture_get_data (texture,
                                    COGL_PIXEL_FORMAT_BGRA_8888,
                                    width * 4,
                                    NULL);
      if (!size)
        {
          g_warning ("Unable to get texture data in "
                     "correct format for window icon");
          cogl_handle_unref (texture);
          return;
        }

      data = g_malloc (size + (sizeof (gulong) * 2));
      ((gulong *)data)[0] = width;
      ((gulong *)data)[1] = height;

      /* Get the window icon */
      if (cogl_texture_get_data (texture,
                                 COGL_PIXEL_FORMAT_BGRA_8888,
                                 width * 4,
                                 data + (sizeof (gulong) * 2)) == size)
        {
          /* Set the property */
          XChangeProperty (dpy, win, net_wm_icon, XA_CARDINAL,
                           32, PropModeReplace, data,
                           (width * height) + 2);
        }
      else
        g_warning ("Size mismatch when retrieving texture data "
                   "for window icon");

      cogl_handle_unref (texture);
      g_free (data);
    }
}

static void
mx_window_mapped_notify_cb (ClutterActor *actor,
                            GParamSpec   *pspec,
                            MxWindow     *window)
{
  if (CLUTTER_ACTOR_IS_MAPPED (actor))
    mx_window_set_wm_hints (window);
}

static void
mx_window_post_paint_cb (ClutterActor *actor, MxWindow *window)
{
  gfloat width, height;

  MxWindowPrivate *priv = window->priv;

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
mx_window_pre_paint_cb (ClutterActor *actor,
                        MxWindow     *window)
{
  MxPadding padding;
  MxWindowPrivate *priv;
  gfloat width, height, toolbar_height, stage_width, stage_height;

  g_signal_handlers_disconnect_by_func (actor,
                                        mx_window_pre_paint_cb,
                                        window);

  /* Note, ideally this would happen just before allocate, but there's
   * no signal we can connect to for that without overriding an actor.
   *
   * Instead, when the allocation is changed, we hook onto pre-paint
   * and resize/position everything here.
   *
   * It will cause another relayout, so everything will lag behind
   * window size changes by 1 frame, but that's not so bad.
   */

  priv = window->priv;

  clutter_actor_get_size (actor, &stage_width, &stage_height);

  /* Don't mess with the window size when we're full-screen, or you
   * get odd race conditions (and you never want to do it anyway)
   */
  if (!clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
    {
      if (!priv->has_mapped)
        {
          Window win;
          Display *dpy;

          win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
          dpy = clutter_x11_get_default_display ();

          priv->has_mapped = TRUE;

          if (priv->small_screen)
            {
              XRRScreenResources *res;

              res = XRRGetScreenResourcesCurrent (dpy, win);

              XMoveResizeWindow (dpy, win,
                                 0, 0,
                                 res->modes[res->nmode].width,
                                 res->modes[res->nmode].height);

              XRRFreeScreenResources (res);
            }
          else
            {
              /* Set the initial size of the window - if the user has set
               * a dimension, it will be used, otherwise the preferred size
               * will be used.
               */
              mx_window_get_size (window, NULL, NULL, &width, &height);

              if (priv->width_set)
                width = priv->natural_width + 2;
              if (priv->height_set)
                height = priv->natural_height + 2;

              XResizeWindow (dpy, win, width, height);
              stage_width = width;
              stage_height = height;
            }
        }
#if CLUTTER_CHECK_VERSION(1,2,0)
      else
        {
          /* Update minimum size */
          mx_window_get_size (window, &width, &height, NULL, NULL);
          if (width < 1.0)
            width = 1.0;
          if (height < 1.0)
            height = 1.0;
          clutter_stage_set_minimum_size (CLUTTER_STAGE (actor),
                                          (guint)width,
                                          (guint)height);
        }
#endif
    }

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

      clutter_actor_set_position (priv->toolbar, padding.left, padding.top);
      g_object_set (G_OBJECT (priv->toolbar),
                    "natural-width", stage_width - padding.left - padding.right,
                    NULL);
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
mx_window_allocation_changed_cb (ClutterActor           *actor,
                                 ClutterActorBox        *box,
                                 ClutterAllocationFlags  flags,
                                 MxWindow               *window)
{
  g_signal_connect (actor, "paint",
                    G_CALLBACK (mx_window_pre_paint_cb),
                    window);
}

static gboolean
mx_window_button_press_event_cb (ClutterActor       *actor,
                                 ClutterButtonEvent *event,
                                 MxWindow           *window)
{
  unsigned int width, height, border_width, depth, mask;
  Window win, root, child;
  int x, y, win_x, win_y;
  MxWindowPrivate *priv;
  Display *dpy;

  priv = window->priv;

  /* Bail out early in no-toolbar, small-screen or fullscreen mode */
  if (!priv->has_toolbar || priv->small_screen ||
      clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
    return FALSE;

  /* We're already moving/resizing */
  if (priv->is_moving != -1)
    return FALSE;

  /* We only care about the first mouse button */
  if (event->button != 1)
    return FALSE;

  priv->is_moving = clutter_input_device_get_device_id (event->device);

  win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
  dpy = clutter_x11_get_default_display ();

  if (win == None)
    return FALSE;

  /* Get the initial width/height */
  XGetGeometry (dpy, win, &root, &x, &y, &width, &height,
                &border_width, &depth);

  priv->drag_win_x_start = x;
  priv->drag_win_y_start = y;
  priv->drag_width_start = width;
  priv->drag_height_start = height;

  /* Get the initial cursor position */
  XQueryPointer (dpy, root, &root, &child, &x, &y, &win_x, &win_y, &mask);

  priv->drag_x_start = x;
  priv->drag_y_start = y;

  /* Disable motion events on other actors */
  clutter_set_motion_events_enabled (FALSE);

  /* Grab the mouse so that we receive the release if the cursor
   * goes off-stage.
   */
  clutter_grab_pointer_for_device (actor, priv->is_moving);

  return TRUE;
}

static void
mx_window_button_release (MxWindow *window)
{
  MxWindowPrivate *priv = window->priv;

  if (priv->is_moving != -1)
    {
      clutter_ungrab_pointer_for_device (priv->is_moving);
      clutter_set_motion_events_enabled (TRUE);
      priv->is_moving = -1;
    }
}

static gboolean
mx_window_button_release_event_cb (ClutterActor       *actor,
                                   ClutterButtonEvent *event,
                                   MxWindow           *window)
{
  MxWindowPrivate *priv = window->priv;

  if ((clutter_input_device_get_device_id (event->device) == priv->is_moving) &&
      (event->button == 1))
    {
      mx_window_button_release (window);
      return TRUE;
    }

  return FALSE;
}

static gboolean
mx_window_captured_event_cb (ClutterActor *actor,
                             ClutterEvent *event,
                             MxWindow     *window)
{
  MxWindowPrivate *priv = window->priv;

  switch (event->type)
    {
    case CLUTTER_MOTION:
      /* Check if we're over the resize handle */
      if ((priv->is_moving == -1) && priv->has_toolbar && !priv->small_screen &&
          !clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)) &&
          priv->resize_grip)
        {
          gint x, y;
          Window win;
          Display *dpy;
          gfloat height, width, rwidth, rheight;

          static Cursor csoutheast = 0;
          ClutterMotionEvent *mevent = &event->motion;

          win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
          dpy = clutter_x11_get_default_display ();

          if (win == None)
            return FALSE;

          clutter_actor_get_size (actor, &width, &height);

          x = mevent->x;
          y = mevent->y;

          /* Create the resize cursor */
          if (!csoutheast)
            csoutheast = XCreateFontCursor (dpy, XC_bottom_right_corner);

          clutter_actor_get_size (priv->resize_grip, &rwidth, &rheight);

          /* Set the cursor if necessary */
          if (x > width - rwidth && y > height - rheight)
            {
              if (!priv->is_resizing)
                {
                  XDefineCursor (dpy, win, csoutheast);
                  priv->is_resizing = TRUE;
                }
              return TRUE;
            }
          else
            {
              if (priv->is_resizing)
                {
                  XUndefineCursor (dpy, win);
                  priv->is_resizing = FALSE;
                }
              return FALSE;
            }
        }
      return FALSE;

    case CLUTTER_BUTTON_PRESS:
      /* We want resizing to happen even if there are active widgets
       * underneath the resize-handle.
       */
      if (priv->is_resizing)
        return mx_window_button_press_event_cb (actor, &event->button, window);
      else
        return FALSE;

    default:
      return FALSE;
    }
}

static gboolean
mx_window_motion_event_cb (ClutterActor       *actor,
                           ClutterMotionEvent *event,
                           MxWindow           *window)
{
  gint offsetx, offsety;
  gint x, y, winx, winy;
  guint mask;
  MxWindowPrivate *priv;
  Window win, root_win, root, child;
  Display *dpy;
  gfloat height, width;

  priv = window->priv;

  /* Ignore motion events while in small-screen mode, fullscreen mode,
   * if we have no toolbar, or if they're not from our grabbed device.
   */
  if ((!priv->has_toolbar) ||
      (priv->small_screen) ||
      (clutter_stage_get_fullscreen (CLUTTER_STAGE (actor))) ||
      (clutter_input_device_get_device_id (event->device) != priv->is_moving))
    return FALSE;

  /* Check if the mouse button is still down - if the user releases the
   * mouse button while outside of the stage (which can happen), we don't
   * get the release event.
   */
  if (!(event->modifier_state & CLUTTER_BUTTON1_MASK))
    {
      mx_window_button_release (window);
      return TRUE;
    }

  win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
  dpy = clutter_x11_get_default_display ();

  if (win == None)
    return FALSE;

  clutter_actor_get_size (actor, &width, &height);

  x = event->x;
  y = event->y;

  /* Move/resize the window if we're dragging */
  offsetx = priv->drag_x_start;
  offsety = priv->drag_y_start;

  root_win = clutter_x11_get_root_window ();
  XQueryPointer (dpy, root_win, &root, &child, &x, &y, &winx, &winy, &mask);

  if (priv->is_resizing)
    {
      XRRScreenResources *res;
      gfloat min_width, min_height;

      mx_window_get_size (window, &min_width, &min_height, NULL, NULL);

      x = MAX (priv->drag_width_start + (x - priv->drag_x_start), min_width);
      y = MAX (priv->drag_height_start + (y - priv->drag_y_start), min_height);

      res = XRRGetScreenResourcesCurrent (dpy, win);
      width = res->modes[res->nmode].width;
      height = res->modes[res->nmode].height;
      XRRFreeScreenResources (res);

      width = MIN (x, width - priv->drag_win_x_start);
      height = MIN (y, height - priv->drag_win_y_start);

#if !CLUTTER_CHECK_VERSION(1,2,0)
      /* Set the natural width/height so ClutterStageX11 won't try to
       * resize us back to our minimum size.
       */
      priv->natural_width = width;
      priv->natural_height = height;

      XMoveResizeWindow (dpy, win,
                         priv->drag_win_x_start, priv->drag_win_y_start,
                         width, height);
#else
      clutter_actor_set_size (actor, width, height);
#endif
    }
  else
    XMoveWindow (dpy, win,
                 MAX (0, priv->drag_win_x_start + x - offsetx),
                 MAX (0, priv->drag_win_y_start + y - offsety));

  return TRUE;
}

static void
mx_window_realize_cb (ClutterActor *actor,
                      MxWindow     *window)
{
  gboolean width_set, height_set;

  MxWindowPrivate *priv = window->priv;

  /* See if the user has set a size on the window to use on initial map */
  g_object_get (G_OBJECT (actor),
                "natural-width", &priv->natural_width,
                "natural-width-set", &width_set,
                "natural-height", &priv->natural_height,
                "natural-height-set", &height_set,
                NULL);

  priv->width_set = width_set;
  priv->height_set = height_set;
}

static void
mx_window_fullscreen_set_cb (ClutterStage *stage,
                             GParamSpec   *pspec,
                             MxWindow     *self)
{
  MxWindowPrivate *priv = self->priv;

  /* If we're in small-screen mode, make sure the size gets reset
   * correctly.
   */
  if (!clutter_stage_get_fullscreen (stage))
    {
      if (priv->small_screen)
        priv->has_mapped = FALSE;
      else if (priv->resize_grip && priv->has_toolbar)
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
  if (self->priv->resize_grip && self->priv->child)
    clutter_actor_raise (self->priv->resize_grip, self->priv->child);
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
mx_window_constructed (GObject *object)
{
  MxWindow *self = MX_WINDOW (object);
  MxWindowPrivate *priv = self->priv;

  if (!priv->stage)
    priv->stage = clutter_stage_new ();
  g_object_add_weak_pointer (G_OBJECT (priv->stage),
                             (gpointer *)&priv->stage);
  g_object_set_qdata (G_OBJECT (priv->stage), window_quark, self);

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
      !priv->has_toolbar)
    clutter_actor_hide (priv->resize_grip);
  g_object_add_weak_pointer (G_OBJECT (priv->resize_grip),
                             (gpointer *)&priv->resize_grip);

  priv->focus_manager =
    mx_focus_manager_get_for_stage (CLUTTER_STAGE (priv->stage));

  clutter_stage_set_user_resizable (CLUTTER_STAGE (priv->stage), TRUE);

  g_signal_connect (priv->stage, "notify::fullscreen-set",
                    G_CALLBACK (mx_window_fullscreen_set_cb), self);
  g_signal_connect (priv->stage, "realize",
                    G_CALLBACK (mx_window_realize_cb), self);
  g_signal_connect (priv->stage, "notify::mapped",
                    G_CALLBACK (mx_window_mapped_notify_cb), self);
  g_signal_connect_after (priv->stage, "paint",
                          G_CALLBACK (mx_window_post_paint_cb), self);
  g_signal_connect (priv->stage, "allocation-changed",
                    G_CALLBACK (mx_window_allocation_changed_cb), self);
  g_signal_connect (priv->stage, "button-press-event",
                    G_CALLBACK (mx_window_button_press_event_cb), self);
  g_signal_connect (priv->stage, "button-release-event",
                    G_CALLBACK (mx_window_button_release_event_cb), self);
  g_signal_connect (priv->stage, "captured-event",
                    G_CALLBACK (mx_window_captured_event_cb), self);
  g_signal_connect (priv->stage, "motion-event",
                    G_CALLBACK (mx_window_motion_event_cb), self);
  g_signal_connect (priv->stage, "destroy",
                    G_CALLBACK (mx_window_destroy_cb), self);
  g_signal_connect (priv->stage, "actor-added",
                    G_CALLBACK (mx_window_actor_added_cb), self);
  g_signal_connect (priv->stage, "actor-removed",
                    G_CALLBACK (mx_window_actor_removed_cb), self);

#if CLUTTER_CHECK_VERSION(1,2,0)
  g_object_set (G_OBJECT (priv->stage), "use-alpha", TRUE, NULL);
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
                               MX_PARAM_WRITABLE);
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
mx_window_init (MxWindow *self)
{
  MxWindowPrivate *priv = self->priv = WINDOW_PRIVATE (self);

  priv->is_moving = -1;
  priv->icon_changed = TRUE;
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
      if (CLUTTER_ACTOR_IS_MAPPED (priv->stage))
        mx_window_pre_paint_cb (priv->stage, window);
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

  if (window->priv->has_toolbar != toolbar)
    {
      window->priv->has_toolbar = toolbar;

      if (!toolbar)
        {
          clutter_actor_hide (priv->toolbar);
          clutter_actor_hide (priv->resize_grip);
        }
      else
        {
          clutter_actor_show (priv->toolbar);
          clutter_actor_show (priv->resize_grip);
        }

      g_object_notify (G_OBJECT (window), "has-toolbar");

      /* Remove/add window decorations */
      mx_window_set_wm_hints (window);
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
 * Returns: A #MxToolbar
 */
MxToolbar *
mx_window_get_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return (MxToolbar*) window->priv->toolbar;
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
      ClutterStage *stage;
      Display *dpy;
      Window win;

      if (!priv->stage)
        return;

      stage = CLUTTER_STAGE (priv->stage);
      win = clutter_x11_get_stage_window (stage);
      dpy = clutter_x11_get_default_display ();

      priv->small_screen = small_screen;

      /* If there's no window, we're not mapped yet - we'll resize
       * on map.
       */
      if (win == None)
        return;

      /* In case we were in the middle of a move/resize */
      if (priv->is_moving != -1)
        {
          clutter_ungrab_pointer_for_device (priv->is_moving);
          clutter_set_motion_events_enabled (TRUE);
          priv->is_moving = -1;
          if (priv->is_resizing)
            {
              XUndefineCursor (dpy, win);
              priv->is_resizing = FALSE;
            }
        }

      if (small_screen)
        {
          if (!clutter_stage_get_fullscreen (stage))
            {
              int width, height;
              XRRScreenResources *res;

              clutter_actor_get_size (priv->stage,
                                      &priv->last_width,
                                      &priv->last_height);

              /* Move/size ourselves to the size of the screen. We could
               * also set ourselves as not resizable, but a WM that respects
               * our small-screen mode won't give the user controls to
               * modify the window, and if it does, just let them.
               */
              res = XRRGetScreenResourcesCurrent (dpy, win);
              width = res->modes[res->nmode].width;
              height = res->modes[res->nmode].height;
              XRRFreeScreenResources (res);

              XMoveResizeWindow (dpy, win, 0, 0, width, height);
            }

          if (priv->resize_grip)
            clutter_actor_hide (priv->resize_grip);
        }
      else
        {
          /* If we started off in small-screen mode, our last size won't
           * be known, so use the preferred size.
           */
          if (!priv->last_width && !priv->last_height)
            mx_window_get_size (window,
                                NULL, NULL,
                                &priv->last_width, &priv->last_height);

          clutter_actor_set_size (priv->stage,
                                  priv->last_width,
                                  priv->last_height);

          if (priv->resize_grip && priv->has_toolbar)
            {
              clutter_actor_show (priv->resize_grip);
              if (priv->child)
                clutter_actor_raise (priv->resize_grip, priv->child);
            }
        }

      g_object_notify (G_OBJECT (window), "small-screen");
    }
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
mx_window_get_window_position (MxWindow *window, gint *x, gint *y)
{
  unsigned int width, height, border_width, depth;
  MxWindowPrivate *priv;
  Window win, root_win;
  ClutterStage *stage;
  int win_x, win_y;
  Display *dpy;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

  if (!priv->stage)
    return;

  stage = CLUTTER_STAGE (priv->stage);

  if (priv->small_screen || clutter_stage_get_fullscreen (stage))
    {
      if (x)
        *x = 0;
      if (y)
        *y = 0;
      return;
    }

  win = clutter_x11_get_stage_window (stage);
  dpy = clutter_x11_get_default_display ();

  XGetGeometry (dpy, win,
                &root_win,
                &win_x, &win_y,
                &width, &height,
                &border_width,
                &depth);

  if (x)
    *x = win_x;
  if (y)
    *y = win_y;
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
mx_window_set_window_position (MxWindow *window, gint x, gint y)
{
  Window win;
  Display *dpy;
  ClutterStage *stage;
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

  if (!priv->stage)
    return;

  stage = CLUTTER_STAGE (priv->stage);

  /* Don't try to move a full-screen/small-screen window */
  if (priv->small_screen || clutter_stage_get_fullscreen (stage))
    return;

  win = clutter_x11_get_stage_window (stage);
  dpy = clutter_x11_get_default_display ();

  XMoveWindow (dpy, win, x, y);
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

  priv->icon_changed = TRUE;
  mx_window_set_wm_hints (window);
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

  priv->icon_changed = TRUE;
  mx_window_set_wm_hints (window);
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
  return (ClutterStage *)window->priv->stage;
}

