/* mx-window.c */

#include "mx-window.h"
#include "mx-toolbar.h"
#include "mx-focus-manager.h"
#include "mx-private.h"
#include <clutter/x11/clutter-x11.h>

/* for pointer cursor support */
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);
static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxWindow, mx_window, CLUTTER_TYPE_STAGE,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

#define WINDOW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW, MxWindowPrivate))

struct _MxWindowPrivate
{
  guint is_fullscreen : 1;
  guint has_toolbar   : 1;
  guint is_resizing   : 1;
  guint small_screen  : 1;

  gint  is_moving;

  CoglHandle resize_grip;
  gfloat     last_width;
  gfloat     last_height;
#if !CLUTTER_CHECK_VERSION(1,1,8)
  gfloat     natural_width;
  gfloat     natural_height;
#endif

  ClutterActor *toolbar;
  ClutterActor *child;

  gint  drag_x_start;
  gint  drag_y_start;
  gint  drag_win_x_start;
  gint  drag_win_y_start;
  guint drag_width_start;
  guint drag_height_start;

  MxStyle *style;
  gchar   *pseudo_class;
  gchar   *style_class;

  MxFocusManager *focus_manager;
};

enum
{
  PROP_STYLE = 1,
  PROP_STYLE_CLASS,
  PROP_STYLE_PSEUDO_CLASS,
  PROP_SMALL_SCREEN
};

/* clutter container iface implementation */
static void
mx_window_add (ClutterContainer *container,
               ClutterActor     *actor)
{
  mx_window_set_child (MX_WINDOW (container), actor);
}

static void
mx_window_remove (ClutterContainer *container,
                  ClutterActor     *actor)
{
  MxWindowPrivate *priv = MX_WINDOW (container)->priv;

  if (priv->child == actor)
    mx_window_set_child (MX_WINDOW (container), NULL);
}

static void
mx_window_foreach (ClutterContainer *container,
                   ClutterCallback   callback,
                   gpointer          user_data)
{
  MxWindowPrivate *priv = MX_WINDOW (container)->priv;

  if (priv->child)
    callback (priv->child, user_data);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_window_add;
  iface->remove = mx_window_remove;
  iface->foreach = mx_window_foreach;
}

/* stylable implementation */
static void
mx_window_set_style (MxStylable *stylable,
                     MxStyle    *style)
{
  MxWindowPrivate *priv = MX_WINDOW (stylable)->priv;

  if (priv->style)
    g_object_unref (priv->style);

  priv->style = style;
}

static MxStyle*
mx_window_get_style (MxStylable *stylable)
{
  MxWindowPrivate *priv = MX_WINDOW (stylable)->priv;

  if (!priv->style)
    return priv->style = mx_style_get_default ();
  else
    return priv->style;
}

static void
mx_window_set_style_class (MxStylable  *actor,
                           const gchar *style_class)
{
  MxWindowPrivate *priv;

  priv = MX_WINDOW (actor)->priv;

  if (g_strcmp0 (style_class, priv->style_class))
    {
      g_free (priv->style_class);
      priv->style_class = g_strdup (style_class);

      mx_stylable_changed (actor);

      g_object_notify (G_OBJECT (actor), "style-class");
    }
}

static void
mx_window_set_style_pseudo_class (MxStylable  *actor,
                                   const gchar *pseudo_class)
{
  MxWindowPrivate *priv;

  priv = MX_WINDOW (actor)->priv;

  if (g_strcmp0 (pseudo_class, priv->pseudo_class))
    {
      g_free (priv->pseudo_class);
      priv->pseudo_class = g_strdup (pseudo_class);

      mx_stylable_changed (actor);

      g_object_notify (G_OBJECT (actor), "style-pseudo-class");
    }
}

static const gchar*
mx_window_get_style_pseudo_class (MxStylable *actor)
{
  return ((MxWindow *) actor)->priv->pseudo_class;
}


static const gchar*
mx_window_get_style_class (MxStylable *actor)
{
  return ((MxWindow *) actor)->priv->style_class;
}



static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialised = FALSE;

  if (!is_initialised)
    {
      GParamSpec *pspec;
      ClutterColor bg_color = { 0xff, 0xff, 0xff, 0x00 };

      is_initialised = TRUE;

      iface->get_style = mx_window_get_style;
      iface->set_style = mx_window_set_style;
      iface->set_style_class = mx_window_set_style_class;
      iface->get_style_class = mx_window_get_style_class;
      iface->set_style_pseudo_class = mx_window_set_style_pseudo_class;
      iface->get_style_pseudo_class = mx_window_get_style_pseudo_class;

      pspec = clutter_param_spec_color ("background-color",
                                        "Background Color",
                                        "The background color of the window",
                                        &bg_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WINDOW, pspec);

      pspec = g_param_spec_boxed ("mx-resize-grip",
                                  "Resize Grip",
                                  "Resize grip used in the corner of the"
                                  " window to allow the user to resize.",
                                  MX_TYPE_BORDER_IMAGE,
                                  G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WINDOW, pspec);
    }
}

static void
mx_window_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxWindowPrivate *priv = MX_WINDOW (object)->priv;

  switch (property_id)
    {
    case PROP_STYLE:
      g_value_set_object (value, priv->style);
      break;

    case PROP_STYLE_CLASS:
      g_value_set_string (value, priv->style_class);
      break;

    case PROP_STYLE_PSEUDO_CLASS:
      g_value_set_string (value, priv->pseudo_class);
      break;

    case PROP_SMALL_SCREEN:
      g_value_set_boolean (value, priv->small_screen);
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
  MxStylable *win = MX_STYLABLE (object);


  switch (property_id)
    {
    case PROP_STYLE:
      mx_window_set_style (win, MX_STYLE (g_value_get_object (value)));
      break;

    case PROP_STYLE_CLASS:
      mx_window_set_style_class (win, g_value_get_string (value));
      break;

    case PROP_STYLE_PSEUDO_CLASS:
      mx_window_set_style_pseudo_class (win, g_value_get_string (value));
      break;

    case PROP_SMALL_SCREEN:
      mx_window_set_small_screen (MX_WINDOW (object),
                                  g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_window_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_window_parent_class)->dispose (object);
}

static void
mx_window_finalize (GObject *object)
{
  MxWindowPrivate *priv = MX_WINDOW (object)->priv;

  if (priv->resize_grip)
    {
      cogl_handle_unref (priv->resize_grip);
      priv->resize_grip = NULL;
    }

  G_OBJECT_CLASS (mx_window_parent_class)->finalize (object);
}




static void
mx_window_get_minimum_size (MxWindow *self,
                            gfloat   *width_p,
                            gfloat   *height)
{
  gfloat width;
  MxWindowPrivate *priv = self->priv;

  width = 0;

  if (priv->toolbar)
    clutter_actor_get_preferred_width (priv->toolbar,
                                       -1,
                                       &width,
                                       NULL);

  if (priv->child)
    {
      gfloat child_min_width;
      clutter_actor_get_preferred_width (priv->child,
                                         -1,
                                         &child_min_width,
                                         NULL);
      if (child_min_width > width)
        width = child_min_width;
    }

  if (width_p)
    *width_p = width;

  if (height)
    {
      *height = 0;

      if (priv->toolbar)
        clutter_actor_get_preferred_height (priv->toolbar,
                                            width,
                                            height,
                                            NULL);

      if (priv->child)
        {
          gfloat child_min_height;
          clutter_actor_get_preferred_height (priv->child,
                                              width,
                                              &child_min_height,
                                              NULL);
          *height += child_min_height;
        }
    }
}

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long inputMode;
    unsigned long status;
} PropMotifWmHints;

typedef PropMotifWmHints PropMwmHints;


static void
mx_window_map (ClutterActor *actor)
{
  MxWindow *window;
  MxWindowPrivate *priv;
  static Atom hint_atom = 0;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->map (actor);

  window = MX_WINDOW (actor);
  priv = window->priv;

  clutter_actor_map (priv->toolbar);

  if (priv->child)
    clutter_actor_map (priv->child);

  /* Remove the window decorations */
  if (!hint_atom)
    {
      PropMotifWmHints new_hints = {0,};
      PropMotifWmHints *hints;
      Display *dpy;
      Window win;

      dpy = clutter_x11_get_default_display ();
      win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));

      hint_atom = XInternAtom (dpy, "_MOTIF_WM_HINTS", 0);

      hints = &new_hints;

      hints->flags = 0x2;
      hints->functions = 0x0;
      hints->decorations = 0x0;

      XChangeProperty (dpy, win, hint_atom, hint_atom, 32, PropModeReplace,
                       (guchar*) hints,
                       sizeof(PropMotifWmHints)/ sizeof (long));
    }
}

static void
mx_window_unmap (ClutterActor *actor)
{
  MxWindowPrivate *priv;

  priv = MX_WINDOW (actor)->priv;

  clutter_actor_unmap (priv->toolbar);

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->unmap (actor);
}

static void
mx_window_paint (ClutterActor *actor)
{
  MxWindowPrivate *priv;
  gfloat width, height;

  priv = MX_WINDOW (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->paint (actor);

  if (priv->child)
    clutter_actor_paint (priv->child);

  clutter_actor_paint (priv->toolbar);

  /* If we're in small-screen or fullscreen mode, we don't want a frame
   * or a resize handle.
   */
  if (priv->small_screen ||
      clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
    return;

  /* paint frame */

  clutter_actor_get_size (actor, &width, &height);
  cogl_set_source_color4f (0.2, 0.2, 0.2, 1);

  cogl_rectangle (0, 0, width, 1);
  cogl_rectangle (0, height - 1, width, height);

  cogl_rectangle (0, 1, 1, height - 1);
  cogl_rectangle (width - 1, 1, width, height - 1);

  if (priv->resize_grip)
    {
      guint rheight, rwidth;

      cogl_set_source_texture (priv->resize_grip);
      rwidth = cogl_texture_get_width (priv->resize_grip);
      rheight = cogl_texture_get_height (priv->resize_grip);
      cogl_rectangle (width - rwidth - 1, height - rheight - 1,
                      width - 1, height - 1);
    }
}

static void
mx_window_pick (ClutterActor       *actor,
                const ClutterColor *color)
{
  MxWindowPrivate *priv;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->pick (actor, color);

  priv = MX_WINDOW (actor)->priv;

  /* Don't pick while we're moving/resizing */
  if (priv->is_moving != -1)
    return;

  clutter_actor_paint (priv->toolbar);

  if (priv->child)
    clutter_actor_paint (priv->child);
}

static void
mx_window_allocate (ClutterActor           *actor,
                    const ClutterActorBox  *box,
                    ClutterAllocationFlags  flags)
{
  ClutterActorBox toolbarbox, childbox;
  MxPadding padding;
  MxWindow *window;
  MxWindowPrivate *priv;
  gfloat height;
#if CLUTTER_CHECK_VERSION(1,1,8)
  gfloat width;
#endif

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->allocate (actor, box, flags);

  window = MX_WINDOW (actor);
  priv = window->priv;

  if (priv->small_screen ||
      clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
      padding.top = padding.right = padding.bottom = padding.left = 0;
  else
      padding.top = padding.right = padding.bottom = padding.left = 1;

  clutter_actor_get_preferred_height (priv->toolbar, (box->x2 - box->x1), NULL,
                                      &height);

  toolbarbox.x1 = padding.left;
  toolbarbox.y1 = padding.top;
  toolbarbox.x2 = (box->x2 - box->x1) - padding.right;
  toolbarbox.y2 = toolbarbox.y1 + height;

  clutter_actor_allocate (priv->toolbar, &toolbarbox, flags);

  if (priv->child)
    {
      childbox.x1 = padding.left;
      childbox.y1 = toolbarbox.y2;
      childbox.x2 = (box->x2 - box->x1) - padding.right;
      childbox.y2 = (box->y2 - box->y1) - padding.bottom;
      clutter_actor_allocate (priv->child, &childbox, flags);
    }

#if CLUTTER_CHECK_VERSION(1,1,8)
  /* Update minimum size */
  mx_window_get_minimum_size (window, &width, &height);
  if (width < 1.0)
    width = 1.0;
  if (height < 1.0)
    height = 1.0;
  clutter_stage_set_minimum_size (CLUTTER_STAGE (window),
                                  (guint)width,
                                  (guint)height);
#endif
}

static gboolean
mx_window_button_press_event (ClutterActor       *actor,
                              ClutterButtonEvent *event)
{
  unsigned int width, height, border_width, depth, mask;
  Window win, root, child;
  int x, y, win_x, win_y;
  MxWindowPrivate *priv;
  Display *dpy;

  priv = MX_WINDOW (actor)->priv;

  /* Bail out early in small-screen or fullscreen mode */
  if (priv->small_screen ||
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
mx_window_button_release_event (ClutterActor       *actor,
                                ClutterButtonEvent *event)
{
  MxWindow *window = MX_WINDOW (actor);
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
mx_window_captured_event (ClutterActor *actor,
                          ClutterEvent *event)
{
  MxWindowPrivate *priv = MX_WINDOW (actor)->priv;

  switch (event->type)
    {
    case CLUTTER_MOTION:
      /* Check if we're over the resize handle */
      if ((priv->is_moving == -1) && !priv->small_screen &&
          !clutter_stage_get_fullscreen (CLUTTER_STAGE (actor)))
        {
          gint x, y;
          Window win;
          Display *dpy;
          gfloat height, width;
          guint rheight, rwidth;

          static Cursor csoutheast = 0;
          ClutterMotionEvent *mevent = &event->motion;

          win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
          dpy = clutter_x11_get_default_display ();

          clutter_actor_get_size (actor, &width, &height);

          x = mevent->x;
          y = mevent->y;

          /* Create the resize cursor */
          if (!csoutheast)
            csoutheast = XCreateFontCursor (dpy, XC_bottom_right_corner);

          rwidth = cogl_texture_get_width (priv->resize_grip);
          rheight = cogl_texture_get_height (priv->resize_grip);

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

    case CLUTTER_BUTTON_PRESS:
      /* We want resizing to happen even if there are active widgets
       * underneath the resize-handle.
       */
      if (priv->is_resizing)
        return mx_window_button_press_event (actor, &event->button);
      else
        return FALSE;

    default:
      return FALSE;
    }
}

static gboolean
mx_window_motion_event (ClutterActor       *actor,
                        ClutterMotionEvent *event)
{
  gint offsetx, offsety;
  gint x, y, winx, winy;
  guint mask;
  MxWindow *window;
  MxWindowPrivate *priv;
  Window win, root_win, root, child;
  Display *dpy;
  gfloat height, width;

  window = MX_WINDOW (actor);
  priv = window->priv;

  /* Ignore motion events while in small-screen mode, fullscreen mode and
   * if they're not from our grabbed device.
   */
  if ((priv->small_screen) ||
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
      int screen;
      gfloat min_width, min_height;

      screen = DefaultScreen (dpy);
      mx_window_get_minimum_size (window, &min_width, &min_height);

      x = MAX (priv->drag_width_start + (x - priv->drag_x_start), min_width);
      y = MAX (priv->drag_height_start + (y - priv->drag_y_start), min_height);

      width = MIN (x, DisplayWidth (dpy, screen) - priv->drag_win_x_start);
      height = MIN (y, DisplayHeight (dpy, screen) - priv->drag_win_y_start);

#if !CLUTTER_CHECK_VERSION(1,1,8)
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
style_changed_cb (MxWindow *window)
{
  MxWindowPrivate *priv = window->priv;
  MxBorderImage *grip_filename;
  ClutterColor *color;

  if (priv->resize_grip)
    {
      cogl_handle_unref (priv->resize_grip);
      priv->resize_grip = NULL;
    }

  mx_stylable_get (MX_STYLABLE (window),
                   "mx-resize-grip", &grip_filename,
                   "background-color", &color,
                   NULL);

  if (color)
    {
      clutter_stage_set_color (CLUTTER_STAGE (window), color);
      clutter_color_free (color);
    }

  if (grip_filename)
    {
      priv->resize_grip = cogl_texture_new_from_file (grip_filename->uri,
                                                      COGL_TEXTURE_NONE,
                                                      COGL_PIXEL_FORMAT_ANY,
                                                      NULL);
      if (priv->resize_grip == COGL_INVALID_HANDLE)
        {
          priv->resize_grip = NULL;
          g_warning ("Error loading resize grip image");
        }

      g_boxed_free (MX_TYPE_BORDER_IMAGE, grip_filename);

    }
}

#if !CLUTTER_CHECK_VERSION(1,1,8)
static void
mx_window_get_preferred_width (ClutterActor *self,
                               gfloat        for_height,
                               gfloat       *min_width_p,
                               gfloat       *nat_width_p)
{
  if (clutter_stage_get_fullscreen (CLUTTER_STAGE (self)))
    CLUTTER_ACTOR_CLASS (mx_window_parent_class)->
      get_preferred_width (self, for_height, min_width_p, nat_width_p);
  else
    {
      gfloat min_width;
      MxWindowPrivate *priv = MX_WINDOW (self)->priv;

      mx_window_get_minimum_size (MX_WINDOW (self), &min_width, NULL);
      if (min_width_p)
        *min_width_p = min_width;
      if (nat_width_p)
        *nat_width_p = priv->natural_width ? priv->natural_width : min_width;
    }
}

static void
mx_window_get_preferred_height (ClutterActor *self,
                                gfloat        for_width,
                                gfloat       *min_height_p,
                                gfloat       *nat_height_p)
{
  if (clutter_stage_get_fullscreen (CLUTTER_STAGE (self)))
    CLUTTER_ACTOR_CLASS (mx_window_parent_class)->
      get_preferred_height (self, for_width, min_height_p, nat_height_p);
  else
    {
      gfloat min_height;
      MxWindowPrivate *priv = MX_WINDOW (self)->priv;

      mx_window_get_minimum_size (MX_WINDOW (self), NULL, &min_height);
      if (min_height_p)
        *min_height_p = min_height;
      if (nat_height_p)
        *nat_height_p = priv->natural_height ?
                          priv->natural_height : min_height;
    }
}
#else
static void
mx_window_realize_cb (ClutterActor *actor)
{
  gfloat width, height, nat_width, nat_height;
  gboolean width_set, height_set;

  MxWindow *window = MX_WINDOW (actor);

  mx_window_get_minimum_size (window, &width, &height);

  if (width < 1.0)
    width = 1.0;
  if (height < 1.0)
    height = 1.0;
  clutter_stage_set_minimum_size (CLUTTER_STAGE (window),
                                  (guint)width,
                                  (guint)height);

  /* If the user has set a size on the window already,
   * make sure to use it (but still enforce the
   * minimum size).
   */
  g_object_get (G_OBJECT (window),
                "natural-width", &nat_width,
                "natural-width-set", &width_set,
                "natural-height", &nat_height,
                "natural-height-set", &height_set,
                NULL);

  if (width_set && (nat_width > width))
    width = nat_width;
  if (height_set && (nat_height > height))
    height = nat_height;

  clutter_actor_set_size (actor, width, height);
}
#endif

static void
mx_window_class_init (MxWindowClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWindowPrivate));

  object_class->get_property = mx_window_get_property;
  object_class->set_property = mx_window_set_property;
  object_class->dispose = mx_window_dispose;
  object_class->finalize = mx_window_finalize;

  actor_class->map = mx_window_map;
  actor_class->unmap = mx_window_unmap;
  actor_class->paint = mx_window_paint;
  actor_class->pick = mx_window_pick;
  actor_class->allocate = mx_window_allocate;
  actor_class->button_press_event = mx_window_button_press_event;
  actor_class->button_release_event = mx_window_button_release_event;
  actor_class->captured_event = mx_window_captured_event;
  actor_class->motion_event = mx_window_motion_event;
#if !CLUTTER_CHECK_VERSION(1,1,8)
  actor_class->get_preferred_width = mx_window_get_preferred_width;
  actor_class->get_preferred_height = mx_window_get_preferred_height;
#endif

  /* stylable interface properties */
  g_object_class_override_property (object_class, PROP_STYLE, "style");
  g_object_class_override_property (object_class, PROP_STYLE_CLASS,
                                    "style-class");
  g_object_class_override_property (object_class, PROP_STYLE_PSEUDO_CLASS,
                                    "style-pseudo-class");

  pspec = g_param_spec_boolean ("small-screen",
                                "Small screen",
                                "Window should occupy the entire screen "
                                "contents, without explicitly setting "
                                "itself fullscreen.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SMALL_SCREEN, pspec);
}

static void
mx_window_init (MxWindow *self)
{
  MxWindowPrivate *priv;

  priv = self->priv = WINDOW_PRIVATE (self);

  priv->is_moving = -1;

  priv->toolbar = mx_toolbar_new ();
  clutter_actor_set_parent (priv->toolbar, CLUTTER_ACTOR (self));

  priv->has_toolbar = TRUE;

  priv->focus_manager = mx_focus_manager_new (CLUTTER_STAGE (self));

  priv->style = mx_style_get_default ();

  g_signal_connect (priv->style, "changed",
                    G_CALLBACK (style_changed_cb), NULL);

  style_changed_cb (self);

  clutter_stage_set_user_resizable (CLUTTER_STAGE (self), TRUE);

  g_signal_connect (self, "notify::fullscreen-set",
                    G_CALLBACK (clutter_actor_queue_relayout), NULL);

#if CLUTTER_CHECK_VERSION(1,1,8)
  g_signal_connect (self, "realize",
                    G_CALLBACK (mx_window_realize_cb), NULL);

  g_object_set (G_OBJECT (self), "use-alpha", TRUE, NULL);
#endif
}

MxWindow *
mx_window_new (void)
{
  return g_object_new (MX_TYPE_WINDOW, NULL);
}

void
mx_window_set_child (MxWindow     *window,
                     ClutterActor *actor)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));
  g_return_if_fail (actor == NULL || CLUTTER_IS_ACTOR (actor));

  priv = window->priv;

  if (priv->child != actor)
    {
      if (priv->child)
        {
          ClutterActor *old_child = priv->child;

          g_object_ref (old_child);

          priv->child = NULL;
          clutter_actor_unparent (old_child);

          g_signal_emit_by_name (window, "actor-removed", old_child);

          g_object_unref (old_child);
        }

      if (actor)
        {
          priv->child = actor;
          clutter_actor_set_parent (actor, CLUTTER_ACTOR (window));
          g_signal_emit_by_name (window, "actor-added", priv->child);
        }
    }

}

ClutterActor*
mx_window_get_child (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return window->priv->child;
}

/*
void
mx_window_set_fullscreen (MxWindow *window,
                          gboolean  fullscreen)
{
  g_return_if_fail (MX_IS_WINDOW (window));

  if (window->priv->has_fullscreen != fullscreen)
    {
      window->priv->has_fullscreen = fullscreen;

      if (fullscreen)
        {
          // hide decorations and go fullscreen
        }

      //g_object_notify (window, "fullscreen");
    }
}

gboolean
mx_window_get_fullscreen (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);

  return window->priv->has_fullscreen;
}
*/

void
mx_window_set_has_toolbar (MxWindow *window,
                           gboolean  toolbar)
{
  g_return_if_fail (MX_IS_WINDOW (window));

  if (window->priv->has_toolbar != toolbar)
    {
      window->priv->has_toolbar = toolbar;

      if (!toolbar)
        {
          // hide toolbar and add window decorations
        }
      else
        {
          // show toolbar and hide window decorations
        }

      //g_object_notify (window, "toolbar");
    }
}

gboolean
mx_window_get_has_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);

  return window->priv->has_toolbar;
}

MxToolbar *
mx_window_get_toolbar (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), NULL);

  return (MxToolbar*) window->priv->toolbar;
}

gboolean
mx_window_get_small_screen (MxWindow *window)
{
  g_return_val_if_fail (MX_IS_WINDOW (window), FALSE);

  return window->priv->small_screen;
}

void
mx_window_set_small_screen (MxWindow *window, gboolean small_screen)
{
  MxWindowPrivate *priv;

  g_return_if_fail (MX_IS_WINDOW (window));

  priv = window->priv;

  if (priv->small_screen != small_screen)
    {
      ClutterStage *stage = CLUTTER_STAGE (window);
      Window win = clutter_x11_get_stage_window (stage);
      Display *dpy = clutter_x11_get_default_display ();

      priv->small_screen = small_screen;

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

      /* Get out of fullscreen mode if we're in fullscreen
       * and switching to small-screen.
       */
      if (small_screen && clutter_stage_get_fullscreen (CLUTTER_STAGE (window)))
        clutter_stage_set_fullscreen (CLUTTER_STAGE (window), FALSE);

      if (small_screen)
        {
          int screen, width, height;

          screen = DefaultScreen (dpy);
          width = DisplayWidth (dpy, screen);
          height = DisplayHeight (dpy, screen);

          clutter_actor_get_size (CLUTTER_ACTOR (window),
                                  &priv->last_width,
                                  &priv->last_height);

          /* Move/size ourselves to the size of the screen. We could
           * also set ourselves as not resizable, but a WM that respects
           * our small-screen mode won't give the user controls to
           * modify the window, and if it does, just let them.
           */
          XMoveResizeWindow (dpy, win, 0, 0, width, height);
        }
      else
        {
          /* This is the situation where resizing ourselves to the size of
           * the screen caused the WM to fullscreen us. In this case, just
           * set ourselves as un-fullscreened and we'll restore the size.
           */
          if (clutter_stage_get_fullscreen (stage))
            clutter_stage_set_fullscreen (stage, FALSE);
          else
            clutter_actor_set_size (CLUTTER_ACTOR (window),
                                    priv->last_width,
                                    priv->last_height);
        }

      g_object_notify (G_OBJECT (window), "small-screen");
    }
}
