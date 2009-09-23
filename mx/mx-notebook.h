/*
 * mx-notebook: notebook actor
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

#ifndef _MX_NOTEBOOK_H
#define _MX_NOTEBOOK_H

#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_NOTEBOOK mx_notebook_get_type()

#define MX_NOTEBOOK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_NOTEBOOK, MxNotebook))

#define MX_NOTEBOOK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_NOTEBOOK, MxNotebookClass))

#define MX_IS_NOTEBOOK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_NOTEBOOK))

#define MX_IS_NOTEBOOK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_NOTEBOOK))

#define MX_NOTEBOOK_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_NOTEBOOK, MxNotebookClass))

typedef struct _MxNotebook MxNotebook;
typedef struct _MxNotebookClass MxNotebookClass;
typedef struct _MxNotebookPrivate MxNotebookPrivate;

struct _MxNotebook
{
  MxWidget parent;

  MxNotebookPrivate *priv;
};

struct _MxNotebookClass
{
  MxWidgetClass parent_class;
};

GType mx_notebook_get_type (void) G_GNUC_CONST;

MxNotebook *mx_notebook_new (void);
void        mx_notebook_set_page (MxNotebook *notebook, gint page);
gint        mx_notebook_get_page (MxNotebook *notebook);

G_END_DECLS

#endif /* _MX_NOTEBOOK_H */
