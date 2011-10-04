#!/usr/bin/env gjs
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

const Clutter = imports.gi.Clutter;
const Mx = imports.gi.Mx;

function add_button (action, container, group)
{
  let button = new Mx.Button ({label: action.display_name });
  button.toggle_mode = true;
  container.add_actor (button);
  group.add (button);
  button.connect ("clicked", function (b) { action.emit ("activated"); });
  return button;
}

function connect_cb (text)
{
  let app = new Mx.Application ({flags: 1, "application-name": text.text});

  if (!app.is_running ())
    return;

  let stage = new Clutter.Stage ();
  stage.title = text.text;

  let group = new Mx.ButtonGroup ({allow_no_active: true});
  let grid = new Mx.Grid ();

  let actions = app.get_actions ();
  for (let a = 0; a < actions.length; a++)
    add_button (actions[a], grid, group);

  grid.set_size (stage.width, stage.height);
  stage.add_actor (grid);

  stage.show ();
}

Clutter.init (null);

let main_stage = Clutter.Stage.get_default ();
main_stage.title = "Test Mx Actions";

let entry = new Mx.Entry ({text: "TestMx"});
entry.set_width (main_stage.width);
main_stage.add_actor (entry);

entry.get_clutter_text ().connect ("activate", connect_cb);
entry.grab_key_focus ();

main_stage.show ();
Clutter.main ();

