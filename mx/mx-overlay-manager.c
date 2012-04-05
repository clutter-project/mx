/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-overlay-manager.c: A layer mecanism for tooltips and menus.
 *
 * Copyright 2012 Intel Corporation.
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
 * Written by: Lionel Landwerlin <lionel.g.landwerlin@linux.intel.com>
 *
 */

#include "mx-overlay-manager.h"
#include "mx-overlay.h"

#include <mx/mx-private.h>

G_DEFINE_TYPE (MxOverlayManager, mx_overlay_manager, G_TYPE_OBJECT)

#define OVERLAY_MANAGER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_OVERLAY_MANAGER, MxOverlayManagerPrivate))

struct _MxOverlayManagerPrivate
{
  GArray       *overlays;
  ClutterActor *stage;
};

enum
{
  PROP_0,

  PROP_STAGE,
};

/**/

static void
mx_overlay_manager_actor_added (ClutterActor     *stage,
                                ClutterActor     *actor,
                                MxOverlayManager *manager);
static void
mx_overlay_manager_stage_destroyed (MxOverlayManager *manager,
                                    GObject          *object,
                                    gboolean          is_last_ref);
static void
mx_overlay_manager_set_stage (MxOverlayManager *manager,
                              ClutterActor     *stage);

/**/

static void
mx_overlay_manager_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  MxOverlayManagerPrivate *priv = MX_OVERLAY_MANAGER (object)->priv;

  switch (property_id)
    {
    case PROP_STAGE:
      g_value_set_object (value, priv->stage);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_overlay_manager_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_STAGE:
      mx_overlay_manager_set_stage (MX_OVERLAY_MANAGER (object),
                                    (ClutterActor *) g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_overlay_manager_dispose (GObject *object)
{
  MxOverlayManagerPrivate *priv = MX_OVERLAY_MANAGER (object)->priv;

  g_object_remove_toggle_ref (G_OBJECT (priv->stage),
                              (GToggleNotify) mx_overlay_manager_stage_destroyed,
                              object);

  G_OBJECT_CLASS (mx_overlay_manager_parent_class)->dispose (object);
}

static void
mx_overlay_manager_finalize (GObject *object)
{
  MxOverlayManagerPrivate *priv = MX_OVERLAY_MANAGER (object)->priv;

  if (priv->overlays)
    g_array_unref (priv->overlays);

  G_OBJECT_CLASS (mx_overlay_manager_parent_class)->finalize (object);
}

static void
mx_overlay_manager_class_init (MxOverlayManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxOverlayManagerPrivate));

  object_class->get_property = mx_overlay_manager_get_property;
  object_class->set_property = mx_overlay_manager_set_property;
  object_class->dispose = mx_overlay_manager_dispose;
  object_class->finalize = mx_overlay_manager_finalize;

  pspec = g_param_spec_object ("stage",
                               "Stage",
                               "Stage on which the overlays are managed",
                               CLUTTER_TYPE_STAGE,
                               MX_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_STAGE, pspec);

}

static void
mx_overlay_manager_init (MxOverlayManager *self)
{
  MxOverlayManagerPrivate *priv;
  gint i;

  self->priv = priv = OVERLAY_MANAGER_PRIVATE (self);

  priv->overlays = g_array_new (FALSE, TRUE,  sizeof (ClutterActor *));
  for (i = 0; i < MX_OVERLAY_LAST; i++)
    {
      ClutterActor *overlay = NULL;
      g_array_append_val (priv->overlays, overlay);
    }
}

static void
mx_overlay_manager_restack_overlays (MxOverlayManager *manager)
{
  MxOverlayManagerPrivate *priv = manager->priv;
  ClutterActor *overlay;
  gint i;

  g_signal_handlers_block_by_func (priv->stage,
                                   mx_overlay_manager_actor_added,
                                   manager);

  for (i = 0; i < priv->overlays->len; i++)
    {
      overlay = g_array_index (priv->overlays, ClutterActor *, i);
      if (overlay)
        clutter_actor_set_child_below_sibling (priv->stage,
                                               overlay,
                                               NULL);
    }

  g_signal_handlers_unblock_by_func (priv->stage,
                                     mx_overlay_manager_actor_added,
                                     manager);
}

static void
mx_overlay_manager_stage_destroyed (MxOverlayManager *manager,
                                    GObject          *object,
                                    gboolean          is_last_ref)
{
  g_object_unref (manager);
}

static void
mx_overlay_manager_actor_added (ClutterActor *stage,
                                ClutterActor *actor,
                                MxOverlayManager *manager)
{
  mx_overlay_manager_restack_overlays (manager);
}

static void
mx_overlay_manager_set_stage (MxOverlayManager *manager,
                              ClutterActor     *stage)
{
  MxOverlayManagerPrivate *priv = manager->priv;

  priv->stage = stage;

  g_object_add_toggle_ref (G_OBJECT (stage),
                           (GToggleNotify) mx_overlay_manager_stage_destroyed,
                           manager);
  g_signal_connect (stage, "actor-added",
                    G_CALLBACK (mx_overlay_manager_actor_added),
                    manager);
}

/**
 * mx_overlay_manager_get_overlay
 * @manager: A #MxOverlayManager
 * @overlay: A #MxOverlay
 *
 * Retrieves the actor associated to a given overlay in a #MxOverlayManager.
 *
 * Returns: (transfer none): a #ClutterActor
 */
ClutterActor *
mx_overlay_manager_get_overlay (MxOverlayManager *manager,
                                MxOverlayLayer    layer)
{
  MxOverlayManagerPrivate *priv;
  ClutterActor **overlay;

  g_return_val_if_fail (MX_IS_OVERLAY_MANAGER (manager), NULL);
  g_return_val_if_fail (layer < MX_OVERLAY_LAST, NULL);

  priv = manager->priv;

  overlay = &g_array_index (priv->overlays, ClutterActor *, layer);
  if (*overlay == NULL)
    {
      *overlay = mx_overlay_new ();
      clutter_actor_add_child (priv->stage, *overlay);
    }

  return *overlay;
}

/**
 * mx_overlay_manager_get_for_stage
 * @stage: A #ClutterStage
 *
 * Retrieves the #MxOverlayManager associated with @stage.
 *
 * Returns: (transfer none): a #MxOverlayManager
 */
MxOverlayManager *
mx_overlay_manager_get_for_stage (ClutterActor *stage)
{
  static GQuark overlay_manager_quark = 0;
  MxOverlayManager *manager;

  g_return_val_if_fail (CLUTTER_IS_STAGE (stage), NULL);

  if (G_UNLIKELY (!overlay_manager_quark))
    overlay_manager_quark = g_quark_from_static_string ("mx-overlay-manager");

  if (!(manager = g_object_get_qdata (G_OBJECT (stage), overlay_manager_quark)))
    {
      manager = g_object_new (MX_TYPE_OVERLAY_MANAGER,
                              "stage", stage,
                              NULL);
      g_object_set_qdata (G_OBJECT (stage), overlay_manager_quark, manager);
    }

  return manager;
}
