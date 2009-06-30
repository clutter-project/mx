/*
 * nbtk-gtk-expander.h: GTK+ Expander widget with extra styling properties
 *
 * Copyright 2009 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

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


typedef struct _NbtkGtkExpander NbtkGtkExpander;
typedef struct _NbtkGtkExpanderClass NbtkGtkExpanderClass;
typedef struct _NbtkGtkExpanderPrivate NbtkGtkExpanderPrivate;

struct _NbtkGtkExpander
{
  GtkBin parent;

  NbtkGtkExpanderPrivate *priv;
};

struct _NbtkGtkExpanderClass
{
  GtkBinClass parent_class;
};

GType nbtk_gtk_expander_get_type (void);

GtkWidget* nbtk_gtk_expander_new (void);

void nbtk_gtk_expander_set_expanded (NbtkGtkExpander *expander, gboolean expanded);
gboolean nbtk_gtk_expander_get_expanded (NbtkGtkExpander *expander);

void nbtk_gtk_expander_set_label_widget (NbtkGtkExpander *expander, GtkWidget *label);
GtkWidget* nbtk_gtk_expander_get_label_widget (NbtkGtkExpander *expander);

void nbtk_gtk_expander_set_has_indicator (NbtkGtkExpander *expander, gboolean has_indicator);
gboolean nbtk_gtk_expander_get_has_indicator (NbtkGtkExpander *expander);
G_END_DECLS

#endif /* _NBTK_GTK_EXPANDER_H */
