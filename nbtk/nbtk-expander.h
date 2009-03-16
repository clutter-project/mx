/*
 * Copyright (C) 2009 Intel Corporation
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
 * Written by: Robert Staudinger <robsta@openedhand.com>
 */

#ifndef __NBTK_EXPANDER_H__
#define __NBTK_EXPANDER_H__

#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_EXPANDER                (nbtk_expander_get_type ())
#define NBTK_EXPANDER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_EXPANDER, NbtkExpander))
#define NBTK_IS_EXPANDER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_EXPANDER))
#define NBTK_EXPANDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_EXPANDER, NbtkExpanderClass))
#define NBTK_IS_EXPANDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_EXPANDER))
#define NBTK_EXPANDER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_EXPANDER, NbtkExpanderClass))

typedef struct _NbtkExpander              NbtkExpander;
typedef struct _NbtkExpanderPrivate       NbtkExpanderPrivate;
typedef struct _NbtkExpanderClass         NbtkExpanderClass;

struct _NbtkExpander
{
  /*< private >*/
  NbtkWidget parent_instance;

  NbtkExpanderPrivate *priv;
};

struct _NbtkExpanderClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_expander_get_type (void) G_GNUC_CONST;

NbtkWidget *    nbtk_expander_new       (const gchar *label);

gboolean        nbtk_expander_get_expanded  (NbtkExpander *self);
void            nbtk_expander_set_expanded  (NbtkExpander *self,
                                             gboolean      expanded);

ClutterActor *  nbtk_expander_get_child     (NbtkExpander *self);
const gchar *   nbtk_expander_get_label     (NbtkExpander *self);

G_END_DECLS

#endif /* __NBTK_EXPANDER_H__ */
