/* nbtk-expander.h */

#ifndef _NBTK_EXPANDER
#define _NBTK_EXPANDER

#include <glib-object.h>
#include <nbtk/nbtk-bin.h>

G_BEGIN_DECLS

#define NBTK_TYPE_EXPANDER nbtk_expander_get_type()

#define NBTK_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_EXPANDER, NbtkExpander))

#define NBTK_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_EXPANDER, NbtkExpanderClass))

#define NBTK_IS_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_EXPANDER))

#define NBTK_IS_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_EXPANDER))

#define NBTK_EXPANDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_EXPANDER, NbtkExpanderClass))

typedef struct _NbtkExpanderPrivate NbtkExpanderPrivate;

typedef struct {
  NbtkBin parent;
  NbtkExpanderPrivate *priv;
} NbtkExpander;

typedef struct {
  NbtkBinClass parent_class;
} NbtkExpanderClass;

GType nbtk_expander_get_type (void);

NbtkWidget* nbtk_expander_new (void);
void nbtk_expander_set_label (NbtkExpander *expander,
                              const gchar *label);

gboolean nbtk_expander_get_expanded (NbtkExpander *expander);
void nbtk_expander_set_expanded (NbtkExpander *expander,
                                 gboolean      expanded);
G_END_DECLS

#endif /* _NBTK_EXPANDER */
