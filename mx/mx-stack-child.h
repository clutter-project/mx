/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-stack-child.h: stack child actor
 *
 * Copyright 2010 Intel Corporation
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
 *             Chris Lord <chris@linux.intel.com>
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_STACK_CHILD_H
#define _MX_STACK_CHILD_H

#include <clutter/clutter.h>
#include "mx-enum-types.h"
#include "mx-stack.h"

G_BEGIN_DECLS

#define MX_TYPE_STACK_CHILD mx_stack_child_get_type()

#define MX_STACK_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_STACK_CHILD, MxStackChild))

#define MX_STACK_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_STACK_CHILD, MxStackChildClass))

#define MX_IS_STACK_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_STACK_CHILD))

#define MX_IS_STACK_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_STACK_CHILD))

#define MX_STACK_CHILD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_STACK_CHILD, MxStackChildClass))

typedef struct _MxStackChild MxStackChild;
typedef struct _MxStackChildClass MxStackChildClass;
typedef struct _MxStackChildPrivate MxStackChildPrivate;

/**
 * MxStackChild:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxStackChild
{
  /*< private >*/
  ClutterChildMeta parent;

  guint x_fill     : 1;
  guint y_fill     : 1;
  guint fit        : 1;
  guint crop       : 1;
  MxAlign x_align;
  MxAlign y_align;
};

struct _MxStackChildClass
{
  ClutterChildMetaClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_stack_child_get_type (void);

gboolean mx_stack_child_get_x_fill  (MxStack      *stack,
                                     ClutterActor *child);
void     mx_stack_child_set_x_fill  (MxStack      *stack,
                                     ClutterActor *child,
                                     gboolean      x_fill);

gboolean mx_stack_child_get_y_fill  (MxStack      *stack,
                                     ClutterActor *child);
void     mx_stack_child_set_y_fill  (MxStack      *stack,
                                     ClutterActor *child,
                                     gboolean      y_fill);

MxAlign  mx_stack_child_get_x_align (MxStack      *stack,
                                     ClutterActor *child);
void     mx_stack_child_set_x_align (MxStack      *stack,
                                     ClutterActor *child,
                                     MxAlign       x_align);

MxAlign  mx_stack_child_get_y_align (MxStack      *stack,
                                     ClutterActor *child);
void     mx_stack_child_set_y_align (MxStack      *stack,
                                     ClutterActor *child,
                                     MxAlign       y_align);

gboolean mx_stack_child_get_fit     (MxStack      *stack,
                                     ClutterActor *child);
void     mx_stack_child_set_fit     (MxStack      *stack,
                                     ClutterActor *child,
                                     gboolean      fit);

gboolean mx_stack_child_get_crop (MxStack      *stack,
                                  ClutterActor *child);
void     mx_stack_child_set_crop (MxStack      *stack,
                                  ClutterActor *child,
                                  gboolean      crop);


G_END_DECLS

#endif /* _MX_STACK_CHILD_H */
