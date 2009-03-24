#ifndef __NBTK_TYPES_H__
#define __NBTK_TYPES_H__

#include <glib-object.h>
#include <ccss/ccss.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_BORDER_IMAGE          (nbtk_border_image_get_type ())
#define NBTK_TYPE_PADDING               (nbtk_padding_get_type ())

typedef ccss_border_image_t             NbtkBorderImage;
typedef struct _NbtkPadding             NbtkPadding;

GType nbtk_border_image_get_type (void) G_GNUC_CONST;

/**
 * NbtkPadding:
 * @top: padding from the top
 * @right: padding from the right
 * @bottom: padding from the bottom
 * @left: padding from the left
 *
 * The padding from the internal border of the parent container.
 */
struct _NbtkPadding
{
  ClutterUnit top;
  ClutterUnit right;
  ClutterUnit bottom;
  ClutterUnit left;
};

GType nbtk_padding_get_type (void) G_GNUC_CONST;

/**
 * NbtkAlignment:
 * @NBTK_ALIGN_TOP: align to the top (vertically)
 * @NBTK_ALIGN_RIGHT: align to the right (horizontally)
 * @NBTK_ALIGN_BOTTOM: align to the bottom (vertically)
 * @NBTK_ALIGN_LEFT: align to the left (horizontally)
 * @NBTK_ALIGN_CENTER: align to the center (horizontally or vertically)
 *
 * The alignment values for a #NbtkBin.
 */
typedef enum {
  NBTK_ALIGN_TOP,
  NBTK_ALIGN_RIGHT,
  NBTK_ALIGN_BOTTOM,
  NBTK_ALIGN_LEFT,
  NBTK_ALIGN_CENTER
} NbtkAlignment;

G_END_DECLS

#endif /* __NBTK_TYPES_H__ */
