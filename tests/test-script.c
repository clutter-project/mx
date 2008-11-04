#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <clutter/clutter.h>

static ClutterScript *script = NULL;

int
main (int argc, char *argv[])
{
  GObject *stage;
  GError *error = NULL;
  gint res;

  clutter_init (&argc, &argv);

  script = clutter_script_new ();
  g_assert (CLUTTER_IS_SCRIPT (script));

  clutter_script_load_from_file (script, "test-script.json", &error);
  if (error)
    {
      g_print ("*** Error:\n"
               "***   %s\n", error->message);
      g_error_free (error);
      g_object_unref (script);
      return EXIT_FAILURE;
    }

  clutter_script_connect_signals (script, NULL);

  res = clutter_script_get_objects (script,
                                    "main-stage", &stage,
                                    NULL);

  clutter_actor_show (CLUTTER_ACTOR (stage));

  clutter_main ();

  g_object_unref (script);

  return EXIT_SUCCESS;
}
