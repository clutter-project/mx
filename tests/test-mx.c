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
 * Author: Thomas Wood <thomas.wood@intel.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "test-mx.h"


static void
rotate_left_clicked_cb (ClutterActor *button,
                        MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static void
rotate_right_clicked_cb (ClutterActor *button,
                         MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static void
rotate_180_clicked_cb (ClutterActor *button,
                       MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static ClutterActor *
create_rotate_box (MxWindow *window)
{
  ClutterActor *button, *icon, *icon2, *layout2;
  ClutterActor *layout = mx_box_layout_new ();

  /* Create rotate-left button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-left");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), icon);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_left_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  /* Create rotate-180 button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-left");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  icon2 = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon2), "object-rotate-right");
  mx_icon_set_icon_size (MX_ICON (icon2), 16);
  layout2 = mx_box_layout_new ();
  clutter_container_add (CLUTTER_CONTAINER (layout2), icon, icon2, NULL);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), layout2);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_180_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  /* Create rotate-right button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-right");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), icon);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_right_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  return layout;
}

static void
show_page (MxButton         *button,
           GParamSpec       *pspec,
           ClutterContainer *holder)
{
  GList *children;
  TestMxCallback callback;

  if (!mx_button_get_toggled (button))
    return;

  children = clutter_container_get_children (holder);

  g_list_foreach (children, (GFunc) clutter_actor_destroy, NULL);

  callback = g_object_get_data (G_OBJECT (button), "callback");
  callback ((gpointer)holder);
}

static void
next_tab_activated_cb (MxAction      *action,
                       MxButtonGroup *group)
{
  GSList *b;

  gboolean activate_next = FALSE;
  MxButton *button = mx_button_group_get_active_button (group);
  const GSList *buttons = mx_button_group_get_buttons (group);

  if (!buttons)
    return;

  if (button)
    for (b = (GSList *)buttons; b; b = b->next)
      {
        MxButton *current_button = (MxButton *)b->data;

        if (activate_next)
          {
            button = current_button;
            break;
          }

        if ((current_button == button) && b->next)
          activate_next = TRUE;
      }

  if (!activate_next)
    button = (MxButton *)buttons->data;

  mx_button_group_set_active_button (group, button);
}

static void
prev_tab_activated_cb (MxAction      *action,
                       MxButtonGroup *group)
{
  GSList *b;

  MxButton *last_button = NULL;
  MxButton *button = mx_button_group_get_active_button (group);
  const GSList *buttons = mx_button_group_get_buttons (group);

  if (!buttons)
    return;

  for (b = (GSList *)buttons; b; b = b->next)
    {
      MxButton *current_button = (MxButton *)b->data;

      if (current_button == button)
        break;

      last_button = current_button;
    }

  if (!last_button)
    last_button = (MxButton *)g_slist_last ((GSList *)buttons)->data;

  mx_button_group_set_active_button (group, last_button);
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
  mx_button_set_is_toggle (MX_BUTTON (button), TRUE);

  clutter_container_add_actor (box, button);
  mx_button_group_add (group, MX_BUTTON (button));

  if (callback)
    {
      g_object_set_data (G_OBJECT (button), "callback", callback);
      g_signal_connect (button, "notify::toggled",
                        G_CALLBACK (show_page), holder);
    }

}

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *vbox, *hbox, *holder, *mainbox, *toolbar, *combo,
               *layout;
  MxAction *prev, *next;
  MxButtonGroup *group;
  MxApplication *application;
  MxWindow *window;

  application = mx_application_new (&argc, &argv, "Test Mx",
                                    MX_APPLICATION_SINGLE_INSTANCE);

  window = mx_application_create_window (application);
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);

  mainbox = mx_box_layout_new ();
  clutter_actor_set_size (mainbox, 800, 600);
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (mainbox), MX_ORIENTATION_VERTICAL);
  mx_window_set_child (window, mainbox);

  /* create the toolbar */
  toolbar = (ClutterActor *)mx_window_get_toolbar (window);
  mx_bin_set_fill (MX_BIN (toolbar), TRUE, FALSE);

  layout = mx_box_layout_new ();

  combo = mx_combo_box_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), combo);
  mx_combo_box_set_active_text (MX_COMBO_BOX (combo), "Select");
  mx_combo_box_set_active_icon_name (MX_COMBO_BOX (combo), "dialog-question");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "Hello");
  mx_combo_box_append_text (MX_COMBO_BOX (combo), "World");

  mx_box_layout_add_actor_with_properties (MX_BOX_LAYOUT (layout),
                                           create_rotate_box (window),
                                           1,
                                           "x-fill", FALSE,
                                           "x-align", MX_ALIGN_END,
                                           "expand", TRUE,
                                           NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (toolbar), layout);

  /* create the horizontal layout */
  hbox = mx_box_layout_new ();

  /* add horizontal layout to main container */
  clutter_container_add (CLUTTER_CONTAINER (mainbox), hbox, NULL);

  /* create a vbox for the list of tests */
  vbox = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (vbox), MX_ORIENTATION_VERTICAL);
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
  add_tab (CLUTTER_CONTAINER (vbox), group, "Button group",
           (GCallback) button_group_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Combo Box",
           (GCallback) combo_box_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Progress Bar",
           (GCallback) progress_bar_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Slider",
           (GCallback) slider_main, CLUTTER_CONTAINER (holder));
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
  add_tab (CLUTTER_CONTAINER (vbox), group, "Dialog",
           (GCallback) dialog_main, CLUTTER_CONTAINER (holder));
  add_tab (CLUTTER_CONTAINER (vbox), group, "Spinner",
           (GCallback) spinner_main, CLUTTER_CONTAINER (holder));

  prev = mx_action_new_full ("Previous tab",
                             "Previous tab",
                             G_CALLBACK (prev_tab_activated_cb),
                             group);
  next = mx_action_new_full ("Next tab",
                             "Next tab",
                             G_CALLBACK (next_tab_activated_cb),
                             group);
  mx_application_add_action (application, prev);
  mx_application_add_action (application, next);

  clutter_actor_show (stage);

  mx_application_run (application);

  return EXIT_SUCCESS;
}
