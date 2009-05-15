/* nbtk-gtk-expander.h */

#ifndef _NBTK_GTK_EXPANDER_H
#define _NBTK_GTK_EXPANDER_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NBTK_TYPE_GTK_EXPANDER nbtk_gtk_expander_get_type()

#define NBTK_GTK_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_GTK_EXPANDER, NbtkGtkExpander))

#define NBTK_GTK_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_GTK_EXPANDER, NbtkGtkExpanderClass))

#define NBTK_IS_GTK_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_GTK_EXPANDER))

#define NBTK_IS_GTK_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_GTK_EXPANDER))

#define NBTK_GTK_EXPANDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_GTK_EXPANDER, NbtkGtkExpanderClass))

typedef struct {
  GtkExpander parent;
} NbtkGtkExpander;

typedef struct {
  GtkExpanderClass parent_class;
} NbtkGtkExpanderClass;

GType nbtk_gtk_expander_get_type (void);

GtkWidget* nbtk_gtk_expander_new (void);

G_END_DECLS

#endif /* _NBTK_GTK_EXPANDER_H */
