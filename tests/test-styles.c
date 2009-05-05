#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>


static ClutterActor *stage = NULL;

static NbtkWidget*
create_button (ClutterActor *parent,
               const gchar  *text,
               gint          x,
               gint          y)
{
  NbtkWidget *button;

  button = nbtk_button_new_with_label (text);
  clutter_container_add_actor (CLUTTER_CONTAINER (parent),
                               CLUTTER_ACTOR (button));
  clutter_actor_set_position (CLUTTER_ACTOR (button), x, y);
  clutter_actor_set_size (CLUTTER_ACTOR (button), 150, 100);

  return button;
}

int
main (int argc, char *argv[])
{
  NbtkWidget *button, *table;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };

  clutter_init (&argc, &argv);

  /* load the style sheet */
  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  button = create_button (stage, "Default Style", 100, 100);
  clutter_actor_set_name (CLUTTER_ACTOR (button), "default-button");

  button = create_button (stage, "Red Style", 100, 300);
  clutter_actor_set_name (CLUTTER_ACTOR (button), "red-button");

  button = create_button (stage, "Green Style", 350, 100);
  clutter_actor_set_name (CLUTTER_ACTOR (button), "green-button");

  button = create_button (stage, "Blue Style", 350, 300);
  clutter_actor_set_name (CLUTTER_ACTOR (button), "blue-button");

  table = nbtk_table_new ();
  clutter_actor_set_size (CLUTTER_ACTOR (table), 200, 80);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (table));
  clutter_actor_set_position (CLUTTER_ACTOR (table), 200, 215);

  button = nbtk_button_new_with_label ("Container Test");
  clutter_actor_set_name (CLUTTER_ACTOR (button), "container-button");
  nbtk_table_add_actor (NBTK_TABLE (table), CLUTTER_ACTOR (button), 0, 0);
  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
