/* mx-window-x11.h */

#ifndef _MX_WINDOW_X11_H
#define _MX_WINDOW_X11_H

#include <glib-object.h>
#include <mx/mx-window.h>

G_BEGIN_DECLS

#define MX_TYPE_WINDOW_X11 mx_window_x11_get_type()

#define MX_WINDOW_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW_X11, MxWindowX11))

#define MX_WINDOW_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW_X11, MxWindowX11Class))

#define MX_IS_WINDOW_X11(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW_X11))

#define MX_IS_WINDOW_X11_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW_X11))

#define MX_WINDOW_X11_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW_X11, MxWindowX11Class))

typedef struct _MxWindowX11 MxWindowX11;
typedef struct _MxWindowX11Class MxWindowX11Class;
typedef struct _MxWindowX11Private MxWindowX11Private;

struct _MxWindowX11
{
  MxWindow parent;

  MxWindowX11Private *priv;
};

struct _MxWindowX11Class
{
  MxWindowClass parent_class;
};

GType mx_window_x11_get_type (void) G_GNUC_CONST;

MxWindow *mx_window_new (void);

G_END_DECLS

#endif /* _MX_WINDOW_X11_H */
