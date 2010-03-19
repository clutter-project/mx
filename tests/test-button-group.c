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
#include "test-mx.h"

static ClutterActor *button_box;
static MxButtonGroup *button_group;

static void
_add_button_clicked_cb (MxButton *add_button,
                        gpointer  userdata)
{
  ClutterActor *button;
  gchar *button_str;
  GList *children;

  children = clutter_container_get_children (CLUTTER_CONTAINER (button_box));
  button_str = g_strdup_printf ("Button %d", g_list_length (children));
  button = mx_button_new_with_label (button_str);
  g_free (button_str);
  g_list_free (children);
  mx_button_set_is_toggle ((MxButton *) button, TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (button_box),
                               button);
  mx_button_group_add (button_group, (MxButton *) button);
}

static void
_rm_button_clicked_cb (MxButton *button,
                       gpointer  userdata)
{
  GList *children;
  ClutterActor *actor;

  children = clutter_container_get_children (CLUTTER_CONTAINER (button_box));

  if (!children)
    return;

  actor = (ClutterActor *)g_list_last (children)->data;
  mx_button_group_remove (button_group,
                          (MxButton *)actor);
  clutter_container_remove_actor (CLUTTER_CONTAINER (button_box), actor);

  g_list_free (children);
}

static void
_destroy_button_clicked_cb (MxButton *button,
                            gpointer  userdata)
{
  MxButton *active_button;

  active_button = mx_button_group_get_active_button (button_group);

  if (active_button)
    clutter_actor_destroy ((ClutterActor *) active_button);
}

static void
_no_active_toggled_cb (MxButton   *button,
                       GParamSpec *pspec,
                       gpointer    userdata)
{
  mx_button_group_set_allow_no_active (button_group,
                                       mx_button_get_toggled (button));
}

void
button_group_main (ClutterContainer *stage)
{
  ClutterActor *button;
  ClutterActor *control_box;

  button_group = mx_button_group_new ();
  button_box = mx_box_layout_new ();
  control_box = mx_box_layout_new ();

  button = mx_button_new_with_label ("Add button");
  g_signal_connect (button, "clicked", (GCallback)_add_button_clicked_cb, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (control_box), button);

  button = mx_button_new_with_label ("Remove button");
  g_signal_connect (button, "clicked", (GCallback)_rm_button_clicked_cb, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (control_box), button);

  button = mx_button_new_with_label ("Destroy active");
  g_signal_connect (button, "clicked", (GCallback)_destroy_button_clicked_cb,
                    NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (control_box), button);

  button = mx_button_new_with_label ("Allow no active");
  mx_button_set_is_toggle ((MxButton *) button, TRUE);
  g_signal_connect (button, "notify::toggled",
                    (GCallback) _no_active_toggled_cb, NULL);
  mx_button_set_toggled ((MxButton *)button,
                         mx_button_group_get_allow_no_active (button_group));
  clutter_container_add_actor (CLUTTER_CONTAINER (control_box), button);


  clutter_container_add_actor (CLUTTER_CONTAINER (stage), control_box);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), button_box);
  clutter_actor_set_position (control_box, 100, 100);
  clutter_actor_set_position (button_box, 100, 200);
}
