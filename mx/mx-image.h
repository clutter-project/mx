/*
 * Copyright (C) 2008, 2009, 2010, 2011 Intel Corporation.
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


#ifndef _MX_IMAGE
#define _MX_IMAGE

#include <glib-object.h>
#include "mx-widget.h"

G_BEGIN_DECLS

#define MX_TYPE_IMAGE mx_image_get_type()

#define MX_IMAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_IMAGE, MxImage))

#define MX_IMAGE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_IMAGE, MxImageClass))

#define MX_IS_IMAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_IMAGE))

#define MX_IS_IMAGE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_IMAGE))

#define MX_IMAGE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_IMAGE, MxImageClass))

typedef enum
{
  MX_IMAGE_ERROR_BAD_FORMAT,
  MX_IMAGE_ERROR_NO_ASYNC,
  MX_IMAGE_ERROR_INTERNAL,
  MX_IMAGE_ERROR_INVALID_PARAMETER
} MxImageError;

#define MX_IMAGE_ERROR (mx_image_error_quark ())
GQuark mx_image_error_quark (void);

typedef struct _MxImage MxImage;
typedef struct _MxImageClass MxImageClass;
typedef struct _MxImagePrivate MxImagePrivate;

/**
 * MxImage:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */

struct _MxImage
{
  /*< private >*/
  MxWidget parent;

  MxImagePrivate *priv;
};

struct _MxImageClass
{
  /*< private >*/
  MxWidgetClass parent_class;

  /* signals, not vfuncs */
  void (* image_loaded)     (MxImage *image);
  void (* image_load_error) (MxImage *image, GError *error);
};


GType mx_image_get_type (void);

ClutterActor * mx_image_new (void);

gboolean mx_image_set_from_data (MxImage          *image,
                                 const guchar     *data,
                                 CoglPixelFormat   pixel_format,
                                 gint              width,
                                 gint              height,
                                 gint              rowstride,
                                 GError          **error);

gboolean mx_image_set_from_file (MxImage      *image,
                                 const gchar  *filename,
                                 GError      **error);
gboolean mx_image_set_from_file_at_size (MxImage      *image,
                                         const gchar  *filename,
                                         gint          width,
                                         gint          height,
                                         GError      **error);

gboolean mx_image_set_from_cogl_texture (MxImage    *image,
                                         CoglHandle  texture);

gboolean mx_image_set_from_buffer (MxImage         *image,
                                   guchar          *buffer,
                                   gsize            buffer_size,
                                   GDestroyNotify   buffer_free_func,
                                   GError         **error);
gboolean mx_image_set_from_buffer_at_size (MxImage         *image,
                                           guchar          *buffer,
                                           gsize            buffer_size,
                                           GDestroyNotify   buffer_free_func,
                                           gint             width,
                                           gint             height,
                                           GError         **error);

void     mx_image_clear (MxImage *image);

void             mx_image_set_scale_mode (MxImage          *image,
                                          MxImageScaleMode  mode);
MxImageScaleMode mx_image_get_scale_mode (MxImage          *image);

void     mx_image_set_image_rotation (MxImage *image,
                                      gfloat   rotation);
gfloat   mx_image_get_image_rotation (MxImage *image);

void     mx_image_set_load_async (MxImage  *image,
                                  gboolean  load_async);
gboolean mx_image_get_load_async (MxImage  *image);

void     mx_image_set_allow_upscale (MxImage *image,
                                     gboolean allow);
gboolean mx_image_get_allow_upscale (MxImage *image);

void     mx_image_set_scale_width_threshold (MxImage *image,
                                             guint    pixels);
guint    mx_image_get_scale_width_threshold (MxImage *image);

void     mx_image_set_scale_height_threshold (MxImage *image,
                                              guint    pixels);
guint    mx_image_get_scale_height_threshold (MxImage *image);

void     mx_image_set_transition_duration (MxImage *image,
                                           guint    duration);
guint    mx_image_get_transition_duration (MxImage *image);

void     mx_image_animate_scale_mode (MxImage          *image,
                                      gulong            mode,
                                      guint             duration,
                                      MxImageScaleMode  scale_mode);
G_END_DECLS

#endif /* _MX_IMAGE */

