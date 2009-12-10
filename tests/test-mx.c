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

#include "test-mx.h"


static void
clear_holder (ClutterContainer *holder)
{
  GList *children;

  children = clutter_container_get_children (holder);

  g_list_foreach (children, (GFunc) clutter_actor_destroy, NULL);
}

static void
add_tab (ClutterContainer *box,
         MxButtonGroup    *group,
         const gchar      *name,
         GCallback         callback,
         ClutterContainer *holder)
{
  ClutterActor *button;

  button = mx_button_new_with_label (name);
  mx_button_set_toggle_mode (MX_BUTTON (button), TRUE);

  clutter_container_add_actor (box, button);
  mx_button_group_add (group, MX_BUTTON (button));

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (clear_holder),
                            holder);

  if (callback)
    g_signal_connect_swapped (button, "clicked", callback, holder);

}

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *vbox, *hbox, *holder, *mainbox, *toolbar;
  MxButtonGroup *group;

  clutter_init (&argc, &argv);

  stage = clutter_stage_new ();
  clutter_actor_set_size (stage, 800, 600);

  mainbox = mx_box_layout_new ();
  clutter_actor_set_size (mainbox, 800, 600);
  mx_box_layout_set_vertical (MX_BOX_LAYOUT (mainbox), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), mainbox);

  /* create the toolbar */
  toolbar = mx_toolbar_new ();

  /* create the horizontal layout */
  hbox = mx_box_layout_new ();
  clutter_actor_set_position (hbox, 12, 12);

  /* add toolbar and hoizontal layout to main container */
  clutter_container_add (CLUTTER_CONTAINER (mainbox), toolbar, hbox, NULL);

  /* create a vbox for the list of tests */
  vbox = mx_box_layout_new ();
  mx_box_layout_set_vertical (MX_BOX_LAYOUT (vbox), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), vbox);

  /* create a place holder for the tests */
  holder = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), holder);

  group = mx_button_group_new ();

  add_tab (CLUTTER_CONTAINER (vbox), group, "Label",
           (GCallback) label_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Entry",
           (GCallback) entry_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Button", (GCallback) buttons_main,
           CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Combo Box",
           (GCallback) combo_box_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Progress Bar",
           (GCallback) progress_bar_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Toggle",
           (GCallback) toggle_main, CLUTTER_CONTAINER (holder));

  add_tab (CLUTTER_CONTAINER (vbox), group, "Tooltips",
           (GCallback) tooltips_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Expander",
           (GCallback) expander_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Scroll Grid",
           (GCallback) scroll_grid_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Scroll Bar",
           (GCallback) scroll_bar_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Scroll View",
           (GCallback) scroll_view_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Styles",
           (GCallback) styles_main, CLUTTER_CONTAINER (holder));

  clutter_actor_show (stage);

  g_signal_connect (stage, "destroy", clutter_main_quit, NULL);

  clutter_main ();

  return EXIT_SUCCESS;
}
