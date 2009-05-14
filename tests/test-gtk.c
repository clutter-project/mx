#include <stdlib.h>
#include <gtk/gtk.h>
#include <nbtk/nbtk-gtk.h>

static void
_toggle_cb (GtkToggleButton *toggle,
            gpointer         user_data)
{
  gboolean active = gtk_toggle_button_get_active (toggle);
  g_debug ("toggling to '%d'", active);
  nbtk_gtk_light_switch_set_active (NBTK_GTK_LIGHT_SWITCH (user_data), active);
}

int
main (int argc, char **argv)
{
  GtkWidget *window, *vbox, *frame, *swtch, *toggle;
  gboolean is_active = FALSE;

  gtk_init (&argc, &argv);

  if (argc > 1)
    is_active = atoi (argv[1]);

  g_debug ("setting switch to '%d'", is_active);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 320, 240);

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  frame = nbtk_gtk_frame_new ();
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  swtch = nbtk_gtk_light_switch_new ();
  nbtk_gtk_light_switch_set_active (NBTK_GTK_LIGHT_SWITCH (swtch), is_active);
  gtk_container_add (GTK_CONTAINER (frame), swtch);

  toggle = gtk_toggle_button_new_with_label ("Toggle");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), is_active);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  g_signal_connect (toggle, "toggled", G_CALLBACK (_toggle_cb), swtch);


  gtk_widget_show_all (window);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_main ();

  return 0;
}
