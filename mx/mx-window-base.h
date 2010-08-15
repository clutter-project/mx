/*
 * mx-window-base.h: A top-level window base class
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *             Chris Lord <chris@linux.intel.com>
 *
 */

#ifndef _MX_WINDOW_BASE_H
#define _MX_WINDOW_BASE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_WINDOW_BASE mx_window_base_get_type()

#define MX_WINDOW_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_WINDOW_BASE, MxWindowBase))

#define MX_WINDOW_BASE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_WINDOW_BASE, MxWindowBaseClass))

#define MX_IS_WINDOW_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_WINDOW_BASE))

#define MX_IS_WINDOW_BASE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_WINDOW_BASE))

#define MX_WINDOW_BASE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_WINDOW_BASE, MxWindowBaseClass))

typedef struct _MxWindowBase MxWindowBase;
typedef struct _MxWindowBaseClass MxWindowBaseClass;

struct _MxWindowBase
{
  GObject parent;
};

struct _MxWindowBaseClass
{
  GObjectClass parent_class;
};

GType mx_window_base_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _MX_WINDOW_BASE_H */
