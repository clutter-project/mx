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


function add_button (name, box, group)
{
  let button = new Mx.Button ({label: name })
  button.toggle_mode = true;
  box.add_actor (button);
  group.add (button)
  return button;
}

Clutter.init (null);

let stage = Clutter.Stage.get_default ();
stage.title = "Test Toolkit"

let group = new Mx.ButtonGroup ({allow_no_active: true})

let box = new Mx.BoxLayout ()
stage.add_actor (box)

add_button ("Entry", box, group)
add_button ("Table", box, group)
add_button ("Scroll View", box, group)
add_button ("Progress Bar", box, group)
add_button ("Drag and Drop", box, group)
let button1 = add_button ("Expander", box, group)
let button2 = add_button ("Combo Box", box, group)

group.connect ("notify::active-button",
               function (g, p) { if (g.active_button)
                                  stage.title = g.active_button.label
                                 else
                                  stage.title = "" } )

stage.connect ("button-press-event", function (s, e) { group.remove (button1)});
stage.connect ("key-press-event", function (s, e) { button2.destroy() });

stage.show ();
Clutter.main ();

stage.destroy ();
