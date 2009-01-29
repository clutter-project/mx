#ifndef __NBTK_TYPES_H__
#define __NBTK_TYPES_H__

#include <glib-object.h>
#include <ccss/ccss.h>
#include <clutter/clutter-units.h>

G_BEGIN_DECLS

#define NBTK_TYPE_BORDER_IMAGE          (nbtk_border_image_get_type ())
#define NBTK_TYPE_PADDING               (nbtk_padding_get_type ())

typedef ccss_border_image_t             NbtkBorderImage;
typedef struct _NbtkPadding             NbtkPadding;
typedef enum   _NbtkTransitionType      NbtkTransitionType;

struct _NbtkPadding
{
  ClutterUnit top;
  ClutterUnit right;
  ClutterUnit bottom;
  ClutterUnit left;
};

GType nbtk_border_image_get_type (void) G_GNUC_CONST;
GType nbtk_padding_get_type (void) G_GNUC_CONST;

enum _NbtkTransitionType
{
  NBTK_TRANSITION_NONE,
  NBTK_TRANSITION_FADE,
  NBTK_TRANSITION_BOUNCE
};


G_END_DECLS

#endif /* __NBTK_TYPES_H__ */
