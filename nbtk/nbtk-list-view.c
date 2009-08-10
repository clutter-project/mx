/*
 * nbtk-list-view.c: NbtkBoxLayout powered by a model
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


#include "nbtk-list-view.h"
#include "nbtk-box-layout.h"
#include "nbtk-private.h"
#include "nbtk-item-factory.h"

G_DEFINE_TYPE (NbtkListView, nbtk_list_view, NBTK_TYPE_BOX_LAYOUT)

#define LIST_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_LIST_VIEW, NbtkListViewPrivate))

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

struct _NbtkListViewPrivate
{
  ClutterModel *model;
  GSList       *attributes;
  GType         item_type;

  NbtkItemFactory *factory;

  gulong filter_changed;
  gulong row_added;
  gulong row_changed;
  gulong row_removed;
  gulong sort_changed;
};

/* gobject implementations */

static void
nbtk_list_view_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  NbtkListViewPrivate *priv = NBTK_LIST_VIEW (object)->priv;
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
nbtk_list_view_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_MODEL:
      nbtk_list_view_set_model ((NbtkListView*) object,
                                (ClutterModel*) g_value_get_object (value));
    case PROP_ITEM_TYPE:
      nbtk_list_view_set_item_type ((NbtkListView*) object,
                                    g_value_get_gtype (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_list_view_dispose (GObject *object)
{
  NbtkListViewPrivate *priv = NBTK_LIST_VIEW (object)->priv;

  G_OBJECT_CLASS (nbtk_list_view_parent_class)->dispose (object);

  /* This will cause the unref of the model and also disconnect the signals */
  nbtk_list_view_set_model (NBTK_LIST_VIEW (object), NULL);

  if (priv->factory) {
    g_object_unref (priv->factory);
    priv->factory = NULL;
  }
}

static void
nbtk_list_view_finalize (GObject *object)
{
  NbtkListViewPrivate *priv = NBTK_LIST_VIEW (object)->priv;

  G_OBJECT_CLASS (nbtk_list_view_parent_class)->finalize (object);


  if (priv->attributes)
    {
      g_slist_foreach (priv->attributes, (GFunc) g_free, NULL);
      g_slist_free (priv->attributes);
      priv->attributes = NULL;
    }
}

static void
nbtk_list_view_class_init (NbtkListViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkListViewPrivate));

  object_class->get_property = nbtk_list_view_get_property;
  object_class->set_property = nbtk_list_view_set_property;
  object_class->dispose = nbtk_list_view_dispose;
  object_class->finalize = nbtk_list_view_finalize;

  pspec = g_param_spec_object ("model",
                               "model",
                               "The model for the item view",
                               CLUTTER_TYPE_MODEL,
                               NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_MODEL, pspec);

  pspec = g_param_spec_gtype ("item-type",
                              "Item Type",
                              "The GType to use as the items in the view. "
                              "Must be a subclass of ClutterActor",
                              CLUTTER_TYPE_ACTOR,
                              NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ITEM_TYPE, pspec);
}

static void
nbtk_list_view_init (NbtkListView *list_view)
{
  list_view->priv = LIST_VIEW_PRIVATE (list_view);

  g_object_set (list_view, "vertical", TRUE, NULL);
}


/* model monitors */
static void
model_changed_cb (ClutterModel *model,
                  NbtkListView *list_view)
{
  GSList *p;
  GList *l, *children;
  NbtkListViewPrivate *priv = list_view->priv;
  ClutterModelIter *iter = NULL;
  gint model_n = 0, child_n = 0;


  /* bail out if we don't yet have an item type or a factory */
  if (!priv->item_type && !priv->factory)
    return;

  if (priv->item_type)
    {
      /* check the item-type is an descendant of ClutterActor */
      if (!g_type_is_a (priv->item_type, CLUTTER_TYPE_ACTOR))
        {
          g_warning ("%s is not a subclass of ClutterActor and therefore"
                     " cannot be used as items in an NbtkListView",
                     g_type_name (priv->item_type));
          return;
        }
    }

  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
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

          if (priv->item_type)
            {
              new_child = g_object_new (priv->item_type, NULL);
            }
          else
            {
              new_child = nbtk_item_factory_create_item (priv->factory);
            }

          clutter_container_add_actor (CLUTTER_CONTAINER (list_view),
                                       new_child);
          child = G_OBJECT (new_child);
          /* we've changed the number of children, so update the children list
           */
          g_list_free (children);
          children
            = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
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

  g_list_free (children);
  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
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
      clutter_container_remove_actor (CLUTTER_CONTAINER (list_view),
                                      child);
      g_list_free (children);
      children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
      child_n = g_list_length (children);
    }

  g_list_free (children);

  if (iter)
    g_object_unref (iter);
}

static void
row_changed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                NbtkListView     *list_view)
{
  model_changed_cb (model, list_view);
}

static void
row_removed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                NbtkListView     *list_view)
{
  GList *children;
  GList *l;
  ClutterActor *child;

  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
  l = g_list_nth (children, clutter_model_iter_get_row (iter));
  child = (ClutterActor *)l->data;
  clutter_container_remove_actor (CLUTTER_CONTAINER (list_view), child);
  g_list_free (children);
}

/* public api */

/**
 * nbtk_list_view_new:
 *
 * Create a new #NbtkListView
 *
 * Returns: a newly allocated #NbtkListView
 */
NbtkWidget*
nbtk_list_view_new (void)
{
  return g_object_new (NBTK_TYPE_LIST_VIEW, NULL);
}

/**
 * nbtk_list_view_get_item_type:
 * @list_view: An #NbtkListView
 *
 * Get the item type currently being used to create items
 *
 * Returns: a #GType
 */
GType
nbtk_list_view_get_item_type (NbtkListView *list_view)
{
  g_return_val_if_fail (NBTK_IS_LIST_VIEW (list_view), G_TYPE_INVALID);

  return list_view->priv->item_type;
}


/**
 * nbtk_list_view_set_item_type:
 * @list_view: An #NbtkListView
 * @item_type: A #GType
 *
 * Set the item type used to create items representing each row in the
 * model
 */
void
nbtk_list_view_set_item_type (NbtkListView *list_view,
                              GType         item_type)
{
  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));
  g_return_if_fail (g_type_is_a (item_type, CLUTTER_TYPE_ACTOR));

  list_view->priv->item_type = item_type;

  /* update the view */
  model_changed_cb (list_view->priv->model, list_view);
}

/**
 * nbtk_list_view_get_model:
 * @list_view: An #NbtkListView
 *
 * Get the model currently used by the #NbtkListView
 *
 * Returns: the current #ClutterModel
 */
ClutterModel*
nbtk_list_view_get_model (NbtkListView *list_view)
{
  g_return_val_if_fail (NBTK_IS_LIST_VIEW (list_view), NULL);

  return list_view->priv->model;
}

/**
 * nbtk_list_view_set_model:
 * @list_view: An #NbtkListView
 * @model: A #ClutterModel
 *
 * Set the model used by the #NbtkListView
 */
void
nbtk_list_view_set_model (NbtkListView *list_view,
                          ClutterModel *model)
{
  NbtkListViewPrivate *priv;

  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));
  g_return_if_fail (model == NULL || CLUTTER_IS_MODEL (model));

  priv = list_view->priv;

  if (priv->model)
    {
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)model_changed_cb,
                                            list_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)row_changed_cb,
                                            list_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback)row_removed_cb,
                                            list_view);
      g_object_unref (priv->model);
    }

  if (model)
    {
      g_return_if_fail (CLUTTER_IS_MODEL (model));

      priv->model = g_object_ref (model);

      priv->filter_changed = g_signal_connect (priv->model,
                                               "filter-changed",
                                               G_CALLBACK (model_changed_cb),
                                               list_view);

      priv->row_added = g_signal_connect (priv->model,
                                          "row-added",
                                          G_CALLBACK (row_changed_cb),
                                          list_view);

      priv->row_changed = g_signal_connect (priv->model,
                                            "row-changed",
                                            G_CALLBACK (row_changed_cb),
                                            list_view);

      /*
       * model_changed_cb (called from row_changed_cb) expect the row to already
       * have been removed, thus we need to use _after
       */
      priv->row_removed = g_signal_connect_after (priv->model,
                                                  "row-removed",
                                                  G_CALLBACK (row_removed_cb),
                                                  list_view);

      priv->sort_changed = g_signal_connect (priv->model,
                                             "sort-changed",
                                             G_CALLBACK (model_changed_cb),
                                             list_view);

      /*
       * Only do this inside this block, setting the model to NULL should have
       * the effect of preserving the view; just disconnect the handlers
       */
      model_changed_cb (priv->model, list_view);
  }
}

/**
 * nbtk_list_view_add_attribute:
 * @list_view: An #NbtkListView
 * @attribute: Name of the attribute
 * @column: Column number
 *
 * Adds an attribute mapping between the current model and the objects from the
 * cell renderer.
 *
 */
void
nbtk_list_view_add_attribute (NbtkListView *list_view,
                              const gchar  *attribute,
                              gint          column)
{
  NbtkListViewPrivate *priv;
  AttributeData *prop;

  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));
  g_return_if_fail (attribute != NULL);
  g_return_if_fail (column >= 0);

  priv = list_view->priv;

  prop = g_new (AttributeData, 1);
  prop->name = g_strdup (attribute);
  prop->col = column;

  priv->attributes = g_slist_prepend (priv->attributes, prop);
  model_changed_cb (priv->model, list_view);
}

/**
 * nbtk_list_view_freeze
 * @list_view: An #NbtkListView
 *
 * Freeze the view. This means that the view will not act on changes to the
 * model until it is thawed. Call nbtk_list_view_thaw() to thaw the view
 */
void
nbtk_list_view_freeze (NbtkListView *list_view)
{
  NbtkListViewPrivate *priv;

  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));

  priv = list_view->priv;

  g_signal_handlers_block_by_func (priv->model,
                                   model_changed_cb,
                                   list_view);

  g_signal_handlers_block_by_func (priv->model,
                                   row_removed_cb,
                                   list_view);

  g_signal_handlers_block_by_func (priv->model,
                                   row_changed_cb,
                                   list_view);
}

/**
 * nbtk_list_view_thaw
 * @list_view: An #NbtkListView
 *
 * Thaw the view. This means that the view will now act on changes to the
 * model.
 */
void
nbtk_list_view_thaw (NbtkListView *list_view)
{
  NbtkListViewPrivate *priv;

  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));

  priv = list_view->priv;

  g_signal_handlers_unblock_by_func (priv->model,
                                     model_changed_cb,
                                     list_view);

  g_signal_handlers_unblock_by_func (priv->model,
                                     row_removed_cb,
                                     list_view);

  g_signal_handlers_unblock_by_func (priv->model,
                                     row_changed_cb,
                                     list_view);

  /* Repopulate */
  model_changed_cb (priv->model, list_view);
}

/**
 * nbtk_list_view_set_item_factory:
 * @list_view: A #NbtkListView
 * @factory: A #NbtkItemFactory
 *
 * Sets @factory to be the factory used for creating new list items
 */
void
nbtk_list_view_set_item_factory (NbtkListView    *list_view,
                                 NbtkItemFactory *factory)
{
  NbtkListViewPrivate *priv;

  g_return_if_fail (NBTK_IS_LIST_VIEW (list_view));

  priv = list_view->priv;

  if (priv->factory)
    {
      g_object_unref (priv->factory);
    }

  priv->factory = g_object_ref (factory);
}
