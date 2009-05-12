#include <gtk/gtk.h>
#include <nbtk/nbtk.h>


int
main (int argc, char **argv)
{
  GtkWidget *window, *frame, *swtch;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 320, 240);

  frame = nbtk_gtk_frame_new ();
  gtk_container_add (GTK_CONTAINER (window), frame);


  swtch = nbtk_gtk_light_switch_new ();
  gtk_container_add (GTK_CONTAINER (frame), swtch);



  gtk_widget_show_all (window);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_main ();

  return 0;
}
