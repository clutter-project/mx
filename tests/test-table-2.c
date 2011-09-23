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
#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <mx/mx.h>

int
main (int     argc,
      char  **argv)
{
  ClutterActor *stage, *txt, *table, *label, *label2;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  mx_style_load_from_file (mx_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 1000);

  table = mx_table_new ();
  mx_table_set_column_spacing (MX_TABLE (table), 10);
  clutter_actor_set_position (table, 50, 50);
  clutter_actor_set_width (table, 300);
  clutter_container_add (CLUTTER_CONTAINER (stage), table, NULL);

  label = mx_label_new_with_text ("Short top text");
  mx_table_add_actor (MX_TABLE (table), label, 0, 0);

  label2 = mx_label_new_with_text ("");
  
  txt = mx_label_get_clutter_text(MX_LABEL(label2));
  clutter_text_set_ellipsize (CLUTTER_TEXT (txt), PANGO_ELLIPSIZE_NONE);
  clutter_text_set_line_alignment (CLUTTER_TEXT (txt), PANGO_ALIGN_LEFT);
  clutter_text_set_line_wrap (CLUTTER_TEXT (txt), TRUE);

  mx_table_add_actor (MX_TABLE (table), label2, 1, 0);

  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               label2,
                               "y-expand", FALSE,
                               "x-expand", FALSE,
                               NULL);

  label = mx_label_new_with_text ("Short Bottom text");
  mx_table_add_actor (MX_TABLE (table), label, 2, 0);

  mx_label_set_text (MX_LABEL (label2), "Really really long long long long long long long long long long long long long long long long long long (ooooh this is verrrrrrry long!) long longlong long long longlong long long long \nlong longlong long long long longlonglonglonglonglonglonglonglonglonglonglonglong long long long long long long long long long Loooooooooooooooong text");

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
