/*
 * Copyright (C) 2009 Intel Corporation
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
 *
 * Written by: Robert Staudinger <robsta@openedhand.com>
 */

/**
 * SECTION:nbtk-expander
 * @short_description: Expander widget that shows and hides it's child widget.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

#include "nbtk-button.h"
#include "nbtk-expander.h"
#include "nbtk-tile.h"

#define NBTK_EXPANDER_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_EXPANDER, NbtkExpanderPrivate))

enum
{
  PROP_0,

  PROP_LABEL
};

struct _NbtkExpanderPrivate
{
  ClutterActor *header_button;
  ClutterActor *payload_tile;
};

static void nbtk_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkExpander, nbtk_expander, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                nbtk_container_iface_init));

static void
nbtk_expander_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value,
                          nbtk_button_get_label (NBTK_BUTTON (priv->header_button)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_expander_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      nbtk_button_set_label (NBTK_BUTTON (priv->header_button),
                             g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_expander_finalize (GObject *gobject)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (gobject)->priv;

  clutter_actor_unparent (priv->header_button);

  G_OBJECT_CLASS (nbtk_expander_parent_class)->finalize (gobject);
}

static void
nbtk_expander_allocate (ClutterActor          *actor,
                        const ClutterActorBox *box,
                        gboolean               origin_changed)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActorBox      header_box;
  NbtkPadding          padding;

  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)
    ->allocate (actor, box, origin_changed);

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  /* Header button. */
  header_box.x1 = padding.left;
  header_box.y1 = padding.top;
  header_box.x2 = box->x2 - box->x1 - padding.left - padding.right; /* Fill w. */
  header_box.y2 = header_box.x1 + clutter_actor_get_heightu (priv->header_button);

  clutter_actor_allocate (priv->header_button, &header_box, origin_changed);

  /* Payload. */
  if (nbtk_button_get_checked (NBTK_BUTTON (priv->header_button)))
    {
      ClutterActorBox payload_box;

      payload_box.x1 = header_box.x1;
      payload_box.y1 = header_box.y2;
      payload_box.x2 = header_box.x2;
      payload_box.y2 = payload_box.x1 +
                       clutter_actor_get_height (priv->payload_tile);

      /* Sanity check. */
      payload_box.y2 = MAX (payload_box.y1, payload_box.y2);

      clutter_actor_allocate (priv->payload_tile, &payload_box, origin_changed);
    }
}

static void
nbtk_expander_paint (ClutterActor *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)->paint (actor);

  clutter_actor_paint (priv->header_button);

  if (nbtk_button_get_checked (NBTK_BUTTON (priv->header_button)))
    clutter_actor_paint (priv->payload_tile);
}


static void
nbtk_expander_pick (ClutterActor       *actor,
                    const ClutterColor *pick_color)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)->pick (actor, pick_color);

  clutter_actor_paint (priv->header_button);

  if (nbtk_button_get_checked (NBTK_BUTTON (priv->header_button)))
    clutter_actor_paint (priv->payload_tile);
}

#if 0 /* Strangely this only works for the first click. */
static void
nbtk_expander_toggled (NbtkExpander *self,
                       NbtkButton   *header_button)
{
  NbtkExpanderPrivate *priv = self->priv;
  gboolean checked;

  g_object_get (header_button, "checked", &checked, NULL);
printf ("%s() %d\n", __FUNCTION__, checked);
  if (checked)
    clutter_actor_show (priv->payload_tile);
  else
    clutter_actor_hide (priv->payload_tile);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}
#endif

static void
nbtk_expander_class_init (NbtkExpanderClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec        *pspec;

  g_type_class_add_private (klass, sizeof (NbtkExpanderPrivate));

  gobject_class->finalize = nbtk_expander_finalize;
  gobject_class->get_property = nbtk_expander_get_property;
  gobject_class->set_property = nbtk_expander_set_property;

  actor_class->allocate = nbtk_expander_allocate;
  actor_class->paint = nbtk_expander_paint;
  actor_class->pick = nbtk_expander_pick;

  pspec = g_param_spec_string ("label",
                               "Label",
                               "Label of the header",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);
}

static void
nbtk_expander_init (NbtkExpander *self)
{
  self->priv = NBTK_EXPANDER_GET_PRIVATE (self);

  self->priv->header_button = (ClutterActor *)
                                g_object_new (NBTK_TYPE_BUTTON,
                                              "toggle-mode", TRUE,
                                              NULL);
  clutter_actor_set_parent (self->priv->header_button, CLUTTER_ACTOR (self));
/*
  g_signal_connect_swapped (self->priv->header_button, "clicked",
                            G_CALLBACK (nbtk_expander_toggled), self);
*/

  /* Initially invisible, for consistency with the un-toggled button. */
  self->priv->payload_tile = (ClutterActor *) nbtk_tile_new ();
  clutter_actor_set_parent (self->priv->payload_tile, CLUTTER_ACTOR (self));
  clutter_actor_hide (self->priv->payload_tile);
}

/**
 * nbtk_expander_new:
 *
 * Create a new expander.
 *
 * Returns: a new #NbtkExpander
 */
NbtkWidget *
nbtk_expander_new (const gchar *label)
{
  return g_object_new (NBTK_TYPE_EXPANDER,
                       "label", label,
                       NULL);
}

/*
 * ClutterContainer interface.
 */

static void
nbtk_expander_add_actor (ClutterContainer *container,
                         ClutterActor     *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (container)->priv;

  clutter_container_add (CLUTTER_CONTAINER (priv->payload_tile),
                         actor, NULL);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);
}

typedef struct {
  const ClutterActor  *actor;
  gboolean             actor_found;
} match_data_t;

static void
match_child_cb (ClutterActor  *actor,
                match_data_t  *match_data)
{
  if (match_data->actor == actor)
    match_data->actor_found = TRUE;
}

static void
nbtk_expander_remove_actor (ClutterContainer *container,
                            ClutterActor     *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (container)->priv;
  match_data_t         match_data;

  /* Need to determine whether child is valid in order to know
   * if the "actor-removed" signal should be emitted. */
  match_data.actor = actor;
  match_data.actor_found = FALSE;
  clutter_container_foreach (CLUTTER_CONTAINER (priv->payload_tile),
                             (ClutterCallback) match_child_cb,
                             &match_data);

  if (match_data.actor_found)
    {
      clutter_container_remove (CLUTTER_CONTAINER (priv->payload_tile),
                                actor, NULL);
      g_signal_emit_by_name (container, "actor-removed", actor);
    }
}

static void
nbtk_expander_foreach (ClutterContainer *container,
                       ClutterCallback   callback,
                       gpointer          callback_data)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (container)->priv;

  clutter_container_foreach (CLUTTER_CONTAINER (priv->payload_tile),
                             callback, callback_data);
}

static void
nbtk_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_expander_add_actor;
  iface->remove = nbtk_expander_remove_actor;
  iface->foreach = nbtk_expander_foreach;
}

