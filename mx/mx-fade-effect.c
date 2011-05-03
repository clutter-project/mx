/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-fade-effect.c: A border-fading offscreen effect
 *
 * Copyright 2011 Intel Corporation
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:mx-fade-effect
 * @short_description: An effect used to fade the borders of actors
 *
 * #MxFadeEffect is a #ClutterEffect that can be used to fade the borders
 * of a #ClutterActor. It provides a configurable bounding box, border
 * size and colour to control the fading effect.
 *
 * Since: 1.2
 */

#include "mx-fade-effect.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxFadeEffect, mx_fade_effect, CLUTTER_TYPE_OFFSCREEN_EFFECT)

#define FADE_EFFECT_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_FADE_EFFECT, MxFadeEffectPrivate))


enum
{
  PROP_0,

  PROP_BOUNDS_X,
  PROP_BOUNDS_Y,
  PROP_BOUNDS_WIDTH,
  PROP_BOUNDS_HEIGHT,

  PROP_BORDER_TOP,
  PROP_BORDER_RIGHT,
  PROP_BORDER_BOTTOM,
  PROP_BORDER_LEFT,

  PROP_COLOR,

  PROP_FREEZE_UPDATE
};

struct _MxFadeEffectPrivate
{
  gint          x;
  gint          y;
  guint         bounds_width;
  guint         bounds_height;

  guint         border[4];
  ClutterColor  color;
  gfloat        width;
  gfloat        height;

  CoglHandle    vbo;
  CoglHandle    indices;
  guint         n_quads;

  CoglMaterial *old_material;

  gulong        blocked_id;

  gfloat        x_offset;
  gfloat        y_offset;

  guint         update_vbo    : 1;
  guint         freeze_update : 1;
};

static void
mx_fade_effect_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MxFadeEffectPrivate *priv = MX_FADE_EFFECT (object)->priv;

  switch (property_id)
    {
    case PROP_BOUNDS_X:
      g_value_set_int (value, priv->x);
      break;

    case PROP_BOUNDS_Y:
      g_value_set_int (value, priv->y);
      break;

    case PROP_BOUNDS_WIDTH:
      g_value_set_uint (value, priv->bounds_width);
      break;

    case PROP_BOUNDS_HEIGHT:
      g_value_set_uint (value, priv->bounds_height);
      break;

    case PROP_BORDER_TOP:
      g_value_set_uint (value, priv->border[0]);
      break;

    case PROP_BORDER_RIGHT:
      g_value_set_uint (value, priv->border[1]);
      break;

    case PROP_BORDER_BOTTOM:
      g_value_set_uint (value, priv->border[2]);
      break;

    case PROP_BORDER_LEFT:
      g_value_set_uint (value, priv->border[3]);
      break;

    case PROP_COLOR:
      clutter_value_set_color (value, &priv->color);
      break;

    case PROP_FREEZE_UPDATE:
      g_value_set_boolean (value, priv->freeze_update);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_fade_effect_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  MxFadeEffect *effect = MX_FADE_EFFECT (object);
  MxFadeEffectPrivate *priv = effect->priv;

  switch (property_id)
    {
    case PROP_BOUNDS_X:
      priv->x = g_value_get_int (value);
      break;

    case PROP_BOUNDS_Y:
      priv->y = g_value_get_int (value);
      break;

    case PROP_BOUNDS_WIDTH:
      priv->bounds_width = g_value_get_uint (value);
      break;

    case PROP_BOUNDS_HEIGHT:
      priv->bounds_height = g_value_get_uint (value);
      break;

    case PROP_BORDER_TOP:
      priv->border[0] = g_value_get_uint (value);
      break;

    case PROP_BORDER_RIGHT:
      priv->border[1] = g_value_get_uint (value);
      break;

    case PROP_BORDER_BOTTOM:
      priv->border[2] = g_value_get_uint (value);
      break;

    case PROP_BORDER_LEFT:
      priv->border[3] = g_value_get_uint (value);
      break;

    case PROP_COLOR:
      priv->color = *(clutter_value_get_color (value));
      break;

    case PROP_FREEZE_UPDATE:
      priv->freeze_update = g_value_get_boolean (value);
      return;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      return;
    }

  priv->update_vbo = TRUE;
}

static void
mx_fade_effect_dispose (GObject *object)
{
  MxFadeEffectPrivate *priv = MX_FADE_EFFECT (object)->priv;

  if (priv->vbo)
    {
      cogl_handle_unref (priv->vbo);
      priv->vbo = NULL;
    }

  if (priv->blocked_id)
    {
      ClutterActor *actor =
        clutter_actor_meta_get_actor (CLUTTER_ACTOR_META (object));

      g_signal_handler_disconnect (actor, priv->blocked_id);
      priv->blocked_id = 0;
    }

  G_OBJECT_CLASS (mx_fade_effect_parent_class)->dispose (object);
}

static void
mx_fade_effect_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_fade_effect_parent_class)->finalize (object);
}

static CoglHandle
mx_fade_effect_create_texture (ClutterOffscreenEffect *effect,
                               gfloat                  width,
                               gfloat                  height)
{
  MxFadeEffectPrivate *priv = MX_FADE_EFFECT (effect)->priv;

  priv->width = width;
  priv->height = height;
  priv->update_vbo = TRUE;

  return CLUTTER_OFFSCREEN_EFFECT_CLASS (mx_fade_effect_parent_class)->
    create_texture (effect, width, height);
}

static void
mx_fade_effect_paint_cb (ClutterActor *actor,
                         MxFadeEffect *self)
{
  MxFadeEffectPrivate *priv = self->priv;

  g_signal_stop_emission (actor,
                          g_signal_lookup ("paint", CLUTTER_TYPE_ACTOR), 0);

  g_signal_handler_disconnect (actor, priv->blocked_id);
  priv->blocked_id = 0;
}

static gboolean
mx_fade_effect_pre_paint (ClutterEffect *effect)
{
  MxFadeEffectPrivate *priv = MX_FADE_EFFECT (effect)->priv;

  if (!priv->freeze_update)
    {
      return CLUTTER_EFFECT_CLASS (mx_fade_effect_parent_class)->
        pre_paint (effect);
    }
  else
    {
      ClutterActorBox box;
      ClutterActor *actor =
        clutter_actor_meta_get_actor (CLUTTER_ACTOR_META (effect));

      /* Store the stage coordinates of the actor for when we post-paint */
      clutter_actor_get_paint_box (actor, &box);
      clutter_actor_box_get_origin (&box, &priv->x_offset, &priv->y_offset);

      /* Connect to the paint signal so we can block it */
      priv->blocked_id =
        g_signal_connect (actor, "paint",
                          G_CALLBACK (mx_fade_effect_paint_cb), effect);

      return TRUE;
    }
}

static void
mx_fade_effect_post_paint (ClutterEffect *effect)
{
  MxFadeEffectPrivate *priv = MX_FADE_EFFECT (effect)->priv;

  if (!priv->freeze_update)
    CLUTTER_EFFECT_CLASS (mx_fade_effect_parent_class)->post_paint (effect);
  else
    {
      CoglMatrix modelview;
      ClutterActor *actor, *stage;

      actor = clutter_actor_meta_get_actor (CLUTTER_ACTOR_META (effect));
      stage = clutter_actor_get_stage (actor);

      /* Set up the draw matrix so we draw the offscreen texture at the
       * absolute coordinates of the actor-box. We need to do this to
       * avoid transforming by the actor matrix twice, as when it's drawn
       * into the offscreen surface, it'll already be transformed.
       */
      cogl_push_matrix ();

      cogl_matrix_init_identity (&modelview);
      CLUTTER_ACTOR_CLASS (G_OBJECT_GET_CLASS (stage))->
        apply_transform (stage, &modelview);
      cogl_matrix_translate (&modelview, priv->x_offset, priv->y_offset, 0.f);
      cogl_set_modelview_matrix (&modelview);

      clutter_offscreen_effect_paint_target (CLUTTER_OFFSCREEN_EFFECT (effect));

      cogl_pop_matrix ();
    }
}

static void
mx_fade_effect_draw_rect (CoglTextureVertex *verts,
                          gfloat             x1,
                          gfloat             y1,
                          gfloat             x2,
                          gfloat             y2,
                          gfloat             width,
                          gfloat             height,
                          CoglColor         *color1,
                          CoglColor         *color2,
                          CoglColor         *color3,
                          CoglColor         *color4,
                          gboolean           clockwise)
{
  gint i, a;
  gfloat tx1, ty1, tx2, ty2;

  /* The Cogl rectangle drawing functions don't allow you to alter
   * the material colour, so we use cogl_polygon and this convenience
   * function instead.
   */
  tx1 = x1 / width;
  ty1 = y1 / height;
  tx2 = x2 / width;
  ty2 = y2 / height;

  if (clockwise)
    {
      i = 3;
      a = -1;
    }
  else
    {
      i = 0;
      a = 1;
    }

  verts[i].x = x1;
  verts[i].y = y1;
  verts[i].z = 0;
  verts[i].tx = tx1;
  verts[i].ty = ty1;
  verts[i].color = *color1;

  i += a;

  verts[i].x = x1;
  verts[i].y = y2;
  verts[i].z = 0;
  verts[i].tx = tx1;
  verts[i].ty = ty2;
  verts[i].color = *color4;

  i += a;

  verts[i].x = x2;
  verts[i].y = y2;
  verts[i].z = 0;
  verts[i].tx = tx2;
  verts[i].ty = ty2;
  verts[i].color = *color3;

  i += a;

  verts[i].x = x2;
  verts[i].y = y1;
  verts[i].z = 0;
  verts[i].tx = tx2;
  verts[i].ty = ty1;
  verts[i].color = *color2;
}

static void
mx_fade_effect_update_vbo (MxFadeEffect *self)
{
  guint n_quads;
  gint bu, br, bb, bl;
  gfloat x1, y1, x2, y2;
  CoglColor opaque, color;
  CoglTextureVertex verts[9*4];

  MxFadeEffectPrivate *priv = self->priv;

  cogl_color_init_from_4ub (&opaque, 0xff, 0xff, 0xff, 0xff);
  cogl_color_init_from_4ub (&color,
                            priv->color.red,
                            priv->color.green,
                            priv->color.blue,
                            priv->color.alpha);

  /* Validate the bounds */
  x1 = priv->x;
  y1 = priv->y;
  x2 = x1 + (priv->bounds_width ? priv->bounds_width : priv->width);
  y2 = y1 + (priv->bounds_height ? priv->bounds_height : priv->height);

  if (x1 < 0)
    x1 = 0;
  if (x2 > priv->width)
    x2 = priv->width;
  if (y1 < 0)
    y1 = 0;
  if (y2 > priv->height)
    y2 = priv->height;

  /* Validate the border sizes */
  /* Note,
   *   bu = Border-up
   *   br = Border-right
   *   bb = Border-bottom
   *   bl = Border-left
   */
  bu = priv->border[0];
  br = priv->border[1];
  bb = priv->border[2];
  bl = priv->border[3];

  if (y1 + bu >= y2)
    bu = y2 - y1 - 1;
  if (x1 + bl >= x2)
    bl = x2 - x1 - 1;
  if (x2 - br <= x1 + bl)
    br = x2 - (x1 + bl) - 1;
  if (y2 - bb <= y1 + bu)
    bb = y2 - (y1 + bu) - 1;

  n_quads = 0;

  /* Generate the top-left square */
  if (bl && bu)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x1, y1,
                                x1 + bl, y1 + bu,
                                priv->width, priv->height,
                                &color, &color,
                                &opaque, &color,
                                FALSE);
      n_quads ++;
    }

  /* Generate the top-middle square */
  if (bu)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x1 + bl, y1,
                                x2 - br, y1 + bu,
                                priv->width, priv->height,
                                &color, &color,
                                &opaque, &opaque,
                                FALSE);
      n_quads ++;
    }

  /* Generate the top-right square */
  if (br && bu)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x2 - br, y1,
                                x2, y1 + bu,
                                priv->width, priv->height,
                                &color, &color,
                                &color, &opaque,
                                TRUE);
      n_quads ++;
    }

  /* Generate the left square */
  if (bl)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x1, y1 + bu,
                                x1 + bl, y2 - bb,
                                priv->width, priv->height,
                                &color, &opaque,
                                &opaque, &color,
                                TRUE);
      n_quads ++;
    }

  /* Generate the middle square */
  mx_fade_effect_draw_rect (&verts[n_quads*4],
                            x1 + bl, y1 + bu,
                            x2 - br, y2 - bb,
                            priv->width, priv->height,
                            &opaque, &opaque,
                            &opaque, &opaque,
                            TRUE);
  n_quads ++;

  /* Generate the right square */
  if (br)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x2 - br, y1 + bu,
                                x2, y2 - bb,
                                priv->width, priv->height,
                                &opaque, &color,
                                &color, &opaque,
                                TRUE);
      n_quads ++;
    }

  /* Generate the bottom-left square */
  if (bb && bl)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x1, y2 - bb,
                                x1 + bl, y2,
                                priv->width, priv->height,
                                &color, &opaque,
                                &color, &color,
                                TRUE);
      n_quads ++;
    }

  /* Generate the bottom-middle square */
  if (bb)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x1 + bl, y2 - bb,
                                x2 - br, y2,
                                priv->width, priv->height,
                                &opaque, &opaque,
                                &color, &color,
                                FALSE);
      n_quads ++;
    }

  /* Generate the bottom-right square */
  if (bb && br)
    {
      mx_fade_effect_draw_rect (&verts[n_quads*4],
                                x2 - br, y2 - bb,
                                x2, y2,
                                priv->width, priv->height,
                                &opaque, &color,
                                &color, &color,
                                FALSE);
      n_quads ++;
    }

  /* Unref the old vbo if it's a different size - otherwise we reuse it */
  if (priv->vbo && (n_quads != priv->n_quads))
    {
      cogl_handle_unref (priv->vbo);
      priv->vbo = NULL;
    }

  priv->n_quads = n_quads;

  if (!priv->vbo)
    {
      priv->vbo = cogl_vertex_buffer_new (n_quads * 4);
      if (!priv->vbo)
        return;

      priv->indices = cogl_vertex_buffer_indices_get_for_quads (n_quads * 6);
      if (!priv->indices)
        return;
    }

  cogl_vertex_buffer_add (priv->vbo,
                          "gl_Vertex",
                          2,
                          COGL_ATTRIBUTE_TYPE_FLOAT,
                          FALSE,
                          sizeof (CoglTextureVertex),
                          &(verts[0].x));
  cogl_vertex_buffer_add (priv->vbo,
                          "gl_MultiTexCoord0",
                          2,
                          COGL_ATTRIBUTE_TYPE_FLOAT,
                          FALSE,
                          sizeof (CoglTextureVertex),
                          &(verts[0].tx));
  cogl_vertex_buffer_add (priv->vbo,
                          "gl_Color",
                          4,
                          COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE,
                          FALSE,
                          sizeof (CoglTextureVertex),
                          &(verts[0].color));

  cogl_vertex_buffer_submit (priv->vbo);
  priv->update_vbo = FALSE;
}

static void
mx_fade_effect_paint_target (ClutterOffscreenEffect *effect)
{
  guint8 opacity;
  CoglColor color;
  ClutterActor *actor;

  CoglMaterial *material = clutter_offscreen_effect_get_target (effect);
  MxFadeEffect *self = MX_FADE_EFFECT (effect);
  MxFadeEffectPrivate *priv = self->priv;

  if (priv->update_vbo)
    mx_fade_effect_update_vbo (self);

  if (!priv->vbo || !priv->indices || !material)
    return;

  /* Set the blend string if the material has changed so we can blend with
   * the paint opacity.
   */
  if (material != priv->old_material)
    {
      GError *error = NULL;
      priv->old_material = material;

      if (!cogl_material_set_layer_combine (material, 1,
                                            "RGBA = MODULATE(PREVIOUS,CONSTANT)", &error))
        {
          g_warning (G_STRLOC ": Error setting layer combine blend string: %s",
                     error->message);
          g_error_free (error);
        }
    }

  /* Set the layer-combine constant so the texture is blended with the paint
   * opacity when painted.
   */
  actor = clutter_actor_meta_get_actor (CLUTTER_ACTOR_META (effect));
  opacity = clutter_actor_get_paint_opacity (actor);
  cogl_color_init_from_4ub (&color, opacity, opacity, opacity, opacity);
  cogl_material_set_layer_combine_constant (material, 1, &color);

  /* Draw the texture */
  cogl_set_source (material);
  cogl_vertex_buffer_draw_elements (priv->vbo,
                                    COGL_VERTICES_MODE_TRIANGLES,
                                    priv->indices,
                                    0,
                                    (priv->n_quads * 4) - 1,
                                    0,
                                    priv->n_quads * 6);
}

static void
mx_fade_effect_class_init (MxFadeEffectClass *klass)
{
  GParamSpec *pspec;

  ClutterColor transparent = { 0, };
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterEffectClass *effect_class = CLUTTER_EFFECT_CLASS (klass);
  ClutterOffscreenEffectClass *offscreen_class =
    CLUTTER_OFFSCREEN_EFFECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFadeEffectPrivate));

  object_class->get_property = mx_fade_effect_get_property;
  object_class->set_property = mx_fade_effect_set_property;
  object_class->dispose = mx_fade_effect_dispose;
  object_class->finalize = mx_fade_effect_finalize;

  effect_class->pre_paint = mx_fade_effect_pre_paint;
  effect_class->post_paint = mx_fade_effect_post_paint;

  offscreen_class->create_texture = mx_fade_effect_create_texture;
  offscreen_class->paint_target = mx_fade_effect_paint_target;

  pspec = g_param_spec_int ("bounds-x",
                            "Bounds X",
                            "X-coordinate of the texture bounding box",
                            -G_MAXINT, G_MAXINT, 0,
                            MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BOUNDS_X, pspec);

  pspec = g_param_spec_int ("bounds-y",
                            "Bounds Y",
                            "Y-coordinate of the texture bounding boy",
                            -G_MAXINT, G_MAXINT, 0,
                            MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BOUNDS_Y, pspec);

  pspec = g_param_spec_uint ("bounds-width",
                             "Bounds width",
                             "Width of the texture bounding box",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BOUNDS_WIDTH, pspec);

  pspec = g_param_spec_uint ("bounds-height",
                             "Bounds height",
                             "Height of the texture bounding box",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BOUNDS_HEIGHT, pspec);

  pspec = g_param_spec_uint ("border-top",
                             "Border top",
                             "Border at the top of the effect",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BORDER_TOP, pspec);

  pspec = g_param_spec_uint ("border-right",
                             "Border right",
                             "Border at the right of the effect",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BORDER_RIGHT, pspec);

  pspec = g_param_spec_uint ("border-bottom",
                             "Border bottom",
                             "Border at the bottom of the effect",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BORDER_BOTTOM, pspec);

  pspec = g_param_spec_uint ("border-left",
                             "Border left",
                             "Border at the left of the effect",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_BORDER_LEFT, pspec);

  pspec = clutter_param_spec_color ("color",
                                    "Color",
                                    "Color of the faded border",
                                    &transparent,
                                    MX_PARAM_READWRITE |
                                    MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_COLOR, pspec);

  pspec = g_param_spec_boolean ("freeze-update",
                                "Freeze update",
                                "Stop updating the offscreen buffer",
                                FALSE,
                                MX_PARAM_READWRITE |
                                MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (object_class, PROP_FREEZE_UPDATE, pspec);
}

static void
mx_fade_effect_init (MxFadeEffect *self)
{
  self->priv = FADE_EFFECT_PRIVATE (self);
}

/**
 * mx_fade_effect_new:
 *
 * Creates a new #MxFadeEffect to be used with clutter_actor_add_effect().
 *
 * Returns: the newly created #MxFadeEffect, or %NULL
 *
 * Since: 1.2
 */
ClutterEffect *
mx_fade_effect_new (void)
{
  return g_object_new (MX_TYPE_FADE_EFFECT, NULL);
}

/**
 * mx_fade_effect_set_border:
 * @effect: A #MxFadeEffect
 * @top: The upper border, in pixels
 * @right: The right border, in pixels
 * @bottom: The lower border, in pixels
 * @left: The left border, in pixels
 *
 * Sets the border to be used for the fading effect. This is the number of
 * pixels on each side of the effect that should be used to fade.
 *
 * Since: 1.2
 */
void
mx_fade_effect_set_border (MxFadeEffect *effect,
                           guint         top,
                           guint         right,
                           guint         bottom,
                           guint         left)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;

  g_object_freeze_notify (G_OBJECT (effect));

  if (priv->border[0] != top)
    {
      priv->border[0] = top;
      g_object_notify (G_OBJECT (effect), "border-top");
    }

  if (priv->border[1] != right)
    {
      priv->border[1] = right;
      g_object_notify (G_OBJECT (effect), "border-right");
    }

  if (priv->border[2] != bottom)
    {
      priv->border[2] = bottom;
      g_object_notify (G_OBJECT (effect), "border-bottom");
    }

  if (priv->border[3] != left)
    {
      priv->border[3] = left;
      g_object_notify (G_OBJECT (effect), "border-left");
    }

  priv->update_vbo = TRUE;

  g_object_thaw_notify (G_OBJECT (effect));
}

/**
 * mx_fade_effect_get_border:
 * @effect: A #MxFadeEffect
 * @top: (out): The upper border, in pixels
 * @right: (out): The right border, in pixels
 * @bottom: (out): The lower border, in pixels
 * @left: (out): The left border, in pixels
 *
 * Retrieves the border values for @effect.
 *
 * Since: 1.2
 */
void
mx_fade_effect_get_border (MxFadeEffect *effect,
                           guint        *top,
                           guint        *right,
                           guint        *bottom,
                           guint        *left)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;

  if (top)
    *top = priv->border[0];
  if (right)
    *right = priv->border[1];
  if (bottom)
    *bottom = priv->border[2];
  if (left)
    *left = priv->border[3];
}

/**
 * mx_fade_effect_set_color:
 * @effect: A #MxFadeEffect
 * @color: A #ClutterColor
 *
 * Sets the color of the fade effect. The effect will fade out towards
 * the set border to this color.
 *
 * Since: 1.2
 */
void
mx_fade_effect_set_color (MxFadeEffect       *effect,
                          const ClutterColor *color)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;
  if (!clutter_color_equal (&priv->color, color))
    {
      priv->color = *color;
      priv->update_vbo = TRUE;
      g_object_notify (G_OBJECT (effect), "color");
    }
}

/**
 * mx_fade_effect_get_color:
 * @effect: A #MxFadeEffect
 * @color: (out): A #ClutterColor to store the color in
 *
 * Retrieves the color used for the fade effect.
 *
 * Since: 1.2
 */
void
mx_fade_effect_get_color (MxFadeEffect *effect,
                          ClutterColor *color)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;
  if (color)
    *color = priv->color;
}

/**
 * mx_fade_effect_set_bounds:
 * @effect: A #MxFadeEffect
 * @x: The x value of the effect bounds, in pixels
 * @y: The y value of the effect bounds, in pixels
 * @width: The width of the effect bounds, in pixels, or %0
 * @height: The height of the effect bounds, in pixels, or %0
 *
 * Sets the bounding box of the effect. The effect will essentially treat
 * this box as a clipping rectangle. Setting width or height to %0 will
 * use the width or height of the #ClutterActor the effect is attached to.
 *
 * <note><para>
 * The effect border will apply to the bounds, and not to the un-altered
 * rectangle, so an effect with an %x of %5 and a %left-border of %5 will
 * have a gap of 5 blank pixels to the left, with a fade length of 5 pixels.
 * </para></note>
 *
 * Since: 1.2
 */
void
mx_fade_effect_set_bounds (MxFadeEffect *effect,
                           gint          x,
                           gint          y,
                           guint         width,
                           guint         height)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;

  g_object_freeze_notify (G_OBJECT (effect));

  if (priv->x != x)
    {
      priv->x = x;
      g_object_notify (G_OBJECT (effect), "bounds-x");
    }

  if (priv->y != y)
    {
      priv->y = y;
      g_object_notify (G_OBJECT (effect), "bounds-y");
    }

  if (priv->bounds_width != width)
    {
      priv->bounds_width = width;
      g_object_notify (G_OBJECT (effect), "bounds-width");
    }

  if (priv->bounds_height != height)
    {
      priv->bounds_height = height;
      g_object_notify (G_OBJECT (effect), "bounds-height");
    }

  priv->update_vbo = TRUE;

  g_object_thaw_notify (G_OBJECT (effect));
}

/**
 * mx_fade_effect_get_bounds:
 * @effect: A #MxFadeEffect
 * @x: (out): The x value of the effect bounds, in pixels
 * @y: (out): The y value of the effect bounds, in pixels
 * @width: (out): The width of the effect bounds, in pixels, or %0
 * @height: (out): The height of the effect bounds, in pixels, or %0
 *
 * Retrieves the bounding box of the effect.
 *
 * Since: 1.2
 */
void
mx_fade_effect_get_bounds (MxFadeEffect *effect,
                           gint         *x,
                           gint         *y,
                           guint        *width,
                           guint        *height)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;

  if (x)
    *x = priv->x;
  if (y)
    *y = priv->y;
  if (width)
    *width = priv->bounds_width;
  if (height)
    *height = priv->bounds_height;
}

/*
 * mx_fade_effect_set_freeze_update:
 * @effect: A #MxFadeEffect
 * @freeze: %TRUE to freeze updates, %FALSE to unfreeze them
 *
 * This will freeze the current image. Any further updates to the
 * #ClutterActor the effect is applied to will not be reflected until the
 * effect is unfrozen.
 *
 * <note><para>
 * Note, that this may conflict with other effects, and in such a situation,
 * should not be used.
 * </note></para>
 *
 * Since: 1.2
 */
void
_mx_fade_effect_set_freeze_update (MxFadeEffect *effect,
                                  gboolean      freeze)
{
  MxFadeEffectPrivate *priv;

  g_return_if_fail (MX_IS_FADE_EFFECT (effect));

  priv = effect->priv;
  if (priv->freeze_update != freeze)
    {
      priv->freeze_update = freeze;
      g_object_notify (G_OBJECT (effect), "freeze-update");
    }
}

/*
 * mx_fade_effect_get_freeze_update:
 * @effect: A #MxFadeEffect
 *
 * Determines if the #MxFadeEffect has had its updates frozen.
 *
 * Returns: %TRUE if updates are frozen, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
_mx_fade_effect_get_freeze_update (MxFadeEffect *effect)
{
  g_return_val_if_fail (MX_IS_FADE_EFFECT (effect), FALSE);
  return effect->priv->freeze_update;
}
