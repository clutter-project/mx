/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-draggable.c: draggable interface
 *
 * Copyright 2009 Intel Corporation.
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
 * Written by: Emmanuele Bassi <ebassi@linux.intel.com>
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-draggable.h"
#include "mx-enum-types.h"
#include "mx-marshal.h"
#include "mx-private.h"

typedef struct _DragContext DragContext;

struct _DragContext
{
  MxDraggable        *draggable;
  ClutterActor       *stage;
  ClutterActor       *actor;

  guint               threshold;

  MxDragAxis          axis;

#if 0
  MxDragContainment   containment;
  ClutterActorBox    *containment_area;
#endif

  gfloat              press_x;
  gfloat              press_y;
  guint               press_button;
  ClutterModifierType press_modifiers;

  gfloat              last_x;
  gfloat              last_y;

  guint               emit_delayed_press : 1;
  guint               in_drag            : 1;
};

enum
{
  DRAG_BEGIN,
  DRAG_MOTION,
  DRAG_END,

  LAST_SIGNAL
};

static GQuark quark_draggable_context = 0;
static guint draggable_signals[LAST_SIGNAL] = { 0, };

static gboolean on_stage_capture (ClutterActor *stage,
                                  ClutterEvent *event,
                                  DragContext  *context);

static gboolean
draggable_release (DragContext        *context,
                   ClutterButtonEvent *event)
{
  ClutterActor *stage, *actor;
  gfloat event_x, event_y;
  gfloat actor_x, actor_y;
  gboolean res;

  if (!context->in_drag)
    return FALSE;

  event_x = event->x;
  event_y = event->y;
  actor_x = 0;
  actor_y = 0;

  if (context->actor && !context->emit_delayed_press)
    actor = context->actor;
  else
    actor = CLUTTER_ACTOR (context->draggable);

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (context->draggable));

  context->last_x = actor_x;
  context->last_y = actor_y;

  context->in_drag = FALSE;

  g_signal_handlers_disconnect_by_func (stage,
                                        G_CALLBACK (on_stage_capture),
                                        context);

  if (!context->emit_delayed_press)
    g_signal_emit (context->draggable, draggable_signals[DRAG_END], 0,
                   context->last_x,
                   context->last_y);

  g_object_set_data (G_OBJECT (stage), "mx-drag-actor", NULL);

  return FALSE;
}

static gboolean
draggable_motion (DragContext        *context,
                  ClutterMotionEvent *event)
{
  gfloat event_x, event_y;
  gfloat actor_x, actor_y;
  gfloat delta_x, delta_y;
  ClutterActor *actor;
  gboolean res;

  if (!context->in_drag)
    return FALSE;

  event_x = event->x;
  event_y = event->y;
  actor_x = 0;
  actor_y = 0;

  if (context->actor && !context->emit_delayed_press)
    actor = context->actor;
  else
    actor = CLUTTER_ACTOR (context->draggable);

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  context->last_x = actor_x;
  context->last_y = actor_y;

  delta_x = delta_y = 0;

  if (context->axis == 0)
    {
      delta_x = context->last_x - context->press_x;
      delta_y = context->last_y - context->press_y;
    }
  else
    {
      if (context->axis == MX_DRAG_AXIS_X)
        delta_x = context->last_x - context->press_x;
      else
        delta_y = context->last_y - context->press_y;
    }

  if (context->emit_delayed_press)
    {
      if (ABS (delta_x) >= context->threshold
          || ABS (delta_y) >= context->threshold)
        {
          ClutterActor *stage;

          context->emit_delayed_press = FALSE;

          g_signal_emit (context->draggable, draggable_signals[DRAG_BEGIN], 0,
                         context->press_x,
                         context->press_y,
                         context->press_button,
                         context->press_modifiers);

          actor = CLUTTER_ACTOR (context->draggable);
          stage = clutter_actor_get_stage (actor);
          g_object_set_data (G_OBJECT (stage), "mx-drag-actor", actor);
        }
      else
        return FALSE;
    }

  g_signal_emit (context->draggable, draggable_signals[DRAG_MOTION], 0,
                 delta_x,
                 delta_y);

  return FALSE;
}

static gboolean
on_stage_capture (ClutterActor *stage,
                  ClutterEvent *event,
                  DragContext  *context)
{
  switch (event->type)
    {
    case CLUTTER_MOTION:
      if (context->in_drag)
        {
          ClutterMotionEvent *mevent = (ClutterMotionEvent *) event;

          /* We can miss release events in the case of grabs, so check that
           * the button is still down here.
           */
          if (!(mevent->modifier_state & CLUTTER_BUTTON1_MASK))
            return draggable_release (context, (ClutterButtonEvent *) event);
          else
            return draggable_motion (context, (ClutterMotionEvent *) event);
        }
      break;

    case CLUTTER_BUTTON_RELEASE:
      if (context->in_drag)
        return draggable_release (context, (ClutterButtonEvent *) event);
      break;

    default:
      break;
    }

  return FALSE;
}

static gboolean
on_draggable_press (ClutterActor       *actor,
                    ClutterButtonEvent *event,
                    DragContext        *context)
{
  MxDraggable *draggable = context->draggable;
  ClutterActor *stage;
  gfloat event_x, event_y;
  gfloat actor_x, actor_y;
  gboolean res;

  event_x = event->x;
  event_y = event->y;
  actor_x = 0;
  actor_y = 0;

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  stage = clutter_actor_get_stage (actor);

  context->press_x = actor_x;
  context->press_y = actor_y;
  context->last_x = context->press_x;
  context->last_y = context->press_y;
  context->press_button = event->button;
  context->press_modifiers = event->modifier_state;
  context->emit_delayed_press = FALSE;

  g_object_get (G_OBJECT (draggable),
                "drag-threshold", &context->threshold,
                "axis", &context->axis,
#if 0
                "containment-type", &context->containment,
                "containment-area", &context->containment_area,
#endif
                "drag-actor", &context->actor,
                NULL);

  if (context->threshold == 0)
    {
      g_signal_emit (draggable, draggable_signals[DRAG_BEGIN], 0,
                     context->press_x,
                     context->press_y,
                     context->press_button,
                     context->press_modifiers);

      g_object_set_data (G_OBJECT (stage), "mx-drag-actor", actor);
    }
  else
    context->emit_delayed_press = TRUE;

  context->in_drag = TRUE;

  context->stage = stage;
  g_signal_connect_after (stage,
                          "captured-event", G_CALLBACK (on_stage_capture),
                          context);

  return FALSE;
}

static void
drag_context_free (gpointer data)
{
  if (G_LIKELY (data))
    {
      DragContext *context = data;

      /* disconnect any signal handlers we may have installed */
      g_signal_handlers_disconnect_by_func (context->draggable,
                                            G_CALLBACK (on_draggable_press),
                                            context);
      if (context->stage)
        {
          g_signal_handlers_disconnect_by_func (context->stage,
                                                G_CALLBACK (on_stage_capture),
                                                context);
          context->stage = NULL;
        }

      if (context->actor)
        {
          g_object_unref (G_OBJECT (context->actor));
          context->actor = NULL;
        }
#if 0
      if (context->containment_area)
        g_boxed_free (CLUTTER_TYPE_ACTOR_BOX, context->containment_area);
#endif

      g_slice_free (DragContext, context);
    }
}

static DragContext *
drag_context_create (MxDraggable *draggable)
{
  DragContext *context;

  context = g_slice_new (DragContext);

  context->draggable = draggable;
  context->threshold = 0;
  context->axis = 0;
#if 0
  context->containment = MX_DISABLE_CONTAINMENT;
  context->containment_area = NULL;
#endif
  context->in_drag = FALSE;
  context->emit_delayed_press = FALSE;
  context->stage = NULL;
  context->actor = NULL;

  /* attach the context to the draggable */
  g_object_set_qdata_full (G_OBJECT (draggable), quark_draggable_context,
                           context,
                           drag_context_free);

  return context;
}

static void
mx_draggable_real_enable (MxDraggable *draggable)
{
  DragContext *context;
  ClutterActor *stage;

  context = g_object_get_qdata (G_OBJECT (draggable), quark_draggable_context);
  if (G_UNLIKELY (context != NULL))
    return;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (draggable));
  if (G_UNLIKELY (stage == NULL))
    {
      g_warning ("Draggable actors can only be enabled when they "
                 "on the stage");
      return;
    }

  context = drag_context_create (draggable);
  g_signal_connect (draggable,
                    "button-press-event", G_CALLBACK (on_draggable_press),
                    context);

  g_object_notify (G_OBJECT (draggable), "drag-enabled");
}

static void
mx_draggable_real_disable (MxDraggable *draggable)
{
  DragContext *context;
  ClutterActor *stage;

  context = g_object_get_qdata (G_OBJECT (draggable), quark_draggable_context);
  if (G_UNLIKELY (context == NULL))
    return;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (draggable));

  g_signal_handlers_disconnect_by_func (draggable,
                                        G_CALLBACK (on_draggable_press),
                                        context);
  g_signal_handlers_disconnect_by_func (stage,
                                        G_CALLBACK (on_stage_capture),
                                        context);
  context->stage = NULL;

  g_object_set_qdata (G_OBJECT (draggable), quark_draggable_context, NULL);

  g_object_notify (G_OBJECT (draggable), "drag-enabled");
}

static void
mx_draggable_base_init (gpointer g_iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      MxDraggableIface *iface = g_iface;
      GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);
      GParamSpec *pspec;

      is_initialized = TRUE;

      quark_draggable_context =
        g_quark_from_static_string ("mx-draggable-context");

      pspec = g_param_spec_boolean ("drag-enabled",
                                    "Drag Enabled",
                                    "Whether the Draggable is enabled",
                                    TRUE,
                                    MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_uint ("drag-threshold",
                                 "Drag Threshold",
                                 "The amount of pixels required to "
                                 "start dragging",
                                 0, G_MAXUINT,
                                 0,
                                 MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

#if 0
      pspec = g_param_spec_enum ("containment-type",
                                 "Containment Type",
                                 "The type of containment to be used",
                                 MX_TYPE_DRAG_CONTAINMENT,
                                 MX_DISABLE_CONTAINMENT,
                                 MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_boxed ("containment-area",
                                  "Containment Area",
                                  "The area to which the draggable is "
                                  "contained",
                                  CLUTTER_TYPE_ACTOR_BOX,
                                  MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);
#endif

      pspec = g_param_spec_enum ("axis",
                                 "Axis",
                                 "The axis along which the dragging "
                                 "should be performed",
                                 MX_TYPE_DRAG_AXIS,
                                 MX_DRAG_AXIS_NONE,
                                 MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_object ("drag-actor",
                                   "Drag Actor",
                                   "An actor to use in place of the "
                                   "draggable while dragging.",
                                   CLUTTER_TYPE_ACTOR,
                                   MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      draggable_signals[DRAG_BEGIN] =
        g_signal_new (g_intern_static_string ("drag-begin"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (MxDraggableIface, drag_begin),
                      NULL, NULL,
                      _mx_marshal_VOID__FLOAT_FLOAT_INT_ENUM,
                      G_TYPE_NONE, 4,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT,
                      G_TYPE_INT,
                      CLUTTER_TYPE_MODIFIER_TYPE);

      draggable_signals[DRAG_MOTION] =
        g_signal_new (g_intern_static_string ("drag-motion"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (MxDraggableIface, drag_motion),
                      NULL, NULL,
                      _mx_marshal_VOID__FLOAT_FLOAT,
                      G_TYPE_NONE, 2,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT);

      draggable_signals[DRAG_END] =
        g_signal_new (g_intern_static_string ("drag-end"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (MxDraggableIface, drag_end),
                      NULL, NULL,
                      _mx_marshal_VOID__FLOAT_FLOAT,
                      G_TYPE_NONE, 2,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT);

      iface->enable = mx_draggable_real_enable;
      iface->disable = mx_draggable_real_disable;
    }
}

GType
mx_draggable_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    {
      const GTypeInfo draggable_info = {
        sizeof (MxDraggableIface),
        mx_draggable_base_init,
        NULL, /* base_finalize */
      };

      our_type = g_type_register_static (G_TYPE_INTERFACE,
                                         g_intern_static_string ("MxDraggable"),
                                         &draggable_info, 0);

      g_type_interface_add_prerequisite (our_type, CLUTTER_TYPE_ACTOR);
    }

  return our_type;
}

void
mx_draggable_set_axis (MxDraggable *draggable,
                       MxDragAxis   axis)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "axis", axis, NULL);
}

MxDragAxis
mx_draggable_get_axis (MxDraggable *draggable)
{
  MxDragAxis retval = 0;

  g_return_val_if_fail (MX_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "axis", &retval, NULL);

  return retval;
}

void
mx_draggable_set_drag_threshold (MxDraggable *draggable,
                                 guint        threshold)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "drag-threshold", threshold, NULL);
}

guint
mx_draggable_get_drag_threshold (MxDraggable *draggable)
{
  guint retval = 0;

  g_return_val_if_fail (MX_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "drag-threshold", &retval, NULL);

  return retval;
}
#if 0
void
mx_draggable_set_containment_type (MxDraggable      *draggable,
                                   MxDragContainment containment)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "containment-type", containment, NULL);
}

MxDragContainment
mx_draggable_get_containment_type (MxDraggable *draggable)
{
  MxDragContainment retval = MX_DISABLE_CONTAINMENT;

  g_return_val_if_fail (MX_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "containment-type", &retval, NULL);

  return retval;
}

void
mx_draggable_set_containment_area (MxDraggable *draggable,
                                   gfloat       x_1,
                                   gfloat       y_1,
                                   gfloat       x_2,
                                   gfloat       y_2)
{
  ClutterActorBox box;

  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  box.x1 = x_1;
  box.y1 = y_1;
  box.x2 = x_2;
  box.y2 = y_2;

  g_object_set (G_OBJECT (draggable), "containment-area", &box, NULL);
}

void
mx_draggable_get_containment_area (MxDraggable *draggable,
                                   gfloat      *x_1,
                                   gfloat      *y_1,
                                   gfloat      *x_2,
                                   gfloat      *y_2)
{
  ClutterActorBox *box = NULL;

  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  g_object_get (G_OBJECT (draggable), "containment-area", &box, NULL);

  if (box == NULL)
    return;

  if (x_1)
    *x_1 = box->x1;

  if (y_1)
    *y_1 = box->y1;

  if (x_2)
    *x_2 = box->x2;

  if (y_2)
    *y_2 = box->y2;

  g_boxed_free (CLUTTER_TYPE_ACTOR_BOX, box);
}
#endif

void
mx_draggable_set_drag_actor (MxDraggable  *draggable,
                             ClutterActor *actor)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "drag-actor", actor, NULL);
}

/**
 * mx_draggable_get_drag_actor:
 * @draggable: a #MxDraggable
 *
 * FIXME
 *
 * Return value: (transfer none): a #ClutterActor, or %NULL
 */
ClutterActor *
mx_draggable_get_drag_actor (MxDraggable *draggable)
{
  ClutterActor *actor = NULL;

  g_return_val_if_fail (MX_IS_DRAGGABLE (draggable), NULL);

  g_object_get (G_OBJECT (draggable), "drag-actor", &actor, NULL);

  return actor;
}

void
mx_draggable_enable (MxDraggable *draggable)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  MX_DRAGGABLE_GET_IFACE (draggable)->enable (draggable);
}

void
mx_draggable_disable (MxDraggable *draggable)
{
  g_return_if_fail (MX_IS_DRAGGABLE (draggable));

  MX_DRAGGABLE_GET_IFACE (draggable)->disable (draggable);
}

gboolean
mx_draggable_is_enabled (MxDraggable *draggable)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (MX_IS_DRAGGABLE (draggable), FALSE);

  g_object_get (G_OBJECT (draggable), "drag-enabled", &retval, NULL);

  return retval;
}
