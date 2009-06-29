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
#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

static void
button_hide (NbtkButton *button)
{
  clutter_actor_hide (CLUTTER_ACTOR (button));
}

int
main (int argc, char *argv[])
{
  NbtkWidget *button, *bin;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = nbtk_button_new_with_label ("Hide Me");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_hide),
                    "hello");
  nbtk_widget_set_tooltip_text (button, "Disappear");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 100);

  bin = nbtk_bin_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (bin));
  clutter_actor_set_position (CLUTTER_ACTOR (bin), 50, 200);

  button = nbtk_button_new_with_label ("Testing 123");
  nbtk_widget_set_tooltip_text (button, "Testing tooltips in a container...");
  clutter_container_add_actor (CLUTTER_CONTAINER (bin),
                               CLUTTER_ACTOR (button));

  button = nbtk_button_new_with_label ("Testing Long Text");
  nbtk_widget_set_tooltip_text (button,
                           "Here is some really"
                           " long text to test the handling in NbtkTooltip");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 50, 300);

  button = nbtk_button_new_with_label ("Testing Long Text");
  nbtk_widget_set_tooltip_text (button,
                           "Here is some really"
                           " long text to test the handling in NbtkTooltip");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 500, 300);

  button = nbtk_button_new_with_label ("Crazy");
  nbtk_widget_set_tooltip_text (button,
                           "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 250, 5);

  button = nbtk_button_new_with_label ("Bottom");
  nbtk_widget_set_tooltip_text (button,
                           "Hello Hello Hello");
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), 250, 440);




  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
