/* nbtk-clipboard.c */

#include "nbtk-clipboard.h"

G_DEFINE_TYPE (NbtkClipboard, nbtk_clipboard, G_TYPE_OBJECT)

#define CLIPBOARD_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_CLIPBOARD, NbtkClipboardPrivate))

struct _NbtkClipboardPrivate
{
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
  static default_clipboard = NULL;

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
                         NbtkClipboardContentsFunc  callback)
{
  g_return_if_fail (NBTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (callback != NULL);
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
nbtk_clipboard_set_text (NbtkClipboard *cliboard,
                         const gchar   *text)
{
  g_return_if_fail (NBTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (text != NULL);
}
