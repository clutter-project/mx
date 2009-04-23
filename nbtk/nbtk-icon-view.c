/* nbtk-icon-view.c */

#include "nbtk-icon-view.h"
#include "nbtk-cell-renderer.h"
#include "nbtk-private.h"

G_DEFINE_TYPE (NbtkIconView, nbtk_icon_view, NBTK_TYPE_GRID)

#define ICON_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_ICON_VIEW, NbtkIconViewPrivate))

typedef struct
{
  gchar *name;
  gint   col;
} AttributeData;

enum
{
  PROP_0,

  PROP_MODEL,
  PROP_RENDERER
};

struct _NbtkIconViewPrivate
{
  ClutterModel      *model;
  NbtkCellRenderer  *renderer;
  GSList            *attributes;

  gulong filter_changed;
  gulong row_added;
  gulong row_changed;
  gulong row_removed;
  gulong sort_changed;
};

/* gobject implementations */

static void
nbtk_icon_view_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  NbtkIconViewPrivate *priv = NBTK_ICON_VIEW (object)->priv;
  switch (property_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, priv->model);
      break;
    case PROP_RENDERER:
      g_value_set_object (value, priv->renderer);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_icon_view_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    case PROP_MODEL:
      nbtk_icon_view_set_model ((NbtkIconView*) object,
                                (ClutterModel*) g_value_get_object (value));
    case PROP_RENDERER:
      nbtk_icon_view_set_cell_renderer ((NbtkIconView*) object,
                                        (NbtkCellRenderer*)
                                          g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_icon_view_dispose (GObject *object)
{
  NbtkIconViewPrivate *priv = NBTK_ICON_VIEW (object)->priv;

  G_OBJECT_CLASS (nbtk_icon_view_parent_class)->dispose (object);

  if (priv->model)
    {
      g_object_unref (priv->model);
      priv->model = NULL;
    }

  if (priv->renderer)
    {
      g_object_unref (priv->renderer);
      priv->renderer = NULL;
    }
}

static void
nbtk_icon_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_icon_view_parent_class)->finalize (object);


  if (priv->attributes)
    {
      g_slist_foreach (priv->attributes, (GFunc) g_free, NULL);
      g_slist_free (priv->attributes);
      priv->attributes = NULL;
    }
}

static void
nbtk_icon_view_class_init (NbtkIconViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkIconViewPrivate));

  object_class->get_property = nbtk_icon_view_get_property;
  object_class->set_property = nbtk_icon_view_set_property;
  object_class->dispose = nbtk_icon_view_dispose;
  object_class->finalize = nbtk_icon_view_finalize;

  pspec = g_param_spec_object ("model",
                               "model",
                               "The model for the icon view",
                               CLUTTER_TYPE_MODEL,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_MODEL, pspec);

  pspec = g_param_spec_object ("cell-renderer",
                               "cell-renderer",
                               "The renderer to use for the icon view",
                               NBTK_TYPE_CELL_RENDERER,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_RENDERER, pspec);
}

static void
nbtk_icon_view_init (NbtkIconView *self)
{
  self->priv = ICON_VIEW_PRIVATE (self);
}


/* model monitors */
static void
model_changed_cb (ClutterModel *model,
                  NbtkIconView *self)
{
  GSList *p;
  GList *l, *children;
  NbtkIconViewPrivate *priv = self->priv;
  ClutterModelIter *iter;
  gint model_n, child_n;


  /* bail out if we don't yet have a renderer */
  if (!priv->renderer)
    return;

  children = clutter_container_get_children (CLUTTER_CONTAINER (self));

  child_n = g_list_length (children);
  model_n = clutter_model_get_n_rows (model);

  while (child_n < model_n)
    {
      ClutterActor *new_child;

      new_child = nbtk_cell_renderer_get_actor (priv->renderer);
      if (!new_child)
        {
          g_warning ("Cell Renderer return NULL!");
          break;
        }

      clutter_container_add_actor (CLUTTER_CONTAINER (self),
                                   new_child);

      children = clutter_container_get_children (CLUTTER_CONTAINER (self));
      child_n = g_list_length (children);
    }

  while (child_n > model_n)
    {
      /* remove some children */
      clutter_container_remove_actor (CLUTTER_CONTAINER (self),
                                      CLUTTER_ACTOR (children->data));

      children = clutter_container_get_children (CLUTTER_CONTAINER (self));
      child_n = g_list_length (children);
    }

  if (model_n == 0)
    {
      /* no rows so we don't need to set properties */
      return;
    }

  /* set the properties on the objects */

  iter = clutter_model_get_first_iter (priv->model);

  for (l = children; l && iter; l = l->next)
    {
      GObject *child = G_OBJECT (l->data);

      g_object_freeze_notify (child);
      for (p = priv->attributes; p; p = p->next)
        {
          GValue value = { 0, };
          AttributeData *attr = p->data;

          clutter_model_iter_get_value (iter, attr->col, &value);

          g_object_set_property (child, attr->name, &value);
        }
      g_object_thaw_notify (child);

      if (clutter_model_iter_is_last (iter))
        break;

      clutter_model_iter_next (iter);
    }
  g_object_unref (iter);
}

static void
row_changed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                NbtkIconView     *self)
{
  model_changed_cb (model, self);
}



/* public api */

NbtkWidget*
nbtk_icon_view_new (void)
{
  return g_object_new (NBTK_TYPE_ICON_VIEW, NULL);
}

NbtkCellRenderer*
nbtk_icon_view_get_cell_renderer (NbtkIconView *self)
{
  g_return_val_if_fail (NBTK_IS_ICON_VIEW (self), NULL);

  return self->priv->renderer;
}


void
nbtk_icon_view_set_cell_renderer (NbtkIconView     *self,
                                  NbtkCellRenderer *renderer)
{
  NbtkIconViewPrivate *priv;

  g_return_if_fail (NBTK_IS_ICON_VIEW (self));
  g_return_if_fail (NBTK_IS_CELL_RENDERER (renderer));

  priv = self->priv;

  if (priv->renderer)
    {
      g_object_unref (priv->renderer);
    }

  priv->renderer = renderer;

  model_changed_cb (priv->model, self);
}

ClutterModel*
nbtk_icon_view_get_model (NbtkIconView *self)
{
  g_return_val_if_fail (NBTK_IS_ICON_VIEW (self), NULL);

  return self->priv->model;
}

void
nbtk_icon_view_set_model (NbtkIconView *self,
                          ClutterModel *model)
{
  NbtkIconViewPrivate *priv;

  g_return_if_fail (NBTK_IS_ICON_VIEW (self));
  g_return_if_fail (CLUTTER_IS_MODEL (model));

  priv = self->priv;

  if (priv->model)
    {
      g_object_unref (priv->model);
      /* remove all items */
    }

  priv->model = g_object_ref (model);

  priv->filter_changed = g_signal_connect (priv->model,
                                           "filter-changed",
                                           G_CALLBACK (model_changed_cb),
                                           self);

  priv->row_added = g_signal_connect (priv->model,
                                      "row-added",
                                      G_CALLBACK (row_changed_cb),
                                      self);

  priv->row_changed = g_signal_connect (priv->model,
                                        "row-changed",
                                        G_CALLBACK (row_changed_cb),
                                        self);

  priv->row_removed = g_signal_connect (priv->model,
                                        "row-removed",
                                        G_CALLBACK (row_changed_cb),
                                        self);

  priv->sort_changed = g_signal_connect (priv->model,
                                         "sort-changed",
                                         G_CALLBACK (model_changed_cb),
                                         self);

  model_changed_cb (priv->model, self);
}

void
nbtk_icon_view_add_attribute (NbtkIconView *self,
                              const gchar *attr,
                              gint column)
{
  NbtkIconViewPrivate *priv;
  AttributeData *prop;

  g_return_if_fail (NBTK_IS_ICON_VIEW (self));
  g_return_if_fail (attr != NULL);
  g_return_if_fail (column >= 0);

  priv = self->priv;

  prop = g_new (AttributeData, 1);
  prop->name = g_strdup (attr);
  prop->col = column;

  priv->attributes = g_slist_prepend (priv->attributes, prop);
  model_changed_cb (priv->model, self);
}
