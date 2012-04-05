/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2012 Intel Corporation.
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
#include <mx/mx.h>
#include <stdlib.h>

#define MIN_INSPECTOR_WIDTH 300
#define MIN_TREE_WIDTH 200



typedef struct
{
  ClutterActor *tree;
  ClutterActor *frame;
  ClutterActor *inspector;
  ClutterActor *selected_widget;
  ClutterActor *combobox;
  ClutterActor *status;

  MxButtonGroup *group;

  guint  paint_overlay;
} MxBuilder;

static void mx_builder_set_selected_widget (MxBuilder *builder, ClutterActor *widget);

typedef GType (*TypeFunc) ();

static gboolean
mx_builder_is_container (gpointer widget)
{
  if (MX_IS_SCROLL_VIEW(widget) || MX_IS_BOX_LAYOUT(widget)
      || MX_IS_TABLE (widget) || MX_IS_GRID (widget) || MX_IS_FRAME(widget))
    return TRUE;

  return FALSE;
}

static void
mx_builder_tree_button_clicked_cb (MxButton *button, MxBuilder *builder)
{
  mx_builder_set_selected_widget (builder,
                                  g_object_get_data (G_OBJECT (button),
                                                     "builder-widget"));
}

static void
mx_builder_widget_tree_traverse_children (MxBuilder *builder,
                                          GList     *children,
                                          gint       depth)
{
  GList *subchildren;
  ClutterActor *button, *label, *hbox;

  while (children)
    {
      button = mx_button_new_with_label (G_OBJECT_TYPE_NAME (children->data));
      g_object_set_data (G_OBJECT (button), "builder-widget", children->data);
      g_signal_connect (button, "clicked",
                        G_CALLBACK (mx_builder_tree_button_clicked_cb), builder);
      mx_button_set_is_toggle (MX_BUTTON (button), TRUE);
      mx_button_group_add (builder->group, MX_BUTTON (button));

      if (builder->selected_widget == children->data)
        mx_button_set_toggled (MX_BUTTON (button), TRUE);

      label = mx_label_new ();
      clutter_actor_set_width (label, depth * 10);

      hbox = mx_box_layout_new ();
      mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), label, 0);
      mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), button, 1);

      mx_box_layout_insert_actor (MX_BOX_LAYOUT (builder->tree), hbox, -1);

      if (mx_builder_is_container (children->data))
        {
          subchildren = clutter_actor_get_children (children->data);
          if (subchildren)
            mx_builder_widget_tree_traverse_children (builder, subchildren,
                                                      depth + 1);
        }

      children = g_list_delete_link (children, children);
    }
}

static void
mx_builder_widget_tree_rebuild (MxBuilder *builder)
{
  GList *children, *l;
  ClutterActor *label;

  children = clutter_actor_get_children (builder->tree);

  for (l = children; l; l = g_list_next (l))
    clutter_actor_destroy (l->data);
  g_list_free (children);

  label = mx_label_new_with_text ("<b>Tree</b>");
  mx_label_set_use_markup (MX_LABEL (label), TRUE);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (builder->tree), label, 0);

  clutter_actor_set_width (builder->tree, MIN_TREE_WIDTH);

  children = clutter_actor_get_children (builder->frame);
  mx_builder_widget_tree_traverse_children (builder, children, 0);
}

static gboolean
num_to_string (GBinding     *binding,
               const GValue *source_value,
               GValue       *target_value,
               gpointer      userdata)
{
  gchar *string;

  if (G_VALUE_HOLDS_INT (source_value))
    string = g_strdup_printf ("%d", g_value_get_int (source_value));
  else if (G_VALUE_HOLDS_UINT (source_value))
    string = g_strdup_printf ("%d", g_value_get_uint (source_value));
  else if (G_VALUE_HOLDS_DOUBLE (source_value))
    string = g_strdup_printf ("%.2f", g_value_get_double (source_value));
  else if (G_VALUE_HOLDS_FLOAT (source_value))
    string = g_strdup_printf ("%.2f", g_value_get_float (source_value));
  else
    return FALSE;

  g_value_take_string (target_value, string);

  return TRUE;
}

static gboolean
string_to_num (GBinding     *binding,
               const GValue *source_value,
               GValue       *target_value,
               gpointer      userdata)
{
  if (G_VALUE_HOLDS_INT (target_value))
    g_value_set_int (target_value,
                     atoi (g_value_get_string (source_value)));
  else if (G_VALUE_HOLDS_UINT (target_value))
    g_value_set_uint (target_value,
                      atoi (g_value_get_string (source_value)));
  else if (G_VALUE_HOLDS_DOUBLE (target_value))
    g_value_set_double (target_value,
                        strtod (g_value_get_string (source_value), NULL));
  else if (G_VALUE_HOLDS_FLOAT (target_value))
    g_value_set_float (target_value,
                       strtof (g_value_get_string (source_value), NULL));
  else
    return FALSE;

  return TRUE;
}

static ClutterActor *
mx_builder_create_property_editor (GObject    *object,
                                   GParamSpec *pspec)
{
  ClutterActor *hbox, *label, *value;

  /* skip properties that are not writable */
  if (!(pspec->flags & G_PARAM_WRITABLE))
    return NULL;

  /* skip other properties */
  if (!g_str_has_prefix (g_type_name (pspec->owner_type), "Mx"))
      return NULL;

  hbox = mx_box_layout_new ();

  label = mx_label_new_with_text (pspec->name);
  clutter_actor_set_width (label, 150);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), label, 0);

  if (pspec->value_type == G_TYPE_BOOLEAN)
    {
      value = mx_toggle_new ();

      g_object_bind_property (object, pspec->name, value, "active",
                              G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    }
  else if (pspec->value_type == G_TYPE_STRING)
    {
      value = mx_entry_new ();

      g_object_bind_property (object, pspec->name, value, "text",
                              G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    }
  else if (pspec->value_type == G_TYPE_INT || pspec->value_type == G_TYPE_UINT
           || pspec->value_type == G_TYPE_FLOAT
           || pspec->value_type == G_TYPE_DOUBLE)
    {
      value = mx_entry_new ();

      g_object_bind_property_full (object, pspec->name, value, "text",
                                   G_BINDING_BIDIRECTIONAL
                                   | G_BINDING_SYNC_CREATE,
                                   num_to_string,
                                   string_to_num,
                                   NULL, NULL);
    }
  else if (g_type_is_a (pspec->value_type, G_TYPE_ENUM))
    {
      GEnumValue *evalue;
      GEnumClass *eclass;
      gint init = 0;

      value = mx_combo_box_new ();
      clutter_actor_set_width (value, 100);

      eclass = g_type_class_ref (pspec->value_type);

      while ((evalue = g_enum_get_value (eclass, init)))
        {
          mx_combo_box_append_text (MX_COMBO_BOX (value), evalue->value_nick);
          init++;
        }

      g_type_class_unref (eclass);

      g_object_bind_property (object, pspec->name, value, "index",
                              G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    }
  else
    value = NULL;

  if (value)
    {
      mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), value, 1);
      return hbox;
    }
  else
    return NULL;
}

static void
widget_paint (ClutterActor *widget)
{
  ClutterActorBox actorbox;

  clutter_actor_get_allocation_box (widget, &actorbox);

  cogl_set_source_color4f (1, 0, 0, 1);
  cogl_path_rectangle (0, 0, actorbox.x2 - actorbox.x1, actorbox.y2 - actorbox.y1);
  cogl_path_stroke ();
}


static ClutterActor *
mx_builder_create_packing_properties_inspector (ClutterActor *child)
{
  ClutterActor *container;
  GObjectClass *class;
  ClutterActor *vbox, *label;
  guint n_properties = 0;
  GParamSpec **properties;
  gint i;
  ClutterChildMeta *meta;

  container = clutter_actor_get_parent (child);


  vbox = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);

  class = G_OBJECT_GET_CLASS (container);

  properties = clutter_container_class_list_child_properties (class,
                                                              &n_properties);

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (container),
                                           child);

  if (n_properties > 0)
    {
      /* title */
      label = mx_label_new_with_text ("<b>Packing</b>");
      mx_label_set_use_markup (MX_LABEL (label), TRUE);
      mx_box_layout_insert_actor (MX_BOX_LAYOUT (vbox), label, -1);
    }

  for (i = n_properties - 1; i >= 0; i--)
    {
      ClutterActor *editor;

      editor = mx_builder_create_property_editor (G_OBJECT (meta), properties[i]);

      if (editor)
        mx_box_layout_insert_actor (MX_BOX_LAYOUT (vbox), editor, -1);
    }

  clutter_actor_set_width (vbox, MIN_INSPECTOR_WIDTH);

  return vbox;
}


/**
 * mx_builder_set_selected_widget:
 *
 * Set the currently selected widget in the UI
 */
static void
mx_builder_set_selected_widget (MxBuilder    *builder,
                                ClutterActor *new_widget)
{
  ClutterActor *vbox, *packing_properties;
  gint i;
  guint n_properties;
  GParamSpec **properties;
  GList *children, *l;
  ClutterActor *label, *scroll;
  GObjectClass *class;

  if (builder->paint_overlay)
    {
      g_signal_handler_disconnect (builder->selected_widget,
                                   builder->paint_overlay);
      builder->paint_overlay = 0;
      clutter_actor_queue_redraw (builder->selected_widget);
    }

  builder->selected_widget = new_widget;


  /* clear the current inspector window */
  children = clutter_actor_get_children (builder->inspector);
  for (l = children; l; l = g_list_next (l))
    clutter_actor_destroy (l->data);
  g_list_free (children);


  /* Title */
  label = mx_label_new_with_text ("<b>Inspector</b>");
  mx_label_set_use_markup (MX_LABEL (label), TRUE);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (builder->inspector), label, 0);
  clutter_actor_set_width (CLUTTER_ACTOR (label), MIN_INSPECTOR_WIDTH);


  /* set the combo box state */
  mx_widget_set_disabled (MX_WIDGET (builder->combobox),
                          !mx_builder_is_container (new_widget));

  /* rebuild the widget tree */
  mx_builder_widget_tree_rebuild (builder);

  /* don't create a property inspector for the frame */
  if (builder->selected_widget == builder->frame)
    return;


  /* connect to the paint signal to draw a rectangle around the selected
   * widget */
  mx_label_set_text (MX_LABEL (builder->status),
                     G_OBJECT_TYPE_NAME (builder->selected_widget));
  builder->paint_overlay = g_signal_connect_after (builder->selected_widget,
                                                   "paint",
                                                   G_CALLBACK (widget_paint),
                                                   NULL);
  clutter_actor_queue_redraw (builder->selected_widget);

  /* create the scroll view for the inspector */
  scroll = mx_scroll_view_new ();
  clutter_actor_set_width (scroll, 300);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (builder->inspector), scroll, 1);


  /* create the property editors */
  vbox = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);
  mx_bin_set_child (MX_BIN (scroll), vbox);

  class = G_OBJECT_GET_CLASS (new_widget);
  properties = g_object_class_list_properties (class, &n_properties);

  for (i = n_properties - 1; i >= 0; i--)
    {
      ClutterActor *editor;

      editor = mx_builder_create_property_editor (G_OBJECT (new_widget), properties[i]);

      if (editor)
        mx_box_layout_insert_actor (MX_BOX_LAYOUT (vbox), editor, 0);
    }

  /* create the packing properties inspector */
  packing_properties = mx_builder_create_packing_properties_inspector (builder->selected_widget);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (vbox), packing_properties, -1);

  /* add the property inspector to the scroll view */
  mx_bin_set_child (MX_BIN (scroll), vbox);

  /* find the tree item to select */
  children = clutter_actor_get_children (builder->tree);
  for (l = children; l; l = g_list_next (l))
    {
      ClutterActor *last_child;

      last_child = clutter_actor_get_last_child (l->data);
      if (g_object_get_data (G_OBJECT (last_child), "builder-widget")
          == builder->selected_widget)
        {
          mx_button_set_toggled (MX_BUTTON (last_child), TRUE);
          break;
        }
    }
  g_list_free (children);
}

static gboolean
widget_captured_event (ClutterActor *widget,
                       ClutterEvent *event,
                       MxBuilder    *builder)
{
  mx_builder_set_selected_widget (builder, widget);

  return TRUE;
}

static void
mx_builder_add_widget (MxBuilder *builder,
                       MxButton  *button)
{
  const gchar *typename;
  guint id;
  ClutterActor *new_widget;
  GParamSpec *pspec;

  typename = mx_combo_box_get_active_text (MX_COMBO_BOX (builder->combobox));

  /* skip separators */
  if (typename[0] == ' ')
    return;

  id = g_type_from_name (typename);

  new_widget = g_object_new (id, NULL);

  clutter_actor_set_reactive (new_widget, TRUE);
  g_signal_connect (new_widget, "button-press-event",
                    G_CALLBACK (widget_captured_event),
                    builder);

  clutter_container_add_actor (CLUTTER_CONTAINER (builder->selected_widget),
                               new_widget);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (new_widget), "label");
  if (pspec && g_type_is_a (pspec->value_type, G_TYPE_STRING))
    g_object_set (new_widget, "label", &typename[2], NULL);


  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (new_widget), "text");
  if (pspec && g_type_is_a (pspec->value_type, G_TYPE_STRING))
    g_object_set (new_widget, "text", &typename[2], NULL);

  mx_builder_set_selected_widget (builder, new_widget);
}

static void
mx_builder_remove_widget (MxBuilder *builder,
                          MxButton  *button)
{
  ClutterActor *old_widget;
  ClutterActor *parent = clutter_actor_get_parent (builder->selected_widget);

  if (builder->selected_widget == builder->frame)
    return;

  old_widget = builder->selected_widget;

  mx_builder_set_selected_widget (builder, parent);

  clutter_actor_destroy (old_widget);

  /* rebuild the widget tree now that the old widget has been destroyed */
  mx_builder_widget_tree_rebuild (builder);
}

/* this is private API from json-glib */
JsonNode *json_serialize_pspec (const GValue *real_value, GParamSpec *pspec);

static JsonArray*
save_children (GList *children)
{
  GList *subchildren;
  JsonArray *array;
  gint i;
  GObjectClass *object_class;
  guint n_properties;
  GParamSpec **properties;

  array = json_array_new ();

  while (children)
    {
      ClutterActor *child;
      JsonObject *object;

      child = children->data;

      object = json_object_new ();

      json_object_set_string_member (object, "type", G_OBJECT_TYPE_NAME (child));

      object_class = G_OBJECT_GET_CLASS (child);
      properties = g_object_class_list_properties (object_class, &n_properties);
      for (i = 0; i < n_properties; i++)
        {
          GParamSpec *pspec;
          JsonNode *node;
          GValue value = { 0, };

          pspec = properties[i];

          /* don't write out read-only properties */
          if (!(pspec->flags & G_PARAM_WRITABLE))
            continue;

          if (g_type_is_a (pspec->value_type, G_TYPE_OBJECT))
            continue;

          /* only include Mx properties */
          if (!g_str_has_prefix (g_type_name (pspec->owner_type), "Mx"))
            continue;

          g_value_init (&value, pspec->value_type);
          g_object_get_property (G_OBJECT (child), pspec->name, &value);

          if (g_param_value_defaults (pspec, &value))
            {
              /* skip properties with default values */
              g_value_unset (&value);
              continue;
            }

          /* this is private API from json-glib */
          node = json_serialize_pspec (&value, pspec);

          json_object_set_member (object, pspec->name, node);

          g_value_unset (&value);
        }
      g_free (properties);


      json_array_add_object_element (array, object);


      if (mx_builder_is_container (child))
        {
          if ((subchildren = clutter_actor_get_children (child)))
            {
              JsonArray *subarray;

              subarray = save_children (subchildren);

              json_object_set_array_member (object, "children", subarray);
            }
        }

      children = g_list_delete_link (children, children);
    }

  return array;
}

static void
mx_builder_save_widgets (MxBuilder *builder,
                         MxButton  *button)
{
  JsonGenerator *generator;
  JsonArray *array;
  JsonNode *root;
  gchar *buf;
  gsize buf_len;

  array = save_children (clutter_actor_get_children (builder->frame));

  generator = json_generator_new ();
  json_generator_set_pretty (generator, TRUE);
  root = json_node_new (JSON_NODE_ARRAY);
  json_node_set_array (root, array);

  json_generator_set_root (generator, root);

  json_array_unref (array);
  json_node_free (root);


  buf = json_generator_to_data (generator, &buf_len);

  g_file_replace_contents_async (g_file_new_for_path ("saved.json"),
                                 buf,
                                 buf_len,
                                 NULL,
                                 FALSE,
                                 G_FILE_CREATE_REPLACE_DESTINATION,
                                 NULL,
                                 (GAsyncReadyCallback)NULL,
                                 buf);

  g_object_unref (generator);
}

static void
mx_builder_frame_paint (ClutterActor *actor)
{
  ClutterActorBox box;
  gfloat width, height, x, y;

  cogl_set_source_color4ub (0x72, 0x9f, 0xcf, 0xff);

  clutter_actor_get_allocation_box (actor, &box);
  width = box.x2 - box.x1;
  height = box.y2 - box.y1;

  for (x = 0; x < width; x += 24)
    for (y = 0; y < height; y += 24)
      {
        cogl_path_rectangle (x, y, 24, 24);
        cogl_path_stroke ();
      }
}

static void
mx_builder_application_activate (GApplication *app,
                                 MxBuilder    *builder)
{
  MxWindow *window = mx_application_create_window (MX_APPLICATION (app),
                                                   "UI Builder");

  ClutterActor *hbox = mx_box_layout_new ();
  ClutterActor *toolbar_hbox, *button, *preview;
  MxToolbar *toolbar;
  gint i;

  GType types[] = {
   mx_box_layout_get_type (),
   mx_frame_get_type (),
   mx_grid_get_type (),
   mx_scroll_view_get_type (),
   mx_table_get_type (),
   0,
   mx_button_get_type (),
   mx_combo_box_get_type (),
   mx_entry_get_type (),
   mx_icon_get_type (),
   mx_label_get_type (),
   mx_progress_bar_get_type (),
   mx_slider_get_type (),
   mx_spinner_get_type (),
   mx_toggle_get_type (),
  };

  mx_window_show (window);

  mx_window_set_child (window, hbox);

  builder->tree = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);
  clutter_actor_set_width (builder->tree, 150);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), builder->tree, 0);

  preview = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);

  mx_box_layout_insert_actor_with_properties (MX_BOX_LAYOUT (hbox), preview, 1,
                                              "expand", TRUE, NULL);

  builder->status = mx_label_new (),
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (preview), builder->status, 0);

  builder->frame = mx_frame_new ();
  clutter_actor_set_size (builder->frame, 400, 400);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (preview), builder->frame, 1);
  g_signal_connect (builder->frame, "paint",
                    G_CALLBACK (mx_builder_frame_paint), NULL);

  builder->inspector = mx_box_layout_new_with_orientation (MX_ORIENTATION_VERTICAL);
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (hbox), builder->inspector, 2);


  toolbar = mx_window_get_toolbar (window);
  toolbar_hbox = mx_box_layout_new ();
  mx_bin_set_child (MX_BIN (toolbar), toolbar_hbox);

  builder->combobox = mx_combo_box_new ();
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (toolbar_hbox), builder->combobox,
                              0);

  for (i = 0; i < G_N_ELEMENTS (types); i++)
    {
      if (types[i] == 0)
        mx_combo_box_append_text (MX_COMBO_BOX (builder->combobox), " ");
      else
        mx_combo_box_append_text (MX_COMBO_BOX (builder->combobox),
                                  g_type_name (types[i]));
    }

  mx_combo_box_set_active_text (MX_COMBO_BOX (builder->combobox), "Add");
  g_signal_connect_swapped (builder->combobox, "notify::active-text",
                            G_CALLBACK (mx_builder_add_widget), builder);

  button = mx_button_new ();
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (toolbar_hbox), button, -1);
  mx_button_set_icon_name (MX_BUTTON (button), "remove");
  mx_button_set_icon_size (MX_BUTTON (button), 24);
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (mx_builder_remove_widget), builder);

  button = mx_button_new ();
  mx_box_layout_insert_actor (MX_BOX_LAYOUT (toolbar_hbox), button, -1);
  mx_button_set_icon_name (MX_BUTTON (button), "document-save");
  mx_button_set_icon_size (MX_BUTTON (button), 24);
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (mx_builder_save_widgets), builder);

  mx_builder_set_selected_widget (builder, builder->frame);

  builder->group = mx_button_group_new ();
}

static void
mx_builder_open_file (GApplication *application,
                      gpointer      files,
                      gint          n_files,
                      gchar        *hint,
                      gpointer      user_data)
{
  GFile *file;
  ClutterScript *script;
  GError *err = NULL;
  GList *objects, *l;
  gboolean found;
  MxBuilder *builder = user_data;

  if (n_files == 0)
    return;

  file = ((GFile**)files)[0];

  script = clutter_script_new ();
  if (!clutter_script_load_from_file (script, g_file_get_path (file), &err))
    {
      g_warning ("Error opening file: %s", err->message);
      g_error_free (err);
      return;
    }

  objects = clutter_script_list_objects (script);

  /* find the first ClutterActor and use it to find the top level */
  found = FALSE;
  for (l = objects; l; l = g_list_next (l))
    {
      if (CLUTTER_IS_ACTOR (l->data))
        {
          if (!found)
            {
              ClutterActor *toplevel;

              toplevel = l->data;

              /* find the top level actor by walking up the scene graph */
              while (clutter_actor_get_parent (toplevel))
                toplevel = clutter_actor_get_parent (toplevel);

              mx_bin_set_child (MX_BIN (builder->frame), toplevel);
              mx_builder_set_selected_widget (builder, toplevel);
              found = TRUE;
            }

          clutter_actor_set_reactive (l->data, TRUE);
          g_signal_connect (l->data, "button-press-event",
                            G_CALLBACK (widget_captured_event),
                            NULL);
        }
    }
  g_list_free (objects);
}

int
main (int argc, char **argv)
{
  MxApplication *app;

  MxBuilder builder = { 0, };

  app = mx_application_new ("org.clutter-project.Mx.Builder",
                            G_APPLICATION_HANDLES_OPEN);

  g_signal_connect (app, "startup",
                    G_CALLBACK (mx_builder_application_activate), &builder);
  g_signal_connect (app, "open", G_CALLBACK (mx_builder_open_file), &builder);

  g_application_run (G_APPLICATION (app), argc, argv);

  return 0;
}
