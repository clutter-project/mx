/* nbtk-icon.h */

#ifndef _NBTK_ICON
#define _NBTK_ICON

#include <glib-object.h>
#include "nbtk-widget.h"

G_BEGIN_DECLS

#define NBTK_TYPE_ICON nbtk_icon_get_type()

#define NBTK_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_ICON, NbtkIcon))

#define NBTK_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_ICON, NbtkIconClass))

#define NBTK_IS_ICON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_ICON))

#define NBTK_IS_ICON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_ICON))

#define NBTK_ICON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_ICON, NbtkIconClass))

typedef struct {
  NbtkWidget parent;
} NbtkIcon;

typedef struct {
  NbtkWidgetClass parent_class;
} NbtkIconClass;

GType nbtk_icon_get_type (void);

NbtkIcon* nbtk_icon_new (void);

G_END_DECLS

#endif /* _NBTK_ICON */

