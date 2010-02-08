/*
 * mx-deform-cloth.h: A cloth deformation actor
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

#include "mx-deform-cloth.h"
#include "mx-private.h"

#include <math.h>

G_DEFINE_TYPE (MxDeformCloth, mx_deform_cloth, MX_TYPE_DEFORM_TEXTURE)

#define DEFORM_CLOTH_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_DEFORM_CLOTH, MxDeformClothPrivate))

struct _MxDeformClothPrivate
{
  gdouble period;
  gdouble angle;
  gdouble radius;
  gdouble amplitude;
};

enum
{
  PROP_0,

  PROP_PERIOD,
  PROP_ANGLE,
  PROP_RADIUS,
  PROP_AMPLITUDE
};

static void
mx_deform_cloth_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MxDeformClothPrivate *priv = MX_DEFORM_CLOTH (object)->priv;

  switch (property_id)
    {
    case PROP_PERIOD:
      g_value_set_double (value, priv->period);
      break;

    case PROP_ANGLE:
      g_value_set_double (value, priv->angle);
      break;

    case PROP_RADIUS:
      g_value_set_double (value, priv->radius);
      break;

    case PROP_AMPLITUDE:
      g_value_set_double (value, priv->amplitude);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_cloth_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  gdouble set_value;
  MxDeformTexture *texture = MX_DEFORM_TEXTURE (object);
  MxDeformClothPrivate *priv = MX_DEFORM_CLOTH (object)->priv;

  switch (property_id)
    {
    case PROP_PERIOD:
      set_value = g_value_get_double (value);
      if (priv->period != set_value)
        {
          priv->period = set_value;
          g_object_notify (object, "period");
          mx_deform_texture_invalidate (texture);
        }
      break;

    case PROP_ANGLE:
      set_value = g_value_get_double (value);
      if (priv->angle != set_value)
        {
          priv->angle = set_value;
          g_object_notify (object, "angle");
          mx_deform_texture_invalidate (texture);
        }
      break;

    case PROP_RADIUS:
      set_value = g_value_get_double (value);
      if (priv->radius != set_value)
        {
          priv->radius = set_value;
          g_object_notify (object, "radius");
          mx_deform_texture_invalidate (texture);
        }
      break;

    case PROP_AMPLITUDE:
      set_value = g_value_get_double (value);
      if (priv->amplitude != set_value)
        {
          priv->amplitude = set_value;
          g_object_notify (object, "amplitude");
          mx_deform_texture_invalidate (texture);
        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
mx_deform_cloth_deform (MxDeformTexture   *texture,
                        CoglTextureVertex *vertex,
                        gfloat             width,
                        gfloat             height)
{
  gfloat cx, cy, rx, turn_angle, height_radius;
  guint shade;

  MxDeformClothPrivate *priv = ((MxDeformCloth *)texture)->priv;

  /* Rotate the point around the centre of the curl ray to align it with
   * the y-axis.
   */

  cx = (1.f - priv->period) * width;
  cy = (1.f - priv->period) * height;

  rx = ((vertex->x - cx) * cos (-priv->angle)) -
       ((vertex->y - cy) * sin (-priv->angle)) - priv->radius;

  /* Calculate the angle as a function of the distance from the curl ray */
  turn_angle = ((rx / priv->radius) * G_PI_2) - G_PI_2;

  /* Add a gradient that makes it look like lighting and hides the switch
   * between textures.
   */
  shade = (255 * (1.f - priv->amplitude)) +
          (((sin (turn_angle) * 96) + 159) * priv->amplitude);
  vertex->color.red = shade;
  vertex->color.green = shade;
  vertex->color.blue = shade;

  /* Make the wave amplitude lower as its distance from the curl ray increases.
   * Not really necessary, but looks a little nicer I think.
   */
  height_radius = (1 - rx / width) * priv->radius;
  vertex->z = height_radius * sin (turn_angle) * priv->amplitude;
}

static void
mx_deform_cloth_class_init (MxDeformClothClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MxDeformTextureClass *deform_class = MX_DEFORM_TEXTURE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDeformClothPrivate));

  object_class->get_property = mx_deform_cloth_get_property;
  object_class->set_property = mx_deform_cloth_set_property;

  deform_class->deform = mx_deform_cloth_deform;

  pspec = g_param_spec_double ("period",
                               "Period",
                               "Effect period",
                               0.0, G_MAXDOUBLE, 0.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PERIOD, pspec);

  pspec = g_param_spec_double ("angle",
                               "Angle",
                               "Effect rotation angle",
                               0.0, G_PI * 2, 0.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ANGLE, pspec);

  pspec = g_param_spec_double ("radius",
                               "Radius",
                               "Cloth ripple radius",
                               G_MINDOUBLE, G_MAXDOUBLE, 32.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_RADIUS, pspec);

  pspec = g_param_spec_double ("amplitude",
                               "Amplitude",
                               "Effect amplitude",
                               0.0, G_MAXDOUBLE, 1.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_AMPLITUDE, pspec);
}

static void
mx_deform_cloth_init (MxDeformCloth *self)
{
  MxDeformClothPrivate *priv = self->priv = DEFORM_CLOTH_PRIVATE (self);

  priv->radius = 32.0;
  priv->amplitude = 1.0;
}

ClutterActor *
mx_deform_cloth_new (void)
{
  return g_object_new (MX_TYPE_DEFORM_CLOTH, NULL);
}
