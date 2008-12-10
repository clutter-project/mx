/*
 * Nbtk.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *             Jorn Baayen  <jorn@openedhand.com>
 *             Emmanuele Bassi  <ebassi@openedhand.com>
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

#ifndef __NBTK_BEHAVIOUR_BOUNCE_H__
#define __NBTK_BEHAVIOUR_BOUNCE_H__

#include <clutter/clutter-actor.h>
#include <clutter/clutter-alpha.h>
#include <clutter/clutter-behaviour.h>
#include <clutter/clutter-types.h>

G_BEGIN_DECLS

#define NBTK_TYPE_BEHAVIOUR_BOUNCE (nbtk_behaviour_bounce_get_type ())

#define NBTK_BEHAVIOUR_BOUNCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_BEHAVIOUR_BOUNCE, NbtkBehaviourBounce))

#define NBTK_BEHAVIOUR_BOUNCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_BEHAVIOUR_BOUNCE, NbtkBehaviourBounceClass))

#define NBTK_IS_BEHAVIOUR_BOUNCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_BEHAVIOUR_BOUNCE))

#define NBTK_IS_BEHAVIOUR_BOUNCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_BEHAVIOUR_BOUNCE))

#define NBTK_BEHAVIOUR_BOUNCE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_BEHAVIOUR_BOUNCE, NbtkBehaviourBounceClass))

typedef struct _NbtkBehaviourBounce        NbtkBehaviourBounce;
typedef struct _NbtkBehaviourBouncePrivate NbtkBehaviourBouncePrivate;
typedef struct _NbtkBehaviourBounceClass   NbtkBehaviourBounceClass;

struct _NbtkBehaviourBounce
{
  ClutterBehaviour parent_instance;

  /*< private >*/
  NbtkBehaviourBouncePrivate *priv;
};

struct _NbtkBehaviourBounceClass
{
  ClutterBehaviourClass parent_class;
};

GType nbtk_behaviour_bounce_get_type (void) G_GNUC_CONST;

ClutterBehaviour *
nbtk_behaviour_bounce_new (ClutterAlpha *alpha);

ClutterTimeline*
nbtk_bounce_scale (ClutterActor *actor, gint duration);

G_END_DECLS

#endif /* __NBTK_BEHAVIOUR_BOUNCE_H__ */
