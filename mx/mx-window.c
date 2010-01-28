/* mx-window.c */

#include "mx-window.h"
#include "mx-toolbar.h"
#include "mx-focus-manager.h"
#include <clutter/x11/clutter-x11.h>

/* for pointer cursor support */
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxWindow, mx_window, CLUTTER_TYPE_STAGE,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))

#define WINDOW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_WINDOW, MxWindowPrivate))

struct _MxWindowPrivate
{
  guint is_fullscreen : 1;
  guint has_toolbar   : 1;
  guint is_moving     : 1;

  enum {NONE, NORTH, SOUTH, EAST, WEST} is_resizing;

  ClutterActor *toolbar;
  ClutterActor *child;

  guint drag_x_start;
  guint drag_y_start;

  MxFocusManager *focus_manager;
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


static void
mx_window_get_property (GObject    *object,
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
mx_window_set_property (GObject      *object,
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
mx_window_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_window_parent_class)->dispose (object);
}

static void
mx_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_window_parent_class)->finalize (object);
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
  MxWindowPrivate *priv;
  static Atom hint_atom = 0;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->map (actor);

  priv = MX_WINDOW (actor)->priv;

  clutter_actor_map (priv->toolbar);

  if (priv->child)
    clutter_actor_map (priv->child);

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

      clutter_stage_set_user_resizable (CLUTTER_STAGE (actor), TRUE);

      printf ("property changed\n");

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

  /* paint frame */

  clutter_actor_get_size (actor, &width, &height);
  cogl_set_source_color4f (0.2, 0.2, 0.2, 1);

  cogl_rectangle (0, 0, width, 1);
  cogl_rectangle (0, height - 1, width, height);

  cogl_rectangle (0, 1, 1, height - 1);
  cogl_rectangle (width - 1, 1, width, height - 1);
}

static void
mx_window_pick (ClutterActor       *actor,
                const ClutterColor *color)
{
  MxWindowPrivate *priv;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->pick (actor, color);

  priv = MX_WINDOW (actor)->priv;

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
  MxWindowPrivate *priv;
  gfloat height;

  CLUTTER_ACTOR_CLASS (mx_window_parent_class)->allocate (actor, box, flags);

  priv = MX_WINDOW (actor)->priv;

  if (0 /* full_screen */)
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
      childbox.y2 = (box->y2 - box->y1) - toolbarbox.y2 - padding.bottom;
      clutter_actor_allocate (priv->child, &childbox, flags);
    }

}

static gboolean
mx_window_button_press_event (ClutterActor       *actor,
                              ClutterButtonEvent *event)
{
  MxWindowPrivate *priv;
  gfloat height, width;
  gint x, y;


  priv = MX_WINDOW (actor)->priv;

  priv->is_moving = TRUE;

  x = priv->drag_x_start = event->x;
  y = priv->drag_y_start = event->y;

  clutter_actor_get_size (actor, &width, &height);

#if 0

  if (x <= 4 && y <= height - 4)
    priv->is_resizing = WEST;
  else if (x >= width - 4)
    priv->is_resizing = EAST;
  else if (y >= height - 4)
    priv->is_resizing = SOUTH;
  else if (y <= 4)
    priv->is_resizing = NORTH;
  else
    priv->is_resizing = NONE;

#endif

  clutter_set_motion_events_enabled (FALSE);
  return TRUE;
}

static gboolean
mx_window_button_release_event (ClutterActor       *actor,
                                ClutterButtonEvent *event)
{
  MxWindowPrivate *priv;

  priv = MX_WINDOW (actor)->priv;

  priv->is_moving = FALSE;

  clutter_set_motion_events_enabled (TRUE);

  return TRUE;
}

static gboolean
mx_window_motion_event (ClutterActor       *actor,
                        ClutterMotionEvent *event)
{
  gint offsetx, offsety;
  gint x, y, winx, winy;
  guint mask;
  MxWindowPrivate *priv;
  Window win, root_win, root, child;
  Display *dpy;
  gfloat height, width;

  priv = MX_WINDOW (actor)->priv;

  win = clutter_x11_get_stage_window (CLUTTER_STAGE (actor));
  dpy = clutter_x11_get_default_display ();

  clutter_actor_get_size (actor, &width, &height);

#if 0
  /* resize */
  static Cursor cwest, cnorthwest, cnorth, cnortheast, ceast, csoutheast, csouth, csouthwest;
  cwest = XCreateFontCursor (dpy, XC_left_side);
  cnorthwest = XCreateFontCursor (dpy, XC_top_left_corner);
  cnorth = XCreateFontCursor (dpy, XC_top_side);
  cnortheast = XCreateFontCursor (dpy, XC_top_right_corner);
  ceast = XCreateFontCursor (dpy, XC_left_side);
  csoutheast = XCreateFontCursor (dpy, XC_bottom_right_corner);
  csouth = XCreateFontCursor (dpy, XC_bottom_side);
  csouthwest = XCreateFontCursor (dpy, XC_bottom_left_corner);

  x = event->x;
  y = event->y;

  if (x <= 4 && y <= height - 4)
    XDefineCursor (dpy, win, cwest);
  else if (x >= width - 4)
    XDefineCursor (dpy, win, ceast);
  else if (y >= height - 4)
    XDefineCursor (dpy, win, csouth);
  else if (y <= 4)
    XDefineCursor (dpy, win, cnorth);
  else
    XUndefineCursor (dpy, win);
#endif

  /* drag */
  if (!priv->is_moving)
    return FALSE;

  offsetx = priv->drag_x_start;
  offsety = priv->drag_y_start;


  root_win = clutter_x11_get_root_window ();
  XQueryPointer (dpy, root_win, &root, &child, &x, &y, &winx, &winy, &mask);

  XMoveWindow (dpy, win, MAX (0, x - offsetx), MAX (0, y - offsety));

  return TRUE;
}

static void
mx_window_class_init (MxWindowClass *klass)
{
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
  actor_class->motion_event = mx_window_motion_event;
}

static void
mx_window_init (MxWindow *self)
{
  MxWindowPrivate *priv;

  priv = self->priv = WINDOW_PRIVATE (self);

  priv->toolbar = mx_toolbar_new ();
  clutter_actor_set_parent (priv->toolbar, CLUTTER_ACTOR (self));

  priv->has_toolbar = TRUE;

  priv->focus_manager = mx_focus_manager_new (CLUTTER_STAGE (self));
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
        clutter_actor_destroy (priv->child);

      priv->child = actor;
      clutter_actor_set_parent (actor, CLUTTER_ACTOR (window));
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
