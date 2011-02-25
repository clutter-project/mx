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
#include <clutter/clutter.h>
#include <mx/mx.h>

typedef void (*TestMxCallback)(ClutterContainer *container);

void label_main (ClutterContainer *container);
void entry_main (ClutterContainer *container);
void tooltips_main (ClutterContainer *container);
void buttons_main (ClutterContainer *container);
void button_group_main (ClutterContainer *container);
void combo_box_main (ClutterContainer *container);
void progress_bar_main (ClutterContainer *container);
void slider_main (ClutterContainer *container);
void expander_main (ClutterContainer *container);
void scroll_grid_main (ClutterContainer *container);
void scroll_bar_main (ClutterContainer *container);
void scroll_view_main (ClutterContainer *container);
void styles_main (ClutterContainer *container);
void toggle_main (ClutterContainer *container);
void dialog_main (ClutterContainer *container);
void spinner_main (ClutterContainer *container);
