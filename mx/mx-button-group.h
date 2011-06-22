/*
 * mx-button-group.h: A group handler for buttons
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_BUTTON_GROUP_H
#define _MX_BUTTON_GROUP_H

#include <glib-object.h>

#include "mx-button.h"

G_BEGIN_DECLS

#define MX_TYPE_BUTTON_GROUP mx_button_group_get_type()

#define MX_BUTTON_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_BUTTON_GROUP, MxButtonGroup))

#define MX_BUTTON_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_BUTTON_GROUP, MxButtonGroupClass))

#define MX_IS_BUTTON_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_BUTTON_GROUP))

#define MX_IS_BUTTON_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_BUTTON_GROUP))

#define MX_BUTTON_GROUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_BUTTON_GROUP, MxButtonGroupClass))

typedef struct _MxButtonGroup MxButtonGroup;
typedef struct _MxButtonGroupClass MxButtonGroupClass;
typedef struct _MxButtonGroupPrivate MxButtonGroupPrivate;

/**
 * MxButtonGroup:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxButtonGroup
{
  GInitiallyUnowned parent;

  MxButtonGroupPrivate *priv;
};

struct _MxButtonGroupClass
{
  GInitiallyUnownedClass parent_class;

  /*< private >*/
  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_button_group_get_type (void) G_GNUC_CONST;

MxButtonGroup *mx_button_group_new (void);

void mx_button_group_add     (MxButtonGroup   *group,
                              MxButton        *button);
void mx_button_group_remove  (MxButtonGroup   *group,
                              MxButton        *button);
void mx_button_group_foreach (MxButtonGroup   *group,
                              ClutterCallback  callback,
                              gpointer         userdata);

void      mx_button_group_set_active_button   (MxButtonGroup *group,
                                               MxButton      *button);
MxButton* mx_button_group_get_active_button   (MxButtonGroup *group);
void      mx_button_group_set_allow_no_active (MxButtonGroup *group,
                                               gboolean       allow_no_active);
gboolean  mx_button_group_get_allow_no_active (MxButtonGroup *group);

const GSList *mx_button_group_get_buttons (MxButtonGroup *group);

G_END_DECLS

#endif /* _MX_BUTTON_GROUP_H */
