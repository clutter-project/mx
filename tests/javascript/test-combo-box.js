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
stage.title = "Test Combo Box";

let combo = new Mx.ComboBox ();
combo.set_position (10, 480 - 20);
stage.add_actor (combo);
combo.title = "London";

combo.append_text ("Strand");
combo.append_text ("Fleet Street");
combo.append_text ("Trafalgar Square");
combo.append_text ("Leicester Square");
combo.append_text ("Coventry Street");
combo.append_text ("Piccadilly");

stage.show ();
Clutter.main ();

stage.destroy ();
