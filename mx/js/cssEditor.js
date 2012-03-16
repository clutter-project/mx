//
// Copyright (c) 2012 Intel Corp.
//
// A Live CSS editor for Mx
//
// Author: Lionel Landwerlin <lionel.g.landwerlin@linux.intel.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

const Lang = imports.lang;
const GLib = imports.gi.GLib;
const Mainloop = imports.mainloop;
const GObject = imports.gi.GObject;
const Clutter = imports.gi.Clutter;
const Mx = imports.gi.Mx;
const CssEditorPaths = imports.cssEditorPaths;

function Overlay()
{
    this._init();
}

Overlay.prototype = {
    _init: function() {
        this._frame = new Mx.Frame({ name: 'mx-css-editor-overlay' });
        this._frame.set_opacity(0x80);
        this._tooltip = new Mx.Tooltip({ name: 'mx-css-editor-tooltip' });

        // var overlay_color = new Clutter.Color();
        // overlay_color.from_pixel(0x2266bb80);
        // var rectangle = new Clutter.Rectangle({ color: overlay_color });
        // this._overlay.set_child(rectangle);
    },

    add: function(stage) {
        log("add");
        if (this._stage == stage)
            return;
        this._stage = stage;

        this._stage.add_actor(this._frame);
        this._frame.raise_top();
        this._frame.hide();
        this._stage.add_actor(this._tooltip);
        this._tooltip.hide();
    },

    remove: function() {
        log("remove");
        this._frame.hide();
        this._stage.remove_actor(this._frame);
        this._tooltip.hide();
        this._stage.remove_actor(this._tooltip);

        this._stage = null;
    },

    set_geometry: function(x, y, width, height) {
        this._tooltip.set_tip_area(new Clutter.Geometry({ x: x,
                                                          y: y,
                                                          width: width,
                                                          height: height }));
        this._tooltip.set_text("" + Math.round(x) + "x" + Math.round(y) +
                               " -> " + Math.round(width) + "x" + Math.round(height));
        log(this._tooltip.get_text());
        this._tooltip.raise_top();
        this._tooltip.show();

        this._frame.set_position(x, y);
        this._frame.set_size(width, height);
        this._frame.show();
    },

    hide: function() {
        log("hide");
        this._frame.hide();
        this._tooltip.hide();
    }
};

//
// Inspector
//

function LiveInspector()
{
    this._init();
}

LiveInspector.prototype = {
    _init: function() {
        var style = Mx.Style.get_default();
        style.load_from_file(CssEditorPaths.get_css_editor_dir() + "/cssEditor.css");

        this._inspect_window = new Mx.Window();

        var main_box = new Mx.BoxLayout({ orientation: Mx.Orientation.HORIZONTAL });
        this._inspect_window.set_child(main_box);

        this._parent_box = new Mx.BoxLayout({ orientation: Mx.Orientation.VERTICAL });
        main_box.add_actor(this._parent_box, 0);
        main_box.child_set_expand(this._parent_box, true);
        main_box.child_set_y_fill(this._parent_box, true);

        this._style_box = new Mx.BoxLayout({orientation: Mx.Orientation.VERTICAL })
        main_box.add_actor(this._style_box, 1);
        main_box.child_set_expand(this._style_box, true);
        main_box.child_set_y_fill(this._style_box, true);

        var manager = Clutter.StageManager.get_default();
        manager.connect('stage-added', Lang.bind(this, this._stage_added));

        var stages = manager.list_stages();
        for (var i in stages) {
            if (this._inspect_window.get_clutter_stage() != stages[i])
                stages[i].connect('captured-event',
                                  Lang.bind(this, this._event_captured));
        }

        this._overlay = new Overlay();

        // this._window_visible = false;
        this._inspect_window.show();
    },

    _stage_added: function(manager, stage) {
        if (this._inspect_window.get_clutter_stage () != stage)
            stage.connect('captured-event', Lang.bind(this, this._event_captured));
    },

    _event_captured: function(stage, event) {
        // if (!this._window_visible) {
        //     if (event.type() == Clutter.EventType.KEY_PRESS &&
        //         event.modifier_state & Clutter.CONTROL_MASK &&
        //        ) {


        //     }
        //     return false;
        // }

        if (event.type() == Clutter.EventType.BUTTON_PRESS) {
            var [x, y] = event.get_coords();

            // Remove stuff from previous inspection
            this._clean_inspector();

            // Setup new inspection
            var widget = stage.get_actor_at_pos(Clutter.PickMode.ALL, x, y);
            log("event on " + widget);
            this._inspect_element(widget);
        }

        return false;
    },

    //
    // Inspection part
    //

    _clean_inspector: function() {
        this._parent_box.foreach(Lang.bind(this, function(child) {
            this._parent_box.remove_actor(child);
        }));
        this._clean_style();

        // Remove overlay
        if (this._inspected_widget) {
            var stage = this._inspected_widget.get_stage();

            this._overlay.remove();
        }

        this._inspected_widget = null;
    },

    _inspect_element: function(widget) {
        // Add overlay
        var stage = widget.get_stage();
        this._overlay.add(stage);

        // Inspect from lower child up to top parent
        this._inspected_widget = widget;
        this._widget_tree = new Array();
        var it_widget = widget;
        while (it_widget != null) {
            var w_name = "" + it_widget;
            var button = new Mx.Button({ label: w_name });

            this._widget_tree[w_name] = it_widget;

            // This test could be replaced by a check on the interface
            if (it_widget.list_properties != null) {
                // MxWidget -> inspect css props
                button.connect('clicked',
                               Lang.bind(this, function(button) {
                                   var widget = this._widget_tree[button.get_label()];
                                   this._inspect_style(widget);
                               }));
            } else {
                // ClutterActor -> nothing yet...
                button.connect('clicked',
                               Lang.bind(this, function(button) {
                                   var widget = this._widget_tree[button.get_label()];
                                   this._clean_style();
                                   this._inspected_widget = widget;
                               }));
            }

            button.connect('enter-event',
                           Lang.bind(this, function(button) {
                               var widget = this._widget_tree[button.get_label()];
                               var pos = widget.get_transformed_position();
                               var size = widget.get_transformed_size();

                               this._overlay.set_geometry(pos[0], pos[1],
                                                          size[0], size[1]);
                           }));
            button.connect('leave-event',
                           Lang.bind(this, function(button) {
                               //this._overlay.hide();
                           }));

            this._parent_box.add_actor(button, 0);
            this._parent_box.child_set_y_fill(button, true);
            this._parent_box.child_set_expand(button, true);

            it_widget = it_widget.get_parent();
        }
    },

    _create_css_view: function(text, begin_offset, end_offset) {
        var scroll = new Mx.ScrollView();
        var view = new Mx.Viewport();
        var ctext = new Clutter.Text({ text: text,
                                       editable: true,
                                       selectable: true,
                                       reactive: true });
        var coords = ctext.position_to_coords(begin_offset);

        ctext.set_selection(begin_offset, end_offset);
        view.set_child(ctext);
        scroll.set_child(view);
        log(coords);
        Mainloop.timeout_add(10,
                  Lang.bind(this,
                            function() {
                                log("  ->" + coords + " " + begin_offset + "/" + end_offset);
                                scroll.ensure_visible(new Clutter.Geometry({x: coords[1],
                                                                            y: coords[2],
                                                                            width: 10,
                                                                            height: coords[3] * 5}));
                            }));

        return scroll;
    },

    _clean_style: function() {
        this._style_box.foreach(Lang.bind(this, function(child) {
            this._style_box.remove_actor(child);
        }));
    },

    _inspect_style: function(widget) {
        this._style_box.foreach(Lang.bind(this, function(child) {
            this._style_box.remove_actor(child);
        }));

        this._inspected_widget = widget;
        log("Inspecting style for " + widget);

        var style = widget.get_style();
        var selectors = style.get_selectors(widget);

        for (var i in selectors) {
            log(selectors[i].filename + ":" + selectors[i].line);

            var ctext = GLib.file_get_contents(selectors[i].filename);
            var text = "" + ctext;
            var begin_offset = 0;
            var end_offset;
            var nb_lines = 0;
            for (var j in text) {
                begin_offset++;
                if (text[j] == '\n')
                    nb_lines++;
                if (nb_lines >= selectors[i].line) {
                    end_offset = begin_offset;
                    while (end_offset < text.length &&
                           text[end_offset] != '\n') {
                        end_offset++;
                    }
                    break;
                }
            }

            this._style_box.add_actor(this._create_css_view("" + text, begin_offset, end_offset), i);
        }
    }
};
