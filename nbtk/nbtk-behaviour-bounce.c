/*
 * Nbtk.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
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
 * SECTION:nbtk-behaviour-bounce
 * @short_description: A behaviour controlling bounce
 *
 * A #NbtkBehaviourBounce interpolates actors size between two values.
 *
 */

#include "nbtk-behaviour-bounce.h"
#include <math.h>

G_DEFINE_TYPE (NbtkBehaviourBounce,   \
               nbtk_behaviour_bounce, \
	       CLUTTER_TYPE_BEHAVIOUR);

struct _NbtkBehaviourBouncePrivate
{
  gint foo;
};

#define NBTK_BEHAVIOUR_BOUNCE_GET_PRIVATE(obj)        \
              (G_TYPE_INSTANCE_GET_PRIVATE ((obj),    \
               NBTK_TYPE_BEHAVIOUR_BOUNCE,            \
               NbtkBehaviourBouncePrivate))

typedef struct {
  ClutterFixed scale;
  guint8       opacity;
} BounceFrameClosure;

static void
bounce_frame_foreach (ClutterBehaviour *behaviour,
                      ClutterActor     *actor,
                      gpointer          data)
{
  BounceFrameClosure *closure = data;
  ClutterUnit         anchor_x, anchor_y;
  ClutterActorBox     box;

  clutter_actor_set_opacity (actor, closure->opacity);
  clutter_actor_set_scalex (actor, closure->scale, closure->scale);

  clutter_actor_get_allocation_box (actor, &box);
  anchor_x = CLUTTER_UNITS_FROM_FIXED (CLUTTER_FIXED_DIV (
                CLUTTER_FIXED_MUL (CLUTTER_UNITS_TO_FIXED (box.x2 - box.x1),
                                   closure->scale - CFX_ONE) / 2,
                                   closure->scale));
  anchor_y = CLUTTER_UNITS_FROM_FIXED (CLUTTER_FIXED_DIV (
                CLUTTER_FIXED_MUL (CLUTTER_UNITS_TO_FIXED (box.y2 - box.y1),
                                   closure->scale - CFX_ONE) / 2,
                                   closure->scale));
  clutter_actor_set_anchor_pointu (actor, anchor_x, anchor_y);
}

static gfloat 
ease_out_bounce (gfloat t)
{
  gfloat b = 0.0, c = 1.0, d = 1.0;

  if ((t/=d) < (1/2.75)) {
    return c*(7.5625*t*t);
  } else if (t < (2/2.75)) {
    return c*(7.5625*(t-=(1.5/2.75))*t + .75);
  } else if (t < (2.5/2.75)) {
    return c*(7.5625*(t-=(2.25/2.75))*t + .9375);
  } else {
    return c*(7.5625*(t-=(2.625/2.75))*t + .984375);
  }
}

static gfloat
ease_in_bounce (gfloat t) 
{
  return 1.0 - ease_out_bounce (1.0-t);
}

static gfloat 
ease_in_elastic (gfloat t)
{
  /* Likely could be optimised */
  return (1 - cos(t*M_PI) + (1 - cos(t*t*M_PI*4))*(1 - t)*(2 - t))*0.5;
}

static void
nbtk_behaviour_bounce_alpha_notify (ClutterBehaviour *behave,
                                    guint32           alpha_value)
{
  NbtkBehaviourBouncePrivate *priv;
  guint boing_alpha;
  ClutterFixed factor;
  gfloat       af, ff;

  BounceFrameClosure closure = { 0, };

  priv = NBTK_BEHAVIOUR_BOUNCE (behave)->priv;

  af = (gfloat)(alpha_value)/(gfloat)CLUTTER_ALPHA_MAX_ALPHA;

  ff = ease_in_elastic (af);

#if 0
  boing_alpha = (CLUTTER_ALPHA_MAX_ALPHA/4)*3;

  if (alpha_value < boing_alpha)
    {
      factor = CLUTTER_INT_TO_FIXED (alpha_value) / boing_alpha;

      closure.scale = factor + CLUTTER_FLOAT_TO_FIXED(0.25);
      closure.opacity = CLUTTER_FIXED_TO_INT(0xff * factor);
    }
  else
    {
      closure.opacity = 0xff;

      if (alpha_value < boing_alpha/4)
        boing_alpha /= 4;
      else
        boing_alpha -= (boing_alpha/4);

      /* scale down from 1.25 -> 1.0 */

      factor = CLUTTER_INT_TO_FIXED (alpha_value - boing_alpha)
                / (CLUTTER_ALPHA_MAX_ALPHA  - boing_alpha);

      factor /= 4;

      closure.scale = CLUTTER_FLOAT_TO_FIXED(1.25) - factor;
    }
#endif

  closure.scale = CLUTTER_FLOAT_TO_FIXED(ff);
  closure.opacity = 0xff;

  //closure.opacity = CLUTTER_FIXED_TO_INT(0xff * factor);

  clutter_behaviour_actors_foreach (behave,
                                    bounce_frame_foreach,
                                    &closure);
}

static void
nbtk_behaviour_bounce_class_init (NbtkBehaviourBounceClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = nbtk_behaviour_bounce_alpha_notify;

  g_type_class_add_private (klass, sizeof (NbtkBehaviourBouncePrivate));
}

static void
nbtk_behaviour_bounce_init (NbtkBehaviourBounce *self)
{
  NbtkBehaviourBouncePrivate *priv;

  self->priv = priv = NBTK_BEHAVIOUR_BOUNCE_GET_PRIVATE (self);
}

/**
 * nbtk_behaviour_bounce_newx:
 * @alpha: a #NbtkAlpha
 * @x_bounce_start: initial bounce factor on the X axis
 * @y_bounce_start: initial bounce factor on the Y axis
 * @x_bounce_end: final bounce factor on the X axis
 * @y_bounce_end: final bounce factor on the Y axis
 *
 * A fixed point implementation of nbtk_behaviour_bounce_new()
 *
 * Return value: the newly created #NbtkBehaviourBounce
 *
 * Since: 0.2
 */
ClutterBehaviour *
nbtk_behaviour_bounce_new (ClutterAlpha   *alpha)
{
  NbtkBehaviourBounce *behave;

  g_return_val_if_fail (alpha == NULL || CLUTTER_IS_ALPHA (alpha), NULL);

  behave = g_object_new (NBTK_TYPE_BEHAVIOUR_BOUNCE, "alpha", alpha, NULL);

  return CLUTTER_BEHAVIOUR (behave);
}


typedef struct BounceClosure
{
  ClutterActor             *actor;
  ClutterAlpha             *alpha;
  ClutterBehaviour         *behave;
}
BounceClosure;

static void
on_bounce_complete (gpointer         user_data,
                    GObject         *behaviour)
{
  BounceClosure *b =  (BounceClosure*)user_data;

  g_object_unref (b->actor);
  g_object_unref (b->behave);

  g_slice_free (BounceClosure, b);
}

ClutterTimeline*
nbtk_bounce_scale (ClutterActor *actor, gint duration)
{
  BounceClosure *b;
  ClutterTimeline *timeline;

  timeline = clutter_timeline_new_for_duration (duration);

  b = g_slice_new0(BounceClosure);
  b->actor  = g_object_ref (actor);
  b->alpha  = clutter_alpha_new_full (timeline, CLUTTER_ALPHA_SINE_INC,
                                      NULL, NULL);
  b->behave = nbtk_behaviour_bounce_new (b->alpha);

  g_object_weak_ref (G_OBJECT (timeline), on_bounce_complete, b);

  clutter_behaviour_apply (b->behave, b->actor);
  clutter_timeline_start (timeline);

  return timeline;
}

