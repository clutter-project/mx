/*
 * mx-deform-waves.h: A waves deformation actor
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

#include "mx-deform-waves.h"
#include "mx-private.h"

#include <math.h>

G_DEFINE_TYPE (MxDeformWaves, mx_deform_waves, MX_TYPE_DEFORM_TEXTURE)

#define DEFORM_WAVES_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_DEFORM_WAVES, MxDeformWavesPrivate))

struct _MxDeformWavesPrivate
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
mx_deform_waves_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MxDeformWavesPrivate *priv = MX_DEFORM_WAVES (object)->priv;

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
mx_deform_waves_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_PERIOD:
      mx_deform_waves_set_period (MX_DEFORM_WAVES (object),
                                  g_value_get_double (value));
      break;

    case PROP_ANGLE:
      mx_deform_waves_set_angle (MX_DEFORM_WAVES (object),
                                 g_value_get_double (value));
      break;

    case PROP_RADIUS:
      mx_deform_waves_set_radius (MX_DEFORM_WAVES (object),
                                  g_value_get_double (value));
      break;

    case PROP_AMPLITUDE:
      mx_deform_waves_set_amplitude (MX_DEFORM_WAVES (object),
                                     g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
mx_deform_waves_deform (MxDeformTexture   *texture,
                        CoglTextureVertex *vertex,
                        gfloat             width,
                        gfloat             height)
{
  gfloat cx, cy, rx, turn_angle, height_radius;
  guint shade;

  MxDeformWavesPrivate *priv = ((MxDeformWaves *)texture)->priv;

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
  cogl_color_set_from_4ub (&vertex->color, shade, shade, shade, 0xff);

  /* Make the wave amplitude lower as its distance from the curl ray increases.
   * Not really necessary, but looks a little nicer I think.
   */
  height_radius = (1 - rx / width) * priv->radius;
  vertex->z = height_radius * sin (turn_angle) * priv->amplitude;
}

static void
mx_deform_waves_class_init (MxDeformWavesClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MxDeformTextureClass *deform_class = MX_DEFORM_TEXTURE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDeformWavesPrivate));

  object_class->get_property = mx_deform_waves_get_property;
  object_class->set_property = mx_deform_waves_set_property;

  deform_class->deform = mx_deform_waves_deform;

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
                               "Wave ripple radius",
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
mx_deform_waves_init (MxDeformWaves *self)
{
  MxDeformWavesPrivate *priv = self->priv = DEFORM_WAVES_PRIVATE (self);

  priv->radius = 32.0;
  priv->amplitude = 1.0;
}

ClutterActor *
mx_deform_waves_new (void)
{
  return g_object_new (MX_TYPE_DEFORM_WAVES, NULL);
}

gdouble
mx_deform_waves_get_period (MxDeformWaves *waves)
{
  g_return_val_if_fail (MX_IS_DEFORM_WAVES (waves), 0.0);
  return waves->priv->period;
}

void
mx_deform_waves_set_period (MxDeformWaves *waves,
                            gdouble        period)
{
  MxDeformWavesPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_WAVES (waves));

  priv = waves->priv;

  if (priv->period != period)
    {
      priv->period = period;
      g_object_notify (G_OBJECT (waves), "period");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (waves));
    }
}

gdouble
mx_deform_waves_get_angle (MxDeformWaves *waves)
{
  g_return_val_if_fail (MX_IS_DEFORM_WAVES (waves), 0.0);
  return waves->priv->angle;
}

void
mx_deform_waves_set_angle (MxDeformWaves *waves,
                           gdouble        angle)
{
  MxDeformWavesPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_WAVES (waves));

  priv = waves->priv;

  if (priv->angle != angle)
    {
      priv->angle = angle;
      g_object_notify (G_OBJECT (waves), "angle");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (waves));
    }
}

gdouble
mx_deform_waves_get_radius (MxDeformWaves *waves)
{
  g_return_val_if_fail (MX_IS_DEFORM_WAVES (waves), 0.0);
  return waves->priv->radius;
}

void
mx_deform_waves_set_radius (MxDeformWaves *waves,
                            gdouble        radius)
{
  MxDeformWavesPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_WAVES (waves));

  priv = waves->priv;

  if (priv->radius != radius)
    {
      priv->radius = radius;
      g_object_notify (G_OBJECT (waves), "radius");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (waves));
    }
}

gdouble
mx_deform_waves_get_amplitude (MxDeformWaves *waves)
{
  g_return_val_if_fail (MX_IS_DEFORM_WAVES (waves), 0.0);
  return waves->priv->amplitude;
}

void
mx_deform_waves_set_amplitude (MxDeformWaves *waves,
                               gdouble        amplitude)
{
  MxDeformWavesPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_WAVES (waves));

  priv = waves->priv;

  if (priv->amplitude != amplitude)
    {
      priv->amplitude = amplitude;
      g_object_notify (G_OBJECT (waves), "amplitude");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (waves));
    }
}
