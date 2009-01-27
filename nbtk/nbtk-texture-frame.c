/* nbtk-texture-frame.h: Expandible texture actor
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:nbtk-texture-frame
 * @short_description: Stretch a texture to fit the entire allocation
 *
 * #NbtkTextureFrame
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cogl/cogl.h>

#include "nbtk-texture-frame.h"
#include "nbtk-private.h"

enum
{
  PROP_0,

  PROP_PARENT_TEXTURE,

  PROP_LEFT,
  PROP_TOP,
  PROP_RIGHT,
  PROP_BOTTOM
};

G_DEFINE_TYPE (NbtkTextureFrame, nbtk_texture_frame, CLUTTER_TYPE_ACTOR);

#define NBTK_TEXTURE_FRAME_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_TEXTURE_FRAME, NbtkTextureFramePrivate))

struct _NbtkTextureFramePrivate
{
  ClutterTexture *parent_texture;

  gint left;
  gint top;
  gint right;
  gint bottom;

  CoglHandle material;
};

static void
nbtk_texture_frame_get_preferred_width (ClutterActor *self,
                                        ClutterUnit   for_height,
                                        ClutterUnit   *min_width_p,
                                        ClutterUnit   *natural_width_p)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (self)->priv;

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
nbtk_texture_frame_get_preferred_height (ClutterActor *self,
                                         ClutterUnit   for_width,
                                         ClutterUnit   *min_height_p,
                                         ClutterUnit   *natural_height_p)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (self)->priv;

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
nbtk_texture_frame_realize (ClutterActor *self)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (self)->priv;

  if (priv->material != COGL_INVALID_HANDLE)
    return;

  priv->material = cogl_material_new ();

  CLUTTER_ACTOR_SET_FLAGS (self, CLUTTER_ACTOR_REALIZED);
}

static void
nbtk_texture_frame_unrealize (ClutterActor *self)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (self)->priv;

  if (priv->material == COGL_INVALID_HANDLE)
    return;

  cogl_material_unref (priv->material);
  priv->material = COGL_INVALID_HANDLE;

  CLUTTER_ACTOR_UNSET_FLAGS (self, CLUTTER_ACTOR_REALIZED);
}

static void
nbtk_texture_frame_paint (ClutterActor *self)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (self)->priv;
  ClutterActorBox box = { 0, };
  gfloat tex_width, tex_height;
  gfloat ex, ey;
  gfloat tx1, ty1, tx2, ty2;
  CoglHandle cogl_texture;
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

  tex_width  = cogl_texture_get_width (cogl_texture);
  tex_height = cogl_texture_get_height (cogl_texture);

  clutter_actor_get_allocation_box (self, &box);

  tx1 = (gfloat) priv->left / tex_width;
  tx2 = (gfloat) (tex_width - priv->right) / tex_width;
  ty1 = (gfloat) priv->top / tex_height;
  ty2 = (gfloat) (tex_height - priv->bottom) / tex_height;

  ex = (box.x2 - box.x1) - priv->right;
  if (ex < 0)
    ex = priv->right; 		/* FIXME ? */

  ey = (box.y2 - box.y1) - priv->bottom;
  if (ey < 0)
    ey = priv->bottom; 		/* FIXME ? */

  opacity = clutter_actor_get_paint_opacity (self);

  g_assert (priv->material != COGL_INVALID_HANDLE);

  /* set the source material using the parent texture's COGL handle */
  cogl_material_set_color4ub (priv->material, 255, 255, 255, opacity);
  cogl_material_set_layer (priv->material, 0, cogl_texture);
  cogl_set_source (priv->material);

  /* top left corner */
  cogl_rectangle_with_texture_coords (0, 0,
                                      priv->left, /* FIXME: clip if smaller */
                                      priv->top,
                                      0.0, 0.0,
                                      tx1, ty1);

  /* top middle */
  cogl_rectangle_with_texture_coords (priv->left, 0,
                                      ex, priv->top,
                                      tx1, 0.0,
                                      tx2, ty1);

  /* top right */
  cogl_rectangle_with_texture_coords (ex, 0,
                                      (box.x2 - box.x1), priv->top,
                                      tx2, 0.0,
                                      1.0, ty1);

  /* mid left */
  cogl_rectangle_with_texture_coords (0, priv->top,
                                      priv->left,
                                      ey,
                                      0.0, ty1,
                                      tx1, ty2);

  /* center */
  cogl_rectangle_with_texture_coords (priv->left, priv->top,
                                      ex,
                                      ey,
                                      tx1, ty1,
                                      tx2, ty2);

  /* mid right */
  cogl_rectangle_with_texture_coords (ex, priv->top,
                                      (box.x2 - box.x1),
                                      ey,
                                      tx2, ty1,
                                      1.0, ty2);
  
  /* bottom left */
  cogl_rectangle_with_texture_coords (0, ey,
                                      priv->left,
                                      (box.y2 - box.y1),
                                      0.0, ty2,
                                      tx1, 1.0);

  /* bottom center */
  cogl_rectangle_with_texture_coords (priv->left, ey,
                                      ex,
                                      (box.y2 - box.y1),
                                      tx1, ty2,
                                      tx2, 1.0);

  /* bottom right */
  cogl_rectangle_with_texture_coords (ex, ey,
                                      (box.x2 - box.x1),
                                      (box.y2 - box.y1),
                                      tx2, ty2,
                                      1.0, 1.0);
}

static inline void
nbtk_texture_frame_set_frame_internal (NbtkTextureFrame *frame,
                                       gint              left,
                                       gint              top,
                                       gint              right,
                                       gint              bottom)
{
  NbtkTextureFramePrivate *priv = frame->priv;
  GObject *gobject = G_OBJECT (frame);
  gboolean changed = FALSE;

  g_object_freeze_notify (gobject);

  if (priv->left != left)
    {
      priv->left = left;
      g_object_notify (gobject, "left");
      changed = TRUE;
    }

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

  if (changed && CLUTTER_ACTOR_IS_VISIBLE (frame))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (frame));

  g_object_thaw_notify (gobject);
}

static void
nbtk_texture_frame_set_property (GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  NbtkTextureFrame *frame = NBTK_TEXTURE_FRAME (gobject);
  NbtkTextureFramePrivate *priv = frame->priv;

  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      nbtk_texture_frame_set_parent_texture (frame,
                                             g_value_get_object (value));
      break;

    case PROP_LEFT:
      nbtk_texture_frame_set_frame_internal (frame,
                                             g_value_get_int (value),
                                             priv->top,
                                             priv->right,
                                             priv->bottom);
      break;

    case PROP_TOP:
      nbtk_texture_frame_set_frame_internal (frame,
                                             priv->left,
                                             g_value_get_int (value),
                                             priv->right,
                                             priv->bottom);
      break;

    case PROP_RIGHT:
      nbtk_texture_frame_set_frame_internal (frame,
                                             priv->left,
                                             priv->top,
                                             g_value_get_int (value),
                                             priv->bottom);
      break;

    case PROP_BOTTOM:
      nbtk_texture_frame_set_frame_internal (frame,
                                             priv->left,
                                             priv->top,
                                             priv->right,
                                             g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_texture_frame_get_property (GObject    *gobject,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (gobject)->priv;

  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      g_value_set_object (value, priv->parent_texture);
      break;

    case PROP_LEFT:
      g_value_set_int (value, priv->left);
      break;

    case PROP_TOP:
      g_value_set_int (value, priv->top);
      break;

    case PROP_RIGHT:
      g_value_set_int (value, priv->right);
      break;

    case PROP_BOTTOM:
      g_value_set_int (value, priv->bottom);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_texture_frame_dispose (GObject *gobject)
{
  NbtkTextureFramePrivate *priv = NBTK_TEXTURE_FRAME (gobject)->priv;

  if (priv->parent_texture)
    {
      g_object_unref (priv->parent_texture);
      priv->parent_texture = NULL;
    }

  if (priv->material)
    {
      cogl_material_unref (priv->material);
      priv->material = COGL_INVALID_HANDLE;
    }

  G_OBJECT_CLASS (nbtk_texture_frame_parent_class)->dispose (gobject);
}

static void
nbtk_texture_frame_class_init (NbtkTextureFrameClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (gobject_class, sizeof (NbtkTextureFramePrivate));

  actor_class->get_preferred_width =
    nbtk_texture_frame_get_preferred_width;
  actor_class->get_preferred_height =
    nbtk_texture_frame_get_preferred_height;
  actor_class->realize = nbtk_texture_frame_realize;
  actor_class->unrealize = nbtk_texture_frame_unrealize;
  actor_class->paint = nbtk_texture_frame_paint;

  gobject_class->set_property = nbtk_texture_frame_set_property;
  gobject_class->get_property = nbtk_texture_frame_get_property;
  gobject_class->dispose = nbtk_texture_frame_dispose;

  pspec = g_param_spec_object ("parent-texture",
                               "Parent Texture",
                               "The parent ClutterTexture",
                               CLUTTER_TYPE_TEXTURE,
                               NBTK_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_PARENT_TEXTURE, pspec);

  pspec = g_param_spec_int ("left",
                            "Left",
                            "Left offset",
			    0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LEFT, pspec);

  pspec = g_param_spec_int ("top",
                            "Top",
                            "Top offset",
                            0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TOP, pspec);

  pspec = g_param_spec_int ("bottom",
                            "Bottom",
                            "Bottom offset",
                            0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_BOTTOM, pspec);

  pspec = g_param_spec_int ("right",
                            "Right",
                            "Right offset",
                            0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_RIGHT, pspec);
}

static void
nbtk_texture_frame_init (NbtkTextureFrame *self)
{
  NbtkTextureFramePrivate *priv;

  self->priv = priv = NBTK_TEXTURE_FRAME_GET_PRIVATE (self);

  priv->material = COGL_INVALID_HANDLE;
}

/**
 * nbtk_texture_frame_new:
 * @texture: a #ClutterTexture or %NULL
 * @left: left margin preserving its content
 * @top: top margin preserving its content
 * @right: right margin preserving its content
 * @bottom: bottom margin preserving its content
 *
 * A #NbtkTextureFrame is a specialized texture that efficiently clones
 * an area of the given @texture while keeping preserving portions of the
 * same texture.
 *
 * A #NbtkTextureFrame can be used to make a rectangular texture fit a
 * given size without stretching its borders.
 *
 * Return value: the newly created #NbtkTextureFrame
 */
ClutterActor*
nbtk_texture_frame_new (ClutterTexture *texture, 
			gint            left,
			gint            top,
			gint            right,
			gint            bottom)
{
  g_return_val_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture), NULL);

  return g_object_new (NBTK_TYPE_TEXTURE_FRAME,
 		       "parent-texture", texture,
		       "left", left,
		       "top", top,
		       "right", right,
		       "bottom", bottom,
		       NULL);
}

ClutterTexture *
nbtk_texture_frame_get_parent_texture (NbtkTextureFrame *frame)
{
  g_return_val_if_fail (NBTK_IS_TEXTURE_FRAME (frame), NULL);

  return frame->priv->parent_texture;
}

void
nbtk_texture_frame_set_parent_texture (NbtkTextureFrame *frame,
                                       ClutterTexture   *texture)
{
  NbtkTextureFramePrivate *priv;

  g_return_if_fail (NBTK_IS_TEXTURE_FRAME (frame));
  g_return_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture));

  priv = frame->priv;

  if (priv->parent_texture)
    {
      g_object_unref (priv->parent_texture);
      priv->parent_texture = NULL;
    }

  if (texture)
    priv->parent_texture = g_object_ref (texture);

  if (CLUTTER_ACTOR_IS_VISIBLE (frame))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (frame));

  g_object_notify (G_OBJECT (frame), "parent-texture");
}
