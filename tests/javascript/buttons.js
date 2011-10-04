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

Clutter.init (null);

Mx.Style.get_default ().load_from_file ("tests.css");


let script = new Clutter.Script ();
script.load_from_file ("buttons.json", -1);

let stage = script.get_object ("stage");

stage.show ();

stage.connect ("destroy", Clutter.main_quit);

let button = script.get_object ("action-button");

let action = button.action;

let i = 0;

action.connect ("activated", function (a) {a.display_name = "Click " + ++i;
                                           a.icon = "dialog-information";})

button = script.get_object ("custom-content");
button.connect ("clicked", function (b) { b.label = "Clicked"; });

Clutter.main ();

