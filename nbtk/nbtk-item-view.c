/*
 * nbtk-item-view.c: NbtkGrid powered by a model
 *
 * Copyright 2008, 2009 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */


#include "nbtk-item-view.h"
#include "nbtk-cell-renderer.h"
#include "nbtk-private.h"

G_DEFINE_TYPE (NbtkItemView, nbtk_item_view, NBTK_TYPE_GRID)

#define ITEM_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_ITEM_VIEW, NbtkItemViewPrivate))

typedef struct
{
  gchar *name;
  gint   col;
} AttributeData;

enum
{
  PROP_0,

  PROP_MODEL,
  PROP_ITEM_TYPE
};

struct _NbtkItemViewPrivate
{
  ClutterModel *model;
  GSList       *attributes;
  GType         item_type;

  gulong filter_changed;
  gulong row_added;
  gulong row_changed;
  gulong row_removed;
  gulong sort_changed;
};

/* gobject implementations */

static void
nbtk_item_view_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  NbtkItemViewPrivate *priv = NBTK_ITEM_VIEW (object)->priv;
  switch (property_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, priv->model);
      break;
    case PROP_ITEM_TYPE:
      g_value_set_gtype (value, priv->item_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_item_view_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_MODEL:
      nbtk_item_view_set_model ((NbtkItemView*) object,
                                (ClutterModel*) g_value_get_object (value));
    case PROP_ITEM_TYPE:
      nbtk_item_view_set_item_type ((NbtkItemView*) object,
                                    g_value_get_gtype (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_item_view_dispose (GObject *object)
{
  G_OBJECT_CLASS (nbtk_item_view_parent_class)->dispose (object);

  /* This will cause the unref of the model and also disconnect the signals */
  nbtk_item_view_set_model (NBTK_ITEM_VIEW (object), NULL);
}

static void
nbtk_item_view_finalize (GObject *object)
{
  NbtkItemViewPrivate *priv = NBTK_ITEM_VIEW (object)->priv;

  G_OBJECT_CLASS (nbtk_item_view_parent_class)->finalize (object);


  if (priv->attributes)
    {
      g_slist_foreach (priv->attributes, (GFunc) g_free, NULL);
      g_slist_free (priv->attributes);
      priv->attributes = NULL;
    }
}

static void
nbtk_item_view_class_init (NbtkItemViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkItemViewPrivate));

  object_class->get_property = nbtk_item_view_get_property;
  object_class->set_property = nbtk_item_view_set_property;
  object_class->dispose = nbtk_item_view_dispose;
  object_class->finalize = nbtk_item_view_finalize;

  pspec = g_param_spec_object ("model",
                               "model",
                               "The model for the item view",
                               CLUTTER_TYPE_MODEL,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_MODEL, pspec);

  pspec = g_param_spec_object ("item-type",
                               "Item Type",
                               "The GType to use as the items in the view. "
                               "Must be a subclass of ClutterActor",
                               NBTK_TYPE_CELL_RENDERER,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ITEM_TYPE, pspec);
}

static void
nbtk_item_view_init (NbtkItemView *item_view)
{
  item_view->priv = ITEM_VIEW_PRIVATE (item_view);
}


/* model monitors */
static void
model_changed_cb (ClutterModel *model,
                  NbtkItemView *item_view)
{
  GSList *p;
  GList *l, *children;
  NbtkItemViewPrivate *priv = item_view->priv;
  ClutterModelIter *iter = NULL;
  gint model_n = 0, child_n = 0;


  /* bail out if we don't yet have an item type */
  if (!priv->item_type)
    return;

  /* check the item-type is an descendant of ClutterActor */
  if (!g_type_is_a (priv->item_type, CLUTTER_TYPE_ACTOR))
    {
      g_warning ("%s is not a subclass of ClutterActor and therefore"
                 " cannot be used as items in an NbtkItemView",
                 g_type_name (priv->item_type));
      return;
    }

  children = clutter_container_get_children (CLUTTER_CONTAINER (item_view));
  child_n = g_list_length (children);

  /* set the properties on the objects */
  if (priv->model)
    iter = clutter_model_get_first_iter (priv->model);

  model_n = 0;

  /* We count the number of rows in the model ourselves, as
   * clutter_list_model_get_n_rows() does not currently take into account the
   * filtered model. See Clutter bug 1562.
   */
  while (iter && !clutter_model_iter_is_last (iter))
    {
      GObject *child;

      model_n++;

      if (child_n < model_n)
        {
          ClutterActor *new_child;

          new_child = g_object_new (priv->item_type, NULL);


          clutter_container_add_actor (CLUTTER_CONTAINER (item_view),
                                       new_child);
          child = G_OBJECT (new_child);
          /* we've changed the number of children, so update the children list
           */
          children
            = clutter_container_get_children (CLUTTER_CONTAINER (item_view));
          child_n = g_list_length (children);

        }
      else
        {
          l = g_list_nth (children, model_n - 1);
          child = G_OBJECT (l->data);
        }

      g_object_freeze_notify (child);
      for (p = priv->attributes; p; p = p->next)
        {
          GValue value = { 0, };
          AttributeData *attr = p->data;

          clutter_model_iter_get_value (iter, attr->col, &value);

          g_object_set_property (child, attr->name, &value);
        }
      g_object_thaw_notify (child);

      clutter_model_iter_next (iter);
    }

  children = clutter_container_get_children (CLUTTER_CONTAINER (item_view));
  child_n = g_list_length (children);

  /* if the model shrank, we need to remove some items */
  while (child_n > model_n)
    {
      ClutterActor *child;

      /*
       * Here we remove surplus children from the end of the list as we have
       * already set properties on children at the beginning of the list.
       */
      l = g_list_last (children);
      child = CLUTTER_ACTOR (l->data);

      /* remove some children */
      clutter_container_remove_actor (CLUTTER_CONTAINER (item_view),
                                      child);

      children = clutter_container_get_children (CLUTTER_CONTAINER (item_view));
      child_n = g_list_length (children);
    }

  if (iter)
    g_object_unref (iter);
}

static void
row_changed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                NbtkItemView     *item_view)
{
  model_changed_cb (model, item_view);
}

static void
row_removed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                NbtkItemView     *item_view)
{
  GList *children;
  GList *l;
  ClutterActor *child;

  children = clutter_container_get_children (CLUTTER_CONTAINER (item_view));
  l = g_list_nth (children, clutter_model_iter_get_row (iter));
  child = (ClutterActor *)l->data;
  clutter_container_remove_actor (CLUTTER_CONTAINER (item_view), child);
  g_list_free (children);
}

/* public api */

/**
 * nbtk_item_view_new:
 *
 * Create a new #NbtkItemView
 *
 * Returns: a newly allocated #NbtkItemView
 */
NbtkWidget*
nbtk_item_view_new (void)
{
  return g_object_new (NBTK_TYPE_ITEM_VIEW, NULL);
}

/**
 * nbtk_item_view_get_item_type:
 * @item_view: An #NbtkItemView
 *
 * Get the item type currently being used to create items
 *
 * Returns: a #GType
 */
GType
nbtk_item_view_get_item_type (NbtkItemView *item_view)
{
  g_return_val_if_fail (NBTK_IS_ITEM_VIEW (item_view), G_TYPE_INVALID);

  return item_view->priv->item_type;
}


/**
 * nbtk_item_view_set_item_type:
 * @item_view: An #NbtkItemView
 * @item_type: A #GType
 *
 * Set the item type used to create items representing each row in the
 * model
 */
void
nbtk_item_view_set_item_type (NbtkItemView *item_view,
                              GType         item_type)
{
  g_return_if_fail (NBTK_IS_ITEM_VIEW (item_view));
  g_return_if_fail (g_type_is_a (item_type, CLUTTER_TYPE_ACTOR));

  item_view->priv->item_type = item_type;

  /* update the view */
  model_changed_cb (item_view->priv->model, item_view);
}

/**
 * nbtk_item_view_get_model:
 * @item_view: An #NbtkItemView
 *
 * Get the model currently used by the #NbtkItemView
 *
 * Returns: the current #ClutterModel
 */
ClutterModel*
nbtk_item_view_get_model (NbtkItemView *item_view)
{
  g_return_val_if_fail (NBTK_IS_ITEM_VIEW (item_view), NULL);

  return item_view->priv->model;
}

/**
 * nbtk_item_view_set_model:
 * @item_view: An #NbtkItemView
 * @model: A #ClutterModel
 *
 * Set the model used by the #NbtkItemView
 */
void
nbtk_item_view_set_model (NbtkItemView *item_view,
                          ClutterModel *model)
{
  NbtkItemViewPrivate *priv;

  g_return_if_fail (NBTK_IS_ITEM_VIEW (item_view));
  g_return_if_fail (model == NULL || CLUTTER_IS_MODEL (model));

  priv = item_view->priv;

  if (priv->model)
    {
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)model_changed_cb,
                                            item_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)row_changed_cb,
                                            item_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)row_removed_cb,
                                            item_view);
      g_object_unref (priv->model);
    }

  if (model)
    {
      g_return_if_fail (CLUTTER_IS_MODEL (model));

      priv->model = g_object_ref (model);

      priv->filter_changed = g_signal_connect (priv->model,
                                               "filter-changed",
                                               G_CALLBACK (model_changed_cb),
                                               item_view);

      priv->row_added = g_signal_connect (priv->model,
                                          "row-added",
                                          G_CALLBACK (row_changed_cb),
                                          item_view);

      priv->row_changed = g_signal_connect (priv->model,
                                            "row-changed",
                                            G_CALLBACK (row_changed_cb),
                                            item_view);

      /*
       * model_changed_cb (called from row_changed_cb) expect the row to already
       * have been removed, thus we need to use _after
       */
      priv->row_removed = g_signal_connect_after (priv->model,
                                                  "row-removed",
                                                  G_CALLBACK (row_removed_cb),
                                                  item_view);

      priv->sort_changed = g_signal_connect (priv->model,
                                             "sort-changed",
                                             G_CALLBACK (model_changed_cb),
                                             item_view);

      /*
       * Only do this inside this block, setting the model to NULL should have
       * the effect of preserving the view; just disconnect the handlers
       */
      model_changed_cb (priv->model, item_view);
  }
}

/**
 * nbtk_item_view_add_attribute:
 * @item_view: An #NbtkItemView
 * @attribute: Name of the attribute
 * @column: Column number
 *
 * Adds an attribute mapping between the current model and the objects from the
 * cell renderer.
 *
 */
void
nbtk_item_view_add_attribute (NbtkItemView *item_view,
                              const gchar  *attribute,
                              gint          column)
{
  NbtkItemViewPrivate *priv;
  AttributeData *prop;

  g_return_if_fail (NBTK_IS_ITEM_VIEW (item_view));
  g_return_if_fail (attribute != NULL);
  g_return_if_fail (column >= 0);

  priv = item_view->priv;

  prop = g_new (AttributeData, 1);
  prop->name = g_strdup (attribute);
  prop->col = column;

  priv->attributes = g_slist_prepend (priv->attributes, prop);
  model_changed_cb (priv->model, item_view);
}

/**
 * nbtk_item_view_freeze
 * @item_view: An #NbtkItemView
 *
 * Freeze the view. This means that the view will not act on changes to the
 * model until it is thawed. Call nbtk_item_view_thaw() to thaw the view
 */
void
nbtk_item_view_freeze (NbtkItemView *item_view)
{
  NbtkItemViewPrivate *priv;

  g_return_if_fail (NBTK_IS_ITEM_VIEW (item_view));

  priv = item_view->priv;

  g_signal_handlers_block_by_func (priv->model,
                                   model_changed_cb,
                                   item_view);

  g_signal_handlers_block_by_func (priv->model,
                                   row_removed_cb,
                                   item_view);

  g_signal_handlers_block_by_func (priv->model,
                                   row_changed_cb,
                                   item_view);
}

/**
 * nbtk_item_view_thaw
 * @item_view: An #NbtkItemView
 *
 * Thaw the view. This means that the view will now act on changes to the
 * model.
 */
void
nbtk_item_view_thaw (NbtkItemView *item_view)
{
  NbtkItemViewPrivate *priv;

  g_return_if_fail (NBTK_IS_ITEM_VIEW (item_view));

  priv = item_view->priv;

  g_signal_handlers_unblock_by_func (priv->model,
                                     model_changed_cb,
                                     item_view);

  g_signal_handlers_unblock_by_func (priv->model,
                                     row_removed_cb,
                                     item_view);

  g_signal_handlers_unblock_by_func (priv->model,
                                     row_changed_cb,
                                     item_view);

  /* Repopulate */
  model_changed_cb (priv->model, item_view);
}

