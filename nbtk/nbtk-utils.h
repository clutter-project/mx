/*
 * nbtk-utils.h: General utility functions used in Moblin
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
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Emmanuele Bassi <ebassi@linux.intel.com>
 *          Rob Bradford <rob@linux.intel.com>
 *          Neil Roberts <neil@linux.intel.com>
 *
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_UTILS_H__
#define __NBTK_UTILS_H__

#include <glib.h>

G_BEGIN_DECLS

gchar *nbtk_utils_format_time (GTimeVal *time_);

G_END_DECLS

#endif /* __NBTK_UTILS_H__ */
