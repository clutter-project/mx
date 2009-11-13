/*
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
 * Author: Thomas Wood <thos@gnome.org>
 *
 */


#ifndef MX_CSS_H
#define MX_CSS_H

#include <glib.h>
#include "mx-stylable.h"

typedef struct _MxNode MxNode;
typedef struct _MxStyleSheetValue MxStyleSheetValue;
typedef struct _MxStyleSheet MxStyleSheet;

struct _MxNode
{
  gchar *type;
  gchar *id;
  gchar *class;
  gchar *pseudo_class;
  MxNode *parent;
};

struct _MxStyleSheetValue
{
  const gchar *string;
  const gchar *source;
};

MxStyleSheet*  mx_style_sheet_new            ();
void           mx_style_sheet_destroy        ();
gboolean       mx_style_sheet_add_from_file  (MxStyleSheet *sheet,
                                              const gchar  *filename,
                                              GError       **error);
GHashTable*    mx_style_sheet_get_properties (MxStyleSheet *sheet,
                                              MxStylable   *node);

#endif /* MX_CSS_H */
