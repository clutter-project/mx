#ifndef __NBTK_STYLE_H__
#define __NBTK_STYLE_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_STYLE                 (nbtk_style_get_type ())
#define NBTK_STYLE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_STYLE, NbtkStyle))
#define NBTK_IS_STYLE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_STYLE))
#define NBTK_STYLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_STYLE, NbtkStyleClass))
#define NBTK_IS_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_STYLE))
#define NBTK_STYLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_STYLE, NbtkStyleClass))

typedef struct _NbtkStyle               NbtkStyle;
typedef struct _NbtkStylePrivate        NbtkStylePrivate;
typedef struct _NbtkStyleClass          NbtkStyleClass;

/* forward declaration */
typedef struct _NbtkStylable            NbtkStylable; /* dummy typedef */
typedef struct _NbtkStylableIface       NbtkStylableIface;

typedef enum { /*< prefix=NBTK_STYLE_ERROR >*/
  NBTK_STYLE_ERROR_INVALID_FILE
} NbtkStyleError;

struct _NbtkStyle
{
  /*< private >*/
  GObject parent_instance;

  NbtkStylePrivate *priv;
};

struct _NbtkStyleClass
{
  GObjectClass parent_class;

  void (* changed) (NbtkStyle *style);
};

GType            nbtk_style_get_type     (void) G_GNUC_CONST;

NbtkStyle *      nbtk_style_get_default  (void);
NbtkStyle *      nbtk_style_new          (void);

gboolean         nbtk_style_load_from_file (NbtkStyle     *style,
                                            const gchar   *filename,
                                            GError       **error);
void             nbtk_style_get_property   (NbtkStyle     *style,
                                            NbtkStylable  *stylable,
                                            GParamSpec    *pspec,
                                            GValue        *value);
void             nbtk_style_set_property   (NbtkStyle     *style,
                                            const gchar   *property_name,
                                            const GValue  *value);

G_END_DECLS

#endif /* __NBTK_STYLE_H__ */
