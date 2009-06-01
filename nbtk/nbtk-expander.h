/*
 * nbtk-expander.h: Expander Widget
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
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */


#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

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

/**
 * NbtkExpander:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  NbtkBin parent;

  NbtkExpanderPrivate *priv;
} NbtkExpander;

typedef struct {
  NbtkBinClass parent_class;

  /* signals */
  void (* expand_complete) (NbtkExpander *expander);
  void (* contract_complete) (NbtkExpander *expander);

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
