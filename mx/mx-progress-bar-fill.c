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

#include "mx-progress-bar-fill.h"

static void
mx_progress_bar_fill_class_init (MxProgressBarFillClass *klass)
{
}

static void
mx_progress_bar_fill_init (MxProgressBarFill *self)
{
}

/*
 * Define the type by hand instead of using G_DEFINE_TYPE() so the symbol can
 * be kept private to the library.
 */
GType
_mx_progress_bar_fill_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
  {
    GType g_define_type_id =
      g_type_register_static_simple (MX_TYPE_WIDGET,
                                     g_intern_static_string ("MxProgressBarFill"),
                                     sizeof (MxProgressBarFillClass),
                                     (GClassInitFunc) mx_progress_bar_fill_class_init,
                                     sizeof (MxProgressBarFill),
                                     (GInstanceInitFunc) mx_progress_bar_fill_init,
                                     (GTypeFlags) 0);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

ClutterActor *
_mx_progress_bar_fill_new (void)
{
  return g_object_new (MX_TYPE_PROGRESS_BAR_FILL, NULL);
}
