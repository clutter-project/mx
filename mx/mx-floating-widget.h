/*
 * mx-floating-widget: always-on-top base actor
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

#ifndef _MX_FLOATING_WIDGET_H
#define _MX_FLOATING_WIDGET_H

#include "mx-widget.h"

G_BEGIN_DECLS

#define MX_TYPE_FLOATING_WIDGET mx_floating_widget_get_type()

#define MX_FLOATING_WIDGET(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_FLOATING_WIDGET, MxFloatingWidget))

#define MX_FLOATING_WIDGET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_FLOATING_WIDGET, MxFloatingWidgetClass))

#define MX_IS_FLOATING_WIDGET(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_FLOATING_WIDGET))

#define MX_IS_FLOATING_WIDGET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_FLOATING_WIDGET))

#define MX_FLOATING_WIDGET_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_FLOATING_WIDGET, MxFloatingWidgetClass))

typedef struct _MxFloatingWidget MxFloatingWidget;
typedef struct _MxFloatingWidgetClass MxFloatingWidgetClass;
typedef struct _MxFloatingWidgetPrivate MxFloatingWidgetPrivate;

struct _MxFloatingWidget
{
  MxWidget parent;

  MxFloatingWidgetPrivate *priv;
};

struct _MxFloatingWidgetClass
{
  MxWidgetClass parent_class;

  void (*floating_pick) (ClutterActor *actor, const ClutterColor *color);
  void (*floating_paint) (ClutterActor *actor);
};

GType mx_floating_widget_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _MX_FLOATING_WIDGET_H */
