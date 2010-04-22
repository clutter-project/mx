/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
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
 * Boston, MA 02111-1307, USA.
 *
 */
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <mx/mx.h>

static void
toggle_expand (MxButton *expand_button, ClutterActor *button)
{
  gboolean x_expand;
  gchar *label;

  ClutterContainer *table =
    CLUTTER_CONTAINER (clutter_actor_get_parent (button));

  clutter_container_child_get (table, button,
                               "x-expand", &x_expand,
                               NULL);

  x_expand = !x_expand;

  clutter_container_child_set (table, button,
                               "x-expand", x_expand,
                               NULL);

  label = g_strdup_printf ("button6 x-expand = %d", x_expand);
  mx_button_set_label (expand_button, label);

  g_free (label);
}

static void
switch_align (ClutterActor *button, ClutterContainer *table)
{
  static gint pattern = 1;

  MxAlign x_align, y_align;
  const gchar *label;

  switch (pattern)
    {
    default:
    case 1:
      x_align = y_align = MX_ALIGN_MIDDLE;
      label = "Align (Middle, Middle)";
      break;
    case 2:
      x_align = y_align = MX_ALIGN_START;
      label = "Align (Start, Start)";
      break;
    case 3:
      x_align = MX_ALIGN_MIDDLE;
      y_align = MX_ALIGN_START;
      label = "Align (Middle, Start)";
      break;
    case 4:
      x_align = MX_ALIGN_END;
      y_align = MX_ALIGN_START;
      label = "Align (End, Start)";
      break;
    case 5:
      x_align = MX_ALIGN_END;
      y_align = MX_ALIGN_MIDDLE;
      label = "Align (End, Middle)";
      break;
    case 6:
      x_align = MX_ALIGN_END;
      y_align = MX_ALIGN_END;
      label = "Align (End, End)";
      break;
    case 7:
      x_align = MX_ALIGN_MIDDLE;
      y_align = MX_ALIGN_END;
      label = "Align (Middle, End)";
      break;
    case 8:
      x_align = MX_ALIGN_START;
      y_align = MX_ALIGN_END;
      label = "Align (Start, End)";
      break;
    case 9:
      x_align = MX_ALIGN_START;
      y_align = MX_ALIGN_MIDDLE;
      label = "Align (Start, Middle)";
      break;
    }

  if (++pattern > 9)
    pattern = 1;

  clutter_container_child_set (table, button,
                               "x-align", x_align,
                               "y-align", y_align,
                               NULL);

  mx_button_set_label (MX_BUTTON (button), label);
}

static void
toggle_visible (ClutterActor *button)
{
  clutter_actor_hide (button);
}

gboolean drag = FALSE;

static gboolean
button_press (ClutterActor *actor, ClutterButtonEvent *event,
              ClutterActor *table)
{
  if (event->button == 1)
    {
      drag = TRUE;
      return TRUE;
    }

  return FALSE;
}

static gboolean
button_release (ClutterActor *actor, ClutterButtonEvent *event,
                ClutterActor *table)
{
  if (event->button == 1)
    {
      drag = FALSE;
      return TRUE;
    }
  if (event->button == 3)
    clutter_actor_set_size (table, -1, -1);

  return FALSE;
}

static void
motion_event (ClutterActor *actor, ClutterMotionEvent *event,
              ClutterActor *table)
{
  if (drag)
    clutter_actor_set_size (table, event->x - 5, event->y - 5);
}

int
main (int argc, char *argv[])
{
  MxApplication *application;
  MxWindow *window;
  ClutterActor *stage, *button2, *table;
  ClutterActor *button1, *button3, *button4, *button5, *button6, *button7,
               *button8, *button9, *button10;

  application = mx_application_new (&argc, &argv, "Test Table", 0);

  window = mx_application_create_window (application);
  stage = (ClutterActor *) mx_window_get_clutter_stage (window);

  table = mx_table_new ();
  mx_table_set_column_spacing (MX_TABLE (table), 10);
  mx_table_set_row_spacing (MX_TABLE (table), 10);

  button1 = mx_button_new_with_label ("button1");
  button2 = clutter_texture_new_from_file ("redhand.png", NULL);
  button3 = mx_button_new_with_label ("button3");
  button4 = mx_button_new_with_label ("button6 x-expand = 1");
  button5 = mx_button_new_with_label ("button5");
  button6 = mx_button_new_with_label ("button6");
  button7 = mx_button_new ();
  button8 = mx_button_new_with_label ("button8");
  button9 = mx_button_new_with_label ("button9");
  button10 = mx_button_new_with_label ("button10");

  mx_table_add_actor (MX_TABLE (table), button1, 0, 0);
  mx_table_add_actor (MX_TABLE (table), button2, 0, 1);
  mx_table_add_actor (MX_TABLE (table), button3, 1, 1);
  mx_table_add_actor (MX_TABLE (table), button4, 2, 0);
  mx_table_add_actor (MX_TABLE (table), button5, 3, 0);
  mx_table_add_actor (MX_TABLE (table), button6, 3, 1);
  mx_table_add_actor (MX_TABLE (table), button7, 4, 1);
  mx_table_add_actor (MX_TABLE (table), button8, 4, 0);
  mx_table_add_actor (MX_TABLE (table), button9, 5, 0);
  mx_table_add_actor (MX_TABLE (table), button10, -1, 0);
  mx_table_child_set_row_span (MX_TABLE (table), button1, 2);
  mx_table_child_set_row_span (MX_TABLE (table), button7, 2);
  mx_table_child_set_column_span (MX_TABLE (table), button4, 2);


  clutter_actor_set_size (button1, 100, 100);
  clutter_actor_set_width (button4, 250);

  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button1,
                               "x-expand", TRUE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button2,
                               "x-expand", FALSE, "y-expand", TRUE,
                               "x-fill", FALSE, "y-fill", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button3,
                               "x-expand", FALSE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button4,
                               "x-expand", FALSE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button5,
                               "x-expand", FALSE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button6,
                               "x-expand", TRUE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button7,
                               "x-expand", FALSE, "y-expand", FALSE,
                               "x-fill", FALSE, "y-fill", FALSE,
                               NULL);
  switch_align (button7, CLUTTER_CONTAINER (table));
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button8,
                               "x-expand", FALSE, "y-expand", TRUE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button9,
                               "x-expand", FALSE, "y-expand", FALSE,
                               NULL);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               button10,
                               "x-expand", FALSE, "y-expand", FALSE,
                               NULL);

  g_object_set (G_OBJECT (button2), "keep-aspect-ratio", TRUE, NULL);

  mx_window_set_child (window, table);

  g_signal_connect (button4, "clicked", G_CALLBACK (toggle_expand), button6);
  g_signal_connect (button7, "clicked", G_CALLBACK (switch_align), table);
  g_signal_connect (button10, "clicked", G_CALLBACK (toggle_visible), NULL);

  g_signal_connect (stage, "button-press-event", G_CALLBACK (button_press),
                    table);
  g_signal_connect (stage, "motion-event", G_CALLBACK (motion_event),
                    table);
  g_signal_connect (stage, "button-release-event", G_CALLBACK (button_release),
                    table);

  clutter_actor_show (stage);

  g_debug ("table row count = %d",
           mx_table_get_row_count (MX_TABLE (table)));
  g_debug ("table column count = %d",
           mx_table_get_column_count (MX_TABLE (table)));

  mx_application_run (application);

  return EXIT_SUCCESS;
}
