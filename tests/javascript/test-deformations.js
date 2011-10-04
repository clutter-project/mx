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
const grey = new Clutter.Color(); grey.from_string ("grey");

Clutter.init (null);
let app = new Mx.Application ({"application-name": "Test deformations"});

let win = app.create_window ();
let stage = win.get_clutter_stage ();
stage.set_color (grey);
stage.set_size (400, 500);

let deform = new Mx.DeformPageTurn ();
deform.set_actors (new Mx.Button ({label: "Front"}),
                   new Mx.Button ({label: "Back"}));
//deform.set_from_files ("redhand.png", "redhand.png");

deform.angle = Math.PI/5;
deform.radius = 32;
deform.animatev (Clutter.AnimationMode.LINEAR, 10000,
                 1,
                 ["period"],
                 [1.0]);

win.set_child (deform);

stage.show ();
app.run ();

