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
const GObject = imports.gi.GObject;
const Clutter = imports.gi.Clutter;
const Mx = imports.gi.Mx;

//
// Composed properties (MxBorderImage/MxPadding)
//

function CssSubProperty(name, value, scope, callback)
{
    this._init(name, value, scope, callback);
}

CssSubProperty.prototype = {
    _init: function(name, value, scope, callback) {
        this.name = name;
        this.scope = scope;
        this.callback = callback

        this.actor = new Mx.BoxLayout({ orientation: Mx.Orientation.HORIZONTAL });

        //log("\t" + name + " -> " + value);

        var label_actor = new Mx.Label({ text: name + ":" });
        this.actor.add_actor(label_actor, 0);

        var value_actor = new Mx.Entry({ text: "" + value });
        this.actor.add_actor(value_actor, 1);

        value_actor.connect('event',
                            Lang.bind(this, this._value_event_captured));

    },

    _value_event_captured: function(value_actor, event) {
        if (event.type() == Clutter.EventType.KEY_RELEASE) {
            var symbol = event.get_key_symbol();
            if (symbol == Clutter.KEY_Return) {
                this.callback(this.scope, this.name, value_actor.get_text());
            }
        }

        return false;
    }
};

function ComposedCssProperty(widget, prop_name)
{
    this._init(widget, prop_name);
}

ComposedCssProperty.prototype = {
    _init: function(widget, prop_name) {
        this.widget = widget;
        this.prop_name = prop_name;

        this.actor = new Mx.BoxLayout({ orientation: Mx.Orientation.HORIZONTAL });

        var prop_value = widget.get_property(prop_name);

        //log("Added composed prop " + prop_name + " -> " + prop_value);

        var label = new Mx.Label({ text: prop_name + ":" });
        this.actor.add_actor(label, 0);

        var pbox = new Mx.BoxLayout({ orientation: Mx.Orientation.VERTICAL });
        this.actor.add_actor(pbox, 1);

        this._sub_props = new Array();
        for (var name in prop_value) {
            var value = prop_value[name];
            var sub_prop = new CssSubProperty(name, value,
                                              this, this._value_updated)
            this._sub_props.push(sub_prop);
            pbox.add_actor(sub_prop.actor, 0);
        }
    },

    _value_updated: function(name, value) {
        var prop_value = this.widget.get_property(this.prop_name);

        if (typeof(prop_value[name]) == "string")
            prop_value[name] = value;
        else if (typeof(prop_value[name]) == "integer")
            prop_value[name] = parseInt(value, 10);
        else {
            log("WTF: can't handle type for  " + name + " -> " + value);
            return;
        }

        this.widget.set_property(this.prop_name, prop_value);
    }
};

//
// Simple properties
//

function LitteralCssProperty(widget, prop_name)
{
    this._init(widget, prop_name);
}

LitteralCssProperty.prototype = {
    _init: function(widget, prop_name) {
        this.widget = widget;
        this.prop_name = prop_name;

        this.actor = new Mx.BoxLayout({ orientation: Mx.Orientation.HORIZONTAL });

        var prop_value = widget.get_property(prop_name);

        //log("Added simple prop " + prop_name + " -> " + prop_value);

        var label = new Mx.Label({ text: prop_name + ":" });
        this.actor.add_actor(label, 0);

        this._entry = new Mx.Entry({ text: "" + prop_value });
        this.actor.add_actor(this._entry, 1);

        this._entry.connect('event', Lang.bind(this, this._value_event_captured));
    },

    _value_event_captured: function(actor, event) {
        if (event.type() == Clutter.EventType.KEY_RELEASE) {
            var symbol = event.get_key_symbol();
            if (symbol == Clutter.KEY_Return) {
                var prop_value = this.widget.get_property(this.prop_name);

                if (typeof(prop_value) == "string") {
                    prop_value = actor.get_text();
                } else if (typeof(prop_value == "integer")) {
                    prop_value = parseInt(actor.get_text(), 10);
                } else {
                    log("WTF: can't handle type for  " + name + " -> " + value);
                    return false;
                }

                this.widget.set_property(this.prop_name, prop_value);
            }
        }

        return false;
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

        var overlay_color = new Clutter.Color();
        overlay_color.from_pixel(0x2266bb80);
        this._overlay = new Clutter.Rectangle({ color: overlay_color });

        var manager = Clutter.StageManager.get_default();
        manager.connect('stage-added', Lang.bind(this, this._stage_added));

        var stages = manager.list_stages();
        for (var i in stages) {
            if (this._inspect_window.get_clutter_stage() != stages[i])
                stages[i].connect('captured-event',
                                  Lang.bind(this, this._event_captured));
        }

        this._inspect_window.show();
    },

    _stage_added: function(manager, stage) {
        if (this._inspect_window.get_clutter_stage () != stage)
            stage.connect('captured-event', Lang.bind(this, this._event_captured));
    },

    _event_captured: function(stage, event) {
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

    _clean_style: function() {
        this._style_box.foreach(Lang.bind(this, function(child) {
            this._style_box.remove_actor(child);
        }));
    },

    _clean_inspector: function() {
        this._parent_box.foreach(Lang.bind(this, function(child) {
            this._parent_box.remove_actor(child);
        }));
        this._clean_style();

        // Remove overlay
        if (this._inspected_widget) {
            var stage = this._inspected_widget.get_stage();
            this._overlay.hide();

            stage.remove_actor(this._overlay);
        }

        this._inspected_widget = null;
    },

    _inspect_element: function(widget) {
        // Add overlay
        var stage = widget.get_stage();
        stage.add_actor(this._overlay);
        this._overlay.raise_top();
        this._overlay.hide();


        // var style = widget.get_style();
        // var stylesheetval = style.get_stylable_properties(widget);

        // for (var i in stylesheetval) {
        //     log ("" + i + " -> " + stylesheetval[i]);
        // }


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

                               this._overlay.set_position(pos[0], pos[1]);
                               this._overlay.set_size(size[0], size[1]);
                               this._overlay.show();
                           }));
            button.connect('leave-event',
                           Lang.bind(this, function(button) {
                               this._overlay.hide();
                           }));

            this._parent_box.add_actor(button, 0);
            this._parent_box.child_set_y_fill(button, true);
            this._parent_box.child_set_expand(button, true);

            it_widget = it_widget.get_parent();
        }
    },

    _inspect_style: function(widget) {
        this._style_box.foreach(Lang.bind(this, function(child) {
            this._style_box.remove_actor(child);
        }));

        this._inspected_widget = widget;
        log("Inspecting style for " + widget);

        var props = widget.list_properties();
        for (var i in props) {
            if (props[i].name != undefined) {
                var prop_name = props[i].name;
                var prop = props[i];

                if (prop != undefined || prop != null) {
                    var prop_value = widget.get_property(prop_name);
                    var prop_entry;

                    if (prop_value != null) {
                        var b;
                        if (typeof(prop_value) == "object") {
                            prop_entry = new ComposedCssProperty(widget, prop_name);
                        } else {
                            prop_entry = new LitteralCssProperty(widget, prop_name);
                        }

                        this._style_box.add_actor(prop_entry.actor, 0);
                        this._style_box.child_set_expand(prop_entry.actor, true);
                        this._style_box.child_set_x_fill(prop_entry.actor, true);
                    }
                }
            }
        }
    }
};
