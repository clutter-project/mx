/* mx-toggle.h */

#ifndef _MX_TOGGLE_H
#define _MX_TOGGLE_H

#include "mx-widget.h"

G_BEGIN_DECLS

#define MX_TYPE_TOGGLE mx_toggle_get_type()

#define MX_TOGGLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_TOGGLE, MxToggle))

#define MX_TOGGLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_TOGGLE, MxToggleClass))

#define MX_IS_TOGGLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_TOGGLE))

#define MX_IS_TOGGLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_TOGGLE))

#define MX_TOGGLE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_TOGGLE, MxToggleClass))

typedef struct _MxToggle MxToggle;
typedef struct _MxToggleClass MxToggleClass;
typedef struct _MxTogglePrivate MxTogglePrivate;

struct _MxToggle
{
  MxWidget parent;

  MxTogglePrivate *priv;
};

struct _MxToggleClass
{
  MxWidgetClass parent_class;
};

GType mx_toggle_get_type (void) G_GNUC_CONST;

ClutterActor *mx_toggle_new (void);

void      mx_toggle_set_active (MxToggle *toggle, gboolean active);
gboolean  mx_toggle_get_active (MxToggle *toggle);

G_END_DECLS

#endif /* _MX_TOGGLE_H */
