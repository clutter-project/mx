#include <nbtk/nbtk.h>

int
main (int argc, char **argv)
{
  ClutterActor *stage;
  ClutterActor *combo;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  combo = (ClutterActor*) nbtk_combo_box_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), combo);
  clutter_actor_set_position (combo, 10, 10);

  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Strand");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Fleet Street");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Trafalgar Square");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Leicester Square");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Coventry Street");
  nbtk_combo_box_append_text (NBTK_COMBO_BOX (combo), "Piccadilly");
  nbtk_combo_box_set_title (NBTK_COMBO_BOX (combo), "London");

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
