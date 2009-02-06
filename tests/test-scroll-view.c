
#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

#define RECT_W 300
#define RECT_H 300
#define RECT_N 20
#define RECT_GAP 50

static void
viewport_x_origin_notify_cb (NbtkViewport *viewport,
                             GParamSpec *args1,
                             ClutterActor *group)
{
  GList *children, *c;
  gint origin_x, width;

  nbtk_viewport_get_origin (viewport, &origin_x, NULL, NULL);
  width = clutter_actor_get_width (
            clutter_actor_get_parent (CLUTTER_ACTOR (viewport)));

  children = clutter_container_get_children (CLUTTER_CONTAINER (group));
  for (c = children; c; c = c->next)
    {
      gint x;
      gdouble pos;
      ClutterActor *actor;

      actor = (ClutterActor *)c->data;

      /* Get actor position with respect to viewport origin */
      x = clutter_actor_get_x (actor) - origin_x;
      pos = (((gdouble)x / (gdouble)(width-RECT_W)) - 0.5) * 2.0;

      /* Apply a function that transforms the actor depending on its
       * viewport position.
       */
      pos = CLAMP(pos * 3.0, -0.5, 0.5);
      clutter_actor_set_rotation (actor, CLUTTER_Y_AXIS, -pos * 120.0,
                                  RECT_W/2, 0, RECT_H/2);
      clutter_actor_set_depth (actor, -ABS(pos) * RECT_W);
    }
  g_list_free (children);
}

static void
add_rects (ClutterActor *stage, ClutterActor *group)
{
  gint i;

  ClutterColor colour = { 0x72, 0x9f, 0xcf, 0xff };

  for (i = 0; i < RECT_N; i++)
    {
      ClutterActor *rect = clutter_rectangle_new_with_color (&colour);

      clutter_container_add_actor (CLUTTER_CONTAINER (group), rect);

      clutter_actor_set_size (rect, RECT_W, RECT_H);
      clutter_actor_set_position (rect, i * RECT_GAP,
                                  CLUTTER_STAGE_HEIGHT ()/2 - RECT_H/2);

      colour.red = g_random_int_range (0, 255);
      colour.green = g_random_int_range (0, 255);
    }
}

int
main (int argc, char **argv)
{
  gdouble lower, upper;
  NbtkAdjustment *hadjust;
  ClutterActor *stage, *scroll, *viewport, *group;
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  viewport = nbtk_viewport_new ();
  group = nbtk_depth_group_new ();
  add_rects (stage, group);
  clutter_container_add_actor (CLUTTER_CONTAINER (viewport), group);
  g_signal_connect (viewport, "notify::x-origin",
                    G_CALLBACK (viewport_x_origin_notify_cb), group);

  g_object_set (G_OBJECT (viewport), "sync-adjustments", FALSE, NULL);
  nbtk_scrollable_get_adjustments (NBTK_SCROLLABLE (viewport), &hadjust, NULL);
  nbtk_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower -= RECT_W - RECT_GAP;
  upper += RECT_W - RECT_GAP;
  g_object_set (G_OBJECT (hadjust), "lower", lower, "upper", upper, NULL);

  scroll = nbtk_scroll_view_new ();

  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);

  clutter_actor_set_size (scroll, 800, 600);

  clutter_actor_show_all (stage);
  clutter_actor_show_all (viewport);
  clutter_actor_show_all (group);
  clutter_actor_show (scroll);

  g_object_notify (G_OBJECT (viewport), "x-origin");

  cogl_enable_depth_test (TRUE);
  clutter_main ();

  return 0;
}
