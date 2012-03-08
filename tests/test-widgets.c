/*
 * Copyright 2011 Intel Corporation.
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
#include <string.h>

static struct
{
  MxApplication *app;
  ClutterActor *table;
  ClutterActor *frame;
  ClutterActor *inspector;
} data;

/* skip properties that have no effect or could cause other problems */
static gchar *skip_properties[] = {
    "fixed-position-set",
    "fixed-x",
    "fixed-y",
    "height",
    "min-height",
    "min-height-set",
    "min-width",
    "min-width-set",
    "natural-height",
    "natural-height-set",
    "natural-width",
    "natural-width-set",
    "request-mode",
    "show-on-parent-set",
    "width",
    "x",
    "y"
};

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
create_property_editor (GObject    *object,
                        GParamSpec *pspec)
{
  ClutterActor *box, *label, *value;
  gint i;

  /* skip properties that are not writable */
  if (!(pspec->flags & G_PARAM_WRITABLE))
    return NULL;

  /* skip other properties */
  for (i = 0; i < G_N_ELEMENTS (skip_properties); i++)
    {
      if (g_str_equal (pspec->name, skip_properties[i]))
        return NULL;
    }


  box = mx_box_layout_new ();

  label = mx_label_new_with_text (pspec->name);
  clutter_actor_set_width (label, 150);
  clutter_container_add_actor (CLUTTER_CONTAINER (box), label);

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
      clutter_container_add_actor (CLUTTER_CONTAINER (box), value);
      return box;
    }
  else
    return NULL;
}

static void
change_widget (MxComboBox *box,
               GParamSpec *pspec,
               gpointer    userdata)
{
  GObjectClass *class;
  const gchar *typename;
  guint id;
  ClutterActor *actor, *vbox;
  GParamSpec **properties;
  guint n_properties;
  gint i;

  /* test the destroy sequence */
  if (mx_bin_get_child (MX_BIN (data.frame)))
    clutter_actor_destroy (mx_bin_get_child (MX_BIN (data.frame)));

  typename = mx_combo_box_get_active_text (box);
  id = g_type_from_name (typename);

  actor = g_object_new (id, NULL);

  class = G_OBJECT_GET_CLASS (actor);

  mx_bin_set_child (MX_BIN (data.frame), actor);

  if (data.inspector)
    clutter_actor_destroy (data.inspector);

  vbox = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (vbox),
                                 MX_ORIENTATION_VERTICAL);

  properties = g_object_class_list_properties (class, &n_properties);

  for (i = n_properties - 1; i >= 0; i--)
    {
      ClutterActor *editor;

      editor = create_property_editor (G_OBJECT (actor), properties[i]);

      /* set a default value for label or text property */
      if (g_str_equal (properties[i]->name, "label")
          || g_str_equal (properties[i]->name, "text"))
        g_object_set (actor, properties[i]->name, typename, NULL);

      if (editor)
        clutter_container_add_actor (CLUTTER_CONTAINER (vbox), editor);
    }

  /* set up various widgets as needed */
  if (g_str_equal (typename, "MxIcon"))
    g_object_set (actor, "icon-name", "face-smile", NULL);
  else if (g_str_equal (typename, "MxComboBox"))
    {
      mx_combo_box_set_active_text (MX_COMBO_BOX (actor), "MxComboBox");
      mx_combo_box_append_text (MX_COMBO_BOX (actor), "Item 1");
      mx_combo_box_append_text (MX_COMBO_BOX (actor), "Item 2");
      mx_combo_box_append_text (MX_COMBO_BOX (actor), "Item 3");
    }
  else if (g_str_equal (typename, "MxPathBar"))
    {
      mx_path_bar_push (MX_PATH_BAR (actor), "Mx");
      mx_path_bar_push (MX_PATH_BAR (actor), "Path");
      mx_path_bar_push (MX_PATH_BAR (actor), "Bar");
    }
  else if (g_str_equal (typename, "MxProgressBar"))
    {
      mx_progress_bar_set_progress (MX_PROGRESS_BAR (actor), 0.6);
    }
  else if (g_str_equal (typename, "MxScrollBar"))
    {
      mx_scroll_bar_set_adjustment (MX_SCROLL_BAR (actor),
                                    mx_adjustment_new_with_values (10, 0, 100,
                                                                   10, 25, 25));
    }

  data.inspector = mx_scroll_view_new ();
  clutter_actor_set_width (data.inspector, 300);
  clutter_container_add_actor (CLUTTER_CONTAINER (data.inspector), vbox);
  mx_table_add_actor_with_properties (MX_TABLE (data.table), data.inspector,
                                      0, 1,
                                      "x-expand", FALSE,
                                      NULL);
}

static ClutterActor*
create_combo_box (gchar *selected_widget)
{
  ClutterActor *combo;
  gint i, set_index = 0;
  gchar *needle;
  GType types[] = {
   mx_button_get_type (),
   mx_combo_box_get_type (),
   mx_entry_get_type (),
   mx_icon_get_type (),
   mx_label_get_type (),
   mx_path_bar_get_type (),
   mx_progress_bar_get_type (),
   mx_scroll_bar_get_type (),
   mx_slider_get_type (),
   mx_toggle_get_type (),
   mx_spinner_get_type (),
  };

  combo = mx_combo_box_new ();

  needle = g_ascii_strdown (selected_widget, -1);

  for (i = 0; i < G_N_ELEMENTS (types); i++)
    {
      gchar *haystack;

      haystack = g_ascii_strdown (g_type_name (types[i]), -1);

      if (strstr (haystack, needle))
        set_index = i;

      g_free (haystack);

      mx_combo_box_append_text (MX_COMBO_BOX (combo), g_type_name (types[i]));
    }

  g_free (needle);

  g_signal_connect (combo, "notify::index", G_CALLBACK (change_widget), NULL);

  mx_combo_box_set_index (MX_COMBO_BOX (combo), set_index);

  return combo;
}

static void
rotate_left_clicked_cb (ClutterActor *button,
                        MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static void
rotate_right_clicked_cb (ClutterActor *button,
                         MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static void
rotate_180_clicked_cb (ClutterActor *button,
                       MxWindow     *window)
{
  MxWindowRotation rotation = mx_window_get_window_rotation (window);

  switch (rotation)
    {
    case MX_WINDOW_ROTATION_0:
      rotation = MX_WINDOW_ROTATION_180;
      break;
    case MX_WINDOW_ROTATION_90:
      rotation = MX_WINDOW_ROTATION_270;
      break;
    case MX_WINDOW_ROTATION_180:
      rotation = MX_WINDOW_ROTATION_0;
      break;
    case MX_WINDOW_ROTATION_270:
      rotation = MX_WINDOW_ROTATION_90;
      break;
    }

  mx_window_set_window_rotation (window, rotation);
}

static ClutterActor *
create_rotate_box (MxWindow *window)
{
  ClutterActor *button, *icon, *icon2, *layout2;
  ClutterActor *layout = mx_box_layout_new ();

  /* Create rotate-left button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-left");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), icon);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_left_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  /* Create rotate-180 button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-left");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  icon2 = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon2), "object-rotate-right");
  mx_icon_set_icon_size (MX_ICON (icon2), 16);
  layout2 = mx_box_layout_new ();
  clutter_container_add (CLUTTER_CONTAINER (layout2), icon, icon2, NULL);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), layout2);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_180_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  /* Create rotate-right button */
  icon = mx_icon_new ();
  mx_icon_set_icon_name (MX_ICON (icon), "object-rotate-right");
  mx_icon_set_icon_size (MX_ICON (icon), 16);
  button = mx_button_new ();
  mx_bin_set_child (MX_BIN (button), icon);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (rotate_right_clicked_cb), window);
  clutter_container_add_actor (CLUTTER_CONTAINER (layout), button);

  return layout;
}

int
main (int argc, char **argv)
{
  MxApplication *application;
  MxWindow *window;
  ClutterActor *box;
  MxToolbar *toolbar;
  gchar *selected_widget;

  application = mx_application_new (&argc, &argv, "test-widgets", 0);

  window = mx_application_create_window (application);

  if (argc > 1)
    selected_widget = argv[1];

  /* main content */
  data.table = mx_table_new ();
  data.frame = mx_frame_new ();
  data.inspector = NULL;

  mx_table_add_actor (MX_TABLE (data.table), data.frame, 0, 0);

  mx_window_set_child (window, data.table);

  /* toolbar */
  box = mx_box_layout_new ();
  mx_box_layout_add_actor_with_properties (MX_BOX_LAYOUT (box),
                                           create_combo_box (selected_widget),
                                           0,
                                           "expand", TRUE,
                                           "x-fill", FALSE,
                                           "x-align", MX_ALIGN_START,
                                           NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), create_rotate_box (window), 1);

  toolbar = mx_window_get_toolbar (window);
  mx_bin_set_child (MX_BIN (toolbar), box);
  mx_bin_set_fill (MX_BIN (toolbar), TRUE, TRUE);


  /* show the window */
  mx_window_show (window);

  /* run the application */
  mx_application_run (application);

  return 0;
}
