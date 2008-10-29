#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk.h>

typedef enum {
  DEFAULT_STYLE,
  RED_STYLE,
  GREEN_STYLE,
  BLUE_STYLE,

  N_STYLES
} StyleName;

static const gchar *style_names[N_STYLES] = {
  "default",
  "red",
  "green",
  "blue"
};

static ClutterActor *buttons[N_STYLES] = { NULL, };
static NbtkStyle *styles[N_STYLES] = { NULL, };

static ClutterActor *stage = NULL;

static void
button_clicked (NbtkButton *button,
                gpointer    data)
{
  StyleName style_name = GPOINTER_TO_INT (data);
  gint i;
  NbtkStyle *style;

  g_print ("[*] Button clicked (style: `%s')!\n", style_names[style_name]);

  style = styles[style_name];

  for (i = 0; i < N_STYLES; i++)
    {
      NbtkStylable *stylable = NBTK_STYLABLE (buttons[i]);

      nbtk_stylable_set_style (stylable, style);
    }
}

static NbtkStyle *
create_red_style (void)
{
  NbtkStyle *style;
  GValue value = { 0, };
  ClutterColor color = { 255, 0, 0, 255 };

  g_value_init (&value, CLUTTER_TYPE_COLOR);
  g_value_set_boxed (&value, &color);

  style = nbtk_style_new ();
  /* nbtk_style_set_property (style, NBTK_BACKGROUND_COLOR, &value); */

  g_value_unset (&value);

  return style;
}

static NbtkStyle *
create_green_style (void)
{
  NbtkStyle *style;
  GValue value = { 0, };
  ClutterColor color = { 0, 255, 0, 255 };

  g_value_init (&value, CLUTTER_TYPE_COLOR);
  g_value_set_boxed (&value, &color);

  style = nbtk_style_new ();
  /* nbtk_style_set_property (style, NBTK_BACKGROUND_COLOR, &value); */

  g_value_unset (&value);

  return style;
}

static NbtkStyle *
create_blue_style (void)
{
  NbtkStyle *style;
  GValue value = { 0, };
  ClutterColor color = { 0, 0, 255, 255 };

  g_value_init (&value, CLUTTER_TYPE_COLOR);
  g_value_set_boxed (&value, &color);

  style = nbtk_style_new ();
  /* nbtk_style_set_property (style, NBTK_BACKGROUND_COLOR, &value); */

  g_value_unset (&value);

  return style;
}

static ClutterActor *
create_button (ClutterActor *parent,
               const gchar  *text,
               gint          x,
               gint          y,
               StyleName     style_name)
{
  NbtkStyle *style = NULL;
  ClutterActor *button;
  NbtkPadding padding = { 0, };

  padding.top = padding.bottom = 0; 
  padding.left = padding.right = CLUTTER_UNITS_FROM_DEVICE (10); /* 10px */

  switch (style_name)
    {
    case DEFAULT_STYLE:
      style = nbtk_style_get_default ();
      break;

    case RED_STYLE:
      style = create_red_style ();
      break;

    case GREEN_STYLE:
      style = create_green_style ();
      break;

    case BLUE_STYLE:
      style = create_blue_style ();
      break;

    case N_STYLES:
      g_assert_not_reached ();
      return NULL;
    }

  button = nbtk_button_new_with_label (text);
  nbtk_widget_set_padding (NBTK_WIDGET (button), &padding);
  clutter_container_add_actor (CLUTTER_CONTAINER (parent), button);
  clutter_actor_set_position (button, x, y);
  clutter_actor_set_size (button, 200, 50);
  g_signal_connect (button,
                    "clicked", G_CALLBACK (button_clicked),
                    GINT_TO_POINTER (style_name));

  buttons[style_name] = button;
  styles[style_name] = style;

  return button;
}

int
main (int argc, char *argv[])
{
  ClutterActor *button;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = create_button (stage, "Default Style", 100, 100, DEFAULT_STYLE);
  clutter_actor_set_name (button, "defaultbutton");
  clutter_actor_show (button);

  button = create_button (stage, "Red Style", 100, 200, RED_STYLE);
  clutter_actor_set_name (button, "redbutton");
  clutter_actor_show (button);

  button = create_button (stage, "Green Style", 350, 100, GREEN_STYLE);
  clutter_actor_set_name (button, "greenbutton");
  clutter_actor_show (button);

  button = create_button (stage, "Blue Style", 350, 200, BLUE_STYLE);
  clutter_actor_set_name (button, "bluebutton");
  clutter_actor_show (button);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
