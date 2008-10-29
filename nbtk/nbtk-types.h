#ifndef __NBTK_TYPES_H__
#define __NBTK_TYPES_H__

#include <glib-object.h>
#include <clutter/clutter-units.h>

G_BEGIN_DECLS

#define NBTK_TYPE_PADDING               (nbtk_padding_get_type ())

typedef struct _NbtkPadding             NbtkPadding;

struct _NbtkPadding
{
  ClutterUnit top;
  ClutterUnit right;
  ClutterUnit bottom;
  ClutterUnit left;
};

GType nbtk_padding_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __NBTK_TYPES_H__ */
