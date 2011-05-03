/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2009, 2010 Intel Corporation.
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
 * SECTION:mx-types
 * @short_description: type definitions used throughout Mx
 *
 * Common types for MxWidgets.
 */


#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_TYPES_H__
#define __MX_TYPES_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_BORDER_IMAGE          (mx_border_image_get_type ())
#define MX_TYPE_PADDING               (mx_padding_get_type ())

#define MX_PARAM_TRANSLATEABLE 1 << 8

typedef struct _MxBorderImage MxBorderImage;
typedef struct _MxPadding     MxPadding;


GType mx_border_image_get_type (void) G_GNUC_CONST;

/**
 * MxBorderImage:
 * @uri: uri of a supported image file
 * @top: top border slice width
 * @right: right border slice width
 * @bottom: bottom border slice width
 * @left: bottom border slice width
 */
struct _MxBorderImage
{
  gchar *uri;

  gint top;
  gint right;
  gint bottom;
  gint left;
};

void mx_border_image_set_from_string (GValue *value,
                                      const gchar *str,
                                      const gchar *filename);

void mx_font_weight_set_from_string (GValue *value, const gchar *str);

/**
 * MxPadding:
 * @top: padding from the top
 * @right: padding from the right
 * @bottom: padding from the bottom
 * @left: padding from the left
 *
 * The padding from the internal border of the parent container.
 */
struct _MxPadding
{
  gfloat top;
  gfloat right;
  gfloat bottom;
  gfloat left;
};

GType mx_padding_get_type (void) G_GNUC_CONST;


/**
 * MxAlign:
 * @MX_ALIGN_START: Align at the beginning of the axis
 * @MX_ALIGN_MIDDLE: Align in the middle of the axis
 * @MX_ALIGN_END: Align at the end of the axis
 *
 * Set the alignment of the item
 */
typedef enum { /*< prefix=MX_ALIGN >*/
  MX_ALIGN_START,
  MX_ALIGN_MIDDLE,
  MX_ALIGN_END
} MxAlign;

/**
 * MxFontWeight:
 * @MX_FONT_WEIGHT_NORMAL: Normal font weight
 * @MX_FONT_WEIGHT_BOLD: Bold font weight
 * @MX_FONT_WEIGHT_BOLDER: Bolder font weight
 * @MX_FONT_WEIGHT_LIGHTER: Lighter font weight
 *
 * Support values of font weight
 */
typedef enum /*< prefix=MX_FONT_WEIGHT >*/
{
  MX_FONT_WEIGHT_NORMAL,
  MX_FONT_WEIGHT_BOLD,
  MX_FONT_WEIGHT_BOLDER,
  MX_FONT_WEIGHT_LIGHTER
} MxFontWeight;

/**
 * MxScrollPolicy:
 * @MX_SCROLL_POLICY_NONE: Never scroll
 * @MX_SCROLL_POLICY_HORIZONTAL: Only allow horizontal scrolling
 * @MX_SCROLL_POLICY_VERTICAL: Only allow vertical scrolling
 * @MX_SCROLL_POLICY_BOTH: Allow scrolling both horizontally and vertically
 *
 * Defines the scrolling policy of scrollable widgets.
 */
typedef enum /*< prefix=MX_SCROLL_POLICY >*/
{
  MX_SCROLL_POLICY_NONE,
  MX_SCROLL_POLICY_HORIZONTAL,
  MX_SCROLL_POLICY_VERTICAL,
  MX_SCROLL_POLICY_BOTH
} MxScrollPolicy;

/**
 * MxOrientation:
 * @MX_ORIENTATION_HORIZONTAL: horizontal orientation
 * @MX_ORIENTATION_VERTICAL: vertical orientation
 *
 * Defines the orientation of various layout widgets.
 */
typedef enum /*< prefix=MX_ORIENTATION >*/
{
  MX_ORIENTATION_HORIZONTAL,
  MX_ORIENTATION_VERTICAL
} MxOrientation;

/**
 * MxWindowRotation:
 * @MX_WINDOW_ROTATION_0: Zero degrees of rotation
 * @MX_WINDOW_ROTATION_90: 90 degrees of rotation
 * @MX_WINDOW_ROTATION_180: 180 degrees of rotation
 * @MX_WINDOW_ROTATION_270: 270 degrees of rotation
 *
 * Defines the clock-wise rotation angle of a window.
 *
 * Since: 1.2
 */
typedef enum /*< prefix=MX_WINDOW_ROTATION >*/
{
  MX_WINDOW_ROTATION_0,
  MX_WINDOW_ROTATION_90,
  MX_WINDOW_ROTATION_180,
  MX_WINDOW_ROTATION_270
} MxWindowRotation;

/**
 * MxPosition:
 * @MX_POSITION_TOP: The top position
 * @MX_POSITION_RIGHT: The right position
 * @MX_POSITION_BOTTOM: The bottom position
 * @MX_POSITION_LEFT: The left position
 *
 * Defines the position of an interface element.
 *
 * Since: 1.2
 */
typedef enum /*< prefix=MX_POSITION >*/
{
  MX_POSITION_TOP,
  MX_POSITION_RIGHT,
  MX_POSITION_BOTTOM,
  MX_POSITION_LEFT
} MxPosition;

/**
 * MxImageScaleMode:
 * @MX_IMAGE_SCALE_NONE: Do not apply any scaling and center the image within
 * the allocation
 * @MX_IMAGE_SCALE_FIT: Scale the image, but maintain the aspect ratio so that
 * it fits exactly within the allocation
 * @MX_IMAGE_SCALE_CROP: Scale and crop the image so that it covers the entire
 * allocation while retaining the correct aspect ratio
 *
 * Defines the scaling mode of an image.
 *
 */
typedef enum /*< prefix=MX_IMAGE_SCALE >*/
{
  MX_IMAGE_SCALE_NONE,
  MX_IMAGE_SCALE_FIT,
  MX_IMAGE_SCALE_CROP
} MxImageScaleMode;


/**
 * MxTooltipAnimation:
 * @MX_TOOLTIP_ANIMATION_BOUNCE: Bounce the tooltips when they appear
 * @MX_TOOLTIP_ANIMATION_FADE: Fade the tooltips on show and hide
 *
 * Defines the animation when tooltips are shown and hidden.
 *
 * Since: 1.2
 */
typedef enum /*< prefix=MX_TOOLTIP_ANIMATION >*/
{
  MX_TOOLTIP_ANIMATION_BOUNCE,
  MX_TOOLTIP_ANIMATION_FADE
} MxTooltipAnimation;

G_END_DECLS

#endif /* __MX_TYPES_H__ */
