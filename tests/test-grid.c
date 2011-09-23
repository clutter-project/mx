/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2009, 2010 Intel Corporation.
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
 */

/*
 * A layout test, based on a layout test for another toolkit,
 * based on a layout test for a different layout manager
 * by Neil Roberts.
 */

#include <clutter/clutter.h>
#include <mx/mx.h>

#define CHILD_COUNT 11
#define CORNER_SIZE 10

typedef struct _CallbackData CallbackData;

struct _CallbackData
{
  ClutterActor *stage;
  ClutterActor *layout;
  ClutterActor *rect;
  ClutterActor *corners[4];
  int corner;
};


/* minimalistic actors needed to modify gobject properties */

ClutterActor *boolean_toggle    (gpointer     object,
                                 const gchar *prop_name,
                                 const gchar *label_text);
ClutterActor *normalized_slider (GObject     *object,
                                 const gchar *prop_name,
                                 const gchar *label_text);
ClutterActor *units_slider      (GObject     *object,
                                 const gchar *prop_name,
                                 const gchar *label_text);

static void add_one (ClutterActor *container)
{
  ClutterActor *rectangle;
  ClutterColor  color = {0x0,0x0,0x0,0xff};
  clutter_color_from_hls (&color,
                          g_random_double_range (0.0, 360.0),
                          g_random_double_range (0.0, 1.0),
                          g_random_double_range (0.0, 1.0));

  rectangle  = clutter_rectangle_new_with_color (&color);

  clutter_container_add_actor (CLUTTER_CONTAINER (container), rectangle);

  color.alpha = 255;

  clutter_actor_set_size (rectangle, g_random_int_range (20, 50),
                                     g_random_int_range (20, 50));
}

static void fill_container (ClutterActor *container)
{
  gint i;
  for (i=0; i< CHILD_COUNT; i++)
    add_one (container);
}

static gboolean adder_timeout (gpointer data)
{
  static gint count = 0;

  add_one (CLUTTER_ACTOR (data));

  if (count ++ > CHILD_COUNT)
    return FALSE;

  return TRUE;
}

static void
update_size (CallbackData *data)
{
  ClutterGeometry geom;

  clutter_actor_get_geometry (data->rect, &geom);
  clutter_actor_set_geometry (data->layout, &geom);

  clutter_actor_set_position (data->corners[0], geom.x, geom.y);
  clutter_actor_set_position (data->corners[1], geom.x + geom.width, geom.y);
  clutter_actor_set_position (data->corners[2], geom.x, geom.y + geom.height);
  clutter_actor_set_position (data->corners[3],
			      geom.x + geom.width, geom.y + geom.height);
}

static gboolean
on_button_press (ClutterActor *stage, ClutterButtonEvent *event,
		 CallbackData *data)
{
  int corner;

  if (event->button == 1)
    {
      ClutterActor *actor
	= clutter_stage_get_actor_at_pos (CLUTTER_STAGE (stage),
                                          CLUTTER_PICK_ALL,
					  event->x, event->y);

      for (corner = 0; corner < 4; corner++)
	if (actor == data->corners[corner])
	  break;

      if (corner < 4)
	data->corner = corner;
    }

  return FALSE;
}

static gboolean
on_motion (ClutterActor *stage, ClutterMotionEvent *event, CallbackData *data)
{
  if ((event->modifier_state & CLUTTER_BUTTON1_MASK)
      && data->corner != -1)
    {
      ClutterGeometry geom;

      clutter_actor_get_geometry (data->rect, &geom);

      switch (data->corner)
	{
	case 0:
	  geom.width += geom.x - event->x;
	  geom.height += geom.y - event->y;
	  geom.x = event->x;
	  geom.y = event->y;
	  break;

	case 1:
	  geom.width += event->x - geom.width - geom.x;
	  geom.height += geom.y - event->y;
	  geom.y = event->y;
	  break;

	case 2:
	  geom.width += geom.x - event->x;
	  geom.height += event->y - geom.height - geom.y;
	  geom.x = event->x;
	  break;

	case 3:
	  geom.width += event->x - geom.width - geom.x;
	  geom.height += event->y - geom.height - geom.y;
	  break;
	}

      clutter_actor_set_geometry (data->rect, &geom);

      update_size (data);
    }

  return FALSE;
}

static gboolean
on_button_release (ClutterActor *stage, ClutterButtonEvent *event,
		   CallbackData *data)
{
  if (event->button == 1)
    data->corner = -1;

  return FALSE;
}


int
main (int argc, char **argv)
{
  CallbackData data;
  gfloat layout_width, layout_height;
  int i;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  data.corner = -1;

  data.stage = clutter_stage_get_default ();

  data.rect = clutter_rectangle_new_with_color (&(ClutterColor)
						{ 0xd0, 0xd0, 0xff, 0xff});

  data.layout =g_object_new (MX_TYPE_GRID,
                             "child-x-align", MX_ALIGN_MIDDLE,
                             "child-y-align", MX_ALIGN_MIDDLE,
                             "row-spacing", 5.0f,
                             "column-spacing", 5.0f,
                             NULL);


  { /* add controls */
    ClutterActor *vbox = g_object_new (MX_TYPE_GRID,
                             "row-spacing", 5.0f,
                             "orientation", MX_ORIENTATION_VERTICAL,
                             "x", 60.0f,
                             "y", 60.0f,
                             NULL);
    clutter_container_add (CLUTTER_CONTAINER (data.stage), CLUTTER_ACTOR (vbox), NULL);

    clutter_container_add (CLUTTER_CONTAINER (vbox),
      clutter_text_new_with_text ("Sans 20px", "MxGrid properties:"),
      boolean_toggle    (G_OBJECT(data.layout), "line-alignment", "line-alignment"),
      boolean_toggle    (G_OBJECT(data.layout), "orientation",        "columns"),
      boolean_toggle    (G_OBJECT(data.layout), "homogenous-columns", "homogenous columns"),
      boolean_toggle    (G_OBJECT(data.layout), "homogenous-rows",    "homogenous rows"),
     // boolean_toggle    (G_OBJECT(data.layout), "child-x-align",    "child-x-align"),
     // boolean_toggle    (G_OBJECT(data.layout), "child-y-align",    "child-y-align"),
      units_slider      (G_OBJECT(data.layout), "column-spacing",         "column-gap"),
      units_slider      (G_OBJECT(data.layout), "row-spacing",            "row-gap"),
      NULL);
  }

  clutter_container_add (CLUTTER_CONTAINER (data.stage),
			 data.rect, NULL);



  clutter_container_add (CLUTTER_CONTAINER (data.stage), data.layout, NULL);




  for (i = 0; i < 4; i++)
    {
      data.corners[i] = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (data.corners[i]),
				   &(ClutterColor) { 0xff, 0xa0, 0xa0, 0xff });
      clutter_actor_set_size (data.corners[i], CORNER_SIZE, CORNER_SIZE);
      clutter_actor_set_anchor_point (data.corners[i],
				      CORNER_SIZE / 2, CORNER_SIZE / 2);

      clutter_container_add (CLUTTER_CONTAINER (data.stage),
			     data.corners[i], NULL);
    }

  clutter_actor_get_preferred_size (data.layout, NULL, NULL,
				    &layout_width, &layout_height);
  clutter_actor_set_position (data.rect,
			      clutter_actor_get_width (data.stage) / 2
			      - layout_width / 2,
			      clutter_actor_get_height (data.stage) / 2
			      - layout_height / 2);
  clutter_actor_set_size (data.rect, layout_width, layout_height);


  g_timeout_add (150, adder_timeout, data.layout);

  update_size (&data);

  clutter_actor_show (data.stage);

  g_signal_connect (data.stage, "button-press-event",
		    G_CALLBACK (on_button_press), &data);
  g_signal_connect (data.stage, "motion-event",
		    G_CALLBACK (on_motion), &data);
  g_signal_connect (data.stage, "button-release-event",
		    G_CALLBACK (on_button_release), &data);



  {
    ClutterActor *layout = g_object_new (MX_TYPE_GRID,
                                         "x", 5.0f,
                                         "y", 5.0f,
                                         "max-stride", 1,
                                         NULL);
    clutter_container_add (CLUTTER_CONTAINER (data.stage), layout, NULL);
    fill_container (layout);
  }


  {
    ClutterActor *layout = g_object_new (MX_TYPE_GRID,
                                         "x", 500.0f,
                                         "y", 5.0f,
                                         "max-stride", 1,
                                         "child-x-align", MX_ALIGN_MIDDLE,
                                         "homogenous-columns", TRUE,
                                         NULL);
    clutter_container_add (CLUTTER_CONTAINER (data.stage), layout, NULL);
    fill_container (layout);
  }



  {
    ClutterActor *layout = g_object_new (MX_TYPE_GRID,
                                         "x",      60.0f,
                                         "y",      5.0f,
                                         "max-stride", 1,
                                         "orientation", MX_ORIENTATION_VERTICAL,
                                         NULL);
    clutter_container_add (CLUTTER_CONTAINER (data.stage), layout, NULL);
    fill_container (layout);
  }

  {
    ClutterActor *layout = g_object_new (MX_TYPE_GRID,
                                         "x",       60.0f,
                                         "y",       420.0f,
                                         "child-y-align",  MX_ALIGN_END,
                                         "max-stride", 1,
                                         "column-spacing", 4.0f,
                                         NULL);
    clutter_container_add (CLUTTER_CONTAINER (data.stage), layout, NULL);
    fill_container (layout);
  }

  clutter_main ();

  return 0;
}



/* utility routines for creating visual controls for gobject properties */


#define CONTROL_FONT "Sans 12px"


static gboolean boolean_pressed (ClutterActor *toggle,
                                 ClutterEvent *event,
                                 gpointer      data)
{
  const gchar *prop_name;
  gboolean current;

  prop_name = g_object_get_data (G_OBJECT (toggle), "prop-name");

  g_object_get (data, prop_name, &current, NULL);
  if (event)
    {
      current = !current;
    }
  g_object_set (data, prop_name, current, NULL);

  if (current)
    clutter_text_set_text (CLUTTER_TEXT (toggle), "[x]");
  else
    clutter_text_set_text (CLUTTER_TEXT (toggle), "[ ]");

  return TRUE;
}


ClutterActor *boolean_toggle (gpointer     object,
                              const gchar *prop_name,
                              const gchar *label_text)
{
  ClutterActor *group  = clutter_group_new ();
  ClutterActor *label  = clutter_text_new_with_text (CONTROL_FONT, label_text);
  ClutterActor *toggle = clutter_text_new_with_text (CONTROL_FONT, "..");

  clutter_container_add_actor (CLUTTER_CONTAINER (group), label);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), toggle);

  clutter_actor_set_x (toggle, clutter_actor_get_width (label) + 5);
  g_object_set_data (G_OBJECT (toggle), "prop-name", (void*)
                                                  g_intern_string (prop_name));

  g_signal_connect (toggle, "button-press-event", G_CALLBACK (boolean_pressed),
                    object);

  clutter_actor_set_reactive (toggle, TRUE);

  boolean_pressed (toggle, NULL, object);

  return group;
}

static gboolean
slider_motion (ClutterActor *actor,
               ClutterEvent *event,
               gpointer      data)
{
  if (event == NULL || clutter_event_get_state (event) & CLUTTER_BUTTON1_MASK )
    {
      gfloat x;
      const gchar *prop_name;
      gdouble current;

      ClutterActor *indicator;
      indicator = g_object_get_data (G_OBJECT (actor), "indicator");
      prop_name = g_object_get_data (G_OBJECT (actor), "prop-name");

      g_object_get (data, prop_name, &current, NULL);

      if (event)
        {
          clutter_actor_transform_stage_point (actor, (float) event->motion.x,
                                                      (float) event->motion.y,
                                                      &x, NULL);
          current = x / (clutter_actor_get_width (actor) * 65535.0);
        }
      clutter_actor_set_x (indicator, clutter_actor_get_x (actor) + clutter_actor_get_width (actor) * current);

      g_object_set (data, prop_name, current, NULL);
    }
  return TRUE;
}

ClutterActor *normalized_slider (GObject     *object,
                                 const gchar *prop_name,
                                 const gchar *label_text)
{
  ClutterActor *group  = clutter_group_new ();
  ClutterActor *label  = clutter_text_new_with_text (CONTROL_FONT, label_text);
  ClutterActor *through = clutter_text_new_with_text (CONTROL_FONT, "[     ]");
  ClutterActor *indicator = clutter_text_new_with_text (CONTROL_FONT, "X");

  clutter_container_add_actor (CLUTTER_CONTAINER (group), label);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), through);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), indicator);

  clutter_actor_set_x (through, clutter_actor_get_width (label) + 5);
  clutter_actor_set_x (indicator, clutter_actor_get_x (through));
  g_object_set_data (G_OBJECT (through), "prop-name", (void*)
                                                   g_intern_string (prop_name));
  g_object_set_data (G_OBJECT (through), "indicator", indicator);

  g_signal_connect (through, "motion-event", G_CALLBACK (slider_motion),object);
  clutter_actor_set_reactive (through, TRUE);
  slider_motion (through, NULL, object);
  return group;
}


static gboolean
unit_slider_motion (ClutterActor *actor,
                   ClutterEvent *event,
                   gpointer      data)
{
  if (event == NULL || clutter_event_get_state (event) & CLUTTER_BUTTON1_MASK )
    {
      gfloat x;
      const gchar *prop_name;
      gfloat current;

      ClutterActor *indicator;
      indicator = g_object_get_data (G_OBJECT (actor), "indicator");
      prop_name = g_object_get_data (G_OBJECT (actor), "prop-name");

      g_object_get (data, prop_name, &current, NULL);

      if (event)
        {
          clutter_actor_transform_stage_point (actor, (gfloat) event->motion.x,
                                                      (gfloat) event->motion.y,
                                                      &x, NULL);
          current = x;
        }
      clutter_actor_set_x (indicator, clutter_actor_get_x (actor) + current);

      g_object_set (data, prop_name, current, NULL);
    }
  return TRUE;
}


ClutterActor *units_slider (GObject     *object,
                           const gchar *prop_name,
                           const gchar *label_text)
{
  ClutterActor *group  = clutter_group_new ();
  ClutterActor *label  = clutter_text_new_with_text (CONTROL_FONT, label_text);
  ClutterActor *through = clutter_text_new_with_text (CONTROL_FONT, "[     ]");
  ClutterActor *indicator = clutter_text_new_with_text (CONTROL_FONT, "X");

  clutter_container_add_actor (CLUTTER_CONTAINER (group), label);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), through);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), indicator);

  clutter_actor_set_x (through, clutter_actor_get_width (label) + 5);
  clutter_actor_set_x (indicator, clutter_actor_get_x (through));

  g_object_set_data (G_OBJECT (through), "prop-name",
                     (void*) g_intern_string (prop_name));
  g_object_set_data (G_OBJECT (through), "indicator", indicator);

  g_signal_connect (through, "motion-event", G_CALLBACK (unit_slider_motion),
                    object);
  clutter_actor_set_reactive (through, TRUE);
  unit_slider_motion (through, NULL, object);
  return group;
}
