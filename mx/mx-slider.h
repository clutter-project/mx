/*
 * mx-slider.c: Slider widget
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
 * Written by: Damien Lespiau <damien.lespiau@intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_SLIDER_H__
#define __MX_SLIDER_H__

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_SLIDER mx_slider_get_type()

#define MX_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SLIDER, MxSlider))

#define MX_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_SLIDER, MxSliderClass))

#define MX_IS_SLIDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SLIDER))

#define MX_IS_SLIDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_SLIDER))

#define MX_SLIDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_SLIDER, MxSliderClass))

typedef struct _MxSlider MxSlider;
typedef struct _MxSliderClass MxSliderClass;
typedef struct _MxSliderPrivate MxSliderPrivate;

/**
 * MxSlider:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxSlider
{
  /*< private >*/
  MxWidget parent;

  MxSliderPrivate *priv;
};

struct _MxSliderClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType           mx_slider_get_type      (void) G_GNUC_CONST;

ClutterActor *  mx_slider_new           (void);

void            mx_slider_set_value  (MxSlider *bar,
                                      gdouble   value);
gdouble         mx_slider_get_value  (MxSlider *bar);

void            mx_slider_set_buffer_value  (MxSlider *slider,
                                             gdouble   value);
gdouble         mx_slider_get_buffer_value  (MxSlider *slider);

G_END_DECLS

#endif /* __MX_SLIDER_H__ */
