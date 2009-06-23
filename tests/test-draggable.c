#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cogl/cogl.h>
#include <cogl/cogl-pango.h>
#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

#define DRAGGABLE_TYPE_RECTANGLE        (draggable_rectangle_get_type ())
#define DRAGGABLE_RECTANGLE(obj)        (G_TYPE_CHECK_INSTANCE_CAST ((obj), DRAGGABLE_TYPE_RECTANGLE, DraggableRectangle))
#define DRAGGABLE_IS_RECTANGLE(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DRAGGABLE_TYPE_RECTANGLE))

typedef struct _DraggableRectangle      DraggableRectangle;
typedef struct _DraggableRectangleClass DraggableRectangleClass;

struct _DraggableRectangle
{
  ClutterRectangle parent_instance;

  /* Draggable properties */
  guint threshold;

  NbtkDragAxis axis;

  NbtkDragContainment containment;
  ClutterActorBox area;

  guint is_enabled : 1;
};

struct _DraggableRectangleClass
{
  ClutterRectangleClass parent_class;
};

enum
{
  PROP_0,

  PROP_DRAG_THRESHOLD,
  PROP_AXIS,
  PROP_CONTAINMENT_TYPE,
  PROP_CONTAINMENT_AREA,
  PROP_ENABLED
};

static void nbtk_draggable_iface_init (NbtkDraggableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DraggableRectangle,
                         draggable_rectangle,
                         CLUTTER_TYPE_RECTANGLE,
                         G_IMPLEMENT_INTERFACE (NBTK_TYPE_DRAGGABLE,
                                                nbtk_draggable_iface_init));

static void
draggable_rectangle_drag_begin (NbtkDraggable       *draggable,
                                gfloat               event_x,
                                gfloat               event_y,
                                gint                 event_button,
                                ClutterModifierType  modifiers)
{
  ClutterActor *self = CLUTTER_ACTOR (draggable);

  g_debug ("%s: drag of '%s' begin at %.2f, %.2f",
           G_STRLOC,
           clutter_actor_get_name (self),
           event_x,
           event_y);

  clutter_actor_set_opacity (self, 224);
  clutter_actor_set_rotation (self, CLUTTER_Y_AXIS,
                              30.0,
                              clutter_actor_get_width (self) / 2.0,
                              0,
                              clutter_actor_get_height (self) / 2.0);
}

static void
draggable_rectangle_drag_motion (NbtkDraggable *draggable,
                                 gfloat         delta_x,
                                 gfloat         delta_y)
{
  g_debug ("%s: drag motion of '%s' (dx: %.2f, dy: %.2f)",
           G_STRLOC,
           clutter_actor_get_name (CLUTTER_ACTOR (draggable)),
           delta_x,
           delta_y);

  clutter_actor_move_by (CLUTTER_ACTOR (draggable), delta_x, delta_y);
}

static void
draggable_rectangle_drag_end (NbtkDraggable *draggable,
                              gfloat         event_x,
                              gfloat         event_y)
{
  ClutterActor *self = CLUTTER_ACTOR (draggable);

  g_debug ("%s: drag of '%s' end at %.2f, %.2f",
           G_STRLOC,
           clutter_actor_get_name (CLUTTER_ACTOR (draggable)),
           event_x,
           event_y);

  clutter_actor_set_opacity (self, 255);
  clutter_actor_set_rotation (self, CLUTTER_Y_AXIS, 0.0, 0.0, 0.0, 0.0);
}

static void
nbtk_draggable_iface_init (NbtkDraggableIface *iface)
{
  iface->drag_begin = draggable_rectangle_drag_begin;
  iface->drag_motion = draggable_rectangle_drag_motion;
  iface->drag_end = draggable_rectangle_drag_end;
}

static void
draggable_rectangle_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  DraggableRectangle *rect = DRAGGABLE_RECTANGLE (gobject);

  switch (prop_id)
    {
    case PROP_DRAG_THRESHOLD:
      rect->threshold = g_value_get_uint (value);
      break;

    case PROP_AXIS:
      rect->axis = g_value_get_enum (value);
      break;

    case PROP_CONTAINMENT_TYPE:
      rect->containment = g_value_get_enum (value);
      break;

    case PROP_CONTAINMENT_AREA:
      {
        ClutterActorBox *box = g_value_get_boxed (value);

        if (box)
          rect->area = *box;
        else
          memset (&rect->area, 0, sizeof (ClutterActorBox));
      }
      break;

    case PROP_ENABLED:
      rect->is_enabled = g_value_get_boolean (value);
      if (rect->is_enabled)
        nbtk_draggable_enable (NBTK_DRAGGABLE (gobject));
      else
        nbtk_draggable_disable (NBTK_DRAGGABLE (gobject));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
draggable_rectangle_get_property (GObject    *gobject,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  DraggableRectangle *rect = DRAGGABLE_RECTANGLE (gobject);

  switch (prop_id)
    {
    case PROP_DRAG_THRESHOLD:
      g_value_set_uint (value, rect->threshold);
      break;

    case PROP_AXIS:
      g_value_set_enum (value, rect->axis);
      break;

    case PROP_CONTAINMENT_TYPE:
      g_value_set_enum (value, rect->containment);
      break;

    case PROP_CONTAINMENT_AREA:
      g_value_set_boxed (value, &rect->area);
      break;

    case PROP_ENABLED:
      g_value_set_boolean (value, rect->is_enabled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
draggable_rectangle_paint (ClutterActor *actor)
{
  gfloat text_x, text_y;
  ClutterActorBox box = { 0, };
  gint layout_width, layout_height;
  CoglColor color = { 0, };
  PangoLayout *layout;
  const gchar *name;

  CLUTTER_ACTOR_CLASS (draggable_rectangle_parent_class)->paint (actor);

  name = clutter_actor_get_name (actor);

  clutter_actor_get_allocation_box (actor, &box);

  layout = clutter_actor_create_pango_layout (actor, name);
  pango_layout_get_size (layout, &layout_width, &layout_height);

  text_x = ((box.x2 - box.x1) - (layout_width / 1024)) / 2;
  text_y = ((box.y2 - box.y1) - (layout_height / 1024)) / 2;

  cogl_color_set_from_4ub (&color, 0, 0, 0, 255);
  cogl_pango_render_layout (layout,
                            (int) text_x,
                            (int) text_y,
                            &color, 0);

  g_object_unref (layout);
}

static void
draggable_rectangle_class_init (DraggableRectangleClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->set_property = draggable_rectangle_set_property;
  gobject_class->get_property = draggable_rectangle_get_property;

  actor_class->paint = draggable_rectangle_paint;

  g_object_class_override_property (gobject_class,
                                    PROP_DRAG_THRESHOLD,
                                    "drag-threshold");
  g_object_class_override_property (gobject_class,
                                    PROP_AXIS,
                                    "axis");
  g_object_class_override_property (gobject_class,
                                    PROP_CONTAINMENT_TYPE,
                                    "containment-type");
  g_object_class_override_property (gobject_class,
                                    PROP_CONTAINMENT_AREA,
                                    "containment-area");
  g_object_class_override_property (gobject_class,
                                    PROP_ENABLED,
                                    "enabled");
}

static void
draggable_rectangle_init (DraggableRectangle *self)
{
  self->threshold = 0;
  self->axis = 0;
  self->containment = NBTK_DISABLE_CONTAINMENT;
  self->is_enabled = FALSE;
}

int
main (int argc, char *argv[])
{
  ClutterActor *stage;
  ClutterActor *draggable;
  ClutterColor rect_color = { 204, 204, 204, 255 };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_title (CLUTTER_STAGE (stage), "Draggable Example");
  clutter_actor_set_size (stage, 800, 600);

  draggable = g_object_new (DRAGGABLE_TYPE_RECTANGLE, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), draggable);
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (draggable), &rect_color);
  clutter_actor_set_size (draggable, 100, 100);
  clutter_actor_set_position (draggable, 350, 100);
  clutter_actor_set_reactive (draggable, TRUE);
  clutter_actor_set_name (draggable, "h-handle");
  nbtk_draggable_set_axis (NBTK_DRAGGABLE (draggable), NBTK_X_AXIS);
  nbtk_draggable_enable (NBTK_DRAGGABLE (draggable));

  draggable = g_object_new (DRAGGABLE_TYPE_RECTANGLE, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), draggable);
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (draggable), &rect_color);
  clutter_actor_set_size (draggable, 100, 100);
  clutter_actor_set_position (draggable, 350, 300);
  clutter_actor_set_reactive (draggable, TRUE);
  clutter_actor_set_name (draggable, "v-handle");
  nbtk_draggable_set_axis (NBTK_DRAGGABLE (draggable), NBTK_Y_AXIS);
  nbtk_draggable_enable (NBTK_DRAGGABLE (draggable));

  clutter_actor_show_all (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
