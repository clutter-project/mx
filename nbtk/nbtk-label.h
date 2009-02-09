/* nbtk-label.h: Plain label actor
 *
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 */

#ifndef __NBTK_LABEL_H__
#define __NBTK_LABEL_H__

G_BEGIN_DECLS

#include <nbtk/nbtk-widget.h>

#define NBTK_TYPE_LABEL                (nbtk_label_get_type ())
#define NBTK_LABEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_LABEL, NbtkLabel))
#define NBTK_IS_LABEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_LABEL))
#define NBTK_LABEL_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_LABEL, NbtkLabelClass))
#define NBTK_IS_LABEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_LABEL))
#define NBTK_LABEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_LABEL, NbtkLabelClass))

typedef struct _NbtkLabel              NbtkLabel;
typedef struct _NbtkLabelPrivate       NbtkLabelPrivate;
typedef struct _NbtkLabelClass         NbtkLabelClass;

struct _NbtkLabel
{
  /*< private >*/
  NbtkWidget parent_instance;

  NbtkLabelPrivate *priv;
};

struct _NbtkLabelClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_label_get_type (void) G_GNUC_CONST;

NbtkWidget *          nbtk_label_new              (const gchar *text);
G_CONST_RETURN gchar *nbtk_label_get_text         (NbtkLabel   *label);
void                  nbtk_label_set_text         (NbtkLabel   *label,
                                                   const gchar *text);
ClutterActor *        nbtk_label_get_clutter_text (NbtkLabel   *label);

G_END_DECLS

#endif /* __NBTK_LABEL_H__ */
