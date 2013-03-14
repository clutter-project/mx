/*
 * mx-path-bar-button.c: A button actor for the path bar
 *
 * Copyright 2010,2013 Intel Corporation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-path-bar-button.h"
#include <math.h>

G_DEFINE_TYPE (MxPathBarButton, mx_path_bar_button, MX_TYPE_BUTTON)

static void
mx_path_bar_button_class_init (MxPathBarButtonClass *klass)
{
}

static void
mx_path_bar_button_init (MxPathBarButton *self)
{
  g_object_set (G_OBJECT (self), "clip-to-allocation", TRUE, NULL);
}

ClutterActor *
mx_path_bar_button_new (const gchar *label)
{
  return g_object_new (MX_TYPE_PATH_BAR_BUTTON, "label", label, NULL);
}
