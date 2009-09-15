/*
 * nbtk-action.h: NbtkAction object
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
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef _NBTK_ACTION_H
#define _NBTK_ACTION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define NBTK_TYPE_ACTION nbtk_action_get_type()

#define NBTK_ACTION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_ACTION, NbtkAction))

#define NBTK_ACTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_ACTION, NbtkActionClass))

#define NBTK_IS_ACTION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_ACTION))

#define NBTK_IS_ACTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_ACTION))

#define NBTK_ACTION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_ACTION, NbtkActionClass))

typedef struct _NbtkAction NbtkAction;
typedef struct _NbtkActionClass NbtkActionClass;
typedef struct _NbtkActionPrivate NbtkActionPrivate;

/**
 * NbtkAction:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _NbtkAction
{
  /*< private >*/
  GInitiallyUnowned parent;

  NbtkActionPrivate *priv;
};

struct _NbtkActionClass
{
  GInitiallyUnownedClass parent_class;

  void (*activated) (NbtkAction *action);
};

GType nbtk_action_get_type (void);

NbtkAction *nbtk_action_new (void);

NbtkAction *nbtk_action_new_full (const gchar *name,
                                  GCallback    activated_cb,
                                  gpointer     user_data);

const gchar *nbtk_action_get_name (NbtkAction *action);
void nbtk_action_set_name (NbtkAction  *action,
                           const gchar *name);

gboolean nbtk_action_get_active (NbtkAction *action);
void nbtk_action_set_active (NbtkAction *action,
                             gboolean    active);

G_END_DECLS

#endif /* _NBTK_ACTION_H */
