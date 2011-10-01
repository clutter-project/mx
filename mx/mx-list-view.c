/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-list-view.c: MxBoxLayout powered by a model
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

/**
 * SECTION:mx-list-view
 * @short_description: a box layout driven by a model.
 *
 * #MxListView is a box layout container driven by a #ClutterModel. Children
 * are created for each row in the model, either by creating actors from the
 * supplied #ClutterActor derived type, or from a #MxItemFactory.
 *
 * Data is set on the children by mapping columns in the model to object
 * properties on the children.
 */

#include "mx-list-view.h"
#include "mx-box-layout.h"
#include "mx-private.h"
#include "mx-item-factory.h"

G_DEFINE_TYPE (MxListView, mx_list_view, MX_TYPE_BOX_LAYOUT)

#define LIST_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_LIST_VIEW, MxListViewPrivate))

typedef struct
{
  gchar *name;
  gint   col;
} AttributeData;

enum
{
  PROP_0,

  PROP_MODEL,
  PROP_ITEM_TYPE,
  PROP_FACTORY
};

struct _MxListViewPrivate
{
  ClutterModel  *model;
  GSList        *attributes;
  GType          item_type;

  MxItemFactory *factory;

  gulong         filter_changed;
  gulong         row_added;
  gulong         row_changed;
  gulong         row_removed;
  gulong         sort_changed;

  guint          is_frozen : 1;
};

/* gobject implementations */

static void
mx_list_view_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  MxListViewPrivate *priv = MX_LIST_VIEW (object)->priv;
  switch (property_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, priv->model);
      break;
    case PROP_ITEM_TYPE:
      g_value_set_gtype (value, priv->item_type);
      break;
    case PROP_FACTORY:
      g_value_set_object (value, priv->factory);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_list_view_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_MODEL:
      mx_list_view_set_model ((MxListView*) object,
                              (ClutterModel*) g_value_get_object (value));
      break;
    case PROP_ITEM_TYPE:
      mx_list_view_set_item_type ((MxListView*) object,
                                  g_value_get_gtype (value));
      break;
    case PROP_FACTORY:
      mx_list_view_set_factory ((MxListView*) object,
                                (MxItemFactory*) g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_list_view_dispose (GObject *object)
{
  MxListViewPrivate *priv = MX_LIST_VIEW (object)->priv;

  /* This will cause the unref of the model and also disconnect the signals */
  mx_list_view_set_model (MX_LIST_VIEW (object), NULL);

  if (priv->factory)
    {
      g_object_unref (priv->factory);
      priv->factory = NULL;
    }

  G_OBJECT_CLASS (mx_list_view_parent_class)->dispose (object);
}

static void
free_attribute (AttributeData *data)
{
  g_free (data->name);
  g_free (data);
}

static void
mx_list_view_finalize (GObject *object)
{
  MxListViewPrivate *priv = MX_LIST_VIEW (object)->priv;

  if (priv->attributes)
    {
      g_slist_foreach (priv->attributes, (GFunc) free_attribute, NULL);
      g_slist_free (priv->attributes);
      priv->attributes = NULL;
    }

  G_OBJECT_CLASS (mx_list_view_parent_class)->finalize (object);
}

static void
mx_list_view_class_init (MxListViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxListViewPrivate));

  object_class->get_property = mx_list_view_get_property;
  object_class->set_property = mx_list_view_set_property;
  object_class->dispose = mx_list_view_dispose;
  object_class->finalize = mx_list_view_finalize;

  pspec = g_param_spec_object ("model",
                               "model",
                               "The model for the item view",
                               CLUTTER_TYPE_MODEL,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_MODEL, pspec);

  pspec = g_param_spec_gtype ("item-type",
                              "Item Type",
                              "The GType to use as the items in the view. "
                              "Must be a subclass of ClutterActor",
                              CLUTTER_TYPE_ACTOR,
                              MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ITEM_TYPE, pspec);

  /* Note, interfaces aren't necessarily objects, so you can't use
   * MX_TYPE_ITEM_FACTORY here. The function mx_list_view_set_factory does
   * a type check, so this is still safe.
   */
  pspec = g_param_spec_object ("factory",
                               "Factory",
                               "The MxItemFactory used for creating new items.",
                               G_TYPE_OBJECT /*MX_TYPE_ITEM_FACTORY*/,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FACTORY, pspec);
}

static void
mx_list_view_init (MxListView *list_view)
{
  list_view->priv = LIST_VIEW_PRIVATE (list_view);

  mx_box_layout_set_orientation (MX_BOX_LAYOUT (list_view), MX_ORIENTATION_VERTICAL);
}


/* model monitors */
static void
model_changed_cb (ClutterModel *model,
                  MxListView   *list_view)
{
  GSList *p;
  GList *l, *children;
  MxListViewPrivate *priv = list_view->priv;
  ClutterModelIter *iter = NULL;
  gint model_n = 0, child_n = 0;


  /* bail out if we don't yet have an item type or a factory */
  if (!priv->item_type && !priv->factory)
    return;

  if (priv->is_frozen)
    return;

  if (priv->item_type)
    {
      /* check the item-type is an descendant of ClutterActor */
      if (!g_type_is_a (priv->item_type, CLUTTER_TYPE_ACTOR))
        {
          g_warning ("%s is not a subclass of ClutterActor and therefore"
                     " cannot be used as items in an MxListView",
                     g_type_name (priv->item_type));
          return;
        }
    }

  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
  child_n = g_list_length (children);

  if (model)
    model_n = clutter_model_get_n_rows (priv->model);
  else
    model_n = 0;

  /* add children as needed */
  while (model_n > child_n)
    {
      ClutterActor *new_child;

      if (priv->item_type)
        {
          new_child = g_object_new (priv->item_type, NULL);
        }
      else
        {
          new_child = mx_item_factory_create (priv->factory);
        }

      clutter_container_add_actor (CLUTTER_CONTAINER (list_view),
                                   new_child);
      child_n++;
    }

  /* remove children as needed */
  l = g_list_last (children);
  while (child_n > model_n)
    {
      clutter_container_remove_actor (CLUTTER_CONTAINER (list_view),
                                      (ClutterActor*) l->data);
      l = g_list_previous (l);
      child_n--;
    }

  g_list_free (children);

  if (!priv->model)
    return;

  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));

  /* set the properties on the children */
  iter = clutter_model_get_first_iter (priv->model);
  l = children;
  while (iter && !clutter_model_iter_is_last (iter))
    {
      GObject *child;

      child = G_OBJECT (l->data);

      g_object_freeze_notify (child);
      for (p = priv->attributes; p; p = p->next)
        {
          GValue value = { 0, };
          AttributeData *attr = p->data;

          clutter_model_iter_get_value (iter, attr->col, &value);

          g_object_set_property (child, attr->name, &value);

          g_value_unset (&value);
        }
      g_object_thaw_notify (child);

      l = g_list_next (l);
      clutter_model_iter_next (iter);
    }

  g_list_free (children);

  if (iter)
    g_object_unref (iter);
}

static void
row_changed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                MxListView       *list_view)
{
  model_changed_cb (model, list_view);
}

static void
row_removed_cb (ClutterModel     *model,
                ClutterModelIter *iter,
                MxListView       *list_view)
{
  GList *children;
  GList *l;
  ClutterActor *child;

  if (list_view->priv->is_frozen)
    return;

  children = clutter_container_get_children (CLUTTER_CONTAINER (list_view));
  l = g_list_nth (children, clutter_model_iter_get_row (iter));
  child = (ClutterActor *) l->data;
  clutter_container_remove_actor (CLUTTER_CONTAINER (list_view), child);
  g_list_free (children);
}

/* public api */

/**
 * mx_list_view_new:
 *
 * Create a new #MxListView
 *
 * Returns: a newly allocated #MxListView
 */
ClutterActor *
mx_list_view_new (void)
{
  return g_object_new (MX_TYPE_LIST_VIEW, NULL);
}

/**
 * mx_list_view_get_item_type:
 * @list_view: An #MxListView
 *
 * Get the item type currently being used to create items
 *
 * Returns: a #GType
 */
GType
mx_list_view_get_item_type (MxListView *list_view)
{
  g_return_val_if_fail (MX_IS_LIST_VIEW (list_view), G_TYPE_INVALID);

  return list_view->priv->item_type;
}


/**
 * mx_list_view_set_item_type:
 * @list_view: An #MxListView
 * @item_type: A #GType
 *
 * Set the item type used to create items representing each row in the
 * model
 */
void
mx_list_view_set_item_type (MxListView *list_view,
                            GType       item_type)
{
  g_return_if_fail (MX_IS_LIST_VIEW (list_view));
  g_return_if_fail (g_type_is_a (item_type, CLUTTER_TYPE_ACTOR));

  list_view->priv->item_type = item_type;

  /* update the view */
  model_changed_cb (list_view->priv->model, list_view);
}

/**
 * mx_list_view_get_model:
 * @list_view: An #MxListView
 *
 * Get the model currently used by the #MxListView
 *
 * Returns: (transfer none): the current #ClutterModel
 */
ClutterModel*
mx_list_view_get_model (MxListView *list_view)
{
  g_return_val_if_fail (MX_IS_LIST_VIEW (list_view), NULL);

  return list_view->priv->model;
}

/**
 * mx_list_view_set_model:
 * @list_view: An #MxListView
 * @model: A #ClutterModel
 *
 * Set the model used by the #MxListView
 */
void
mx_list_view_set_model (MxListView   *list_view,
                        ClutterModel *model)
{
  MxListViewPrivate *priv;

  g_return_if_fail (MX_IS_LIST_VIEW (list_view));
  g_return_if_fail (model == NULL || CLUTTER_IS_MODEL (model));

  priv = list_view->priv;

  if (priv->model)
    {
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback) model_changed_cb,
                                            list_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback) row_changed_cb,
                                            list_view);
      g_signal_handlers_disconnect_by_func (priv->model,
                                            (GCallback) row_removed_cb,
                                            list_view);
      g_object_unref (priv->model);

      priv->model = NULL;
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
 * mx_list_view_add_attribute:
 * @list_view: An #MxListView
 * @attribute: Name of the attribute
 * @column: Column number
 *
 * Adds an attribute mapping between the current model and the objects from the
 * cell renderer.
 *
 */
void
mx_list_view_add_attribute (MxListView  *list_view,
                            const gchar *_attribute,
                            gint         column)
{
  MxListViewPrivate *priv;
  AttributeData *prop;

  g_return_if_fail (MX_IS_LIST_VIEW (list_view));
  g_return_if_fail (_attribute != NULL);
  g_return_if_fail (column >= 0);

  priv = list_view->priv;

  prop = g_new (AttributeData, 1);
  prop->name = g_strdup (_attribute);
  prop->col = column;

  priv->attributes = g_slist_prepend (priv->attributes, prop);
  model_changed_cb (priv->model, list_view);
}

/**
 * mx_list_view_freeze
 * @list_view: An #MxListView
 *
 * Freeze the view. This means that the view will not act on changes to the
 * model until it is thawed. Call #mx_list_view_thaw to thaw the view.
 */
void
mx_list_view_freeze (MxListView *list_view)
{
  MxListViewPrivate *priv;

  g_return_if_fail (MX_IS_LIST_VIEW (list_view));

  priv = list_view->priv;

  priv->is_frozen = TRUE;
}

/**
 * mx_list_view_thaw
 * @list_view: An #MxListView
 *
 * Thaw the view. This means that the view will now act on changes to the
 * model.
 */
void
mx_list_view_thaw (MxListView *list_view)
{
  MxListViewPrivate *priv;

  g_return_if_fail (MX_IS_LIST_VIEW (list_view));

  priv = list_view->priv;

  priv->is_frozen = FALSE;

  /* Repopulate */
  model_changed_cb (priv->model, list_view);
}

/**
 * mx_list_view_set_factory:
 * @list_view: A #MxListView
 * @factory: (allow-none): A #MxItemFactory
 *
 * Sets @factory to be the factory used for creating new list items
 */
void
mx_list_view_set_factory (MxListView    *list_view,
                          MxItemFactory *factory)
{
  MxListViewPrivate *priv;

  g_return_if_fail (MX_IS_LIST_VIEW (list_view));
  g_return_if_fail (!factory || MX_IS_ITEM_FACTORY (factory));

  priv = list_view->priv;

  if (priv->factory == factory)
    return;

  if (priv->factory)
    {
      g_object_unref (priv->factory);
      priv->factory = NULL;
    }

  if (factory)
    priv->factory = g_object_ref (factory);

  g_object_notify (G_OBJECT (list_view), "factory");
}

/**
 * mx_list_view_get_factory:
 * @list_view: A #MxListView
 *
 * Gets the #MxItemFactory used for creating new list items.
 *
 * Returns: (transfer none): A #MxItemFactory.
 */
MxItemFactory *
mx_list_view_get_factory (MxListView *list_view)
{
  g_return_val_if_fail (MX_IS_LIST_VIEW (list_view), NULL);
  return list_view->priv->factory;
}
