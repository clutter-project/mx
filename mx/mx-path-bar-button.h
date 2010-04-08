/*
 * mx-path-bar-button.h: A button actor for the path bar
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#ifndef _MX_PATH_BAR_BUTTON_H
#define _MX_PATH_BAR_BUTTON_H

#include <glib-object.h>
#include <mx/mx-button.h>

G_BEGIN_DECLS

#define MX_TYPE_PATH_BAR_BUTTON mx_path_bar_button_get_type()

#define MX_PATH_BAR_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_PATH_BAR_BUTTON, MxPathBarButton))

#define MX_PATH_BAR_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_PATH_BAR_BUTTON, MxPathBarButtonClass))

#define MX_IS_PATH_BAR_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_PATH_BAR_BUTTON))

#define MX_IS_PATH_BAR_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_PATH_BAR_BUTTON))

#define MX_PATH_BAR_BUTTON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_PATH_BAR_BUTTON, MxPathBarButtonClass))

typedef struct _MxPathBarButton MxPathBarButton;
typedef struct _MxPathBarButtonClass MxPathBarButtonClass;
typedef struct _MxPathBarButtonPrivate MxPathBarButtonPrivate;

struct _MxPathBarButton
{
  MxButton parent;

  MxPathBarButtonPrivate *priv;
};

struct _MxPathBarButtonClass
{
  MxButtonClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_path_bar_button_get_type (void) G_GNUC_CONST;

ClutterActor *mx_path_bar_button_new (const gchar *label);

G_END_DECLS

#endif /* _MX_PATH_BAR_BUTTON_H */
