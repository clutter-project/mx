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
 */

/**
 * SECTION:nbtk-types
 * @short_description: type definitions used throughout Nbtk
 *
 * Common types for NbtkWidgets.
 */


#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_TYPES_H__
#define __NBTK_TYPES_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_BORDER_IMAGE          (nbtk_border_image_get_type ())
#define NBTK_TYPE_PADDING               (nbtk_padding_get_type ())

typedef struct _NbtkPadding             NbtkPadding;

GType nbtk_border_image_get_type (void) G_GNUC_CONST;

/**
 * NbtkPadding:
 * @top: padding from the top
 * @right: padding from the right
 * @bottom: padding from the bottom
 * @left: padding from the left
 *
 * The padding from the internal border of the parent container.
 */
struct _NbtkPadding
{
  gfloat top;
  gfloat right;
  gfloat bottom;
  gfloat left;
};

GType nbtk_padding_get_type (void) G_GNUC_CONST;

typedef enum {
  NBTK_ALIGN_START,
  NBTK_ALIGN_MIDDLE,
  NBTK_ALIGN_END
} NbtkAlign;

G_END_DECLS

#endif /* __NBTK_TYPES_H__ */
