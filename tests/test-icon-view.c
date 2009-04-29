#include <stdio.h>
#include <stdlib.h>

#include <clutter/clutter.h>
#include <nbtk/nbtk.h>


/* simple rectangle renderer */
#define TEST_TYPE_RENDERER test_renderer_get_type()

typedef struct {
  NbtkCellRenderer parent;
} TestRenderer;

typedef struct {
  NbtkCellRendererClass parent_class;
} TestRendererClass;

GType test_renderer_get_type (void);

G_DEFINE_TYPE (TestRenderer, test_renderer, NBTK_TYPE_CELL_RENDERER)

ClutterActor *
test_renderer_get_actor (NbtkCellRenderer *renderer)
{
  ClutterActor *rectangle;

  rectangle = clutter_rectangle_new ();
  clutter_actor_set_size (rectangle, 64, 64);

  return rectangle;
}

static void
test_renderer_class_init (TestRendererClass *klass)
{
  NbtkCellRendererClass *renderer = NBTK_CELL_RENDERER_CLASS (klass);

  renderer->get_actor = test_renderer_get_actor;
}

static void
test_renderer_init (TestRenderer *renderer)
{
}


gint
sort_func (ClutterModel *model,
           const GValue *a,
           const GValue *b,
           gpointer user_data)
{
  const ClutterColor *ca, *cb;
  guint pa, pb;

  ca = clutter_value_get_color (a);
  cb = clutter_value_get_color (b);

  pa = clutter_color_to_pixel (ca);
  pb = clutter_color_to_pixel (cb);

  return pa-pb;
}

gboolean
filter_func (ClutterModel *model,
             ClutterModelIter *iter,
             gpointer user_data)
{
  ClutterColor *color;
  ClutterColor red = { 0xff, 0x00, 0x00, 0xff };
  gboolean show;

  clutter_model_iter_get (iter, 0, &color, -1);

  show = clutter_color_equal (color, &red);

  clutter_color_free (color);


  return show;
}

gboolean
key_release_cb (ClutterActor *actor,
                ClutterKeyEvent *event,
                ClutterModel *model)
{

  if (event->keyval == 's')
    {
      static gboolean sort_set = 0;

      if (!sort_set)
        clutter_model_set_sort (model, 0, sort_func, NULL, NULL);
      else
        clutter_model_set_sort (model, -1, NULL, NULL, NULL);

      sort_set = !sort_set;
    }

  if (event->keyval == 'f')
    {
      static gboolean filter_set = 0;

      if (!filter_set)
        clutter_model_set_filter (model, filter_func, NULL, NULL);
      else
        clutter_model_set_filter (model, NULL, NULL, NULL);

      filter_set = !filter_set;
    }

  return FALSE;
}

int
main (int argc, char *argv[])
{
  NbtkWidget *view;
  ClutterActor *stage;
  ClutterModel *model;
  ClutterColor red = { 0xff, 0x00, 0x00, 0xff};
  ClutterColor green = { 0x00, 0xff, 0x00, 0xff};
  ClutterColor blue = { 0x00, 0x00, 0xff, 0xff};

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 320, 240);

  view = nbtk_icon_view_new ();
  clutter_actor_set_size (CLUTTER_ACTOR (view), 320, 240);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage),
                               CLUTTER_ACTOR (view));


  model = clutter_list_model_new (1, CLUTTER_TYPE_COLOR, "color");

  clutter_model_append (model, 0, &red, -1);
  clutter_model_append (model, 0, &green, -1);
  clutter_model_append (model, 0, &blue, -1);
  clutter_model_append (model, 0, &red, -1);
  clutter_model_append (model, 0, &green, -1);
  clutter_model_append (model, 0, &blue, -1);
  clutter_model_append (model, 0, &red, -1);
  clutter_model_append (model, 0, &green, -1);
  clutter_model_append (model, 0, &blue, -1);
  clutter_model_append (model, 0, &red, -1);
  clutter_model_append (model, 0, &green, -1);
  clutter_model_append (model, 0, &blue, -1);

  nbtk_icon_view_set_model (NBTK_ICON_VIEW (view), model);
  nbtk_icon_view_set_cell_renderer (NBTK_ICON_VIEW (view),
                                    g_object_new (TEST_TYPE_RENDERER,
                                                  NULL));
  nbtk_icon_view_add_attribute (NBTK_ICON_VIEW (view), "color", 0);


  g_signal_connect (stage, "key-release-event", G_CALLBACK (key_release_cb), model);
  clutter_actor_show (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
