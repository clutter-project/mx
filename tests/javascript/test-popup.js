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

Clutter.init (null);

let stage = Clutter.Stage.get_default ();
stage.title = "Test Widget Popup"

let button = new Mx.Button ( {label: "Click me!"} )
stage.add_actor (button);
button.set_position (100, 50);

let popup = new Mx.Popup ();
let action = new Mx.Action ({name: "action1", "display-name": "A pop-up!"});
popup.add_action (action);

button.set_popup (popup);
button.connect ("clicked",
                function (b) {
                  button.show_popup (0, 0);
                });

stage.show ();
Clutter.main ();

stage.destroy ();
