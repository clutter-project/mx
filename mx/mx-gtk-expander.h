/*
 * mx-gtk-expander.h: GTK+ Expander widget with extra styling properties
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

#ifndef _MX_GTK_EXPANDER_H
#define _MX_GTK_EXPANDER_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MX_TYPE_GTK_EXPANDER mx_gtk_expander_get_type()

#define MX_GTK_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_GTK_EXPANDER, MxGtkExpander))

#define MX_GTK_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_GTK_EXPANDER, MxGtkExpanderClass))

#define MX_IS_GTK_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_GTK_EXPANDER))

#define MX_IS_GTK_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_GTK_EXPANDER))

#define MX_GTK_EXPANDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_GTK_EXPANDER, MxGtkExpanderClass))


typedef struct _MxGtkExpander MxGtkExpander;
typedef struct _MxGtkExpanderClass MxGtkExpanderClass;
typedef struct _MxGtkExpanderPrivate MxGtkExpanderPrivate;

/**
 * MxGtkExpander:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxGtkExpander
{
  /*< private >*/
  GtkBin parent;

  MxGtkExpanderPrivate *priv;
};

struct _MxGtkExpanderClass
{
  GtkBinClass parent_class;
};

GType mx_gtk_expander_get_type (void);

GtkWidget* mx_gtk_expander_new (void);

void       mx_gtk_expander_set_expanded      (MxGtkExpander *expander,
                                              gboolean       expanded);
gboolean   mx_gtk_expander_get_expanded      (MxGtkExpander *expander);

void       mx_gtk_expander_set_label_widget  (MxGtkExpander *expander,
                                              GtkWidget     *label);
GtkWidget* mx_gtk_expander_get_label_widget  (MxGtkExpander *expander);

void       mx_gtk_expander_set_has_indicator (MxGtkExpander *expander,
                                              gboolean       has_indicator);
gboolean   mx_gtk_expander_get_has_indicator (MxGtkExpander *expander);

G_END_DECLS

#endif /* _MX_GTK_EXPANDER_H */
