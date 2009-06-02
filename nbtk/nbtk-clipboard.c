/* nbtk-clipboard.c */

#include "nbtk-clipboard.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <clutter/x11/clutter-x11.h>

G_DEFINE_TYPE (NbtkClipboard, nbtk_clipboard, G_TYPE_OBJECT)

#define CLIPBOARD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_CLIPBOARD, NbtkClipboardPrivate))

struct _NbtkClipboardPrivate
{
  Window clipboard_window;
};

typedef struct _EventFilterData EventFilterData;
struct _EventFilterData
{
  NbtkClipboard *clipboard;
  NbtkClipboardCallbackFunc callback;
  gpointer user_data;
};

static void
nbtk_clipboard_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_clipboard_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_clipboard_dispose (GObject *object)
{
  G_OBJECT_CLASS (nbtk_clipboard_parent_class)->dispose (object);
}

static void
nbtk_clipboard_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_clipboard_parent_class)->finalize (object);
}

static void
nbtk_clipboard_class_init (NbtkClipboardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkClipboardPrivate));

  object_class->get_property = nbtk_clipboard_get_property;
  object_class->set_property = nbtk_clipboard_set_property;
  object_class->dispose = nbtk_clipboard_dispose;
  object_class->finalize = nbtk_clipboard_finalize;
}

static void
nbtk_clipboard_init (NbtkClipboard *self)
{
  self->priv = CLIPBOARD_PRIVATE (self);

  self->priv->clipboard_window =
    XCreateSimpleWindow (clutter_x11_get_default_display (),
                         clutter_x11_get_root_window (),
                         -1, -1, 1, 1, 0, 0, 0);

}

ClutterX11FilterReturn
nbtk_clipboard_x11_event_filter (XEvent          *xev,
                                 ClutterEvent    *cev,
                                 EventFilterData *filter_data)
{
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *data1, *data2;
  char *got;

#define DEFAULT_PROP_SIZE 1024

  if(xev->type != SelectionNotify)
    return CLUTTER_X11_FILTER_CONTINUE;

  if (xev->xselection.property == None)
    {
      /* clipboard empty */
      filter_data->callback (filter_data->clipboard,
                             NULL,
                             filter_data->user_data);

      g_free (filter_data);
      return CLUTTER_X11_FILTER_CONTINUE;
    }

  XGetWindowProperty (xev->xselection.display,
                      xev->xselection.requestor,
                      xev->xselection.property,
                      0L, DEFAULT_PROP_SIZE,
                      True,
                      AnyPropertyType,
                      &actual_type,
                      &actual_format,
                      &nitems,
                      &bytes_after,
                      &data1);

  if (bytes_after > 0)
    {

      if (XGetWindowProperty (xev->xselection.display,
                              xev->xselection.requestor,
                              xev->xselection.property,
                              DEFAULT_PROP_SIZE, bytes_after,
                              True,
                              AnyPropertyType,
                              &actual_type,
                              &actual_format,
                              &nitems,
                              &bytes_after,
                              &data2) != Success)
        {
          /* something went wrong, let's just return what's left */
          got = (char*) data1;
        }
      else
        {
          got = g_strconcat ((char*) data1, data2, NULL);
          g_free (data2);
          g_free (data1);
        }
    }
  else
    {
      got = (char*) data1;
    }


  filter_data->callback (filter_data->clipboard, got, filter_data->user_data);

  clutter_x11_remove_filter ((ClutterX11FilterFunc) nbtk_clipboard_x11_event_filter,
                             filter_data);

  g_free (filter_data);
  g_free (got);

  return CLUTTER_X11_FILTER_CONTINUE;
}

/**
 * nbtk_clipboard_get_default:
 *
 * Get the global #NbtkClipboard object that represents the cliboard.
 *
 * Returns: a #NbtkClipboard owned by Nbtk and must not be unrefferenced or
 * freed.
 */
NbtkClipboard*
nbtk_clipboard_get_default ()
{
  static NbtkClipboard *default_clipboard = NULL;

  if (!default_clipboard)
    {
      default_clipboard = g_object_new (NBTK_TYPE_CLIPBOARD, NULL);
    }

  return default_clipboard;
}

/**
 * nbtk_clipboard_get_text:
 * @clipboard: A #NbtkCliboard
 * @callback: function to be called when the text is retreived
 *
 * Request the data from the clipboard in text form. @callback is executed
 * when the data is retreived.
 *
 */
void
nbtk_clipboard_get_text (NbtkClipboard             *clipboard,
                         NbtkClipboardCallbackFunc  callback,
                         gpointer                   user_data)
{
  EventFilterData *data;
  Atom clip;
  Display *dpy;

  g_return_if_fail (NBTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (callback != NULL);

  data = g_new0 (EventFilterData, 1);
  data->clipboard = clipboard;
  data->callback = callback;
  data->user_data = user_data;

  clutter_x11_add_filter ((ClutterX11FilterFunc)nbtk_clipboard_x11_event_filter,
                          data);

  dpy = clutter_x11_get_default_display ();

  clip = XInternAtom(dpy, "CLIPBOARD", 0);
  XConvertSelection (dpy,
                     clip,
                     XA_STRING, XA_STRING,
                     clipboard->priv->clipboard_window,
                     CurrentTime);
}

/**
 * nbtk_clipboard_set_text:
 * @cliboard: A #NbtkClipboard
 * @text: text to copy to the clipboard
 *
 * Sets text as the current contents of the clipboard.
 *
 */
void
nbtk_clipboard_set_text (NbtkClipboard *clipboard,
                         const gchar   *text)
{
  g_return_if_fail (NBTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (text != NULL);
}
