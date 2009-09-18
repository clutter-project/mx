
#include <stdio.h>
#include <stdlib.h>

#include <gmodule.h>
#include <clutter/clutter.h>
#include <mx/mx.h>

#define MX_SO "../mx/.libs/libmx-1.0.so"

typedef struct
{
  ClutterActor  *stage;
  GModule       *module;
  GList         *types;
  guint          n_iterations;
} test_benchmark_t;

typedef GType (*gobject_get_type_function) (void);

static GList *
read_types (const gchar *filename)
{
  FILE *fp;
  gchar *line;
  gsize len;
  GList *types;

  fp = fopen (filename, "r");
  if (!fp)
    {
      g_warning (G_STRLOC "opening '%s' failed", filename);
      return NULL;
    }

  line = NULL;
  len = 0;
  types = NULL;
  while (-1 < getline (&line, &len, fp))
    {
      if (g_str_has_prefix (line, "mx_"))
        {
          types = g_list_prepend (types, g_strstrip (line));
        }
      line = NULL;
      len = 0;
    }

  fclose (fp);
  return types;
}

static void
stage_test_actor (ClutterActor *stage,
                  MxWidget   *actor,
                  guint         n_iterations)
{
  guint style_changed_id;
  guint i;

  g_message ("testing '%s'", G_OBJECT_TYPE_NAME (actor));

  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (actor));

  style_changed_id = g_signal_lookup ("style-changed", MX_TYPE_WIDGET);
  for (i = 0; i < n_iterations; i++)
    {
      /* g_signal_emit (actor, style_changed_id, 0); */
      MX_WIDGET_GET_CLASS (actor)->style_changed (actor);
    }

  clutter_container_remove_actor (CLUTTER_CONTAINER (stage),
                                  CLUTTER_ACTOR (actor));
}

static gboolean
test_idle_cb (test_benchmark_t *test)
{
  gchar const *symbol_name;
  gobject_get_type_function get_type;

  g_return_val_if_fail (test->types, FALSE);

  symbol_name = (const gchar *) test->types->data;
  if (g_module_symbol (test->module, symbol_name, (gpointer *) &get_type) &&
      g_type_is_a (get_type (), MX_TYPE_WIDGET) &&
      get_type () != MX_TYPE_WIDGET)
    {
      MxWidget *actor = (MxWidget *) g_object_new (get_type (), NULL);
      g_object_ref_sink (actor);
      stage_test_actor (test->stage, actor, test->n_iterations);
      g_object_unref (actor);
    }
  else if (!get_type)
    {
      g_warning (G_STRLOC " failed to resolve symbol '%s' or not an MxWidget", symbol_name);
    }

  free (test->types->data);
  test->types = g_list_delete_link (test->types, test->types);

  if (test->types)
    g_idle_add ((GSourceFunc) test_idle_cb, test);
  else
    clutter_main_quit ();

  return FALSE;
}

int
main (int argc, char *argv[])
{
  test_benchmark_t test;

  clutter_init (&argc, &argv);

  test.stage = clutter_stage_get_default ();
  clutter_actor_set_size (test.stage, 480, 320);

  if (argc > 1)
    test.n_iterations = atoi (argv[1]);
  else
    test.n_iterations = 50;

  mx_style_load_from_file (mx_style_get_default (),
                             "style/default.css", NULL);

  test.module = g_module_open (MX_SO, G_MODULE_BIND_LAZY);
  if (!test.module)
    {
      g_warning (G_STRLOC " Could not dlopen '%s'", MX_SO);
      return EXIT_FAILURE;
    }

  test.types = read_types ("../docs/reference/libmx/mx.types");
  if (test.types)
    g_idle_add ((GSourceFunc) test_idle_cb, &test);

  clutter_actor_show (test.stage);
  clutter_main ();

  g_module_close (test.module);

  return EXIT_SUCCESS;
}
