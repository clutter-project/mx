/*
 * mx-deform-bow-tie.h: A bow-tie deformation actor
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#include "mx-deform-bow-tie.h"
#include "mx-private.h"

#include <math.h>

G_DEFINE_TYPE (MxDeformBowTie, mx_deform_bow_tie, MX_TYPE_DEFORM_TEXTURE)

#define DEFORM_BOW_TIE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                   MX_TYPE_DEFORM_BOW_TIE, MxDeformBowTiePrivate))

struct _MxDeformBowTiePrivate
{
  gdouble period;
  ClutterTexture *back;
  gboolean flip_back;
  gulong back_id;
};

enum
{
  PROP_0,

  PROP_PERIOD,
  PROP_FLIP_BACK
};

static void
mx_deform_bow_tie_texture_vflip (ClutterTexture *texture)
{
  CoglHandle material;

  material = clutter_texture_get_cogl_material (texture);

  if (material)
    {
      CoglMatrix matrix;
      cogl_matrix_init_identity (&matrix);

      /* Vflip */
      cogl_matrix_scale (&matrix, 1.f, -1.f, 1.f);
      cogl_matrix_translate (&matrix, 0.f, 1.f, 0.f);

      cogl_material_set_layer_matrix (material, 0, &matrix);
    }
}

static void
mx_deform_bow_tie_texture_reset (ClutterTexture *texture)
{
  CoglHandle material;

  material = clutter_texture_get_cogl_material (texture);

  if (material)
    {
      CoglMatrix matrix;
      cogl_matrix_init_identity (&matrix);
      cogl_material_set_layer_matrix (material, 0, &matrix);
    }
}

static void
mx_deform_bow_tie_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  MxDeformBowTiePrivate *priv = MX_DEFORM_BOW_TIE (object)->priv;

  switch (property_id)
    {
    case PROP_PERIOD:
      g_value_set_double (value, priv->period);
      break;

    case PROP_FLIP_BACK:
      g_value_set_boolean (value, priv->flip_back);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_bow_tie_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_PERIOD:
      mx_deform_bow_tie_set_period (MX_DEFORM_BOW_TIE (object),
                                    g_value_get_double (value));
      break;

    case PROP_FLIP_BACK:
      mx_deform_bow_tie_set_flip_back (MX_DEFORM_BOW_TIE (object),
                                       g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
mx_deform_bow_tie_deform (MxDeformTexture   *texture,
                          CoglTextureVertex *vertex,
                          gfloat             width,
                          gfloat             height)
{
  gfloat cx, cy, rx, ry, turn_angle, height_radius;
  guint shade;

  MxDeformBowTiePrivate *priv = ((MxDeformBowTie *)texture)->priv;

  cx = priv->period * (width + width/2);
  cy = height/2;

  rx = ((vertex->x - cx) * cos (0)) -
       ((vertex->y - cy) * sin (0));
  ry = ((vertex->x - cx) * sin (0)) +
       ((vertex->y - cy) * cos (0));

  /* Make angle as a function of distance from the curl ray */
  turn_angle = MAX (-G_PI, MIN (0, (rx / (width/4)) * G_PI_2));

  /* Add a gradient that makes it look like lighting */
  shade = (cos (turn_angle * 2) * 96) + 159;
  cogl_color_set_from_4ub (&vertex->color, shade, shade, shade, 0xff);

  /* Calculate the point on a cone (note, a cone, not a right cone) */
  height_radius = ry;
  /*ClutterFixed height_radius =
    clutter_qmulx (clutter_qdivx (ry, height/2), height/2);*/

  ry = height_radius * cos (turn_angle);
  vertex->x = (rx * cos (0)) - (ry * sin (0)) + cx;
  vertex->y = (rx * sin (0)) + (ry * cos (0)) + cy;
  vertex->z = height_radius * sin (turn_angle);
}

static void
mx_deform_bow_tie_back_notify (MxDeformBowTie *self,
                               GParamSpec     *pspec)
{
  MxDeformBowTiePrivate *priv = self->priv;

  if (priv->back_id && priv->back)
    {
      g_signal_handler_disconnect (priv->back, priv->back_id);
      priv->back_id = 0;
    }

  if (priv->back)
    {
      if (priv->flip_back)
        mx_deform_bow_tie_texture_reset (priv->back);

      g_object_remove_weak_pointer (G_OBJECT (priv->back),
                                    (gpointer *)&priv->back);
      priv->back = NULL;
    }

  mx_deform_texture_get_textures (MX_DEFORM_TEXTURE (self), NULL, &priv->back);

  if (priv->back)
    {
      g_object_add_weak_pointer (G_OBJECT (priv->back),
                                 (gpointer)&priv->back);

      if (priv->flip_back)
        {
          priv->back_id =
            g_signal_connect (priv->back, "notify::cogl-texture",
                              G_CALLBACK (mx_deform_bow_tie_texture_vflip),
                              self);
          mx_deform_bow_tie_texture_vflip (priv->back);
        }
    }
}

static void
mx_deform_bow_tie_dispose (GObject *object)
{
  MxDeformBowTiePrivate *priv = MX_DEFORM_BOW_TIE (object)->priv;

  if (priv->back_id && priv->back)
    {
      g_signal_handler_disconnect (priv->back, priv->back_id);
      priv->back_id = 0;
    }

  if (priv->back)
    {
      CoglHandle material;

      /* Reset layer matrix */
      material = clutter_texture_get_cogl_material (priv->back);

      if (material)
        {
          CoglMatrix matrix;
          cogl_matrix_init_identity (&matrix);
          cogl_material_set_layer_matrix (material, 0, &matrix);
        }

      g_object_remove_weak_pointer (G_OBJECT (priv->back),
                                    (gpointer *)&priv->back);
      priv->back = NULL;
    }

  G_OBJECT_CLASS (mx_deform_bow_tie_parent_class)->dispose (object);
}

static void
mx_deform_bow_tie_class_init (MxDeformBowTieClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MxDeformTextureClass *deform_class = MX_DEFORM_TEXTURE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDeformBowTiePrivate));

  object_class->get_property = mx_deform_bow_tie_get_property;
  object_class->set_property = mx_deform_bow_tie_set_property;
  object_class->dispose = mx_deform_bow_tie_dispose;

  deform_class->deform = mx_deform_bow_tie_deform;

  pspec = g_param_spec_double ("period",
                               "Period",
                               "Effect period",
                               0.0, 1.0, 0.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PERIOD, pspec);

  pspec = g_param_spec_boolean ("flip-back",
                                "Flip back-face",
                                "Apply a vertical flip transformation to the "
                                "back face.",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PERIOD, pspec);
}

static void
mx_deform_bow_tie_init (MxDeformBowTie *self)
{
  self->priv = DEFORM_BOW_TIE_PRIVATE (self);
  self->priv->flip_back = TRUE;
  g_signal_connect (self, "notify::back",
                    G_CALLBACK (mx_deform_bow_tie_back_notify), NULL);
}

ClutterActor *
mx_deform_bow_tie_new (void)
{
  return g_object_new (MX_TYPE_DEFORM_BOW_TIE, NULL);
}

gdouble
mx_deform_bow_tie_get_period (MxDeformBowTie *bow_tie)
{
  g_return_val_if_fail (MX_IS_DEFORM_BOW_TIE (bow_tie), 0.0);
  return bow_tie->priv->period;
}

void
mx_deform_bow_tie_set_period (MxDeformBowTie *bow_tie,
                              gdouble         period)
{
  g_return_if_fail (MX_IS_DEFORM_BOW_TIE (bow_tie));

  if (bow_tie->priv->period != period)
    {
      bow_tie->priv->period = period;
      g_object_notify (G_OBJECT (bow_tie), "period");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (bow_tie));
    }
}

gboolean
mx_deform_bow_tie_get_flip_back (MxDeformBowTie *bow_tie)
{
  g_return_val_if_fail (MX_IS_DEFORM_BOW_TIE (bow_tie), FALSE);
  return bow_tie->priv->flip_back;
}

void
mx_deform_bow_tie_set_flip_back (MxDeformBowTie *bow_tie,
                                 gboolean        flip_back)
{
  MxDeformBowTiePrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_BOW_TIE (bow_tie));

  priv = bow_tie->priv;

  if (priv->flip_back != flip_back)
    {
      priv->flip_back = flip_back;
      if (priv->back)
        {
          if (priv->back_id)
            g_signal_handler_disconnect (priv->back, priv->back_id);

          if (flip_back)
            {
              priv->back_id =
                g_signal_connect (priv->back, "notify::cogl-texture",
                                  G_CALLBACK (mx_deform_bow_tie_texture_vflip),
                                  bow_tie);
              mx_deform_bow_tie_texture_vflip (priv->back);
            }
          else
            {
              mx_deform_bow_tie_texture_reset (priv->back);
              priv->back_id = 0;
            }
        }
      g_object_notify (G_OBJECT (bow_tie), "flip-back");
    }
}

