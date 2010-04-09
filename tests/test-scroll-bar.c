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

#include "test-mx.h"

static void
changed_cb (MxAdjustment *adjustment,
            gpointer        data)
{
  printf ("%s() %.2f\n", __FUNCTION__,
                         mx_adjustment_get_value (adjustment));
}

static gboolean
key_press_event (ClutterActor    *actor,
                 ClutterKeyEvent *event,
                 MxAdjustment  *adjustment)
{
  gdouble value, lower, upper, step_increment, page_increment, page_size;
  mx_adjustment_get_values (adjustment,
                              &value,
                              &lower,
                              &upper,
                              &step_increment,
                              &page_increment,
                              &page_size);
  if (event->keyval == CLUTTER_Up)
    {
      page_size += 5;
    }
  else if (event->keyval == CLUTTER_Down)
    {
      page_size -= 5;
    }

  mx_adjustment_set_values (adjustment, value, lower, upper,
                              step_increment, page_increment, page_size);

  printf ("value: %f, lower: %f, upper: %f, step-inc: %f, page-inc: %f, "
          "page: %f\n", value, lower, upper, step_increment, page_increment,
          page_size);

  return TRUE;
}

void
scroll_bar_main (ClutterContainer *stage)
{
  ClutterActor *scroll;
  MxAdjustment  *adjustment;

  adjustment = mx_adjustment_new_with_values (50., 0., 100., 1., 10., 10.);
  g_signal_connect (adjustment, "notify::value", 
                    G_CALLBACK (changed_cb), NULL);  

  scroll = mx_scroll_bar_new_with_adjustment (adjustment);
  clutter_actor_set_position (scroll, 50, 100);
  clutter_container_add_actor (stage, scroll);

  g_signal_connect (stage, "key-press-event", G_CALLBACK (key_press_event),
                    adjustment);

}
