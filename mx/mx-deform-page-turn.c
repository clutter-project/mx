/*
 * mx-deform-page-turn.c: A page-turning deformation actor
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

#include "mx-deform-page-turn.h"
#include "mx-private.h"

#include <math.h>

G_DEFINE_TYPE (MxDeformPageTurn, mx_deform_page_turn, MX_TYPE_DEFORM_TEXTURE)

#define DEFORM_PAGE_TURN_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
   MX_TYPE_DEFORM_PAGE_TURN, MxDeformPageTurnPrivate))

struct _MxDeformPageTurnPrivate
{
  gdouble period;
  gdouble angle;
  gdouble radius;
};

enum
{
  PROP_0,

  PROP_PERIOD,
  PROP_ANGLE,
  PROP_RADIUS,
};

static void
mx_deform_page_turn_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  MxDeformPageTurnPrivate *priv = MX_DEFORM_PAGE_TURN (object)->priv;

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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_page_turn_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_PERIOD:
      mx_deform_page_turn_set_period (MX_DEFORM_PAGE_TURN (object),
                                      g_value_get_double (value));
      break;

    case PROP_ANGLE:
      mx_deform_page_turn_set_angle (MX_DEFORM_PAGE_TURN (object),
                                     g_value_get_double (value));
      break;

    case PROP_RADIUS:
      mx_deform_page_turn_set_radius (MX_DEFORM_PAGE_TURN (object),
                                      g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
mx_deform_page_turn_deform (MxDeformTexture   *texture,
                            CoglTextureVertex *vertex,
                            gfloat             width,
                            gfloat             height)
{
  gfloat cx, cy, rx, ry, turn_angle;
  guint shade;

  MxDeformPageTurnPrivate *priv = ((MxDeformPageTurn *)texture)->priv;

  /* Rotate the point around the centre of the page-curl ray to align it with
   * the y-axis.
   */
  cx = (1.f - priv->period) * width;
  cy = (1.f - priv->period) * height;

  rx = ((vertex->x - cx) * cos (-priv->angle)) -
       ((vertex->y - cy) * sin (-priv->angle)) - priv->radius;
  ry = ((vertex->x - cx) * sin (-priv->angle)) +
       ((vertex->y - cy) * cos (-priv->angle));

  turn_angle = 0.f;
  if (rx > -priv->radius * 2)
    {
      /* Calculate the curl angle as a function from the distance of the curl
       * ray (i.e. the page crease)
       */
      turn_angle = (rx / priv->radius * G_PI_2) - G_PI_2;
      shade = (sin (turn_angle) * 96) + 159;

      /* Add a gradient that makes it look like lighting and hides the switch
       * between textures.
       */
      cogl_color_set_from_4ub (&vertex->color, shade, shade, shade, 0xff);
    }

  if (rx > 0)
    {
      /* Make the curl radius smaller as more circles are formed (stops
       * z-fighting and looks cool)
       */
      /* Note, 10 is a semi-arbitrary number here -
       * divide it by two and it's the amount of space between curled
       * layers of the texture, in pixels.
       */
      gfloat small_radius = priv->radius -
        MIN (priv->radius, (turn_angle * 10) / G_PI);

      /* Calculate a point on a cylinder (maybe make this a cone at some point)
       * and rotate it by the specified angle.
       */
      rx = (small_radius * cos (turn_angle)) + priv->radius;
      vertex->x = (rx * cos (priv->angle)) - (ry * sin (priv->angle)) + cx;
      vertex->y = (rx * sin (priv->angle)) + (ry * cos (priv->angle)) + cy;
      vertex->z = (small_radius * sin (turn_angle)) + priv->radius;
    }
}

static void
mx_deform_page_turn_class_init (MxDeformPageTurnClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MxDeformTextureClass *deform_class = MX_DEFORM_TEXTURE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDeformPageTurnPrivate));

  object_class->get_property = mx_deform_page_turn_get_property;
  object_class->set_property = mx_deform_page_turn_set_property;

  deform_class->deform = mx_deform_page_turn_deform;

  pspec = g_param_spec_double ("period",
                               "Period",
                               "Effect period",
                               0.0, 1.0, 0.0,
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
                               "Radius of the page curl",
                               G_MINDOUBLE, G_MAXDOUBLE, 24.0,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_RADIUS, pspec);
}

static void
mx_deform_page_turn_init (MxDeformPageTurn *self)
{
  MxDeformPageTurnPrivate *priv = self->priv = DEFORM_PAGE_TURN_PRIVATE (self);

  priv->radius = 24.0;
}

ClutterActor *
mx_deform_page_turn_new (void)
{
  return g_object_new (MX_TYPE_DEFORM_PAGE_TURN, NULL);
}

gdouble
mx_deform_page_turn_get_period (MxDeformPageTurn *page_turn)
{
  g_return_val_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn), 0.0);
  return page_turn->priv->period;
}

void
mx_deform_page_turn_set_period (MxDeformPageTurn *page_turn,
                                gdouble           period)
{
  MxDeformPageTurnPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn));

  priv = page_turn->priv;

  if (priv->period != period)
    {
      priv->period = period;
      g_object_notify (G_OBJECT (page_turn), "period");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (page_turn));
    }
}

gdouble
mx_deform_page_turn_get_angle (MxDeformPageTurn *page_turn)
{
  g_return_val_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn), 0.0);
  return page_turn->priv->angle;
}

void
mx_deform_page_turn_set_angle (MxDeformPageTurn *page_turn,
                               gdouble           angle)
{
  MxDeformPageTurnPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn));

  priv = page_turn->priv;

  if (priv->angle != angle)
    {
      priv->angle = angle;
      g_object_notify (G_OBJECT (page_turn), "angle");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (page_turn));
    }
}

gdouble
mx_deform_page_turn_get_radius (MxDeformPageTurn *page_turn)
{
  g_return_val_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn), 0.0);
  return page_turn->priv->radius;
}

void
mx_deform_page_turn_set_radius (MxDeformPageTurn *page_turn,
                                gdouble           radius)
{
  MxDeformPageTurnPrivate *priv;

  g_return_if_fail (MX_IS_DEFORM_PAGE_TURN (page_turn));

  priv = page_turn->priv;

  if (priv->radius != radius)
    {
      priv->radius = radius;
      g_object_notify (G_OBJECT (page_turn), "radius");
      mx_deform_texture_invalidate (MX_DEFORM_TEXTURE (page_turn));
    }
}
