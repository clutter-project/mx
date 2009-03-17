#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int argc, char *argv[])
{
  ClutterActor *stage, *texture, *frame;
  GError *err = NULL;
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 500, 300);

  texture = clutter_texture_new_from_file ("rounded-corner.png", &err);

  if (err)
    {
      g_critical ("%s", err->message);
      return 1;
    }

  frame = nbtk_texture_frame_new (CLUTTER_TEXTURE (texture), 25, 25, 25, 25);
  clutter_actor_set_position (frame, 50, 50);
  clutter_actor_set_size (frame, 400, 200);

  clutter_container_add (CLUTTER_CONTAINER (stage), frame, NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
