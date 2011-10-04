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
stage.title = "Test Notebook";

let notebook = new Mx.Notebook ();
stage.add_actor (notebook);

let buttons = new Array ();
buttons[0] = new Mx.Button ({label: "Paddington"});
notebook.add_actor (buttons[0]);

buttons[1] = new Mx.Button ({label: "Aldgate"});
notebook.add_actor (buttons[1]);

buttons[2] = new Mx.Button ({label: "Baker Street"});
notebook.add_actor (buttons[2]);

let page_no = 0;
stage.connect ("button-release-event",
               function (o, e) {
                  if (++page_no >= buttons.length)
                    page_no = 0;
                  notebook.current_page = buttons[page_no];
               });

stage.show ();
Clutter.main ();

stage.destroy ();
