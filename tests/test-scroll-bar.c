
#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

#define RECT_W 300
#define RECT_H 300
#define RECT_N 20
#define RECT_GAP 50

static void
viewport_x_origin_notify_cb (NbtkViewport *viewport)
{
  GList *children, *c;
  gint origin_x, width;

  nbtk_viewport_get_origin (viewport, &origin_x, NULL, NULL);
  clutter_actor_get_clip (CLUTTER_ACTOR (viewport),
                          NULL, NULL, &width, NULL);

  children = clutter_container_get_children (CLUTTER_CONTAINER (viewport));
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
add_rects (ClutterActor *stage, ClutterActor *viewport)
{
  gint i;

  ClutterColor colour = { 0x72, 0x9f, 0xcf, 0xff };

  for (i = 0; i < RECT_N; i++)
    {
      ClutterActor *rect = clutter_rectangle_new_with_color (&colour);

      clutter_container_add_actor (CLUTTER_CONTAINER (viewport), rect);

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
  NbtkAdjustment *adjustment;
  ClutterActor *stage, *scroll, *viewport;
  ClutterActor  *tex, *frame;
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  viewport = nbtk_viewport_new ();
  clutter_actor_set_clip (viewport, 0, 0, 800, 600);
  g_signal_connect (viewport, "notify::x-origin",
                    G_CALLBACK (viewport_x_origin_notify_cb), viewport);
  add_rects (stage, viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), viewport);

  nbtk_scrollable_get_adjustments (NBTK_SCROLLABLE (viewport),
                                   &adjustment, NULL);
  scroll = nbtk_scroll_bar_new (adjustment);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);
  clutter_actor_set_position (scroll, 0, 500);
  clutter_actor_set_size (scroll, 800, 100);

  tex = g_object_new (CLUTTER_TYPE_TEXTURE,
		      "filename", "frame.png",
		      "disable-slicing", TRUE,
		      NULL);
  frame = nbtk_texture_frame_new (CLUTTER_TEXTURE(tex), 20, 20, 20, 20);
  scroll = nbtk_scroll_bar_new_with_handle (adjustment, frame);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);

  clutter_actor_set_position (scroll, 0, 0);
  clutter_actor_set_size (scroll, 800, 50);
  clutter_actor_set_rotation (scroll, CLUTTER_Z_AXIS, 5.0, 0, 0, 0);

  clutter_actor_show_all (stage);
  clutter_actor_show_all (viewport);

  g_object_notify (G_OBJECT (viewport), "x-origin");

  cogl_enable_depth_test (TRUE);
  clutter_main ();

  return 0;
}
