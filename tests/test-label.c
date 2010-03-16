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

void
label_main (ClutterContainer *stage)
{
  ClutterActor *label;
  MxStyle *style;
  GError *err = NULL;

  style = mx_style_new ();

  mx_style_load_from_file (style, "style/default.css", &err);

  if (err)
    {
      g_warning ("%s", err->message);
    }

  label = mx_label_new_with_text ("Hello World!");
  clutter_actor_set_position (label, 50, 50);
  mx_stylable_set_style (MX_STYLABLE (label), style);

  clutter_container_add (stage, label, NULL);
}
