/*
 * mx-progress-bar-fill.c: Fill used in progress bar/slider widgets
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
 */

/*
 * This class is private to MX
 */

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#ifndef _MX_PROGRESS_BAR_FILL_H
#define _MX_PROGRESS_BAR_FILL_H

#define MX_TYPE_PROGRESS_BAR_FILL _mx_progress_bar_fill_get_type()

#define MX_PROGRESS_BAR_FILL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                               MX_TYPE_PROGRESS_BAR_FILL, MxProgressBarFill))

#define MX_PROGRESS_BAR_FILL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
                            MX_TYPE_PROGRESS_BAR_FILL, MxProgressBarFillClass))

#define MX_IS_PROGRESS_BAR_FILL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                               MX_TYPE_PROGRESS_BAR_FILL))

#define MX_IS_PROGRESS_BAR_FILL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                            MX_TYPE_PROGRESS_BAR_FILL))

#define MX_PROGRESS_BAR_FILL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                              MX_TYPE_PROGRESS_BAR_FILL, MxProgressBarFillClass))

typedef struct
{
  MxWidget parent;

  guint    height;
} MxProgressBarFill;

typedef struct
{
  MxWidgetClass parent_class;
} MxProgressBarFillClass;

GType             _mx_progress_bar_fill_get_type      (void) G_GNUC_CONST;

ClutterActor *    _mx_progress_bar_fill_new           (void);

G_END_DECLS

#endif /* _MX_PROGRESS_BAR_FILL_H */
