#ifndef _NBTK_GTK_FRAME
#define _NBTK_GTK_FRAME

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NBTK_GTK_TYPE_FRAME nbtk_gtk_frame_get_type ()

#define NBTK_GTK_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_GTK_TYPE_FRAME, NbtkGtkFrame))

#define NBTK_GTK_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_GTK_TYPE_FRAME, NbtkGtkFrameClass))

#define NBTK_GTK_IS_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_GTK_TYPE_FRAME))

#define NBTK_GTK_IS_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_GTK_TYPE_FRAME))

#define NBTK_GTK_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_GTK_TYPE_FRAME, NbtkGtkFrameClass))

typedef struct {
  GtkFrame parent;

  GtkAllocation bullet_allocation;

  GdkColor bullet_color;
  GdkColor border_color;
} NbtkGtkFrame;

typedef struct {
  GtkFrameClass parent_class;
} NbtkGtkFrameClass;

GType nbtk_gtk_frame_get_type (void);

GtkWidget* nbtk_gtk_frame_new (void);

G_END_DECLS

#endif
