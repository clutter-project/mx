/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-texture-frame.h: Expandible texture actor
 *
 * Copyright 2007 OpenedHand
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
 */

/**
 * SECTION:mx-texture-frame
 * @short_description: Stretch a texture to fit the entire allocation
 *
 * #MxTextureFrame
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cogl/cogl.h>

#include "mx-texture-frame.h"
#include "mx-private.h"

enum
{
  PROP_0,

  PROP_PARENT_TEXTURE,

  PROP_TOP,
  PROP_RIGHT,
  PROP_BOTTOM,
  PROP_LEFT
};

G_DEFINE_TYPE (MxTextureFrame, mx_texture_frame, CLUTTER_TYPE_ACTOR);

#define MX_TEXTURE_FRAME_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_TEXTURE_FRAME, MxTextureFramePrivate))

struct _MxTextureFramePrivate
{
  ClutterTexture *parent_texture;

  gfloat          top;
  gfloat          right;
  gfloat          bottom;
  gfloat          left;
};

static void
mx_texture_frame_get_preferred_width (ClutterActor *self,
                                      gfloat        for_height,
                                      gfloat       *min_width_p,
                                      gfloat       *natural_width_p)
{
  MxTextureFramePrivate *priv = MX_TEXTURE_FRAME (self)->priv;

  if (G_UNLIKELY (priv->parent_texture == NULL))
    {
      if (min_width_p)
        *min_width_p = 0;

      if (natural_width_p)
        *natural_width_p = 0;
    }
  else
    {
      ClutterActorClass *klass;

      /* by directly querying the parent texture's class implementation
       * we are going around any override mechanism the parent texture
       * might have in place, and we ask directly for the original
       * preferred width
       */
      klass = CLUTTER_ACTOR_GET_CLASS (priv->parent_texture);
      klass->get_preferred_width (CLUTTER_ACTOR (priv->parent_texture),
                                  for_height,
                                  min_width_p,
                                  natural_width_p);
    }
}

static void
mx_texture_frame_get_preferred_height (ClutterActor *self,
                                       gfloat        for_width,
                                       gfloat       *min_height_p,
                                       gfloat       *natural_height_p)
{
  MxTextureFramePrivate *priv = MX_TEXTURE_FRAME (self)->priv;

  if (G_UNLIKELY (priv->parent_texture == NULL))
    {
      if (min_height_p)
        *min_height_p = 0;

      if (natural_height_p)
        *natural_height_p = 0;
    }
  else
    {
      ClutterActorClass *klass;

      /* by directly querying the parent texture's class implementation
       * we are going around any override mechanism the parent texture
       * might have in place, and we ask directly for the original
       * preferred height
       */
      klass = CLUTTER_ACTOR_GET_CLASS (priv->parent_texture);
      klass->get_preferred_height (CLUTTER_ACTOR (priv->parent_texture),
                                   for_width,
                                   min_height_p,
                                   natural_height_p);
    }
}

static void
mx_texture_frame_paint (ClutterActor *self)
{
  MxTextureFramePrivate *priv = MX_TEXTURE_FRAME (self)->priv;
  CoglHandle cogl_texture = COGL_INVALID_HANDLE;
  CoglHandle cogl_material = COGL_INVALID_HANDLE;
  ClutterActorBox box = { 0, };
  gfloat width, height;
  gfloat tex_width, tex_height;
  gfloat ex, ey;
  gfloat tx1, ty1, tx2, ty2;
  guint8 opacity;

  /* no need to paint stuff if we don't have a texture */
  if (G_UNLIKELY (priv->parent_texture == NULL))
    return;

  /* parent texture may have been hidden, so need to make sure it gets
   * realized
   */
  if (!CLUTTER_ACTOR_IS_REALIZED (priv->parent_texture))
    clutter_actor_realize (CLUTTER_ACTOR (priv->parent_texture));

  cogl_texture = clutter_texture_get_cogl_texture (priv->parent_texture);
  if (cogl_texture == COGL_INVALID_HANDLE)
    return;
  cogl_material = clutter_texture_get_cogl_material (priv->parent_texture);
  if (cogl_material == COGL_INVALID_HANDLE)
    return;

  tex_width  = cogl_texture_get_width (cogl_texture);
  tex_height = cogl_texture_get_height (cogl_texture);

  clutter_actor_get_allocation_box (self, &box);
  width = box.x2 - box.x1;
  height = box.y2 - box.y1;


  opacity = clutter_actor_get_paint_opacity (self);

  /* Paint using the parent texture's material. It should already have
     the cogl texture set as the first layer */
  /* NB: for correct blending we need set a preumultiplied color here: */
  cogl_material_set_color4ub (cogl_material,
                              opacity, opacity, opacity, opacity);
  cogl_set_source (cogl_material);

  /* simple stretch */
  if (priv->left == 0 && priv->right == 0 && priv->top == 0
      && priv->bottom == 0)
    {
      cogl_rectangle (0, 0, width, height);
      return;
    }

  tx1 = priv->left / tex_width;
  tx2 = (tex_width - priv->right) / tex_width;
  ty1 = priv->top / tex_height;
  ty2 = (tex_height - priv->bottom) / tex_height;

  ex = width - priv->right;
  if (ex < priv->left)
    ex = priv->left;

  ey = height - priv->bottom;
  if (ey < priv->top)
    ey = priv->top;


  {
    gfloat rectangles[] =
    {
      /* top left corner */
      0, 0,
      priv->left, priv->top,
      0.0, 0.0,
      tx1, ty1,

      /* top middle */
      priv->left, 0,
      MAX (priv->left, ex), priv->top,
      tx1, 0.0,
      tx2, ty1,

      /* top right */
      ex, 0,
      MAX (ex + priv->right, width), priv->top,
      tx2, 0.0,
      1.0, ty1,

      /* mid left */
      0, priv->top,
      priv->left,  ey,
      0.0, ty1,
      tx1, ty2,

      /* center */
      priv->left, priv->top,
      ex, ey,
      tx1, ty1,
      tx2, ty2,

      /* mid right */
      ex, priv->top,
      MAX (ex + priv->right, width), ey,
      tx2, ty1,
      1.0, ty2,

      /* bottom left */
      0, ey,
      priv->left, MAX (ey + priv->bottom, height),
      0.0, ty2,
      tx1, 1.0,

      /* bottom center */
      priv->left, ey,
      ex, MAX (ey + priv->bottom, height),
      tx1, ty2,
      tx2, 1.0,

      /* bottom right */
      ex, ey,
      MAX (ex + priv->right, width), MAX (ey + priv->bottom, height),
      tx2, ty2,
      1.0, 1.0
    };

    cogl_rectangles_with_texture_coords (rectangles, 9);
  }
}

static inline void
mx_texture_frame_set_frame_internal (MxTextureFrame *frame,
                                     gfloat          top,
                                     gfloat          right,
                                     gfloat          bottom,
                                     gfloat          left)
{
  MxTextureFramePrivate *priv = frame->priv;
  GObject *gobject = G_OBJECT (frame);
  gboolean changed = FALSE;

  g_object_freeze_notify (gobject);

  if (priv->top != top)
    {
      priv->top = top;
      g_object_notify (gobject, "top");
      changed = TRUE;
    }

  if (priv->right != right)
    {
      priv->right = right;
      g_object_notify (gobject, "right");
      changed = TRUE;
    }

  if (priv->bottom != bottom)
    {
      priv->bottom = bottom;
      g_object_notify (gobject, "bottom");
      changed = TRUE;
    }

  if (priv->left != left)
    {
      priv->left = left;
      g_object_notify (gobject, "left");
      changed = TRUE;
    }

  if (changed && CLUTTER_ACTOR_IS_VISIBLE (frame))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (frame));

  g_object_thaw_notify (gobject);
}

static void
mx_texture_frame_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MxTextureFrame *frame = MX_TEXTURE_FRAME (gobject);
  MxTextureFramePrivate *priv = frame->priv;

  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      mx_texture_frame_set_parent_texture (frame,
                                           g_value_get_object (value));
      break;

    case PROP_TOP:
      mx_texture_frame_set_frame_internal (frame,
                                           g_value_get_float (value),
                                           priv->right,
                                           priv->bottom,
                                           priv->left);
      break;

    case PROP_RIGHT:
      mx_texture_frame_set_frame_internal (frame,
                                           priv->top,
                                           g_value_get_float (value),
                                           priv->bottom,
                                           priv->left);
      break;

    case PROP_BOTTOM:
      mx_texture_frame_set_frame_internal (frame,
                                           priv->top,
                                           priv->right,
                                           g_value_get_float (value),
                                           priv->left);
      break;

    case PROP_LEFT:
      mx_texture_frame_set_frame_internal (frame,
                                           priv->top,
                                           priv->right,
                                           priv->bottom,
                                           g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_texture_frame_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MxTextureFramePrivate *priv = MX_TEXTURE_FRAME (gobject)->priv;

  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      g_value_set_object (value, priv->parent_texture);
      break;

    case PROP_LEFT:
      g_value_set_float (value, priv->left);
      break;

    case PROP_TOP:
      g_value_set_float (value, priv->top);
      break;

    case PROP_RIGHT:
      g_value_set_float (value, priv->right);
      break;

    case PROP_BOTTOM:
      g_value_set_float (value, priv->bottom);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_texture_frame_dispose (GObject *gobject)
{
  MxTextureFramePrivate *priv = MX_TEXTURE_FRAME (gobject)->priv;

  if (priv->parent_texture)
    {
      g_object_unref (priv->parent_texture);
      priv->parent_texture = NULL;
    }

  G_OBJECT_CLASS (mx_texture_frame_parent_class)->dispose (gobject);
}

static gboolean
mx_texture_frame_get_paint_volume (ClutterActor       *actor,
                                   ClutterPaintVolume *volume)
{
  if (G_OBJECT_TYPE (actor) != mx_texture_frame_get_type ())
    return FALSE;

  return clutter_paint_volume_set_from_allocation (volume, actor);
}

static void
mx_texture_frame_class_init (MxTextureFrameClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (gobject_class, sizeof (MxTextureFramePrivate));

  actor_class->get_preferred_width =
    mx_texture_frame_get_preferred_width;
  actor_class->get_preferred_height =
    mx_texture_frame_get_preferred_height;
  actor_class->paint = mx_texture_frame_paint;
  actor_class->get_paint_volume = mx_texture_frame_get_paint_volume;

  gobject_class->set_property = mx_texture_frame_set_property;
  gobject_class->get_property = mx_texture_frame_get_property;
  gobject_class->dispose = mx_texture_frame_dispose;

  pspec = g_param_spec_object ("parent-texture",
                               "Parent Texture",
                               "The parent ClutterTexture",
                               CLUTTER_TYPE_TEXTURE,
                               MX_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_PARENT_TEXTURE, pspec);

  pspec = g_param_spec_float ("left",
                              "Left",
                              "Left offset",
                              0, G_MAXFLOAT,
                              0,
                              MX_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_LEFT, pspec);

  pspec = g_param_spec_float ("top",
                              "Top",
                              "Top offset",
                              0, G_MAXFLOAT,
                              0,
                              MX_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_TOP, pspec);

  pspec = g_param_spec_float ("bottom",
                              "Bottom",
                              "Bottom offset",
                              0, G_MAXFLOAT,
                              0,
                              MX_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_BOTTOM, pspec);

  pspec = g_param_spec_float ("right",
                              "Right",
                              "Right offset",
                              0, G_MAXFLOAT,
                              0,
                              MX_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_RIGHT, pspec);
}

static void
mx_texture_frame_init (MxTextureFrame *self)
{
  self->priv = MX_TEXTURE_FRAME_GET_PRIVATE (self);
}

/**
 * mx_texture_frame_new:
 * @texture: a #ClutterTexture or %NULL
 * @left: left margin preserving its content
 * @top: top margin preserving its content
 * @right: right margin preserving its content
 * @bottom: bottom margin preserving its content
 *
 * A #MxTextureFrame is a specialized texture that efficiently clones
 * an area of the given @texture while keeping preserving portions of the
 * same texture.
 *
 * A #MxTextureFrame can be used to make a rectangular texture fit a
 * given size without stretching its borders.
 *
 * Return value: the newly created #MxTextureFrame
 */
ClutterActor*
mx_texture_frame_new (ClutterTexture *texture,
                      gfloat          top,
                      gfloat          right,
                      gfloat          bottom,
                      gfloat          left)
{
  g_return_val_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture), NULL);

  return g_object_new (MX_TYPE_TEXTURE_FRAME,
                       "parent-texture", texture,
                       "top", top,
                       "right", right,
                       "bottom", bottom,
                       "left", left,
                       NULL);
}

/**
 * mx_texture_frame_get_parent_texture:
 * @frame: A #MxTextureFrame
 *
 * Return the texture used by the #MxTextureFrame
 *
 * Returns: (transfer none): a #ClutterTexture owned by the #MxTextureFrame
 */
ClutterTexture *
mx_texture_frame_get_parent_texture (MxTextureFrame *frame)
{
  g_return_val_if_fail (MX_IS_TEXTURE_FRAME (frame), NULL);

  return frame->priv->parent_texture;
}

/**
 * mx_texture_frame_set_parent_texture:
 * @frame: A #MxTextureFrame
 * @texture: A #ClutterTexture
 *
 * Set the #ClutterTexture used by this #MxTextureFrame
 *
 */
void
mx_texture_frame_set_parent_texture (MxTextureFrame *frame,
                                     ClutterTexture *texture)
{
  MxTextureFramePrivate *priv;
  gboolean was_visible;

  g_return_if_fail (MX_IS_TEXTURE_FRAME (frame));
  g_return_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture));

  priv = frame->priv;

  was_visible = CLUTTER_ACTOR_IS_VISIBLE (frame);

  if (priv->parent_texture == texture)
    return;

  if (priv->parent_texture)
    {
      g_object_unref (priv->parent_texture);
      priv->parent_texture = NULL;

      if (was_visible)
        clutter_actor_hide (CLUTTER_ACTOR (frame));
    }

  if (texture)
    {
      CoglHandle cogl_material = COGL_INVALID_HANDLE;

      priv->parent_texture = g_object_ref_sink (texture);

      if (was_visible && CLUTTER_ACTOR_IS_VISIBLE (priv->parent_texture))
        clutter_actor_show (CLUTTER_ACTOR (frame));

      /* set the wrap mode */
      cogl_material = clutter_texture_get_cogl_material (priv->parent_texture);
      cogl_material_set_layer_wrap_mode (cogl_material, 0,
                                         COGL_MATERIAL_WRAP_MODE_REPEAT);


      /* The default filter can pull from adjacent pixels which is not what we
       * want.
       */
      cogl_material_set_layer_filters (cogl_material,
                                       0,
                                       COGL_MATERIAL_FILTER_NEAREST,
                                       COGL_MATERIAL_FILTER_NEAREST);


    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (frame));

  g_object_notify (G_OBJECT (frame), "parent-texture");
}

/**
 * mx_texture_frame_set_border_values:
 * @frame: A #MxTextureFrame
 * @top: width of the top slice
 * @right: width of the right slice
 * @bottom: width of the bottom slice
 * @left: width of the left slice
 *
 * Set the slice lines of the specified frame. The slices are calculated as
 * widths from the edge of the frame.
 *
 */
void
mx_texture_frame_set_border_values (MxTextureFrame *frame,
                                    gfloat          top,
                                    gfloat          right,
                                    gfloat          bottom,
                                    gfloat          left)
{
  g_return_if_fail (MX_IS_TEXTURE_FRAME (frame));

  mx_texture_frame_set_frame_internal (frame, top, right, bottom, left);
}

/**
 * mx_texture_frame_get_border_values:
 * @frame: A #MxTextureFrame
 * @top: width of the top slice
 * @right: width of the right slice
 * @bottom: width of the bottom slice
 * @left: width of the left slice
 *
 * Retrieve the current slice lines from the specified frame.
 *
 */
void
mx_texture_frame_get_border_values (MxTextureFrame *frame,
                                    gfloat         *top,
                                    gfloat         *right,
                                    gfloat         *bottom,
                                    gfloat         *left)
{
  MxTextureFramePrivate *priv;

  g_return_if_fail (MX_IS_TEXTURE_FRAME (frame));

  priv = frame->priv;

  if (top)
    *top = priv->top;

  if (right)
    *right = priv->right;

  if (bottom)
    *bottom = priv->bottom;

  if (left)
    *left = priv->left;
}
