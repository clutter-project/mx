/* mx-window.h */

#ifndef _MX_WINDOW_H
#define _MX_WINDOW_H

#include <glib-object.h>
#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_WINDOW mx_window_get_type()

#define MX_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW, MxWindow))

#define MX_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW, MxWindowClass))

#define MX_IS_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW))

#define MX_IS_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW))

#define MX_WINDOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW, MxWindowClass))

typedef struct _MxWindow MxWindow;
typedef struct _MxWindowClass MxWindowClass;
typedef struct _MxWindowPrivate MxWindowPrivate;

struct _MxWindow
{
  ClutterStage parent;

  MxWindowPrivate *priv;
};

struct _MxWindowClass
{
  ClutterStageClass parent_class;
};

GType mx_window_get_type (void) G_GNUC_CONST;

MxWindow *mx_window_new (void);

ClutterActor* mx_window_get_child (MxWindow *window);
void          mx_window_set_child (MxWindow *window, ClutterActor *actor);


MxToolbar* mx_window_get_toolbar     (MxWindow *window);
gboolean   mx_window_get_has_toolbar (MxWindow *window);
void       mx_window_set_has_toolbar (MxWindow *window, gboolean  toolbar);


G_END_DECLS

#endif /* _MX_WINDOW_H */
