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

#include <cogl/cogl.h>

#include "mx-image.h"
#include "mx-enum-types.h"

G_DEFINE_TYPE (MxImage, mx_image, MX_TYPE_WIDGET)

#define MX_IMAGE_GET_PRIVATE(obj)    \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_IMAGE, MxImagePrivate))

#define DEFAULT_DURATION 1000

struct _MxImagePrivate
{
  MxImageScaleMode mode;

  CoglHandle texture;
  CoglHandle old_texture;
  CoglHandle blank_texture;

  CoglMaterial *template_material;
  CoglMaterial *material;

  ClutterTimeline *timeline;

  guint duration;
};

enum
{
  PROP_0,

  PROP_SCALE_MODE
};

GQuark
mx_image_error_quark (void)
{
  return g_quark_from_static_string ("mx-image-error-quark");
}

static void
add_one_pixel_border (CoglHandle *texture)
{
  CoglHandle old_texture = *texture;

  int width = cogl_texture_get_width (old_texture);
  int height = cogl_texture_get_height (old_texture);
  /* Create a new texture with an extra 2 pixels in each dimension and
     force it to have an alpha component */
  CoglHandle new_texture =
    cogl_texture_new_with_size (width + 2, height + 2,
                                COGL_TEXTURE_NO_ATLAS,
                                COGL_PIXEL_FORMAT_RGBA_8888);
  CoglHandle fbo = cogl_offscreen_new_to_texture (new_texture);
  CoglMaterial *tex_material = cogl_material_new ();
  CoglMaterial *clear_material;
  CoglColor transparent_color;

  /* Set the blending equation to directly copy the bits of the old
     texture without blending the destination pixels */
  cogl_material_set_blend (tex_material,
                           "RGBA=ADD(SRC_COLOR, 0)",
                           NULL);
  clear_material = cogl_material_copy (tex_material);

  cogl_material_set_layer (tex_material, 0, old_texture);

  cogl_color_set_from_4ub (&transparent_color, 0, 0, 0, 0);
  cogl_material_set_color (clear_material, &transparent_color);

  cogl_push_framebuffer (fbo);
  cogl_ortho (0, width + 2, height + 2, 0, -1, 1);

  cogl_push_source (tex_material);
  cogl_rectangle (1, 1, width + 1, height + 1);

  /* Clear the 1 pixel border around the texture */
  cogl_set_source (clear_material);
  cogl_rectangle (0, 0, width + 2, 1);
  cogl_rectangle (0, height + 1, width + 2, height + 2);
  cogl_rectangle (0, 1, 1, height + 1);
  cogl_rectangle (width + 1, 1, width + 2, height + 1);

  cogl_pop_framebuffer ();
  cogl_pop_source ();

  cogl_object_unref (clear_material);
  cogl_object_unref (tex_material);
  cogl_handle_unref (fbo);

  cogl_handle_unref (old_texture);

  *texture = new_texture;
}

static void
get_zoom_crop_coords (CoglObject *tex,
                      float       aw,
                      float       ah,
                      float      *tex_coords)
{
  float bw, bh;
  float v;

  bw = (float) cogl_texture_get_width (tex); /* base texture width */
  bh = (float) cogl_texture_get_height (tex); /* base texture height */

  if ((float) bw/bh < (float) aw/ah)
    {
      /* fit width */
      v = (((float) ah * bw) / ((float) aw * bh)) / 2;
      tex_coords[0] = 0;
      tex_coords[2] = 1;
      tex_coords[1] = (0.5 - v);
      tex_coords[3] = (0.5 + v);
    }
  else
    {
      /* fit height */
      v = (((float) aw * bh) / ((float) ah * bw)) / 2;
      tex_coords[0] = (0.5 - v);
      tex_coords[2] = (0.5 + v);
      tex_coords[1] = 0;
      tex_coords[3] = 1;
    }
}

static void
get_center_coords (CoglObject *tex,
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

static void
get_zoom_coords (CoglObject *tex,
                 float       aw,
                 float       ah,
                 float      *tex_coords)
{
  float bw, bh;
  float x, y, w, h;

  bw = (float) cogl_texture_get_width (tex); /* base texture width */
  bh = (float) cogl_texture_get_height (tex); /* base texture height */


  if (ah / aw < bh / bw)
    {
      gfloat factor, new_w, gap;

      y = 0; h = 1;

      factor = (ah / bh);

      new_w = bw * factor;

      gap = (aw - new_w) / 2.0;

      x = 0.0 - gap / new_w;
      w = 1.0 + gap / new_w;
    }
  else
    {
      gfloat factor, new_h, gap;

      x = 0; w = 1;

      factor = (aw / bw);

      new_h = bh * factor;

      gap = (ah - new_h) / 2;

      y = 0 - gap / new_h;
      h = 1 + gap / new_h;
    }


  tex_coords[0] = x;
  tex_coords[1] = y;
  tex_coords[2] = w;
  tex_coords[3] = h;
}

static void
mx_image_paint (ClutterActor *actor)
{
  MxImagePrivate *priv = MX_IMAGE (actor)->priv;
  ClutterActorBox box;
  float aw, ah;
  guint8 alpha;
  float tex_coords[8];
  MxPadding padding;

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


  alpha = clutter_actor_get_paint_opacity (actor);

  cogl_material_set_color4ub (priv->material,
                              alpha,
                              alpha,
                              alpha,
                              alpha);

  cogl_set_source (priv->material);

  if (priv->mode == MX_IMAGE_SCALE_CROP)
    {
      get_zoom_crop_coords (priv->texture, aw, ah, tex_coords);
      get_zoom_crop_coords (priv->old_texture, aw, ah, tex_coords + 4);
    }
  else if (priv->mode == MX_IMAGE_SCALE_FIT)
    {
      get_zoom_coords (priv->texture, aw, ah, tex_coords);
      get_zoom_coords (priv->old_texture, aw, ah, tex_coords + 4);
    }
  else if (priv->mode == MX_IMAGE_SCALE_NONE)
    {
      get_center_coords (priv->texture, aw, ah, tex_coords);
      get_center_coords (priv->old_texture, aw, ah, tex_coords + 4);
    }

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
  switch (prop_id)
    {
    case PROP_SCALE_MODE:
      mx_image_set_scale_mode (MX_IMAGE (object), g_value_get_enum (value));
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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_image_dispose (GObject *object)
{
  MxImagePrivate *priv = MX_IMAGE (object)->priv;

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

  /* add a one pixel border to the image */
  add_one_pixel_border (&priv->texture);

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
  clutter_timeline_start (priv->timeline);

  /* the image has changed size, so update the preferred width/height */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (image));
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

  if (priv->texture)
    cogl_object_unref (priv->texture);

  priv->texture = cogl_object_ref (priv->blank_texture);


  if (priv->old_texture)
    cogl_object_unref (priv->old_texture);

  priv->old_texture = cogl_object_ref (priv->blank_texture);


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
  MxImagePrivate *priv = image->priv;


  if (priv->old_texture)
    cogl_object_unref (priv->old_texture);

  priv->old_texture = priv->texture;

  priv->texture = cogl_texture_new_from_data (width, height,
                                              COGL_TEXTURE_NO_ATLAS,
                                              pixel_format,
                                              COGL_PIXEL_FORMAT_ANY,
                                              rowstride, data);

  if (!priv->texture)
    {
      if (error)
        g_set_error (error, MX_IMAGE_ERROR, MX_IMAGE_ERROR_BAD_FORMAT,
                     "Failed to create Cogl texture");

      return FALSE;
    }

  mx_image_prepare_texture (image);

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
  MxImagePrivate *priv = image->priv;
  GError *err = NULL;

  if (priv->old_texture)
    cogl_object_unref (priv->old_texture);

  priv->old_texture = priv->texture;

  priv->texture = cogl_texture_new_from_file (filename,
                                              COGL_TEXTURE_NO_ATLAS,
                                              COGL_PIXEL_FORMAT_ANY,
                                              &err);

  if (err)
    {
      g_propagate_error (error, err);

      return FALSE;
    }


  mx_image_prepare_texture (image);

  return TRUE;
}
