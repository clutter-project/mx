#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>

int
main (int     argc,
      char  **argv)
{
  NbtkWidget *table, *label, *label2;
  ClutterActor *stage, *txt;

  clutter_init (&argc, &argv);

  nbtk_style_load_from_file (nbtk_style_get_default (),
                             "style/default.css", NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 480);

  table = nbtk_table_new ();
  nbtk_table_set_col_spacing (NBTK_TABLE (table), 10);
  clutter_actor_set_position (CLUTTER_ACTOR (table), 50, 50);
  clutter_actor_set_width (CLUTTER_ACTOR (table), 300);
  clutter_container_add (CLUTTER_CONTAINER (stage), 
			 CLUTTER_ACTOR (table), NULL);

  label = nbtk_label_new ("Short top text");
  nbtk_table_add_widget (NBTK_TABLE (table), label, 0, 0);

  label2 = nbtk_label_new ("");
  
  txt = nbtk_label_get_clutter_text(NBTK_LABEL(label2));
  clutter_text_set_ellipsize (CLUTTER_TEXT (txt), PANGO_ELLIPSIZE_NONE);
  clutter_text_set_line_alignment (CLUTTER_TEXT (txt), PANGO_ALIGN_LEFT);
  clutter_text_set_line_wrap (CLUTTER_TEXT (txt), TRUE);

  nbtk_table_add_widget (NBTK_TABLE (table), label2, 1, 0);

  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               CLUTTER_ACTOR (label2),
                               "y-expand", FALSE,
                               "x-expand", FALSE,
                               NULL);

  label = nbtk_label_new ("Short Bottom text");
  nbtk_table_add_widget (NBTK_TABLE (table), label, 2, 0);

  nbtk_label_set_text (label2, "Really really long long long long long long long long long long long long long long long long long long (ooooh this is verrrrrrry long!) long longlong long long longlong long long long \nlong longlong long long long longlonglonglonglonglonglonglonglonglonglonglonglong long long long long long long long long long Loooooooooooooooong text");

  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
