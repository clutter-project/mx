#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-droppable.h"
#include "nbtk-enum-types.h"
#include "nbtk-marshal.h"
#include "nbtk-private.h"

typedef struct _DropContext     DropContext;

enum
{
  OVER_IN,
  OVER_OUT,
  DROP,

  LAST_SIGNAL
};

static guint droppable_signals[LAST_SIGNAL] = { 0, };
static GQuark quark_drop_context = 0;

struct _DropContext
{
  ClutterActor *stage;

  GSList *targets;

  NbtkDroppable *last_target;

  guint is_over : 1;
};

static gboolean
on_stage_capture (ClutterActor *actor,
                  ClutterEvent *event,
                  DropContext  *context)
{
  NbtkDroppable *droppable;
  NbtkDraggable *draggable;
  ClutterActor *target;
  gfloat event_x, event_y;

  if (!(event->type == CLUTTER_MOTION ||
        event->type == CLUTTER_BUTTON_RELEASE))
    return FALSE;

  draggable = g_object_get_data (G_OBJECT (actor), "nbtk-drag-actor");
  if (G_UNLIKELY (draggable == NULL))
    return FALSE;

  /* get the actor currently under the cursor; we hide the draggable
   * so that it does not intefere with get_actor_at_pos(); the paint
   * that get_actor_at_pos() performs is in the back buffer so the
   * hide/show cycle will not be visible on screen
   */
  clutter_event_get_coords (event, &event_x, &event_y);

  clutter_actor_hide (CLUTTER_ACTOR (draggable));

  target = clutter_stage_get_actor_at_pos (CLUTTER_STAGE (actor),
                                           CLUTTER_PICK_REACTIVE,
                                           event_x,
                                           event_y);

  clutter_actor_show (CLUTTER_ACTOR (draggable));

  if (G_UNLIKELY (target == NULL))
    return FALSE;

  droppable = NULL;
  if (!NBTK_IS_DROPPABLE (target))
    {
      ClutterActor *parent = target;

      /* check if we're not on a child of a droppable */
      while (parent != NULL)
        {
          parent = clutter_actor_get_parent (parent);
          if (parent != NULL &&
              NBTK_IS_DROPPABLE (parent) &&
              nbtk_droppable_accept_drop (NBTK_DROPPABLE (parent), draggable))
            {
              droppable = NBTK_DROPPABLE (parent);
              break;
            }
        }
    }
  else
    {
      if (nbtk_droppable_accept_drop (NBTK_DROPPABLE (target), draggable))
        droppable = NBTK_DROPPABLE (target);
    }

  /* we are on a new target, so emit ::over-out and unset the last target */
  if (context->last_target && droppable != context->last_target)
    {
      g_signal_emit (context->last_target,
                     droppable_signals[OVER_OUT], 0,
                     draggable);

      context->last_target = NULL;
      return FALSE;
    }


  if (droppable == NULL)
    return FALSE;

  if (event->type == CLUTTER_MOTION)
    {
      if (context->last_target == NULL)
        {
          context->last_target = droppable;
          g_signal_emit (context->last_target,
                         droppable_signals[OVER_IN], 0,
                         draggable);
        }
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      gfloat drop_x, drop_y;
      gboolean res;
      ClutterActor *last_target = CLUTTER_ACTOR (context->last_target);

      drop_x = drop_y = 0;
      res = clutter_actor_transform_stage_point (last_target,
                                                 event_x, event_y,
                                                 &drop_x, &drop_y);
      if (!res)
        return FALSE;

      g_signal_emit (context->last_target,
                     droppable_signals[DROP], 0,
                     draggable,
                     drop_x,
                     drop_y,
                     event->button.button,
                     event->button.modifier_state);

      context->last_target = NULL;
    }

  return FALSE;
}

static void
drop_context_destroy (gpointer data)
{
  if (G_LIKELY (data != NULL))
    {
      DropContext *context = data;

      g_slist_free (context->targets);
      g_object_unref (context->stage);
      g_slice_free (DropContext, context);
    }
}

static void
drop_context_update (DropContext   *context,
                     NbtkDroppable *droppable)
{
  context->targets = g_slist_prepend (context->targets, droppable);
}

static DropContext *
drop_context_create (ClutterActor  *stage,
                     NbtkDroppable *droppable)
{
  DropContext *retval;

  retval = g_slice_new (DropContext);
  retval->stage = g_object_ref (stage);
  retval->targets = g_slist_prepend (NULL, droppable);
  retval->last_target = NULL;
  retval->is_over = FALSE;

  g_object_set_qdata_full (G_OBJECT (stage), quark_drop_context,
                           retval,
                           drop_context_destroy);

  return retval;
}

static void
nbtk_droppable_real_enable (NbtkDroppable *droppable)
{
  ClutterActor *stage;
  DropContext *context;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (droppable));
  if (G_UNLIKELY (stage == NULL))
    {
      g_warning ("A NbtkDroppable must be on the stage before "
                 "being enabled.");
      return;
    }

  context = g_object_get_qdata (G_OBJECT (stage), quark_drop_context);
  if (context == NULL)
    {
      context = drop_context_create (stage, droppable);

      g_signal_connect_after (stage, "captured-event",
                              G_CALLBACK (on_stage_capture),
                              context);
    }
  else
    drop_context_update (context, droppable);
}

static void
nbtk_droppable_real_disable (NbtkDroppable *droppable)
{
  ClutterActor *stage;
  DropContext *context;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (droppable));
  if (G_UNLIKELY (stage == NULL))
    return;

  context = g_object_get_qdata (G_OBJECT (stage), quark_drop_context);
  if (G_UNLIKELY (context == NULL))
    return;

  context->targets = g_slist_remove (context->targets, droppable);
  if (context->targets == NULL)
    {
      g_signal_handlers_disconnect_by_func (stage,
                                            G_CALLBACK (on_stage_capture),
                                            context);

      g_object_set_qdata (G_OBJECT (droppable), quark_drop_context, NULL);
    }
}

static gboolean
nbtk_droppable_real_accept_drop (NbtkDroppable *droppable,
                                 NbtkDraggable *draggable)
{
  /* we always accept by default */
  return TRUE;
}

static void
nbtk_droppable_base_init (gpointer g_iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      NbtkDroppableIface *iface = g_iface;
      GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);
      GParamSpec *pspec;

      is_initialized = TRUE;

      quark_drop_context =
        g_quark_from_static_string ("nbtk-droppable-context");

      pspec = g_param_spec_boolean ("enabled",
                                    "Enabled",
                                    "Whether the Droppable is enabled",
                                    FALSE,
                                    NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      droppable_signals[OVER_IN] =
        g_signal_new (I_("over-in"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDroppableIface, over_in),
                      NULL, NULL,
                      _nbtk_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1,
                      CLUTTER_TYPE_ACTOR);

      droppable_signals[OVER_OUT] =
        g_signal_new (I_("over-out"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDroppableIface, over_out),
                      NULL, NULL,
                      _nbtk_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1,
                      CLUTTER_TYPE_ACTOR);

      droppable_signals[DROP] =
        g_signal_new (I_("drop"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDroppableIface, drop),
                      NULL, NULL,
                      _nbtk_marshal_VOID__OBJECT_FLOAT_FLOAT_INT_ENUM,
                      G_TYPE_NONE, 5,
                      CLUTTER_TYPE_ACTOR,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT,
                      G_TYPE_INT,
                      CLUTTER_TYPE_MODIFIER_TYPE);

      iface->enable = nbtk_droppable_real_enable;
      iface->disable = nbtk_droppable_real_disable;
      iface->accept_drop = nbtk_droppable_real_accept_drop;
    }
}

GType
nbtk_droppable_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    {
      const GTypeInfo droppable_info = {
        sizeof (NbtkDroppableIface),
        nbtk_droppable_base_init,
        NULL, /* base_finalize */
      };

      our_type = g_type_register_static (G_TYPE_INTERFACE,
                                         I_("NbtkDroppable"),
                                         &droppable_info, 0);

      g_type_interface_add_prerequisite (our_type, CLUTTER_TYPE_ACTOR);
    }

  return our_type;
}

void
nbtk_droppable_enable (NbtkDroppable *droppable)
{
  g_return_if_fail (NBTK_IS_DROPPABLE (droppable));

  NBTK_DROPPABLE_GET_IFACE (droppable)->enable (droppable);
}

void
nbtk_droppable_disable (NbtkDroppable *droppable)
{
  g_return_if_fail (NBTK_IS_DROPPABLE (droppable));

  NBTK_DROPPABLE_GET_IFACE (droppable)->disable (droppable);
}

gboolean
nbtk_droppable_is_enabled (NbtkDroppable *droppable)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (NBTK_IS_DROPPABLE (droppable), FALSE);

  g_object_get (G_OBJECT (droppable), "enabled", &retval, NULL);

  return retval;
}

gboolean
nbtk_droppable_accept_drop (NbtkDroppable *droppable,
                            NbtkDraggable *draggable)
{
  g_return_val_if_fail (NBTK_IS_DROPPABLE (droppable), FALSE);
  g_return_val_if_fail (NBTK_IS_DRAGGABLE (draggable), FALSE);

  return NBTK_DROPPABLE_GET_IFACE (droppable)->accept_drop (droppable,
                                                            draggable);
}
