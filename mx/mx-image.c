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

/**
 * SECTION:mx-image
 * @short_description: A widget to display an image
 *
 * The #MxImage widget can load and display images. The image may be centered
 * or scaled to fit within the allocation. A transition effect occurs when a
 * new image is loaded.
 *
 */

#include <unistd.h>
#include <cogl/cogl.h>

#include "mx-image.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"

G_DEFINE_TYPE (MxImage, mx_image, MX_TYPE_WIDGET)

#define MX_IMAGE_GET_PRIVATE(obj)    \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_IMAGE, MxImagePrivate))

#define DEFAULT_DURATION 1000

/* This stucture holds all that is necessary for cancellable async
 * image loading using thread pools.
 *
 * The idea is that you create this structure (with the pixbuf as NULL)
 * and add it to the thread-pool.
 *
 * The 'complete' member of the struct is protected by the mutex.
 * The thread handler uses this to indicate that the load was completed.
 *
 * The thread will take the mutex while it's loading data - if cancelled
 * is set when it takes the mutex, it will add the idle handler and let the
 * main thread free the data.
 *
 * The idle handler will check that the cancelled member isn't set and if not,
 * will try to upload the image using mx_image_set_from_pixbuf(). It will free
 * the async structure always. It will also reset the pointer to the task in
 * the MxImage priv struct, but only if the cancelled member *isn't* set.
 */
typedef struct
{
  MxImage   *parent;

  GMutex         *mutex;
  guint           complete  : 1;
  guint           cancelled : 1;
  guint           upscale   : 1;
  guint           idle_handler;

  gchar          *filename;
  guchar         *buffer;
  gsize           count;
  GDestroyNotify  free_func;

  gint            width;
  gint            height;
  guint           width_threshold;
  guint           height_threshold;

  GdkPixbuf      *pixbuf;
  GError         *error;
} MxImageAsyncData;

struct _MxImagePrivate
{
  MxImageScaleMode mode;
  guint            load_async : 1;
  guint            upscale    : 1;
  guint            width_threshold;
  guint            height_threshold;

  CoglHandle texture;
  CoglHandle old_texture;
  CoglHandle blank_texture;

  gint rotation;
  gint old_rotation;
  MxImageScaleMode old_mode;

  CoglMaterial *template_material;
  CoglMaterial *material;

  ClutterTimeline *timeline;

  guint duration;

  MxImageAsyncData *async_load_data;
};

enum
{
  PROP_0,

  PROP_SCALE_MODE,
  PROP_LOAD_ASYNC,
  PROP_ALLOW_UPSCALE,
  PROP_SCALE_WIDTH_THRESHOLD,
  PROP_SCALE_HEIGHT_THRESHOLD,
  PROP_IMAGE_ROTATION
};

enum
{
  IMAGE_LOADED,
  IMAGE_LOAD_ERROR,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static GThreadPool *mx_image_threads = NULL;

GQuark
mx_image_error_quark (void)
{
  return g_quark_from_static_string ("mx-image-error-quark");
}

static void
mx_image_async_data_free (MxImageAsyncData *data)
{
  g_mutex_free (data->mutex);

  if (data->free_func)
    data->free_func (data->buffer);

  g_free (data->filename);

  if (data->idle_handler)
    g_source_remove (data->idle_handler);

  if (data->pixbuf)
    g_object_unref (data->pixbuf);

  if (data->error)
    g_error_free (data->error);

  g_free (data);
}

static MxImageAsyncData *
mx_image_async_data_new (MxImage *parent)
{
  MxImageAsyncData *data = g_new0 (MxImageAsyncData, 1);

  data->parent = parent;
  data->mutex = g_mutex_new ();
  data->width = -1;
  data->height = -1;
  data->upscale = parent->priv->upscale;
  data->width_threshold = parent->priv->width_threshold;
  data->height_threshold = parent->priv->height_threshold;

  return data;
}

static void
get_center_coords (CoglObject *tex,
                   float       rotation,
                   float       aw,
                   float       ah,
                   float      *tex_coords)
{
  float bw, bh;

  bw = (float) cogl_texture_get_width (tex); /* base texture width */
  bh = (float) cogl_texture_get_height (tex); /* base texture height */

  tex_coords[0] = 0.5 - (aw / bw) / 2;
  tex_coords[1] = 0.5 - (ah / bh) / 2;
  tex_coords[2] = 0.5 + (aw / bw) / 2;
  tex_coords[3] = 0.5 + (ah / bh) / 2;
}

static gfloat
calculate_scale (CoglObject       *texture,
                 float             rotation,
                 float             aw,
                 float             ah,
                 MxImageScaleMode  mode)
{
  float bw, bh, tmp, factor;

  if (mode == MX_IMAGE_SCALE_NONE)
    return 1.0;

  bw = cogl_texture_get_width (texture); /* base texture width */
  bh = cogl_texture_get_height (texture); /* base texture height */

  /* interpolate between scaling for width and height */
  factor = (ABS (rotation) - ((int)(ABS (rotation) / 180) * 180.0)) / 90.0;

  if (factor > 1.0)
    factor = 2.0 - factor;

  tmp = bw + (bh - bw) * factor;
  bh = bh + (bw - bh) * factor;
  bw = tmp;


  if (bw/bh < aw/ah)
    {
      if (mode == MX_IMAGE_SCALE_CROP)
        return bw / aw;
      else
        return bh / ah;
    }
  else
    {
      if (mode == MX_IMAGE_SCALE_CROP)
        return bh / ah;
      else
        return bw / aw;
    }
}

static void
mx_image_paint (ClutterActor *actor)
{
  MxImagePrivate *priv = MX_IMAGE (actor)->priv;
  ClutterActorBox box;
  float aw, ah, bw, bh;
  guint8 alpha;
  float tex_coords[8];
  MxPadding padding;
  CoglMatrix matrix;
  gfloat scale = 1;
  gfloat ratio;

  /* chain up to draw the background */
  CLUTTER_ACTOR_CLASS (mx_image_parent_class)->paint (actor);

  if (!priv->material)
    return;

  clutter_actor_get_allocation_box (actor, &box);

  aw = (float) (box.x2 - box.x1); /* allocation width */
  ah = (float) (box.y2 - box.y1); /* allocation height */

  /* allow for padding */
  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  aw -= (float) (padding.left + padding.right);
  ah -= (float) (padding.top + padding.bottom);

  bw = cogl_texture_get_width (priv->texture); /* base texture width */
  bh = cogl_texture_get_height (priv->texture); /* base texture height */
  ratio = bw/bh;

  alpha = clutter_actor_get_paint_opacity (actor);

  cogl_material_set_color4ub (priv->material,
                              alpha,
                              alpha,
                              alpha,
                              alpha);

  /* calculate texture co-ordinates */
  get_center_coords (priv->texture, priv->rotation, aw, ah, tex_coords);
  get_center_coords (priv->old_texture, priv->old_rotation, aw, ah,
                     tex_coords + 4);

  /* current texture */
  scale = calculate_scale (priv->texture, priv->rotation, aw, ah, priv->mode);

  cogl_matrix_init_identity (&matrix);
  cogl_matrix_translate (&matrix, 0.5, 0.5, 0);

  cogl_matrix_scale (&matrix, 1, ratio, 1);
  cogl_matrix_rotate (&matrix, priv->rotation, 0, 0, -1);
  cogl_matrix_scale (&matrix, 1, 1 / ratio, 1);

  cogl_matrix_scale (&matrix, scale, scale, 1);

  cogl_matrix_translate (&matrix, -0.5, -0.5, 0);
  cogl_material_set_layer_matrix (priv->material, 0, &matrix);


  /* old texture */
  scale = calculate_scale (priv->old_texture, priv->old_rotation, aw, ah,
                           priv->old_mode);

  bw = cogl_texture_get_width (priv->old_texture);
  bh = cogl_texture_get_height (priv->old_texture);
  ratio = bw/bh;

  cogl_matrix_init_identity (&matrix);
  cogl_matrix_translate (&matrix, 0.5, 0.5, 0);

  cogl_matrix_scale (&matrix, 1, ratio, 1);
  cogl_matrix_rotate (&matrix, priv->old_rotation, 0, 0, -1);
  cogl_matrix_scale (&matrix, 1, 1 / ratio, 1);

  cogl_matrix_scale (&matrix, scale, scale, 1);

  cogl_matrix_translate (&matrix, -0.5, -0.5, 0);
  cogl_material_set_layer_matrix (priv->material, 1, &matrix);

  cogl_set_source (priv->material);

  cogl_rectangle_with_multitexture_coords (padding.left, padding.top,
                                           padding.left + aw, padding.top + ah,
                                           tex_coords,
                                           8);
}

static void
mx_image_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width,
                              gfloat       *pref_width)
{
  MxImagePrivate *priv = MX_IMAGE (actor)->priv;
  gfloat width;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  width = cogl_texture_get_width (priv->texture);

  if (min_width)
    *min_width = 0;

  if (pref_width)
    *pref_width = width + padding.left + padding.right;
}

static void
mx_image_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height,
                               gfloat       *pref_height)
{
  MxImagePrivate *priv = MX_IMAGE (actor)->priv;
  gfloat height;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  height = cogl_texture_get_height (priv->texture);

  if (min_height)
    *min_height = 0;

  if (pref_height)
    *pref_height = height + padding.top + padding.bottom;
}

static void
mx_image_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  MxImage *image = MX_IMAGE (object);

  switch (prop_id)
    {
    case PROP_SCALE_MODE:
      mx_image_set_scale_mode (image, g_value_get_enum (value));
      break;

    case PROP_LOAD_ASYNC:
      mx_image_set_load_async (image, g_value_get_boolean (value));
      break;

    case PROP_ALLOW_UPSCALE:
      mx_image_set_allow_upscale (image, g_value_get_boolean (value));
      break;

    case PROP_SCALE_WIDTH_THRESHOLD:
      mx_image_set_scale_width_threshold (image, g_value_get_uint (value));
      break;

    case PROP_SCALE_HEIGHT_THRESHOLD:
      mx_image_set_scale_height_threshold (image, g_value_get_uint (value));
      break;

    case PROP_IMAGE_ROTATION:
      mx_image_set_image_rotation (image, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_image_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  MxImagePrivate *priv = MX_IMAGE (object)->priv;

  switch (prop_id)
    {
    case PROP_SCALE_MODE:
      g_value_set_enum (value, priv->mode);
      break;

    case PROP_LOAD_ASYNC:
      g_value_set_boolean (value, priv->load_async);
      break;

    case PROP_ALLOW_UPSCALE:
      g_value_set_boolean (value, priv->upscale);
      break;

    case PROP_SCALE_WIDTH_THRESHOLD:
      g_value_set_uint (value, priv->width_threshold);
      break;

    case PROP_SCALE_HEIGHT_THRESHOLD:
      g_value_set_uint (value, priv->height_threshold);
      break;

    case PROP_IMAGE_ROTATION:
      g_value_set_float (value, priv->rotation);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_image_dispose (GObject *object)
{
  MxImagePrivate *priv = MX_IMAGE (object)->priv;

  if (priv->timeline)
    {
      clutter_timeline_stop (priv->timeline);

      g_object_unref (priv->timeline);

      priv->timeline = NULL;
    }

  if (priv->material)
    {
      cogl_object_unref (priv->material);

      priv->material = NULL;
    }

  if (priv->texture)
    {
      cogl_object_unref (priv->texture);

      priv->texture = NULL;
    }

  if (priv->old_texture)
    {
      cogl_object_unref (priv->old_texture);

      priv->old_texture = NULL;
    }

  if (priv->blank_texture)
    {
      cogl_object_unref (priv->blank_texture);

      priv->blank_texture = NULL;
    }

  if (priv->template_material)
    {
      cogl_object_unref (priv->template_material);

      priv->template_material = NULL;
    }

  if (priv->async_load_data)
    {
      priv->async_load_data->cancelled = TRUE;
      priv->async_load_data = NULL;
    }

  G_OBJECT_CLASS (mx_image_parent_class)->dispose (object);
}

static void
mx_image_class_init (MxImageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxImagePrivate));

  object_class->dispose = mx_image_dispose;

  object_class->set_property = mx_image_set_property;
  object_class->get_property = mx_image_get_property;

  actor_class->paint = mx_image_paint;
  actor_class->get_preferred_width = mx_image_get_preferred_width;
  actor_class->get_preferred_height = mx_image_get_preferred_height;

  pspec = g_param_spec_enum ("scale-mode",
                             "Scale Mode",
                             "The scaling mode for the images",
                             MX_TYPE_IMAGE_SCALE_MODE,
                             MX_IMAGE_SCALE_NONE,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_SCALE_MODE, pspec);

  pspec = g_param_spec_boolean ("load-async",
                                "Load Asynchronously",
                                "Whether to load images asynchronously",
                                FALSE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_LOAD_ASYNC, pspec);

  pspec = g_param_spec_boolean ("allow-upscale",
                                "Allow Upscale",
                                "Allow images to be up-scaled",
                                FALSE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_ALLOW_UPSCALE, pspec);

  pspec = g_param_spec_uint ("scale-width-threshold",
                             "Scale Width Threshold",
                             "Amount of pixels difference allowed between "
                             "requested width and image width",
                             0, G_MAXUINT, 0,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_SCALE_WIDTH_THRESHOLD,
                                   pspec);

  pspec = g_param_spec_uint ("scale-height-threshold",
                             "Scale Height Threshold",
                             "Amount of pixels difference allowed between "
                             "requested height and image height",
                             0, G_MAXUINT, 0,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_SCALE_HEIGHT_THRESHOLD,
                                   pspec);

  pspec = g_param_spec_float ("image-rotation",
                              "Image Rotation",
                              "Image rotation in degrees",
                              0, G_MAXINT, 0,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_IMAGE_ROTATION, pspec);

  /**
   * MxImage::image-loaded:
   * @image: the #MxImage that emitted the signal
   *
   * Emitted when an asynchronous image load has completed successfully
   */
  signals[IMAGE_LOADED] =
    g_signal_new ("image-loaded",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxImageClass, image_loaded),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * MxImage::image-load-error:
   * @image: the #MxImage that emitted the signal
   *
   * Emitted when an asynchronous image load has encountered an error
   * and cannot load the requested image.
   */
  signals[IMAGE_LOAD_ERROR] =
    g_signal_new ("image-load-error",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxImageClass, image_load_error),
                  NULL, NULL,
                  _mx_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1, G_TYPE_ERROR);
}


static void
new_frame_cb (ClutterTimeline *timeline,
              guint            elapsed_msecs,
              MxImage         *image)
{
  MxImagePrivate *priv = image->priv;
  CoglHandle copy;
  gdouble progress;
  CoglColor constant;

  if (priv->material == COGL_INVALID_HANDLE)
    return;

  /* You should assume that a material can only be modified once, after
   * its creation; if you need to modify it later you should use a copy
   * instead. Cogl makes copying materials reasonably cheap
   */
  copy = cogl_material_copy (priv->material);

  progress = clutter_timeline_get_progress (priv->timeline);

  /* Create the constant color to be used when combining the two
   * material layers; we use a black color with an alpha component
   * depending on the current progress of the timeline
   */
  cogl_color_init_from_4ub (&constant, 0x00, 0x00, 0x00, 0xff * progress);

  /* This sets the value of the constant color we use when combining
   * the two layers
   */
  cogl_material_set_layer_combine_constant (copy, 1, &constant);

  if (priv->material)
    cogl_object_unref (priv->material);

  priv->material = copy;

  clutter_actor_queue_redraw (CLUTTER_ACTOR (image));
}

static void
mx_image_init (MxImage *self)
{
  MxImagePrivate *priv;
  guchar data[4] = { 0, 0, 0, 0 };

  priv = self->priv = MX_IMAGE_GET_PRIVATE (self);

  priv->timeline = clutter_timeline_new (DEFAULT_DURATION);
  priv->duration = DEFAULT_DURATION;

  g_signal_connect (priv->timeline, "new-frame", G_CALLBACK (new_frame_cb),
                    self);

  priv->blank_texture = cogl_texture_new_from_data (1, 1, COGL_TEXTURE_NO_ATLAS,
                                                    COGL_PIXEL_FORMAT_RGBA_8888,
                                                    COGL_PIXEL_FORMAT_ANY,
                                                    1, data);

  /* set up the initial material */
  priv->template_material = cogl_material_new ();

  cogl_material_set_layer (priv->template_material, 1, priv->blank_texture);
  cogl_material_set_layer (priv->template_material, 0, priv->blank_texture);

  cogl_material_set_layer_wrap_mode (priv->template_material, 0,
                                     COGL_MATERIAL_WRAP_MODE_CLAMP_TO_EDGE);
  cogl_material_set_layer_wrap_mode (priv->template_material, 1,
                                     COGL_MATERIAL_WRAP_MODE_CLAMP_TO_EDGE);

  /* Set the layer combination description for the second layer; the
   * default for Cogl is to simply multiply the layer with the
   * precendent one. In this case we interpolate the color for each
   * pixel between the pixel value of the previous layer and the
   * current one, using the alpha component of a constant color as
   * the interpolation factor.
   */
  cogl_material_set_layer_combine (priv->template_material, 1,
                                   "RGBA = INTERPOLATE (PREVIOUS, "
                                                       "TEXTURE, "
                                                       "CONSTANT[A])",
                                   NULL);

  /* set the transparent texture to start from */
  mx_image_clear (self);
}


/**
 * mx_image_new:
 *
 * Creates a new #MxImage object.
 *
 * Returns: A newly created #MxImage object
 */
ClutterActor*
mx_image_new (void)
{
  return g_object_new (MX_TYPE_IMAGE, NULL);
}


/**
 * mx_image_set_scale_mode:
 * @image: An #MxImage
 * @mode: The #MxImageScaleMode to set
 *
 * Set the scale mode on @MxImage
 *
 */
void
mx_image_set_scale_mode (MxImage          *image,
                         MxImageScaleMode  mode)
{
  if (image->priv->mode != mode)
    {
      image->priv->mode = mode;

      g_object_notify (G_OBJECT (image), "scale-mode");
    }

  clutter_actor_queue_redraw (CLUTTER_ACTOR (image));
}

/**
 * mx_image_get_scale_mode:
 * @image: An #MxImage
 *
 * Get the current scale mode of @MxImage.
 *
 * Returns: The current MxImageScaleMode
 */
MxImageScaleMode
mx_image_get_scale_mode (MxImage *image)
{
  return image->priv->mode;
}

static void
mx_image_prepare_texture (MxImage  *image)
{
  MxImagePrivate *priv = image->priv;

  /* Create a new Cogl material holding the two textures inside two
   * separate layers.
   */
  if (priv->material)
    cogl_object_unref (priv->material);

  priv->material = cogl_material_copy (priv->template_material);

  /* set the new textures on the material */
  cogl_material_set_layer (priv->material, 1, priv->old_texture);
  cogl_material_set_layer (priv->material, 0, priv->texture);

  /* start the cross fade animation */
  clutter_timeline_stop (priv->timeline);
  clutter_timeline_start (priv->timeline);

  /* the image has changed size, so update the preferred width/height */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (image));
}

static void
mx_image_cancel_in_progress (MxImage *image)
{
  MxImagePrivate *priv = image->priv;

  /* Cancel any asynchronous image load */
  if (priv->async_load_data)
    {
      priv->async_load_data->cancelled = TRUE;
      priv->async_load_data = NULL;
    }
}

/**
 * mx_image_clear:
 * @image: A #MxImage
 *
 * Clear the current image and set a blank, transparent image.
 *
 * Returns: static void
 */
void
mx_image_clear (MxImage *image)
{
  MxImagePrivate *priv = image->priv;

  mx_image_cancel_in_progress (image);

  if (priv->texture)
    cogl_object_unref (priv->texture);

  priv->texture = cogl_object_ref (priv->blank_texture);


  if (priv->old_texture)
    cogl_object_unref (priv->old_texture);

  priv->old_texture = cogl_object_ref (priv->blank_texture);
  priv->old_rotation = priv->rotation;
  priv->old_mode = priv->mode;


  if (priv->material)
    cogl_object_unref (priv->material);

  priv->material = cogl_object_ref (priv->template_material);

  /* the image has changed size, so update the preferred width/height */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (image));
}

/**
 * mx_image_set_from_data:
 * @image: An #MxImage
 * @data: Image data
 * @pixel_format: The #CoglPixelFormat of the buffer
 * @width: Width in pixels of image data.
 * @height: Height in pixels of image data
 * @rowstride: Distance in bytes between row starts.
 * @error: Return location for a #GError, or #NULL
 *
 * Set the image data from a buffer. In case of failure, #FALSE is returned
 * and @error is set.
 *
 * Returns: #TRUE if the image was successfully updated
 */
gboolean
mx_image_set_from_data (MxImage          *image,
                        const guchar     *data,
                        CoglPixelFormat   pixel_format,
                        gint              width,
                        gint              height,
                        gint              rowstride,
                        GError          **error)
{
  GError *err;
  gint *blank_area;
  MxImagePrivate *priv;

  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);

  err = NULL;
  priv = image->priv;

  mx_image_cancel_in_progress (image);

  if (priv->old_texture)
    cogl_object_unref (priv->old_texture);

  priv->old_texture = priv->texture;
  priv->old_rotation = priv->rotation;
  priv->old_mode = priv->mode;

  /* Create and upload texture */
  priv->texture = cogl_texture_new_with_size (width + 2, height + 2,
                                              COGL_TEXTURE_NO_ATLAS,
                                              COGL_PIXEL_FORMAT_ANY);
  if (!priv->texture)
    {
      if (error)
        g_set_error (error, MX_IMAGE_ERROR, MX_IMAGE_ERROR_BAD_FORMAT,
                     "Failed to create Cogl texture");

      return FALSE;
    }

  cogl_texture_set_region (priv->texture, 0, 0, 1, 1,
                           width, height, width, height,
                           pixel_format, rowstride, data);

  /* Blit a transparent buffer around the texture */
  blank_area = g_new0 (gint, MAX (width, height) + 2);
  cogl_texture_set_region (priv->texture, 0, 0, 0, 0,
                           width, 1, width, 1,
                           COGL_PIXEL_FORMAT_RGBA_8888, width * 4,
                           (const guint8 *)blank_area);
  cogl_texture_set_region (priv->texture, 0, 0, 0, height + 1,
                           width + 2, 1, width + 2, 1,
                           COGL_PIXEL_FORMAT_RGBA_8888, width * 4,
                           (const guint8 *)blank_area);
  cogl_texture_set_region (priv->texture, 0, 0, 0, 0,
                           1, height + 2, 1, height + 2,
                           COGL_PIXEL_FORMAT_RGBA_8888, 4,
                           (const guint8 *)blank_area);
  cogl_texture_set_region (priv->texture, 0, 0, width + 1, 0,
                           1, height + 2, 1, height + 2,
                           COGL_PIXEL_FORMAT_RGBA_8888, 4,
                           (const guint8 *)blank_area);
  g_free (blank_area);

  mx_image_prepare_texture (image);

  return TRUE;
}

static gboolean
mx_image_set_from_pixbuf (MxImage    *image,
                          GdkPixbuf  *pixbuf,
                          GError    **error)
{
  GError *err;
  gboolean has_alpha;
  MxImagePrivate *priv;
  GdkColorspace color_space;
  gint width, height, rowstride, bps, channels;

  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);

  err = NULL;
  priv = image->priv;

  if (!pixbuf)
    return FALSE;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  bps = gdk_pixbuf_get_bits_per_sample (pixbuf);
  channels = gdk_pixbuf_get_n_channels (pixbuf);
  color_space = gdk_pixbuf_get_colorspace (pixbuf);

  if ((bps != 8) ||
      (color_space != GDK_COLORSPACE_RGB) ||
      !((has_alpha && channels == 4) ||
        (!has_alpha && channels == 3)))
    {
      if (error)
        g_set_error (error, MX_IMAGE_ERROR, MX_IMAGE_ERROR_BAD_FORMAT,
                     "Unsupported image formatting");
      g_object_unref (pixbuf);
      return FALSE;
    }

  return mx_image_set_from_data (image, gdk_pixbuf_get_pixels (pixbuf),
                                 has_alpha ? COGL_PIXEL_FORMAT_RGBA_8888 :
                                             COGL_PIXEL_FORMAT_RGB_888,
                                 width, height, rowstride, error);
}

static gboolean
mx_image_load_complete_cb (gpointer task_data)
{
  MxImageAsyncData *data = task_data;

  /* Lock/unlock mutex to make sure the thread is finished. This is necessary
   * as it's possible that this idle handler will run before the thread unlocks
   * the mutex, and freeing a locked mutex results in undefined behaviour
   * (well, it crashes on Linux with an assert in pthreads...)
   */
  g_mutex_lock (data->mutex);
  g_mutex_unlock (data->mutex);

  /* Reset the idle handler id so we don't try to remove it when we free
   * the data later on.
   */
  data->idle_handler = 0;

  /* Don't do anything with the image data if we've been cancelled already */
  if (!data->cancelled && data->complete)
    {
      /* Reset the current async image load data pointer */
      data->parent->priv->async_load_data = NULL;

      /* If we managed to load the pixbuf, set it now, otherwise forward the
       * error on to the user via a signal.
       */
      if (data->pixbuf)
        {
          GError *error = NULL;
          gboolean success =
            mx_image_set_from_pixbuf (data->parent, data->pixbuf, &error);

          if (success)
            g_signal_emit (data->parent, signals[IMAGE_LOADED], 0);
          else
            {
              g_signal_emit (data->parent, signals[IMAGE_LOAD_ERROR], 0, error);
              g_error_free (error);
            }
        }
      else
        g_signal_emit (data->parent, signals[IMAGE_LOAD_ERROR], 0, data->error);
    }

  /* Free the async loading struct */
  mx_image_async_data_free (data);

  return FALSE;
}

typedef struct
{
  gint     width;
  gint     height;
  guint    width_threshold;
  guint    height_threshold;
  gboolean upscale;
} MxImageSizeRequest;

static void
mx_image_size_prepared_cb (GdkPixbufLoader *loader,
                           gint             width,
                           gint             height,
                           gpointer         user_data)
{
  gboolean fit_width;
  MxImageSizeRequest *constraints = user_data;

  if (constraints->width >= 0)
    {
      if (constraints->height >= 0)
        {
          gfloat aspect = constraints->width / (gfloat)constraints->height;
          gfloat aspect_orig = width / (gfloat)height;

          fit_width = (aspect_orig < aspect);
        }
      else
        fit_width = TRUE;
    }
  else if (constraints->height >= 0)
    fit_width = FALSE;
  else
    return;

  if (fit_width)
    {
      if (!constraints->upscale && (width < constraints->width))
        return;

      if (ABS (width - constraints->width) < constraints->width_threshold)
        return;

      gdk_pixbuf_loader_set_size (loader, constraints->width,
                                  (constraints->width / (gfloat)width) *
                                  (gfloat)height);
    }
  else
    {
      if (!constraints->upscale && (height < constraints->height))
        return;

      if (ABS (height - constraints->height) < constraints->height_threshold)
        return;

      gdk_pixbuf_loader_set_size (loader,
                                  (constraints->height / (gfloat)height) *
                                  (gfloat)width,
                                  constraints->height);
    }
}

static GdkPixbuf *
mx_image_pixbuf_new (const gchar  *filename,
                     guchar       *buffer,
                     gsize         count,
                     gint          width,
                     gint          height,
                     guint         width_threshold,
                     guint         height_threshold,
                     gboolean      upscale,
                     GError      **error)
{
  GdkPixbuf *pixbuf;
  GdkPixbufLoader *loader;
  MxImageSizeRequest constraints;

  GError *err = NULL;

  loader = gdk_pixbuf_loader_new ();

  constraints.width = width;
  constraints.height = height;
  constraints.width_threshold = width_threshold;
  constraints.height_threshold = height_threshold;
  constraints.upscale = upscale;

  g_signal_connect (loader, "size-prepared",
                    G_CALLBACK (mx_image_size_prepared_cb),
                    &constraints);

  if (filename)
    {
      if (!g_file_get_contents (filename, (gchar **)&buffer, &count, &err))
        {
          g_propagate_error (error, err);
          g_object_unref (loader);
          return NULL;
        }

      g_object_weak_ref (G_OBJECT (loader), (GWeakNotify)g_free, buffer);
    }

  if (!buffer)
    {
      g_object_unref (loader);
      return NULL;
    }

  if (!gdk_pixbuf_loader_write (loader, buffer, count, &err))
    {
      g_propagate_error (error, err);
      g_object_unref (loader);
      return NULL;
    }

  /* Note, closing the pixbuf loader will make sure that size-prepared
   * will not be called beyond this point.
   */
  if (!gdk_pixbuf_loader_close (loader, &err))
    {
      g_propagate_error (error, err);
      g_object_unref (loader);
      return NULL;
    }

  pixbuf = g_object_ref (gdk_pixbuf_loader_get_pixbuf (loader));

  g_object_unref (loader);

  return pixbuf;
}

static void
mx_image_async_cb (gpointer task_data,
                   gpointer user_data)
{
  MxImageAsyncData *data = task_data;

  g_mutex_lock (data->mutex);

  /* Check if the task has been cancelled and bail out - leave to the main
   * thread to free the data.
   */
  if (data->cancelled)
    {
      data->idle_handler =
        clutter_threads_add_idle_full (G_PRIORITY_HIGH_IDLE,
                                       mx_image_load_complete_cb, data, NULL);
      g_mutex_unlock (data->mutex);

      return;
    }

  /* Try to load the pixbuf */
  data->pixbuf = mx_image_pixbuf_new (data->filename, data->buffer,
                                      data->count, data->width, data->height,
                                      data->width_threshold,
                                      data->height_threshold, data->upscale,
                                      &data->error);

  data->complete = TRUE;
  data->idle_handler =
    clutter_threads_add_idle_full (G_PRIORITY_HIGH_IDLE,
                                   mx_image_load_complete_cb, data, NULL);

  g_mutex_unlock (data->mutex);
}

static gboolean
mx_image_set_async (MxImage         *image,
                    const gchar     *filename,
                    guchar          *buffer,
                    gsize            count,
                    GDestroyNotify   free_func,
                    gint             width,
                    gint             height,
                    GError         **error)
{
  GError *err;
  MxImagePrivate *priv;
  MxImageAsyncData *data;

  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);

  priv = image->priv;

  /* This function should not be called if async loading isn't enabled */
  if (!priv->load_async)
    {
      g_set_error (error, MX_IMAGE_ERROR, MX_IMAGE_ERROR_NO_ASYNC,
                   "Asynchronous image loading is not enabled");
      return FALSE;
    }

  err = NULL;
  data = NULL;

  /* Load the pixbuf in a thread, then later on upload it to the GPU */
  if (!mx_image_threads)
    {
      mx_image_threads = g_thread_pool_new (mx_image_async_cb, NULL,
                                            sysconf (_SC_NPROCESSORS_ONLN),
                                            FALSE, &err);
      if (!mx_image_threads)
        {
          g_propagate_error (error, err);
          return FALSE;
        }
    }

  /* Cancel/free any in-progress load */
  if (priv->async_load_data)
    {
      MxImageAsyncData *old_data = priv->async_load_data;

      if (!g_mutex_trylock (old_data->mutex))
        {
          /* The thread is busy, cancel it and start a new one */
          old_data->cancelled = TRUE;
        }
      else
        {
          if (old_data->complete)
            {
              /* The load finished, cancel the upload */
              old_data->cancelled = TRUE;
              g_mutex_unlock (old_data->mutex);
            }
          else
            {
              /* The load hasn't begun, we'll hijack it */
              g_free (old_data->filename);
              old_data->filename = g_strdup (filename);
              old_data->buffer = buffer;
              old_data->count = count;
              old_data->free_func = free_func;
              old_data->width = width;
              old_data->height = height;
              old_data->cancelled = FALSE;
              g_mutex_unlock (old_data->mutex);

              data = old_data;
            }
        }
    }

  if (!data)
    {
      /* Create the async load data and add it to the thread-pool */
      priv->async_load_data = data = mx_image_async_data_new (image);
      data->filename = g_strdup (filename);
      data->buffer = buffer;
      data->count = count;
      data->free_func = free_func;
      data->width = width;
      data->height = height;
      g_thread_pool_push (mx_image_threads, data, NULL);
    }

  return TRUE;
}

/**
 * mx_image_set_from_file:
 * @image: An #MxImage
 * @filename: Filename to read the file from
 * @error: Return location for a #GError, or #NULL
 *
 * Set the image data from an image file. In case of failure, #FALSE is returned
 * and @error is set.
 *
 * Returns: #TRUE if the image was successfully updated
 */
gboolean
mx_image_set_from_file (MxImage      *image,
                        const gchar  *filename,
                        GError      **error)
{
  return mx_image_set_from_file_at_size (image, filename, -1, -1, error);
}

/**
 * mx_image_set_from_file_at_size:
 * @image: An #MxImage
 * @filename: Filename to read the file from
 * @width: Width to scale the image to, or -1
 * @height: Height to scale the image to, or -1
 * @error: Return location for a #GError, or #NULL
 *
 * Set the image data from an image file, and scale the image during loading.
 * In case of failure, #FALSE is returned and @error is set. The aspect ratio
 * will always be maintained.
 *
 * Returns: #TRUE if the image was successfully updated
 */
gboolean
mx_image_set_from_file_at_size (MxImage      *image,
                                const gchar  *filename,
                                gint          width,
                                gint          height,
                                GError      **error)
{
  gboolean retval;
  GdkPixbuf *pixbuf;
  MxImagePrivate *priv;

  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);

  priv = image->priv;

  /* Load the pixbuf in a thread, then later on upload it to the GPU */
  if (priv->load_async)
    return mx_image_set_async (image, filename, NULL, 0, NULL,
                               width, height, error);

  /* Synchronously load the pixbuf and set it */
  pixbuf = mx_image_pixbuf_new (filename, NULL, 0, width, height,
                                priv->width_threshold, priv->height_threshold,
                                priv->upscale, error);
  if (!pixbuf)
    return FALSE;

  retval = mx_image_set_from_pixbuf (image, pixbuf, error);

  g_object_unref (pixbuf);

  return retval;
}

/**
 * mx_image_set_from_buffer:
 * @image: An #MxImage
 * @buffer: A buffer pointing to encoded image data
 * @buffer_size: The size of @buffer, in bytes
 * @buffer_free_func: A function to free @buffer, or %NULL
 * @error: Return location for a #GError, or #NULL
 *
 * Set the image data from unencoded image data, stored in memory. In case of
 * failure, #FALSE is returned and @error is set. It is expected that @buffer
 * will remain accessible for the duration of the load. Once it is finished
 * with, @buffer_free_func will be called.
 *
 * Returns: #TRUE if the image was successfully updated
 */
gboolean
mx_image_set_from_buffer (MxImage         *image,
                          guchar          *buffer,
                          gsize            buffer_size,
                          GDestroyNotify   buffer_free_func,
                          GError         **error)
{
  return mx_image_set_from_buffer_at_size (image, buffer, buffer_size,
                                           buffer_free_func, -1, -1, error);
}

/**
 * mx_image_set_from_buffer_at_size:
 * @image: An #MxImage
 * @buffer: A buffer pointing to encoded image data
 * @buffer_size: The size of @buffer, in bytes
 * @buffer_free_func: A function to free @buffer, or %NULL
 * @width: Width to scale the image to, or -1
 * @height: Height to scale the image to, or -1
 * @error: Return location for a #GError, or #NULL
 *
 * Set the image data from unencoded image data, stored in memory, and scales
 * it while loading. In case of failure, #FALSE is returned and @error is set.
 * It is expected that @buffer will remain accessible for the duration of the
 * load. Once it is finished with, @buffer_free_func will be called. The aspect
 * ratio will always be maintained.
 *
 * Returns: #TRUE if the image was successfully updated
 */
gboolean
mx_image_set_from_buffer_at_size (MxImage         *image,
                                  guchar          *buffer,
                                  gsize            buffer_size,
                                  GDestroyNotify   buffer_free_func,
                                  gint             width,
                                  gint             height,
                                  GError         **error)
{
  gboolean retval;
  GdkPixbuf *pixbuf;
  MxImagePrivate *priv;

  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);

  priv = image->priv;

  if (priv->load_async)
    return mx_image_set_async (image, NULL, buffer, buffer_size,
                               buffer_free_func, width, height, error);

  pixbuf = mx_image_pixbuf_new (NULL, buffer, buffer_size, width, height,
                                priv->width_threshold, priv->height_threshold,
                                priv->upscale, error);
  if (!pixbuf)
    return FALSE;

  retval = mx_image_set_from_pixbuf (image, pixbuf, error);

  g_object_unref (pixbuf);

  if (buffer_free_func)
    buffer_free_func ((gpointer)buffer);

  return retval;
}

/**
 * mx_image_set_load_async:
 * @image: A #MxImage
 * @load_async: %TRUE to load images asynchronously
 *
 * Sets whether to load images asynchronously. Asynchronous image loading
 * requires thread support (see g_thread_init()).
 *
 * When using asynchronous image loading, all image-loading functions will
 * return immediately as successful. The #MxImage::image-loaded and
 * #MxImage::image-load-error signals are used to signal success or failure
 * of asynchronous image loading.
 */
void
mx_image_set_load_async (MxImage  *image,
                         gboolean  load_async)
{
  MxImagePrivate *priv;

  g_return_if_fail (MX_IS_IMAGE (image));

  priv = image->priv;
  if (priv->load_async != load_async)
    {
      priv->load_async = load_async;
      g_object_notify (G_OBJECT (image), "load-async");

      /* Cancel the old transfer if we're turning async off */
      if (!load_async && priv->async_load_data)
        {
          priv->async_load_data->cancelled = TRUE;
          priv->async_load_data = NULL;
        }
    }
}

/**
 * mx_image_get_load_async:
 * @image: A #MxImage
 *
 * Determines whether asynchronous image loading is in use.
 *
 * Returns: %TRUE if images are set to load asynchronously, %FALSE otherwise
 */
gboolean
mx_image_get_load_async (MxImage *image)
{
  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);
  return image->priv->load_async;
}

/**
 * mx_image_set_allow_upscale:
 * @image: A #MxImage
 * @allow: %TRUE to allow upscaling, %FALSE otherwise
 *
 * Sets whether up-scaling of images is allowed. If set to %TRUE and a size
 * larger than the image is requested, the image will be up-scaled in
 * software.
 *
 * The advantage of this is that software up-scaling is potentially higher
 * quality, but it comes at the expense of video memory.
 */
void
mx_image_set_allow_upscale (MxImage  *image,
                            gboolean  allow)
{
  MxImagePrivate *priv;

  g_return_if_fail (MX_IS_IMAGE (image));

  priv = image->priv;
  if (priv->upscale != allow)
    {
      priv->upscale = allow;
      g_object_notify (G_OBJECT (image), "allow-upscale");
    }
}

/**
 * mx_image_get_allow_upscale:
 * @image: A #MxImage
 *
 * Determines whether image up-scaling is allowed.
 *
 * Returns: %TRUE if upscaling is allowed, %FALSE otherwise
 */
gboolean
mx_image_get_allow_upscale (MxImage *image)
{
  g_return_val_if_fail (MX_IS_IMAGE (image), FALSE);
  return image->priv->upscale;
}

/**
 * mx_image_set_scale_width_threshold:
 * @image: A #MxImage
 * @pixels: Number of pixels
 *
 * Sets the threshold used to determine whether to scale the width of the
 * image. If a specific width is requested, the image width is allowed to
 * differ by this amount before scaling is employed.
 *
 * This can be useful to avoid excessive CPU usage when the image differs
 * only slightly to the desired size.
 */
void
mx_image_set_scale_width_threshold (MxImage *image,
                                    guint    pixels)
{
  MxImagePrivate *priv;

  g_return_if_fail (MX_IS_IMAGE (image));

  priv = image->priv;
  if (priv->width_threshold != pixels)
    {
      priv->width_threshold = pixels;
      g_object_notify (G_OBJECT (image), "scale-width-threshold");
    }
}

/**
 * mx_image_get_scale_width_threshold:
 * @image: A #MxImage
 *
 * Retrieves the width scaling threshold.
 *
 * Returns: The width scaling threshold, in pixels
 */
guint
mx_image_get_scale_width_threshold (MxImage *image)
{
  g_return_val_if_fail (MX_IS_IMAGE (image), 0);
  return image->priv->width_threshold;
}

/**
 * mx_image_set_scale_height_threshold:
 * @image: A #MxImage
 * @pixels: Number of pixels
 *
 * Sets the threshold used to determine whether to scale the height of the
 * image. If a specific height is requested, the image height is allowed to
 * differ by this amount before scaling is employed.
 *
 * This can be useful to avoid excessive CPU usage when the image differs
 * only slightly to the desired size.
 */
void
mx_image_set_scale_height_threshold (MxImage *image,
                                     guint    pixels)
{
  MxImagePrivate *priv;

  g_return_if_fail (MX_IS_IMAGE (image));

  priv = image->priv;
  if (priv->height_threshold != pixels)
    {
      priv->height_threshold = pixels;
      g_object_notify (G_OBJECT (image), "scale-height-threshold");
    }
}

/**
 * mx_image_get_scale_height_threshold:
 * @image: A #MxImage
 *
 * Retrieves the height scaling threshold.
 *
 * Returns: The height scaling threshold, in pixels
 */
guint
mx_image_get_scale_height_threshold (MxImage *image)
{
  g_return_val_if_fail (MX_IS_IMAGE (image), 0);
  return image->priv->height_threshold;
}

/**
 * mx_image_set_image_rotation:
 * @image: A #MxImage
 * @rotation: Rotation angle in degrees
 *
 * Set the MxImage:image-rotation property.
 *
 */
void
mx_image_set_image_rotation (MxImage *image,
                             gfloat   rotation)
{
  g_return_if_fail (MX_IS_IMAGE (image));

  if (image->priv->rotation != rotation)
    {
      image->priv->rotation = rotation;

      clutter_actor_queue_redraw (CLUTTER_ACTOR (image));

      g_object_notify (G_OBJECT (image), "image-rotation");
    }
}

/**
 * mx_image_get_image_rotation:
 * @image: A #MxImage
 *
 * Get the value of the MxImage:image-rotation property.
 *
 * Returns: The value of the image-rotation property.
 */
gfloat
mx_image_get_image_rotation (MxImage *image)
{
  g_return_val_if_fail (MX_IS_IMAGE (image), 0);

  return image->priv->rotation;
}
