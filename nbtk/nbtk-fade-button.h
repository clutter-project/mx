/* nbtk-fade-button.h */

#ifndef _NBTK_FADE_BUTTON
#define _NBTK_FADE_BUTTON

#include <glib-object.h>
#include <nbtk/nbtk.h>

G_BEGIN_DECLS

#define NBTK_TYPE_FADE_BUTTON nbtk_fade_button_get_type()

#define NBTK_FADE_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_FADE_BUTTON, NbtkFadeButton))

#define NBTK_FADE_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_FADE_BUTTON, NbtkFadeButtonClass))

#define NBTK_IS_FADE_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_FADE_BUTTON))

#define NBTK_IS_FADE_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_FADE_BUTTON))

#define NBTK_FADE_BUTTON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_FADE_BUTTON, NbtkFadeButtonClass))

typedef struct {
  NbtkButton parent;
} NbtkFadeButton;

typedef struct {
  NbtkButtonClass parent_class;
} NbtkFadeButtonClass;

GType nbtk_fade_button_get_type (void);

NbtkFadeButton* nbtk_fade_button_new (void);
NbtkFadeButton* nbtk_fade_button_new_with_label (const gchar *text);

G_END_DECLS

#endif /* _NBTK_FADE_BUTTON */


