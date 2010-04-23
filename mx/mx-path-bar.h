/*
 * mx-path-bar.h: A path bar actor
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

#ifndef _MX_PATH_BAR_H
#define _MX_PATH_BAR_H

#include <glib-object.h>
#include <mx/mx-widget.h>
#include <mx/mx-entry.h>

G_BEGIN_DECLS

#define MX_TYPE_PATH_BAR mx_path_bar_get_type()

#define MX_PATH_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_PATH_BAR, MxPathBar))

#define MX_PATH_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_PATH_BAR, MxPathBarClass))

#define MX_IS_PATH_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_PATH_BAR))

#define MX_IS_PATH_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_PATH_BAR))

#define MX_PATH_BAR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_PATH_BAR, MxPathBarClass))

/**
 * MxPathBar:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
typedef struct _MxPathBar MxPathBar;
typedef struct _MxPathBarClass MxPathBarClass;
typedef struct _MxPathBarPrivate MxPathBarPrivate;

struct _MxPathBar
{
  MxWidget parent;

  MxPathBarPrivate *priv;
};

struct _MxPathBarClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_path_bar_get_type (void) G_GNUC_CONST;

ClutterActor *mx_path_bar_new (void);

gint         mx_path_bar_push         (MxPathBar *bar, const gchar *name);
gint         mx_path_bar_pop          (MxPathBar *bar);

gint         mx_path_bar_get_level    (MxPathBar *bar);
void         mx_path_bar_clear        (MxPathBar *bar);

gboolean     mx_path_bar_get_editable (MxPathBar *bar);
void         mx_path_bar_set_editable (MxPathBar *bar, gboolean editable);

gboolean     mx_path_bar_get_clear_on_change (MxPathBar *bar);
void         mx_path_bar_set_clear_on_change (MxPathBar *bar,
                                              gboolean clear_on_change);

const gchar *mx_path_bar_get_label    (MxPathBar *bar, gint level);
void         mx_path_bar_set_label    (MxPathBar *bar, gint level,
                                       const gchar *label);

const gchar *mx_path_bar_get_text     (MxPathBar *bar);
void         mx_path_bar_set_text     (MxPathBar *bar, const gchar *text);

MxEntry     *mx_path_bar_get_entry    (MxPathBar *bar);

G_END_DECLS

#endif /* _MX_PATH_BAR_H */
