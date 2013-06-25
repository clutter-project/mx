/*
 * mx-stack.h: A container that allows the stacking of multiple widgets
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_STACK_H
#define _MX_STACK_H

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_STACK mx_stack_get_type()

#define MX_STACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_STACK, MxStack))

#define MX_STACK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_STACK, MxStackClass))

#define MX_IS_STACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_STACK))

#define MX_IS_STACK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_STACK))

#define MX_STACK_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_STACK, MxStackClass))

typedef struct _MxStack MxStack;
typedef struct _MxStackClass MxStackClass;
typedef struct _MxStackPrivate MxStackPrivate;

/**
 * MxStack:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxStack
{
  /*< private >*/
  MxWidget parent;

  MxStackPrivate *priv;
};

struct _MxStackClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_stack_get_type (void) G_GNUC_CONST;

ClutterActor *mx_stack_new (void);

G_END_DECLS

#endif /* _MX_STACK_H */
