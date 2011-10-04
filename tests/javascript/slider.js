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

Mx.Style.get_default ().load_from_file ("tests.css");

let script = new Clutter.Script ();

script.load_from_file ("slider.json");

let stage = script.get_object ("stage");
stage.connect ("destroy", Clutter.main_quit);

let slider = script.get_object ("slider");

let manager = Mx.FocusManager.get_for_stage (stage);
manager.push_focus (slider);

script.get_object ("reset").connect ("clicked",
                                     function (b) { slider.value = 0 })

stage.show ();

Clutter.main ();
