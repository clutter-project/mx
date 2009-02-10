#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-draggable.h"
#include "nbtk-enum-types.h"
#include "nbtk-marshal.h"
#include "nbtk-private.h"

typedef struct _DragContext     DragContext;

struct _DragContext
{
  NbtkDraggable *draggable;

  guint threshold;

  NbtkDragAxis axis;

  NbtkDragContainment containment;
  ClutterActorBox *containment_area;

  gfloat press_x;
  gfloat press_y;
  guint press_button;
  ClutterModifierType press_modifiers;

  gfloat last_x;
  gfloat last_y;

  guint emit_press : 1;
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

static gboolean on_draggable_press   (ClutterActor       *actor,
                                      ClutterButtonEvent *event,
                                      gpointer            dummy);
static gboolean on_draggable_motion  (ClutterActor       *actor,
                                      ClutterMotionEvent *event,
                                      gpointer            dummy);
static gboolean on_draggable_release (ClutterActor       *actor,
                                      ClutterButtonEvent *event,
                                      gpointer            dummy);

static gboolean
on_draggable_release (ClutterActor       *actor,
                      ClutterButtonEvent *event,
                      gpointer            data)
{
  DragContext *context = data;
  ClutterUnit event_x, event_y;
  ClutterUnit actor_x, actor_y;
  gboolean res;

  event_x = CLUTTER_UNITS_FROM_DEVICE (event->x);
  event_y = CLUTTER_UNITS_FROM_DEVICE (event->y);
  actor_x = 0;
  actor_y = 0;

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  context->last_x = CLUTTER_UNITS_TO_FLOAT (actor_x);
  context->last_y = CLUTTER_UNITS_TO_FLOAT (actor_y);

  g_signal_handlers_disconnect_by_func (actor,
                                        G_CALLBACK (on_draggable_motion),
                                        data);
  g_signal_handlers_disconnect_by_func (actor,
                                        G_CALLBACK (on_draggable_release),
                                        data);

  g_signal_emit (context->draggable, draggable_signals[DRAG_END], 0,
                 context->last_x,
                 context->last_y);

  return TRUE;
}

static gboolean
on_draggable_motion (ClutterActor       *actor,
                     ClutterMotionEvent *event,
                     gpointer            data)
{
  DragContext *context = data;
  ClutterUnit event_x, event_y;
  ClutterUnit actor_x, actor_y;
  gfloat delta_x, delta_y;
  gboolean res;

  event_x = CLUTTER_UNITS_FROM_DEVICE (event->x);
  event_y = CLUTTER_UNITS_FROM_DEVICE (event->y);
  actor_x = 0;
  actor_y = 0;

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  context->last_x = CLUTTER_UNITS_TO_FLOAT (actor_x);
  context->last_y = CLUTTER_UNITS_TO_FLOAT (actor_y);

  delta_x = context->last_x - context->press_x;
  delta_y = context->last_y - context->press_y;

  if (context->emit_press)
    {
      if (delta_x >= context->threshold || delta_y >= context->threshold)
        {
          context->emit_press = FALSE;
          g_signal_emit (context->draggable, draggable_signals[DRAG_BEGIN], 0,
                         context->press_x,
                         context->press_y,
                         context->press_button,
                         context->press_modifiers);
        }
      else
        return FALSE;
    }

  g_signal_emit (context->draggable, draggable_signals[DRAG_MOTION], 0,
                 delta_x,
                 delta_y);

  return TRUE;
}

static gboolean
on_draggable_press (ClutterActor       *actor,
                    ClutterButtonEvent *event,
                    gpointer            dummy)
{
  NbtkDraggable *draggable = NBTK_DRAGGABLE (actor);
  DragContext *context;
  ClutterUnit event_x, event_y;
  ClutterUnit actor_x, actor_y;
  gboolean res;

  event_x = CLUTTER_UNITS_FROM_DEVICE (event->x);
  event_y = CLUTTER_UNITS_FROM_DEVICE (event->y);
  actor_x = 0;
  actor_y = 0;

  res = clutter_actor_transform_stage_point (actor,
                                             event_x, event_y,
                                             &actor_x, &actor_y);
  if (!res)
    return FALSE;

  context = g_object_get_qdata (G_OBJECT (draggable), quark_draggable_context);
  context->press_x = CLUTTER_UNITS_TO_FLOAT (actor_x);
  context->press_y = CLUTTER_UNITS_TO_FLOAT (actor_y);
  context->last_x = context->press_x;
  context->last_y = context->press_y;
  context->press_button = event->button;
  context->press_modifiers = event->modifier_state;
  context->emit_press = FALSE;

  g_object_get (G_OBJECT (draggable),
                "drag-threshold", &context->threshold,
                "axis", &context->axis,
                "containment-type", &context->containment,
                "containment-area", &context->containment_area,
                NULL);

  g_signal_connect (actor,
                    "motion-event", G_CALLBACK (on_draggable_motion),
                    context);
  g_signal_connect (actor,
                    "button-release-event", G_CALLBACK (on_draggable_release),
                    context);

  if (context->threshold == 0)
    {
      g_signal_emit (draggable, draggable_signals[DRAG_BEGIN], 0,
                     context->press_x,
                     context->press_y,
                     context->press_button,
                     context->press_modifiers);
    }
  else
    context->emit_press = TRUE;

  return FALSE;
}

static void
drag_context_free (gpointer data)
{
  if (G_LIKELY (data))
    {
      DragContext *context = data;

      if (context->containment_area)
        g_boxed_free (CLUTTER_TYPE_ACTOR_BOX, context->containment_area);

      g_slice_free (DragContext, context);
    }
}

static DragContext *
drag_context_create (NbtkDraggable *draggable)
{
  DragContext *context;

  context = g_slice_new (DragContext);

  context->draggable = draggable;
  context->threshold = 0;
  context->axis = 0;
  context->containment = NBTK_DISABLE_CONTAINMENT;
  context->containment_area = NULL;

  g_object_set_qdata_full (G_OBJECT (draggable), quark_draggable_context,
                           context,
                           drag_context_free);

  return context;
}

static void
nbtk_draggable_real_enable (NbtkDraggable *draggable)
{
  DragContext *context;

  context = g_object_get_qdata (G_OBJECT (draggable), quark_draggable_context);
  if (G_UNLIKELY (context != NULL))
    return;

  context = drag_context_create (draggable);
  g_signal_connect (draggable,
                    "button-press-event", G_CALLBACK (on_draggable_press),
                    context);

  g_object_notify (G_OBJECT (draggable), "enabled");
}

static void
nbtk_draggable_real_disable (NbtkDraggable *draggable)
{
  DragContext *context;

  context = g_object_get_qdata (G_OBJECT (draggable), quark_draggable_context);
  if (G_UNLIKELY (context == NULL))
    return;

  g_signal_handlers_disconnect_by_func (draggable,
                                        G_CALLBACK (on_draggable_press),
                                        context);
  g_signal_handlers_disconnect_by_func (draggable,
                                        G_CALLBACK (on_draggable_motion),
                                        context);
  g_signal_handlers_disconnect_by_func (draggable,
                                        G_CALLBACK (on_draggable_release),
                                        context);

  g_object_set_qdata (G_OBJECT (draggable), quark_draggable_context, NULL);

  g_object_notify (G_OBJECT (draggable), "enabled");
}

static void
nbtk_draggable_base_init (gpointer g_iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      NbtkDraggableIface *iface = g_iface;
      GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);
      GParamSpec *pspec;

      is_initialized = TRUE;

      quark_draggable_context =
        g_quark_from_static_string ("nbtk-draggable-context");

      pspec = g_param_spec_boolean ("enabled",
                                    "Enabled",
                                    "Whether the Draggable is enabled",
                                    TRUE,
                                    NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_uint ("drag-threshold",
                                 "Drag Threshold",
                                 "The amount of pixels required to "
                                 "start dragging",
                                 0, G_MAXUINT,
                                 0,
                                 NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_enum ("containment-type",
                                 "Containment Type",
                                 "The type of containment to be used",
                                 NBTK_TYPE_DRAG_CONTAINMENT,
                                 NBTK_DISABLE_CONTAINMENT,
                                 NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_boxed ("containment-area",
                                  "Containment Area",
                                  "The area to which the draggable is "
                                  "contained",
                                  CLUTTER_TYPE_ACTOR_BOX,
                                  NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_enum ("axis",
                                 "Axis",
                                 "The axis along which the dragging "
                                 "should be performed",
                                 NBTK_TYPE_DRAG_AXIS,
                                 0,
                                 NBTK_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      draggable_signals[DRAG_BEGIN] =
        g_signal_new (I_("drag-begin"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDraggableIface, drag_begin),
                      NULL, NULL,
                      _nbtk_marshal_VOID__FLOAT_FLOAT_INT_ENUM,
                      G_TYPE_NONE, 4,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT,
                      G_TYPE_INT,
                      CLUTTER_TYPE_MODIFIER_TYPE);

      draggable_signals[DRAG_MOTION] =
        g_signal_new (I_("drag-motion"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDraggableIface, drag_motion),
                      NULL, NULL,
                      _nbtk_marshal_VOID__FLOAT_FLOAT,
                      G_TYPE_NONE, 2,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT);

      draggable_signals[DRAG_END] =
        g_signal_new (I_("drag-end"),
                      iface_type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (NbtkDraggableIface, drag_end),
                      NULL, NULL,
                      _nbtk_marshal_VOID__FLOAT_FLOAT,
                      G_TYPE_NONE, 2,
                      G_TYPE_FLOAT,
                      G_TYPE_FLOAT);

      iface->enable = nbtk_draggable_real_enable;
      iface->disable = nbtk_draggable_real_disable;
    }
}

GType
nbtk_draggable_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    {
      const GTypeInfo draggable_info = {
        sizeof (NbtkDraggableIface),
        nbtk_draggable_base_init,
        NULL, /* base_finalize */
      };

      our_type = g_type_register_static (G_TYPE_INTERFACE,
                                         I_("NbtkDraggable"),
                                         &draggable_info, 0);
    }

  return our_type;
}

void
nbtk_draggable_set_axis (NbtkDraggable *draggable,
                         NbtkDragAxis   axis)
{
  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "axis", axis, NULL);
}

NbtkDragAxis
nbtk_draggable_get_axis (NbtkDraggable *draggable)
{
  NbtkDragAxis retval = 0;

  g_return_val_if_fail (NBTK_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "axis", &retval, NULL);

  return retval;
}

void
nbtk_draggable_set_drag_threshold (NbtkDraggable *draggable,
                                   guint          threshold)
{
  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "drag-threshold", threshold, NULL);
}

guint
nbtk_draggable_get_drag_threshold (NbtkDraggable *draggable)
{
  guint retval = 0;

  g_return_val_if_fail (NBTK_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "drag-threshold", &retval, NULL);

  return retval;
}

void
nbtk_draggable_set_containment_type (NbtkDraggable       *draggable,
                                     NbtkDragContainment  containment)
{
  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  g_object_set (G_OBJECT (draggable), "containment-type", containment, NULL);
}

NbtkDragContainment
nbtk_draggable_get_containment_type (NbtkDraggable *draggable)
{
  NbtkDragContainment retval = NBTK_DISABLE_CONTAINMENT;

  g_return_val_if_fail (NBTK_IS_DRAGGABLE (draggable), 0);

  g_object_get (G_OBJECT (draggable), "containment-type", &retval, NULL);

  return retval;
}

void
nbtk_draggable_set_containment_area (NbtkDraggable *draggable,
                                     gfloat         x_1,
                                     gfloat         y_1,
                                     gfloat         x_2,
                                     gfloat         y_2)
{
  ClutterActorBox box;

  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  box.x1 = x_1;
  box.y1 = y_1;
  box.x2 = x_2;
  box.y2 = y_2;

  g_object_set (G_OBJECT (draggable), "containment-area", &box, NULL);
}

void
nbtk_draggable_get_containment_area (NbtkDraggable *draggable,
                                     gfloat        *x_1,
                                     gfloat        *y_1,
                                     gfloat        *x_2,
                                     gfloat        *y_2)
{
  ClutterActorBox *box = NULL;

  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

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

void
nbtk_draggable_enable (NbtkDraggable *draggable)
{
  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  NBTK_DRAGGABLE_GET_IFACE (draggable)->enable (draggable);
}

void
nbtk_draggable_disable (NbtkDraggable *draggable)
{
  g_return_if_fail (NBTK_IS_DRAGGABLE (draggable));

  NBTK_DRAGGABLE_GET_IFACE (draggable)->disable (draggable);
}

gboolean
nbtk_draggable_is_enabled (NbtkDraggable *draggable)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (NBTK_IS_DRAGGABLE (draggable), FALSE);

  g_object_get (G_OBJECT (draggable), "enabled", &retval, NULL);

  return retval;
}
