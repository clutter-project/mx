/*
 * nbtk-progress-bar.h: Progress bar widget
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef _NBTK_PROGRESS_BAR_H
#define _NBTK_PROGRESS_BAR_H

#include <glib-object.h>
#include <nbtk/nbtk-widget.h>

G_BEGIN_DECLS

#define NBTK_TYPE_PROGRESS_BAR nbtk_progress_bar_get_type()

#define NBTK_PROGRESS_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_PROGRESS_BAR, NbtkProgressBar))

#define NBTK_PROGRESS_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_PROGRESS_BAR, NbtkProgressBarClass))

#define NBTK_IS_PROGRESS_BAR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_PROGRESS_BAR))

#define NBTK_IS_PROGRESS_BAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_PROGRESS_BAR))

#define NBTK_PROGRESS_BAR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_PROGRESS_BAR, NbtkProgressBarClass))

typedef struct _NbtkProgressBar NbtkProgressBar;
typedef struct _NbtkProgressBarClass NbtkProgressBarClass;
typedef struct _NbtkProgressBarPrivate NbtkProgressBarPrivate;

/**
 * NbtkProgressBar:
 *
 * The contents of this structure are private and should only be
 * accessed through the public API.
 */
struct _NbtkProgressBar
{
  /*< private >*/
  NbtkWidget parent;

  NbtkProgressBarPrivate *priv;
};

struct _NbtkProgressBarClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_progress_bar_get_type (void);

NbtkWidget *nbtk_progress_bar_new (void);

void    nbtk_progress_bar_set_progress (NbtkProgressBar *bar,
                                        gdouble          progress);
gdouble nbtk_progress_bar_get_progress (NbtkProgressBar *bar);

G_END_DECLS

#endif /* _NBTK_PROGRESS_BAR_H */
