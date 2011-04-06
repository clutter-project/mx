/*
 * mx-actor-manager: Actor life-cycle manager object
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
 * SECTION:mx-actor-manager
 * @short_description: An object that manages ClutterActor lifecycle
 *
 * #MxActorManager is an object that helps manage the creation, addition
 * and removal of actors. It is bound to a particular stage, and spreads
 * operations over time so as not to interrupt animations or interactivity.
 *
 * Operations added to the #MxActorManager will strictly be performed in the
 * order in which they were added.
 */

#include "mx-actor-manager.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxActorManager, mx_actor_manager, G_TYPE_OBJECT)

#define ACTOR_MANAGER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_ACTOR_MANAGER, MxActorManagerPrivate))

static GQuark actor_manager_quark = 0;
static GQuark actor_manager_error_quark = 0;

enum
{
  PROP_0,

  PROP_STAGE,
  PROP_TIME_SLICE,
  PROP_N_OPERATIONS
};

enum
{
  ACTOR_CREATED,
  ACTOR_ADDED,
  ACTOR_REMOVED,
  ACTOR_FINISHED,
  OP_COMPLETED,
  OP_CANCELLED,
  OP_FAILED,

  LAST_SIGNAL
};

typedef enum
{
  MX_ACTOR_MANAGER_CREATE,
  MX_ACTOR_MANAGER_ADD,
  MX_ACTOR_MANAGER_REMOVE
} MxActorManagerOperationType;

typedef struct
{
  MxActorManager              *manager;
  gulong                       id;
  MxActorManagerOperationType  type;

  MxActorManagerCreateFunc     create_func;
  gpointer                     userdata;

  ClutterActor                *actor;
  ClutterContainer            *container;
} MxActorManagerOperation;

struct _MxActorManagerPrivate
{
  GQueue       *ops;

  GHashTable   *actor_op_count;

  guint         source;
  gulong        post_paint_handler;

  GTimer       *timer;
  guint         time_slice;

  ClutterStage *stage;
};

static guint signals[LAST_SIGNAL] = { 0, };

static void mx_actor_manager_handle_op (MxActorManager *manager);
static void mx_actor_manager_stage_destroyed (gpointer  data,
                                              GObject  *old_stage);

static guint mx_actor_manager_increment_count (MxActorManager *manager,
                                               gpointer        actor);
static guint mx_actor_manager_decrement_count (MxActorManager *manager,
                                               gpointer        actor);

static void mx_actor_manager_ensure_processing (MxActorManager *manager);

static void
mx_actor_manager_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MxActorManagerPrivate *priv = MX_ACTOR_MANAGER (object)->priv;

  switch (property_id)
    {
    case PROP_TIME_SLICE:
      g_value_set_uint (value, priv->time_slice);
      break;

    case PROP_N_OPERATIONS:
      g_value_set_uint (value, g_queue_get_length (priv->ops));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_actor_manager_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MxActorManager *self = MX_ACTOR_MANAGER (object);

  switch (property_id)
    {
    case PROP_TIME_SLICE:
      mx_actor_manager_set_time_slice (self, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_actor_manager_dispose (GObject *object)
{
  MxActorManager *self = MX_ACTOR_MANAGER (object);
  MxActorManagerPrivate *priv = self->priv;

  if (priv->source)
    {
      g_source_remove (priv->source);
      priv->source = 0;
    }

  if (priv->post_paint_handler)
    {
      if (priv->stage)
        g_signal_handler_disconnect (priv->stage, priv->post_paint_handler);
      priv->post_paint_handler = 0;
    }

  while (g_queue_get_length (priv->ops))
    {
      MxActorManagerOperation *op = g_queue_peek_head (priv->ops);
      mx_actor_manager_cancel_operation (self, op->id);
    }

  if (priv->stage)
    {
      g_object_set_qdata (G_OBJECT (priv->stage), actor_manager_quark, NULL);
      g_object_weak_unref (G_OBJECT (priv->stage),
                           mx_actor_manager_stage_destroyed,
                           self);
      priv->stage = NULL;
    }

  G_OBJECT_CLASS (mx_actor_manager_parent_class)->dispose (object);
}

static void
mx_actor_manager_finalize (GObject *object)
{
  MxActorManagerPrivate *priv = MX_ACTOR_MANAGER (object)->priv;

  g_queue_free (priv->ops);
  g_hash_table_unref (priv->actor_op_count);
  g_timer_destroy (priv->timer);

  G_OBJECT_CLASS (mx_actor_manager_parent_class)->finalize (object);
}

static void
mx_actor_manager_class_init (MxActorManagerClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxActorManagerPrivate));

  object_class->get_property = mx_actor_manager_get_property;
  object_class->set_property = mx_actor_manager_set_property;
  object_class->dispose = mx_actor_manager_dispose;
  object_class->finalize = mx_actor_manager_finalize;

  pspec = g_param_spec_object ("stage",
                               "Stage",
                               "The stage that contains the managed actors.",
                               CLUTTER_TYPE_STAGE,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_STAGE, pspec);

  pspec = g_param_spec_uint ("time-slice",
                             "Time slice",
                             "The amount of time to spend performing "
                             "operations, per frame, in ms",
                             0, G_MAXUINT, 5,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TIME_SLICE, pspec);

  pspec = g_param_spec_uint ("n-operations",
                             "N operations",
                             "The amount of operations in the queue",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_N_OPERATIONS, pspec);

  /**
   * MxActorManager::actor-created
   * @manager: the object that received the signal
   * @id: The operation ID
   * @actor: The created #ClutterActor
   *
   * Emitted when an actor creation operation has completed.
   */
  signals[ACTOR_CREATED] =
    g_signal_new ("actor-created",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, actor_created),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT_OBJECT,
                  G_TYPE_NONE, 2,
                  G_TYPE_UINT, CLUTTER_TYPE_ACTOR);

  /**
   * MxActorManager::actor-added
   * @manager: the object that received the signal
   * @id: The operation ID
   * @container: The #ClutterContainer the actor was added to
   * @actor: The added #ClutterActor
   *
   * Emitted when an actor add operation has completed.
   */
  signals[ACTOR_ADDED] =
    g_signal_new ("actor-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, actor_added),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT_OBJECT_OBJECT,
                  G_TYPE_NONE, 3,
                  G_TYPE_UINT, CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR);

  /**
   * MxActorManager::actor-removed
   * @manager: the object that received the signal
   * @id: The operation ID
   * @container: The #ClutterContainer the actor was removed from
   * @actor: The removed #ClutterActor
   *
   * Emitted when an actor remove operation has completed.
   */
  signals[ACTOR_REMOVED] =
    g_signal_new ("actor-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, actor_removed),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT_OBJECT_OBJECT,
                  G_TYPE_NONE, 3,
                  G_TYPE_UINT, CLUTTER_TYPE_ACTOR, CLUTTER_TYPE_ACTOR);

  /**
   * MxActorManager::actor-finished
   * @manager: the object that received the signal
   * @actor: The #ClutterActor to which the signal pertains
   *
   * Emitted when all queued operations involving @actor have completed.
   */
  signals[ACTOR_FINISHED] =
    g_signal_new ("actor-finished",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, actor_created),
                  NULL, NULL,
                  _mx_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  CLUTTER_TYPE_ACTOR);

  /**
   * MxActorManager::operation-completed
   * @manager: the object that received the signal
   * @id: The operation id
   *
   * Emitted when an operation has completed successfully.
   */
  signals[OP_COMPLETED] =
    g_signal_new ("operation-completed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, operation_completed),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT,
                  G_TYPE_NONE, 1,
                  G_TYPE_UINT);

  /**
   * MxActorManager::operation-cancelled
   * @manager: the object that received the signal
   * @id: The operation id
   *
   * Emitted when an operation has been cancelled.
   */
  signals[OP_CANCELLED] =
    g_signal_new ("operation-cancelled",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, operation_cancelled),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT,
                  G_TYPE_NONE, 1,
                  G_TYPE_UINT);

  /**
   * MxActorManager::operation-failed
   * @manager: the object that received the signal
   * @id: The operation id
   * @error: A #GError describing the reason of the failure
   *
   * Emitted when an operation has failed.
   */
  signals[OP_FAILED] =
    g_signal_new ("operation-failed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxActorManagerClass, operation_failed),
                  NULL, NULL,
                  _mx_marshal_VOID__UINT_BOXED,
                  G_TYPE_NONE, 2,
                  G_TYPE_UINT, G_TYPE_ERROR);

  actor_manager_quark = g_quark_from_static_string ("mx-actor-manager");
  actor_manager_error_quark =
    g_quark_from_static_string ("mx-actor-manager-error");
}

static void
mx_actor_manager_init (MxActorManager *self)
{
  MxActorManagerPrivate *priv = self->priv = ACTOR_MANAGER_PRIVATE (self);

  priv->ops = g_queue_new ();
  priv->actor_op_count = g_hash_table_new (NULL, NULL);
  priv->timer = g_timer_new ();
  priv->time_slice = 5;
}

static void
mx_actor_manager_stage_destroyed (gpointer  data,
                                  GObject  *old_stage)
{
  MxActorManager *self = MX_ACTOR_MANAGER (data);
  MxActorManagerPrivate *priv = self->priv;

  priv->stage = NULL;
  g_object_unref (self);
}

/**
 * mx_actor_manager_get_for_stage:
 * @stage: A #ClutterStage
 *
 * Get the MxActorManager associated with a stage, or create one if none exist
 * for the specified stage.
 *
 * Returns: (transfer none): An #MxActorManager
 */
MxActorManager *
mx_actor_manager_get_for_stage (ClutterStage *stage)
{
  MxActorManager *manager;

  g_return_val_if_fail (CLUTTER_IS_STAGE (stage), NULL);

  manager = g_object_get_qdata (G_OBJECT (stage), actor_manager_quark);

  if (manager == NULL)
    {
      MxActorManagerPrivate *priv;

      manager = g_object_new (MX_TYPE_ACTOR_MANAGER, NULL);
      priv = manager->priv;

      priv->stage = stage;

      g_object_set_qdata (G_OBJECT (stage), actor_manager_quark, manager);
      g_object_weak_ref (G_OBJECT (stage),
                         mx_actor_manager_stage_destroyed, manager);

      g_object_notify (G_OBJECT (manager), "stage");
    }

  return manager;
}

static guint
mx_actor_manager_increment_count (MxActorManager *manager,
                                  gpointer        actor)
{
  guint count;
  MxActorManagerPrivate *priv = manager->priv;

  count = GPOINTER_TO_UINT (g_hash_table_lookup (priv->actor_op_count, actor));
  g_hash_table_insert (priv->actor_op_count, actor, GUINT_TO_POINTER (++count));

  return count;
}

static guint
mx_actor_manager_decrement_count (MxActorManager *manager,
                                  gpointer        actor)
{
  guint count;
  MxActorManagerPrivate *priv = manager->priv;

  count = GPOINTER_TO_UINT (g_hash_table_lookup (priv->actor_op_count, actor));

  if (--count == 0)
    {
      g_hash_table_remove (priv->actor_op_count, actor);
      g_signal_emit (manager, signals[ACTOR_FINISHED], 0, actor);
    }
  else
    g_hash_table_insert (priv->actor_op_count, actor, GUINT_TO_POINTER (count));

  return count;
}

static void
mx_actor_manager_actor_destroyed (gpointer  data,
                                  GObject  *old_actor)
{
  MxActorManagerOperation *op = data;
  MxActorManagerPrivate *priv = op->manager->priv;

  g_hash_table_remove (priv->actor_op_count, old_actor);
  op->actor = NULL;
}

static void
mx_actor_manager_container_destroyed (gpointer  data,
                                      GObject  *old_actor)
{
  MxActorManagerOperation *op = data;
  MxActorManagerPrivate *priv = op->manager->priv;

  g_hash_table_remove (priv->actor_op_count, old_actor);
  op->container = NULL;
}

static MxActorManagerOperation *
mx_actor_manager_op_new (MxActorManager              *manager,
                         MxActorManagerOperationType  type,
                         MxActorManagerCreateFunc     create_func,
                         gpointer                     userdata,
                         ClutterActor                *actor,
                         ClutterContainer            *container)
{
  MxActorManagerPrivate *priv = manager->priv;
  MxActorManagerOperation *op = g_slice_new0 (MxActorManagerOperation);

  op->manager = manager;

  if (g_queue_peek_tail (priv->ops))
    op->id = ((MxActorManagerOperation *)g_queue_peek_tail (priv->ops))->id + 1;
  else
    op->id = 1;

  op->type = type;
  op->create_func = create_func;
  op->userdata = userdata;
  op->actor = actor;
  op->container = container;

  if (actor)
    {
      g_object_weak_ref (G_OBJECT (actor),
                         mx_actor_manager_actor_destroyed,
                         op);
      mx_actor_manager_increment_count (manager, actor);

      if (type == MX_ACTOR_MANAGER_ADD)
        g_object_ref_sink (actor);
    }

  if (container)
    {
      g_object_weak_ref (G_OBJECT (container),
                         mx_actor_manager_container_destroyed,
                         op);
      mx_actor_manager_increment_count (manager, container);
    }

  g_queue_push_tail (priv->ops, op);

  return op;
}

static void
mx_actor_manager_op_free (MxActorManager          *manager,
                          MxActorManagerOperation *op,
                          gboolean                 remove)
{
  MxActorManagerPrivate *priv = manager->priv;

  if (remove)
    g_queue_remove (priv->ops, op);

  if (op->actor)
    {
      mx_actor_manager_decrement_count (manager, op->actor);
      g_object_weak_unref (G_OBJECT (op->actor),
                           mx_actor_manager_actor_destroyed,
                           op);

      if (op->type == MX_ACTOR_MANAGER_ADD)
        g_object_unref (op->actor);
    }

  if (op->container)
    {
      mx_actor_manager_decrement_count (manager, op->container);
      g_object_weak_unref (G_OBJECT (op->container),
                           mx_actor_manager_container_destroyed,
                           op);
    }

  g_slice_free (MxActorManagerOperation, op);
}

static void
mx_actor_manager_handle_op (MxActorManager *manager)
{
  ClutterActor *actor;

  GError *error = NULL;
  MxActorManagerPrivate *priv = manager->priv;
  MxActorManagerOperation *op = g_queue_peek_head (priv->ops);

  if (!op)
    return;

  /* We want the actor and container to remain alive during this function,
   * for the purposes of signal emission.
   */
  if (op->actor)
    g_object_ref (op->actor);

  if (op->container)
    g_object_ref (op->container);

  switch (op->type)
    {
    case MX_ACTOR_MANAGER_CREATE:
      actor = op->create_func (manager, op->userdata);

      if (CLUTTER_IS_ACTOR (actor))
        g_signal_emit (manager, signals[ACTOR_CREATED], 0,
                       op->id, actor);
      else
        error = g_error_new (actor_manager_error_quark,
                             MX_ACTOR_MANAGER_CREATION_FAILED,
                             "Actor creation function did not "
                             "return a ClutterActor");
      break;

    case MX_ACTOR_MANAGER_ADD:
      if (op->container)
        {
          if (op->actor)
            {
              clutter_container_add_actor (op->container, op->actor);
              g_signal_emit (manager, signals[ACTOR_ADDED], 0,
                             op->id, op->container, op->actor);
            }
          else
            error = g_error_new (actor_manager_error_quark,
                                 MX_ACTOR_MANAGER_ACTOR_DESTROYED,
                                 "Actor destroyed before addition");
        }
      else
        error = g_error_new (actor_manager_error_quark,
                             MX_ACTOR_MANAGER_CONTAINER_DESTROYED,
                             "Container destroyed before addition");
      break;

    case MX_ACTOR_MANAGER_REMOVE:
      if (op->container)
        {
          if (op->actor)
            {
              clutter_container_remove_actor (op->container, op->actor);
              g_signal_emit (manager, signals[ACTOR_ADDED], 0,
                             op->id, op->container, op->actor);
            }
          else
            error = g_error_new (actor_manager_error_quark,
                                 MX_ACTOR_MANAGER_ACTOR_DESTROYED,
                                 "Actor destroyed before removal");
        }
      else
        error = g_error_new (actor_manager_error_quark,
                             MX_ACTOR_MANAGER_CONTAINER_DESTROYED,
                             "Container destroyed before removal");
      break;

    default:
      g_warning (G_STRLOC ": Unrecognised operation type (%d) "
                 "- Memory corruption?)", op->type);
      error = g_error_new (actor_manager_error_quark,
                           MX_ACTOR_MANAGER_UNKNOWN_OPERATION,
                           "Unrecognised operation, possibly due to "
                           "memory corruption.");
      break;
    }

  if (error)
    {
      g_signal_emit (manager, signals[OP_FAILED], 0, op->id, error);
      g_error_free (error);
    }
  else
    g_signal_emit (manager, signals[OP_COMPLETED], 0, op->id);

  if (op->actor)
    g_object_unref (op->actor);

  if (op->container)
    g_object_unref (op->container);

  mx_actor_manager_op_free (manager, op, TRUE);
}

static void
mx_actor_manager_post_paint_cb (ClutterActor   *stage,
                                MxActorManager *manager)
{
  MxActorManagerPrivate *priv = manager->priv;

  g_signal_handler_disconnect (stage, priv->post_paint_handler);
  priv->post_paint_handler = 0;

  mx_actor_manager_ensure_processing (manager);
}

static gboolean
mx_actor_manager_process_operations (MxActorManager *manager)
{
  MxActorManagerPrivate *priv = manager->priv;

  priv->source = 0;

  g_timer_start (priv->timer);

  while (g_queue_get_length (priv->ops))
    {
      mx_actor_manager_handle_op (manager);

      if (g_timer_elapsed (priv->timer, NULL) * 1000 >= priv->time_slice)
        break;
    }

  g_timer_stop (priv->timer);

  if (g_queue_get_length (priv->ops))
    {
      if (!priv->post_paint_handler)
        priv->post_paint_handler =
          g_signal_connect (priv->stage, "paint",
                            G_CALLBACK (mx_actor_manager_post_paint_cb),
                            manager);
    }

  return FALSE;
}

static void
mx_actor_manager_ensure_processing (MxActorManager *manager)
{
  MxActorManagerPrivate *priv = manager->priv;

  if (!priv->source)
    priv->source =
      g_idle_add_full (G_PRIORITY_HIGH,
                       (GSourceFunc)mx_actor_manager_process_operations,
                       manager,
                       NULL);
}

/**
 * mx_actor_manager_create_actor:
 * @manager: A #MxActorManager
 * @create_func: A #ClutterActor creation function
 * @userdata: data to be passed to the function, or %NULL
 * @destroy_func: callback to invoke before the operation is removed
 *
 * Creates a #ClutterActor. The actor may not be created immediately,
 * or at all, if the operation is cancelled.
 *
 * On successful completion, the #MxActorManager::actor_created signal will
 * be fired.
 *
 * Returns: The ID for this operation.
 */
gulong
mx_actor_manager_create_actor (MxActorManager           *manager,
                               MxActorManagerCreateFunc  create_func,
                               gpointer                  userdata,
                               GDestroyNotify            destroy_func)
{
  MxActorManagerPrivate *priv;
  MxActorManagerOperation *op;

  g_return_val_if_fail (MX_IS_ACTOR_MANAGER (manager), 0);
  g_return_val_if_fail (create_func != NULL, 0);

  priv = manager->priv;
  op = mx_actor_manager_op_new (manager,
                                MX_ACTOR_MANAGER_CREATE,
                                create_func,
                                userdata,
                                NULL,
                                NULL);

  mx_actor_manager_ensure_processing (manager);

  return op->id;
}

/**
 * mx_actor_manager_add_actor:
 * @manager: A #MxActorManager
 * @container: A #ClutterContainer
 * @actor: A #ClutterActor
 *
 * Adds @actor to @container. The actor may not be parented immediately,
 * or at all, if the operation is cancelled.
 *
 * On successful completion, the #MxActorManager::actor_added signal will
 * be fired.
 *
 * Returns: The ID for this operation.
 */
gulong
mx_actor_manager_add_actor (MxActorManager   *manager,
                            ClutterContainer *container,
                            ClutterActor     *actor)
{
  MxActorManagerPrivate *priv;
  MxActorManagerOperation *op;

  g_return_val_if_fail (MX_IS_ACTOR_MANAGER (manager), 0);
  g_return_val_if_fail (CLUTTER_IS_CONTAINER (container), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), 0);

  priv = manager->priv;
  op = mx_actor_manager_op_new (manager,
                                MX_ACTOR_MANAGER_ADD,
                                NULL,
                                NULL,
                                actor,
                                container);

  mx_actor_manager_ensure_processing (manager);

  return op->id;
}

/**
 * mx_actor_manager_remove_actor:
 * @manager: A #MxActorManager
 * @container: A #ClutterContainer
 * @actor: A #ClutterActor
 *
 * Removes @actor from @container.
 *
 * On successful completion, the #MxActorManager::actor_removed signal will
 * be fired.
 *
 * <note><para>
 * The actor may not be removed immediately, and thus you may want to set
 * the actor's opacity to 0 before calling this function.
 * </para></note>
 *
 * Returns: The ID for this operation.
 */
gulong
mx_actor_manager_remove_actor (MxActorManager   *manager,
                               ClutterContainer *container,
                               ClutterActor     *actor)
{
  MxActorManagerPrivate *priv;
  MxActorManagerOperation *op;

  g_return_val_if_fail (MX_IS_ACTOR_MANAGER (manager), 0);
  g_return_val_if_fail (CLUTTER_IS_CONTAINER (container), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), 0);

  priv = manager->priv;
  op = mx_actor_manager_op_new (manager,
                                MX_ACTOR_MANAGER_REMOVE,
                                NULL,
                                NULL,
                                actor,
                                container);

  mx_actor_manager_ensure_processing (manager);

  return op->id;
}

/**
 * mx_actor_manager_remove_container:
 * @manager: A #MxActorManager
 * @container: A #ClutterContainer
 *
 * Removes the container. This is a utility function that works by first
 * removing all the children of the container, then the children itself. This
 * effectively spreads the load of removing a large container.
 *
 * <note><para>
 * The container may not be removed immediately, and thus you may want to set
 * the container's opacity to 0 before calling this function.
 * </para></note>
 */
void
mx_actor_manager_remove_container (MxActorManager   *manager,
                                   ClutterContainer *container)
{
  GList *children;
  ClutterActor *parent;
  MxActorManagerPrivate *priv;

  g_return_if_fail (MX_IS_ACTOR_MANAGER (manager));
  g_return_if_fail (CLUTTER_IS_CONTAINER (container));

  priv = manager->priv;

  children = clutter_container_get_children (container);
  while (children)
    {
      ClutterActor *child = children->data;
      mx_actor_manager_op_new (manager,
                               MX_ACTOR_MANAGER_REMOVE,
                               NULL,
                               NULL,
                               child,
                               container);
      children = g_list_delete_link (children, children);
    }

  parent = clutter_actor_get_parent (CLUTTER_ACTOR (container));
  if (parent && CLUTTER_IS_CONTAINER (parent))
    mx_actor_manager_op_new (manager,
                             MX_ACTOR_MANAGER_REMOVE,
                             NULL,
                             NULL,
                             (ClutterActor *)container,
                             (ClutterContainer *)parent);

  mx_actor_manager_ensure_processing (manager);
}

static gint
mx_actor_manager_find_by_id (gconstpointer a,
                             gconstpointer b)
{
  const MxActorManagerOperation *op = a;
  const gulong *id = b;

  return (op->id == *id) ? 0 : -1;
}

/**
 * mx_actor_manager_cancel_operation:
 * @manager: A #MxActorManager
 * @id: An operation ID
 *
 * Cancels the given operation, if it exists. The
 * #MxActorManager::operation_cancelled signal is fired whenever an operation
 * is cancelled.
 */
void
mx_actor_manager_cancel_operation (MxActorManager *manager,
                                   gulong          id)
{
  GList *op_link;
  MxActorManagerPrivate *priv;
  MxActorManagerOperation *op;

  g_return_if_fail (MX_IS_ACTOR_MANAGER (manager));
  g_return_if_fail (id > 0);

  priv = manager->priv;

  op_link = g_queue_find_custom (priv->ops, &id, mx_actor_manager_find_by_id);

  if (!op_link)
    {
      g_warning (G_STRLOC ": Unknown operation (%lu)", id);
      return;
    }

  op = op_link->data;
  g_queue_delete_link (priv->ops, op_link);

  g_signal_emit (manager, signals[OP_CANCELLED], 0, id);

  mx_actor_manager_op_free (manager, op, FALSE);
}

/**
 * mx_actor_manager_set_time_slice:
 * @manager: A #MxActorManager
 * @msecs: A time, in milliseconds
 *
 * Sets the amount of time the actor manager will spend performing operations,
 * before yielding to allow any necessary redrawing to occur.
 *
 * Lower times will lead to smoother performance, but will increase the amount
 * of time it takes for operations to complete.
 */
void
mx_actor_manager_set_time_slice (MxActorManager *manager,
                                 guint           msecs)
{
  MxActorManagerPrivate *priv;

  g_return_if_fail (MX_IS_ACTOR_MANAGER (manager));

  priv = manager->priv;

  if (priv->time_slice != msecs)
    {
      priv->time_slice = msecs;
      g_object_notify (G_OBJECT (manager), "time-slice");
    }
}

/**
 * mx_actor_manager_get_time_slice:
 * @manager: A #MxActorManager
 *
 * Retrieves the current time slice being used for operations.
 *
 * Returns: The time-slice being used, in milliseconds
 */
guint
mx_actor_manager_get_time_slice (MxActorManager *manager)
{
  g_return_val_if_fail (MX_IS_ACTOR_MANAGER (manager), 0);
  return manager->priv->time_slice;
}

/**
 * mx_actor_manager_get_n_operations:
 * @manager: A #MxActorManager
 *
 * Retrieves the amount of operations left in the queue.
 *
 * Returns: Number of operations left to perform
 */
guint
mx_actor_manager_get_n_operations (MxActorManager *manager)
{
  g_return_val_if_fail (MX_IS_ACTOR_MANAGER (manager), 0);
  return g_queue_get_length (manager->priv->ops);
}

