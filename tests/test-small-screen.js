#!/usr/bin/env gjs
/*
 * Copyright 2010 Intel Corporation.
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

let [app] = Mx.Application.new (0, null, "Test Small Screen", 0);
let win = app.create_window ();
let stage = win.get_clutter_stage ();

let hbox = new Mx.BoxLayout ({spacing: 8});
let toggle = new Mx.Toggle ();
let label = new Mx.Label ({text: "Small-screen mode"});
toggle.active = stage.small_screen;

toggle.connect ("notify::active",
                function (t) {
                  stage.small_screen = toggle.active;
                });

hbox.add_actor (toggle);
hbox.add_actor (label);
win.set_child (hbox);

hbox.child_set_property (toggle, "expand", true);
hbox.child_set_property (toggle, "x-fill", false);
// Mx.Align.end == 2;
// FIXME: Need to add the correct introspection data for this?
hbox.child_set_property (toggle, "x-align", 2);
hbox.child_set_property (label, "y-fill", false);
hbox.child_set_property (label, "expand", true);

stage.show ();
app.run ();

