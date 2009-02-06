
#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

#define RECT_W 300
#define RECT_H 300
#define RECT_N 20
#define RECT_GAP 50

static ClutterTimeline *timeline;
static gint start, target;

static void
stage_key_press_event_cb (ClutterActor *actor, ClutterKeyEvent *event,
                          NbtkViewport *viewport)
{
  gint old_target;

  old_target = target;

  switch (event->keyval)
    {
    case CLUTTER_Left :
    case CLUTTER_KP_Left :
      target -= RECT_GAP;
      break;
    case CLUTTER_Right :
    case CLUTTER_KP_Right :
      target += RECT_GAP;
      break;
    }

  if (target != old_target)
    {
      nbtk_viewport_get_origin (viewport, &start, NULL, NULL);
      if (clutter_timeline_is_playing (timeline))
        {
          clutter_timeline_stop (timeline);
          if (ABS (target - start) > ABS (old_target - start))
            clutter_timeline_set_n_frames (timeline, (30 -
                                           clutter_timeline_get_current_frame (
                                           timeline)) + 15);
          else
            clutter_timeline_set_n_frames (timeline, 30 - MAX (1,
                                           clutter_timeline_get_current_frame (
                                           timeline) - 15));
          clutter_timeline_rewind (timeline);
          clutter_timeline_start (timeline);
        }
      else
        {
          clutter_timeline_set_n_frames (timeline, 30);
          clutter_timeline_rewind (timeline);
          clutter_timeline_start (timeline);
        }
    }
}

static void
new_frame_cb (ClutterTimeline *timeline, gint frame_num, ClutterActor *viewport)
{
  gint y, z;
  gdouble progress;

  nbtk_viewport_get_origin (NBTK_VIEWPORT (viewport), NULL, &y, &z);
  progress = clutter_timeline_get_progress (timeline);
  nbtk_viewport_set_origin (NBTK_VIEWPORT (viewport),
                            (gint)((progress * (gdouble)target) +
                            ((1.0 - progress) * (gdouble)start)), y, z);
}

static void
viewport_x_origin_notify_cb (NbtkViewport *viewport,
                             GParamSpec *args1,
                             ClutterActor *group)
{
  GList *children, *c;
  gint origin_x, width;

  nbtk_viewport_get_origin (viewport, &origin_x, NULL, NULL);
  clutter_actor_get_clip (CLUTTER_ACTOR (viewport),
                          NULL, NULL, &width, NULL);

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
  ClutterActor *stage, *viewport, *group;
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  viewport = nbtk_viewport_new ();
  clutter_actor_set_clip (viewport, 0, 0, 800, 600);
  group = nbtk_depth_group_new ();
  add_rects (stage, group);
  clutter_container_add_actor (CLUTTER_CONTAINER (viewport), group);
  g_signal_connect (viewport, "notify::x-origin",
                    G_CALLBACK (viewport_x_origin_notify_cb), group);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), viewport);

  clutter_actor_show_all (group);
  clutter_actor_show_all (viewport);
  clutter_actor_show_all (stage);

  nbtk_viewport_set_origin (NBTK_VIEWPORT (viewport), RECT_W/2, 0, 0);
  target = RECT_W/2;
  timeline = clutter_timeline_new (30, 60);
  g_signal_connect (timeline, "new_frame",
                    G_CALLBACK (new_frame_cb), viewport);

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (stage_key_press_event_cb), viewport);

  g_object_notify (G_OBJECT (viewport), "x-origin");

  clutter_main ();

  return 0;
}
