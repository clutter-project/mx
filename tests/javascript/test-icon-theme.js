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

let stage = Clutter.Stage.get_default ();
stage.title = "Test Icon Theme"

let vbox = new Mx.BoxLayout ( {orientation: Mx.Orientation.VERTICAL} );
stage.add_actor (vbox);
vbox.set_size (stage.width, stage.height);

let entry = new Mx.Entry ();
vbox.add_actor (entry, -1);
entry.grab_key_focus ();

let theme = Mx.IconTheme.get_default ();
let texture = null;

entry.get_clutter_text ().connect ("activate",
  function (e) {
    if (texture)
      vbox.remove_actor (texture);
    texture = theme.lookup_texture (entry.text, 48);
    if (texture)
      {
        //texture.keep_aspect_ratio = true;
        vbox.add_actor (texture, -1);
        vbox.child_set_property (texture, "x-fill", false);
        vbox.child_set_property (texture, "y-fill", false);
      }
  });

stage.show ();
Clutter.main ();

stage.destroy ();
