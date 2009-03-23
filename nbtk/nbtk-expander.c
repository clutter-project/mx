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

#include "nbtk-bin.h"
#include "nbtk-button.h"
#include "nbtk-expander.h"
#include "nbtk-stylable.h"
#include "nbtk-table.h"

#define NBTK_EXPANDER_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_EXPANDER, NbtkExpanderPrivate))

enum
{
  PROP_0,

  PROP_EXPANDED,
  PROP_LABEL
};

enum
{
  LAST_SIGNAL
};

struct _NbtkExpanderPrivate
{
  ClutterActor  *table;
  ClutterActor  *header_button;
  ClutterActor  *payload_bin;
};

static void nbtk_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkExpander, nbtk_expander, NBTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                nbtk_container_iface_init));

static ClutterContainerIface *nbtk_expander_parent_iface = NULL;

static void
nbtk_expander_finalize (GObject *gobject)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (gobject)->priv;

  /* The other child actors are managed by the table,
   * which is in turn managed by the NbtkBin parent. */

  g_object_unref (priv->payload_bin);
}

static void
nbtk_expander_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_EXPANDED:
      g_value_set_boolean (value,
                           nbtk_expander_get_expanded (NBTK_EXPANDER (gobject)));
      break;
    case PROP_LABEL:
      g_value_set_string (value,
                          nbtk_button_get_label (
                              NBTK_BUTTON (priv->header_button)));
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
    case PROP_EXPANDED:
      nbtk_expander_set_expanded (NBTK_EXPANDER (gobject),
                                  g_value_get_boolean (value));
      break;
    case PROP_LABEL:
      nbtk_button_set_label (NBTK_BUTTON (priv->header_button),
                             g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

typedef struct {
  ClutterActor *actor;
  gboolean      is_child;
} child_data_t;

static void
is_child_cb (ClutterActor *actor,
             child_data_t *child_data)
{
  if (actor == child_data->actor)
    child_data->is_child = TRUE;
}

static void
button_checked_cb (NbtkButton     *button,
                   GParamSpec     *pspec,
                   NbtkExpander   *self)
{
  if (nbtk_button_get_checked (button))
    {
      nbtk_table_add_widget_full (NBTK_TABLE (self->priv->table),
                                  NBTK_WIDGET (self->priv->payload_bin),
                                  1, 0, 1, 1,
                                  NBTK_X_EXPAND | NBTK_X_FILL | NBTK_Y_EXPAND | NBTK_Y_FILL,
                                  0., 0.);
      clutter_actor_show (self->priv->payload_bin);
    }
  else
    {
      child_data_t child_data;
      child_data.actor = self->priv->payload_bin;
      child_data.is_child = FALSE;
      clutter_container_foreach (CLUTTER_CONTAINER (self->priv->table),
                                 (ClutterCallback) is_child_cb, &child_data);
      if (child_data.is_child)
        {
          clutter_container_remove (CLUTTER_CONTAINER (self->priv->table),
                                    self->priv->payload_bin, NULL);
          clutter_actor_hide (self->priv->payload_bin);
        }
    }

  g_object_notify (G_OBJECT (self), "expanded");

  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

static void
natural_width_notify_cb (NbtkExpander *self,
                         GParamSpec   *pspec,
                         gpointer      data)
{
  clutter_actor_set_width (self->priv->table,
                           clutter_actor_get_width (CLUTTER_ACTOR (self)));
}

static void
nbtk_expander_class_init (NbtkExpanderClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  /* ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass); */
  GParamSpec        *pspec;

  g_type_class_add_private (klass, sizeof (NbtkExpanderPrivate));

  gobject_class->finalize = nbtk_expander_finalize;
  gobject_class->get_property = nbtk_expander_get_property;
  gobject_class->set_property = nbtk_expander_set_property;

  pspec = g_param_spec_string ("expanded",
                               "Expanded",
                               "Expansion state of the widget",
                               FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_EXPANDED, pspec);

  pspec = g_param_spec_string ("label",
                               "Label",
                               "Label of the header",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);
}

static void
nbtk_expander_init (NbtkExpander *self)
{
  ClutterActor  *button_icon;
  GValue         background_image = { 0, };

  self->priv = NBTK_EXPANDER_GET_PRIVATE (self);

  g_return_if_fail (nbtk_expander_parent_iface);

  g_signal_connect (self, "notify::natural-width",
                    G_CALLBACK (natural_width_notify_cb), NULL);

  self->priv->table = (ClutterActor *) nbtk_table_new ();
  nbtk_expander_parent_iface->add (CLUTTER_CONTAINER (self), self->priv->table);

  /* Header */
  self->priv->header_button = (ClutterActor *)
                                g_object_new (NBTK_TYPE_BUTTON,
                                              "toggle-mode", TRUE,
                                              "x-align", 0.,
                                              NULL);
  clutter_actor_set_name (self->priv->header_button, "nbtk-expander-header");
  nbtk_table_add_widget_full (NBTK_TABLE (self->priv->table),
                              NBTK_WIDGET (self->priv->header_button),
                              0, 0, 1, 1,
                              NBTK_X_EXPAND | NBTK_X_FILL | NBTK_Y_EXPAND | NBTK_Y_FILL,
                              0., 0.);
  g_signal_connect (self->priv->header_button, "notify::checked",
                    G_CALLBACK (button_checked_cb), self);

  /* Icon */
  button_icon = (ClutterActor *) nbtk_bin_new ();
  clutter_actor_set_name (button_icon, "nbtk-expander-header-icon");
  nbtk_button_set_icon (NBTK_BUTTON (self->priv->header_button), button_icon);
  /* Icon size. */
  g_value_init (&background_image, G_TYPE_STRING);
  nbtk_stylable_get_property (NBTK_STYLABLE (button_icon),
                              "background-image", &background_image);
  if (g_value_get_string (&background_image))
    {
      GError        *error = NULL;
      ClutterActor  *background_texture = clutter_texture_new_from_file (
                                            g_value_get_string (&background_image),
                                            &error);
      if (error)
        {
          g_warning ("%s", error->message);
          g_error_free (error);
        }
      else
        {
          gint width, height;
          clutter_texture_get_base_size (CLUTTER_TEXTURE (background_texture),
                                         &width, &height);
          clutter_actor_set_size (button_icon, width, height);
          g_object_unref (background_texture);
        }
      g_value_unset (&background_image);
    }

  /* Payload container.
   * Initially invisible, for consistency with the un-toggled button. */
  self->priv->payload_bin = (ClutterActor *) g_object_new (NBTK_TYPE_BIN,
                                                            "x-align", 0.,
                                                            NULL);
  clutter_actor_set_name (self->priv->payload_bin, "nbtk-expander-body");
  clutter_actor_hide (self->priv->payload_bin);
  /* Widget is added to the table only when displayed. */
  g_object_ref_sink (self->priv->payload_bin);
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

gboolean
nbtk_expander_get_expanded (NbtkExpander *self)
{
  g_return_val_if_fail (self, FALSE);

  return nbtk_button_get_checked (NBTK_BUTTON (self->priv->header_button));
}

void
nbtk_expander_set_expanded (NbtkExpander *self,
                            gboolean      expanded)
{
  g_return_if_fail (self);

  nbtk_button_set_checked (NBTK_BUTTON (self->priv->header_button), expanded);
}

ClutterActor *
nbtk_expander_get_child (NbtkExpander *self)
{
  g_return_val_if_fail (self, NULL);

  return nbtk_bin_get_child (NBTK_BIN (self->priv->payload_bin));
}

const gchar *
nbtk_expander_get_label (NbtkExpander *self)
{
  g_return_val_if_fail (self, NULL);

  return nbtk_button_get_label (NBTK_BUTTON (self->priv->header_button));
}

/*
 * ClutterContainer interface.
 */

static void
nbtk_expander_add_actor (ClutterContainer *container,
                         ClutterActor     *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (container)->priv;

  clutter_container_add (CLUTTER_CONTAINER (priv->payload_bin),
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
  clutter_container_foreach (CLUTTER_CONTAINER (priv->payload_bin),
                             (ClutterCallback) match_child_cb,
                             &match_data);

  if (match_data.actor_found)
    {
      clutter_container_remove (CLUTTER_CONTAINER (priv->payload_bin),
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

  clutter_container_foreach (CLUTTER_CONTAINER (priv->payload_bin),
                             callback, callback_data);
}

static void
nbtk_container_iface_init (ClutterContainerIface *iface)
{
  nbtk_expander_parent_iface = g_type_interface_peek_parent (iface);

  iface->add = nbtk_expander_add_actor;
  iface->remove = nbtk_expander_remove_actor;
  iface->foreach = nbtk_expander_foreach;
}

