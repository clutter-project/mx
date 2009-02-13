#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-droppable.h"
#include "nbtk-enum-types.h"
#include "nbtk-marshal.h"
#include "nbtk-private.h"

enum
{
  OVER_IN,
  OVER_OUT,
  DROP,

  LAST_SIGNAL
};

static guint droppable_signals[LAST_SIGNAL] = { 0, };

static gboolean
on_stage_capture (ClutterActor  *actor,
                  ClutterEvent  *event,
                  NbtkDroppable *droppable)
{
  static gboolean on_over = FALSE;
  gboolean was_on_over;
  NbtkDraggable *draggable;
  ClutterActor *drop_actor;
  ClutterActor *target;
  gint event_x, event_y;

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
                                           event_x,
                                           event_y);

  clutter_actor_show (CLUTTER_ACTOR (draggable));

  if (G_UNLIKELY (target == NULL))
    return FALSE;

  /* fast path: if we are on the stage and we were on our actor
   * before, the we should simply emit ::over-out now
   */
  if (target == actor)
    {
      if (on_over)
        {
          g_signal_emit (droppable, droppable_signals[OVER_OUT], 0, draggable);
          on_over = FALSE;
        }

      return FALSE;
    }

  /* we might have been dropped on a composite actor */
  if (!NBTK_IS_DROPPABLE (target))
    {
      ClutterActor *parent;

      do
        {
          parent = clutter_actor_get_parent (target);
          target = parent;

          if (NBTK_IS_DROPPABLE (parent))
            break;
        }
      while (parent != NULL);

      if (parent == NULL)
        return FALSE;
    }

  was_on_over = on_over;

  if (target == CLUTTER_ACTOR (droppable))
    on_over = TRUE;
  else
    on_over = FALSE;

  if (!nbtk_droppable_accept_drop (droppable, draggable))
    return FALSE;

  if (event->type == CLUTTER_MOTION)
    {
      if (was_on_over && !on_over)
        g_signal_emit (droppable, droppable_signals[OVER_OUT], 0, draggable);
      else if (on_over)
        g_signal_emit (droppable, droppable_signals[OVER_IN], 0, draggable);
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      ClutterUnit x, y;
      ClutterUnit event_x, event_y;
      gboolean res = FALSE;

      x = CLUTTER_UNITS_FROM_DEVICE (event->button.x);
      y = CLUTTER_UNITS_FROM_DEVICE (event->button.y);

      res = clutter_actor_transform_stage_point (CLUTTER_ACTOR (droppable),
                                                 x, y,
                                                 &event_x, &event_y);
      if (!res)
        return FALSE;

      g_signal_emit (droppable, droppable_signals[DROP], 0,
                     draggable,
                     event_x, event_y,
                     event->button.button,
                     event->button.modifier_state);
    }
  else
    g_assert_not_reached ();

  return FALSE;
}

static void
nbtk_droppable_real_enable (NbtkDroppable *droppable)
{
  ClutterActor *stage;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (droppable));
  if (G_UNLIKELY (stage == NULL))
    {
      g_warning ("A NbtkDroppable must have a parent.");
      return;
    }

  g_signal_connect_after (stage, "captured-event",
                          G_CALLBACK (on_stage_capture),
                          droppable);
}

static void
nbtk_droppable_real_disable (NbtkDroppable *droppable)
{
  ClutterActor *stage;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (droppable));
  if (G_UNLIKELY (stage == NULL))
    {
      g_warning ("A NbtkDroppable must have a parent.");
      return;
    }

  g_signal_handlers_disconnect_by_func (stage,
                                        G_CALLBACK (on_stage_capture),
                                        droppable);
}

static gboolean
nbtk_droppable_real_accept_drop (NbtkDroppable *droppable,
                                 NbtkDraggable *draggable)
{
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

      is_initialized = TRUE;

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
