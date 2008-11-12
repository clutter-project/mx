/* nbtk-button.h: Plain button actor
 *
 * Copyright (C) 2007 OpenedHand
 * Copyright (C) 2008 Intel Corporation
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 */

#ifndef __NBTK_BUTTON_H__
#define __NBTK_BUTTON_H__

G_BEGIN_DECLS

#include <nbtk-widget.h>

#define NBTK_TYPE_BUTTON                (nbtk_button_get_type ())
#define NBTK_BUTTON(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_BUTTON, NbtkButton))
#define NBTK_IS_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_BUTTON))
#define NBTK_BUTTON_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_BUTTON, NbtkButtonClass))
#define NBTK_IS_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_BUTTON))
#define NBTK_BUTTON_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_BUTTON, NbtkButtonClass))

typedef struct _NbtkButton              NbtkButton;
typedef struct _NbtkButtonPrivate       NbtkButtonPrivate;
typedef struct _NbtkButtonClass         NbtkButtonClass;

struct _NbtkButton
{
  NbtkWidget parent_instance;

  NbtkButtonPrivate *priv;
};

struct _NbtkButtonClass
{
  NbtkWidgetClass parent_class;

  /* vfuncs, not signals */
  void (* pressed)  (NbtkButton *button);
  void (* released) (NbtkButton *button);

  /* signals */
  void (* clicked) (NbtkButton *button);
};

GType nbtk_button_get_type (void) G_GNUC_CONST;

NbtkWidget *          nbtk_button_new            (void);
NbtkWidget *          nbtk_button_new_with_label (const gchar  *text);
G_CONST_RETURN gchar *nbtk_button_get_label      (NbtkButton   *button);
void                  nbtk_button_set_label      (NbtkButton   *button,
                                                  const gchar  *text);

G_END_DECLS

#endif /* __NBTK_BUTTON_H__ */
