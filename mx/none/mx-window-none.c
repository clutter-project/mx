/*
 * mx-window-none.c: A top-level window (none backend)
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
 */

#include "mx-window.h"
#include "mx-marshal.h"

G_DEFINE_TYPE (MxWindow, mx_window, MX_TYPE_WINDOW_BASE)

enum
{
  DESTROY,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
mx_window_class_init (MxWindowClass *klass)
{
  signals[DESTROY] = g_signal_new ("destroy",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_LAST,
                                   G_STRUCT_OFFSET (MxWindowClass, destroy),
                                   NULL, NULL,
                                   _mx_marshal_VOID__VOID,
                                   G_TYPE_NONE, 0);
}

static void
mx_window_init (MxWindow *self)
{
}

