/* nbtk-combo-box.h */

#ifndef _NBTK_COMBO_BOX_H
#define _NBTK_COMBO_BOX_H

#include <glib-object.h>
#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_COMBO_BOX nbtk_combo_box_get_type()

#define NBTK_COMBO_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_COMBO_BOX, NbtkComboBox))

#define NBTK_COMBO_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_COMBO_BOX, NbtkComboBoxClass))

#define NBTK_IS_COMBO_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_COMBO_BOX))

#define NBTK_IS_COMBO_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_COMBO_BOX))

#define NBTK_COMBO_BOX_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_COMBO_BOX, NbtkComboBoxClass))

typedef struct _NbtkComboBox NbtkComboBox;
typedef struct _NbtkComboBoxClass NbtkComboBoxClass;
typedef struct _NbtkComboBoxPrivate NbtkComboBoxPrivate;

struct _NbtkComboBox
{
  NbtkWidget parent;

  NbtkComboBoxPrivate *priv;
};

struct _NbtkComboBoxClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_combo_box_get_type (void);

NbtkComboBox *nbtk_combo_box_new (void);


void nbtk_combo_box_insert_text (NbtkComboBox *box, gint position, const gchar *text);
void nbtk_combo_box_append_text (NbtkComboBox *box, const gchar *text);
void nbtk_combo_box_prepend_text (NbtkComboBox *box, const gchar *text);
void nbtk_combo_box_remove_text (NbtkComboBox *box, gint position);

void nbtk_combo_box_set_title (NbtkComboBox *box, const gchar *title);
const gchar* nbtk_combo_box_get_title (NbtkComboBox *box);

G_END_DECLS

#endif /* _NBTK_COMBO_BOX_H */
