/*
 * mx-box-layout.h: box layout actor
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_BOX_LAYOUT_H
#define _MX_BOX_LAYOUT_H

#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_BOX_LAYOUT mx_box_layout_get_type()

#define MX_BOX_LAYOUT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_BOX_LAYOUT, MxBoxLayout))

#define MX_BOX_LAYOUT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_BOX_LAYOUT, MxBoxLayoutClass))

#define MX_IS_BOX_LAYOUT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_BOX_LAYOUT))

#define MX_IS_BOX_LAYOUT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_BOX_LAYOUT))

#define MX_BOX_LAYOUT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_BOX_LAYOUT, MxBoxLayoutClass))

typedef struct _MxBoxLayout MxBoxLayout;
typedef struct _MxBoxLayoutClass MxBoxLayoutClass;
typedef struct _MxBoxLayoutPrivate MxBoxLayoutPrivate;

/**
 * MxBoxLayout:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxBoxLayout
{
  /*< private >*/
  MxWidget parent;

  MxBoxLayoutPrivate *priv;
};

struct _MxBoxLayoutClass
{
  MxWidgetClass parent_class;
};

GType mx_box_layout_get_type (void);

ClutterActor *mx_box_layout_new (void);

void          mx_box_layout_set_vertical   (MxBoxLayout *box,
                                            gboolean     vertical);
gboolean      mx_box_layout_get_vertical   (MxBoxLayout *box);

void          mx_box_layout_set_pack_start (MxBoxLayout *box,
                                            gboolean     pack_start);
gboolean      mx_box_layout_get_pack_start (MxBoxLayout *box);

void          mx_box_layout_set_spacing    (MxBoxLayout *box,
                                            guint        spacing);
guint         mx_box_layout_get_spacing    (MxBoxLayout *box);

G_END_DECLS

#endif /* _MX_BOX_LAYOUT_H */
