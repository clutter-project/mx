/* nbtk-clipboard.h */

#ifndef _NBTK_CLIPBOARD_H
#define _NBTK_CLIPBOARD_H

#include <glib-object.h>

G_BEGIN_DECLS

#define NBTK_TYPE_CLIPBOARD nbtk_clipboard_get_type()

#define NBTK_CLIPBOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_CLIPBOARD, NbtkClipboard))

#define NBTK_CLIPBOARD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_CLIPBOARD, NbtkClipboardClass))

#define NBTK_IS_CLIPBOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_CLIPBOARD))

#define NBTK_IS_CLIPBOARD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_CLIPBOARD))

#define NBTK_CLIPBOARD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_CLIPBOARD, NbtkClipboardClass))

typedef struct _NbtkClipboard NbtkClipboard;
typedef struct _NbtkClipboardClass NbtkClipboardClass;
typedef struct _NbtkClipboardPrivate NbtkClipboardPrivate;

struct _NbtkClipboard
{
  GObject parent;
  NbtkClipboardPrivate *priv;
};

struct _NbtkClipboardClass
{
  GObjectClass parent_class;
};

typedef void (*NbtkClipboardCallbackFunc) (NbtkClipboard *clipboard,
                                           const gchar   *text,
                                           gpointer       user_data);

GType nbtk_clipboard_get_type (void);

NbtkClipboard* nbtk_clipboard_get_default ();
void nbtk_clipboard_get_text (NbtkClipboard *clipboard, NbtkClipboardCallbackFunc cllback, gpointer user_data);
void nbtk_clipboard_set_text (NbtkClipboard *clipboard, const gchar *text);

G_END_DECLS

#endif /* _NBTK_CLIPBOARD_H */
