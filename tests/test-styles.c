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
#include "test-mx.h"

static ClutterActor *
create_button (ClutterContainer *parent,
               const gchar  *text,
               gint          x,
               gint          y)
{
  ClutterActor *button;

  button = mx_button_new_with_label (text);
  clutter_container_add_actor (CLUTTER_CONTAINER (parent), button);
  clutter_actor_set_position (button, x, y);
  clutter_actor_set_size (button, 150, 100);

  return button;
}

void
styles_main (ClutterContainer *stage)
{
  ClutterActor *button, *table;
  MxStyle *style;

  /* load the style sheet */
  style = mx_style_new ();
  mx_style_load_from_file (style, "style/default.css", NULL);

  button = create_button (stage, "Default Style", 10, 10);
  mx_stylable_set_style (MX_STYLABLE (button), style);
  clutter_actor_set_name (button, "default-button");

  button = create_button (stage, "Red Style", 10, 150);
  mx_stylable_set_style (MX_STYLABLE (button), style);
  clutter_actor_set_name (button, "red-button");

  button = create_button (stage, "Green Style", 200, 10);
  mx_stylable_set_style (MX_STYLABLE (button), style);
  clutter_actor_set_name (button, "green-button");

  button = create_button (stage, "Blue Style", 200, 150);
  mx_stylable_set_style (MX_STYLABLE (button), style);
  clutter_actor_set_name (button, "blue-button");

  table = mx_table_new ();
  mx_stylable_set_style (MX_STYLABLE (table), style);
  clutter_actor_set_size (table, 200, 80);
  clutter_container_add_actor (stage, table);
  clutter_actor_set_position (table, 10, 300);

  button = mx_button_new_with_label ("Container Test");
  mx_stylable_set_style (MX_STYLABLE (button), style);
  clutter_actor_set_name (button, "container-button");
  mx_table_add_actor (MX_TABLE (table), button, 0, 0);
}
