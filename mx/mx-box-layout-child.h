/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-box-layout-child.h: box layout child actor
 *
 * Copyright 2009 Intel Corporation
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
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_BOX_LAYOUT_CHILD_H
#define _MX_BOX_LAYOUT_CHILD_H

#include <clutter/clutter.h>
#include "mx-enum-types.h"
#include "mx-box-layout.h"

G_BEGIN_DECLS

#define MX_TYPE_BOX_LAYOUT_CHILD mx_box_layout_child_get_type()

#define MX_BOX_LAYOUT_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_BOX_LAYOUT_CHILD, MxBoxLayoutChild))

#define MX_BOX_LAYOUT_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_BOX_LAYOUT_CHILD, MxBoxLayoutChildClass))

#define MX_IS_BOX_LAYOUT_CHILD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_BOX_LAYOUT_CHILD))

#define MX_IS_BOX_LAYOUT_CHILD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_BOX_LAYOUT_CHILD))

#define MX_BOX_LAYOUT_CHILD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_BOX_LAYOUT_CHILD, MxBoxLayoutChildClass))

typedef struct _MxBoxLayoutChild MxBoxLayoutChild;
typedef struct _MxBoxLayoutChildClass MxBoxLayoutChildClass;
typedef struct _MxBoxLayoutChildPrivate MxBoxLayoutChildPrivate;

/**
 * MxBoxLayoutChild:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxBoxLayoutChild
{
  /*< private >*/
  ClutterChildMeta parent;

  gboolean expand;
  guint x_fill : 1;
  guint y_fill : 1;
  MxAlign x_align;
  MxAlign y_align;
};

struct _MxBoxLayoutChildClass
{
  ClutterChildMetaClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_box_layout_child_get_type (void);

gboolean mx_box_layout_child_get_expand  (MxBoxLayout  *box_layout,
                                          ClutterActor *child);
void     mx_box_layout_child_set_expand  (MxBoxLayout  *box_layout,
                                          ClutterActor *child,
                                          gboolean      expand);

gboolean mx_box_layout_child_get_x_fill  (MxBoxLayout  *box_layout,
                                          ClutterActor *child);
void     mx_box_layout_child_set_x_fill  (MxBoxLayout  *box_layout,
                                          ClutterActor *child,
                                          gboolean      x_fill);

gboolean mx_box_layout_child_get_y_fill  (MxBoxLayout  *box_layout,
                                          ClutterActor *child);
void     mx_box_layout_child_set_y_fill  (MxBoxLayout  *box_layout,
                                          ClutterActor *child,
                                          gboolean      y_fill);

MxAlign  mx_box_layout_child_get_x_align (MxBoxLayout  *box_layout,
                                          ClutterActor *child);
void     mx_box_layout_child_set_x_align (MxBoxLayout  *box_layout,
                                          ClutterActor *child,
                                          MxAlign       x_align);

MxAlign  mx_box_layout_child_get_y_align (MxBoxLayout  *box_layout,
                                          ClutterActor *child);
void     mx_box_layout_child_set_y_align (MxBoxLayout  *box_layout,
                                          ClutterActor *child,
                                          MxAlign       y_align);

G_END_DECLS

#endif /* _MX_BOX_LAYOUT_CHILD_H */
