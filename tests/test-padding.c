#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

gboolean
set_cb (ClutterActor  *actor,
        ClutterEvent  *event,
        NbtkWidget    *label)
{
  NbtkPadding *padding;

  NbtkPadding custom_padding = { CLUTTER_UNITS_FROM_INT (10), 
                                 CLUTTER_UNITS_FROM_INT (20), 
                                 CLUTTER_UNITS_FROM_INT (30), 
                                 CLUTTER_UNITS_FROM_INT (40) };

  g_object_set (label, "padding", &custom_padding, NULL);

  g_object_get (label, "padding", &padding, NULL);
  printf ("Padding: %d, %d, %d, %d\n", CLUTTER_UNITS_TO_INT(padding->top),
                                       CLUTTER_UNITS_TO_INT(padding->right),
                                       CLUTTER_UNITS_TO_INT(padding->bottom),
                                       CLUTTER_UNITS_TO_INT(padding->left));
  g_boxed_free (NBTK_TYPE_PADDING, padding);

  return TRUE;
}

gboolean
reset_cb (ClutterActor  *actor,
          ClutterEvent  *event,
          NbtkWidget    *label)
{
  NbtkPadding *padding;

  g_object_set (label, "padding", NULL, NULL);

  g_object_get (label, "padding", &padding, NULL);
  printf ("Padding: %d, %d, %d, %d\n", CLUTTER_UNITS_TO_INT(padding->top),
                                       CLUTTER_UNITS_TO_INT(padding->right),
                                       CLUTTER_UNITS_TO_INT(padding->bottom),
                                       CLUTTER_UNITS_TO_INT(padding->left));
  g_boxed_free (NBTK_TYPE_PADDING, padding);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  NbtkWidget *label, *set_button, *reset_button;
  ClutterColor stage_color =  { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *stage;
  GError *err = NULL;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 400, 300);

  label = nbtk_label_new ("Padding");
  clutter_actor_set_name (CLUTTER_ACTOR (label), "padded-label");
  clutter_actor_set_position (CLUTTER_ACTOR (label), 50, 50);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (label), NULL);

  set_button = nbtk_button_new_with_label ("Set custom padding");
  clutter_actor_set_position (CLUTTER_ACTOR (set_button), 50, 150);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (set_button), NULL);
  g_signal_connect (set_button, "button-release-event", G_CALLBACK (set_cb), label);

  reset_button = nbtk_button_new_with_label ("Clear custom padding");
  clutter_actor_set_position (CLUTTER_ACTOR (reset_button), 200, 150);
  clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (reset_button), NULL);
  g_signal_connect (reset_button, "button-release-event", G_CALLBACK (reset_cb), label);

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
