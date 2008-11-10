/* nbtk-table.c: Table layout widget
 *
 * Copyright 2008 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas@linux.intel.com>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/clutter-container.h>

#include "nbtk-table.h"

enum
{
  PROP_0,

  PROP_COL_SPACING,
  PROP_ROW_SPACING
};

#define NBTK_TABLE_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_TABLE, NbtkTablePrivate))

struct _NbtkTablePrivate
{
  GSList *children;

  gint col_spacing;
  gint row_spacing;

  gint n_rows;
  gint n_cols;
};

static void nbtk_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkTable, nbtk_table, CLUTTER_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                nbtk_container_iface_init));


/*
 * ClutterChildMeta Implementation
 */

enum {
  CHILD_PROP_0,

  CHILD_PROP_COL,
  CHILD_PROP_ROW
};

#define NBTK_TYPE_TABLE_CHILD          (nbtk_table_child_get_type ())
#define NBTK_TABLE_CHILD(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChild))
#define NBTK_IS_TABLE_CHILD(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))
#define NBTK_IS_TABLE_CHILD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TABLE_CHILD))
#define NBTK_TABLE_CHILD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TABLE_CHILD, NbtkTableChildClass))

typedef struct _NbtkTableChild         NbtkTableChild;
typedef struct _NbtkTableChildClass    NbtkTableChildClass;

struct _NbtkTableChild
{
  ClutterChildMeta parent_instance;

  gint col;
  gint row;
};

struct _NbtkTableChildClass
{
  ClutterChildMetaClass parent_class;
};

GType nbtk_table_child_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (NbtkTableChild, nbtk_table_child, CLUTTER_TYPE_CHILD_META);

static void
table_child_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkTableChild *child = NBTK_TABLE_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_COL:
      child->col = g_value_get_int (value);
      break;

    case CHILD_PROP_ROW:
      child->row = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
table_child_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkTableChild *child = NBTK_TABLE_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_COL:
      g_value_set_int (value, child->col);
      break;

    case CHILD_PROP_ROW:
      g_value_set_int (value, child->row);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}


static void
nbtk_table_child_class_init (NbtkTableChildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->set_property = table_child_set_property;
  gobject_class->get_property = table_child_get_property;

  pspec = g_param_spec_int ("column",
                            "Column Number",
                            "The column the widget resides in",
                            0, G_MAXINT,
                            0,
                            G_PARAM_READABLE | G_PARAM_WRITABLE
                            | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                            | G_PARAM_STATIC_BLURB);

  g_object_class_install_property (gobject_class, CHILD_PROP_COL, pspec);

  pspec = g_param_spec_int ("row",
                            "Row Number",
                            "The row the widget resides in",
                            0, G_MAXINT,
                            0,
                            G_PARAM_READABLE | G_PARAM_WRITABLE
                            | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                            | G_PARAM_STATIC_BLURB);

  g_object_class_install_property (gobject_class, CHILD_PROP_ROW, pspec);
}

static void
nbtk_table_child_init (NbtkTableChild *klass)
{
}

/* 
 * ClutterContainer Implementation 
 */
static void
nbtk_container_add_actor (ClutterContainer *container,
                          ClutterActor     *actor)
{
  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
}

static void
nbtk_container_remove_actor (ClutterContainer *container,
                             ClutterActor     *actor)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (container);

  GSList *item = NULL;

  item = g_slist_find (priv->children, actor);

  if (item == NULL)
    {
      g_warning ("Widget of type '%s' is not a child of container of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  g_object_ref (actor);

  priv->children = g_slist_delete_link (priv->children, item);

  clutter_actor_unparent (actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_object_unref (actor);
}

static void
nbtk_container_foreach (ClutterContainer *container,
                        ClutterCallback   callback,
                        gpointer          callback_data)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (container);

  g_slist_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
nbtk_container_lower (ClutterContainer *container,
                      ClutterActor     *actor,
                      ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_container_raise (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_container_sort_depth_order (ClutterContainer *container)
{
  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

static void
nbtk_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = nbtk_container_add_actor;
  iface->remove = nbtk_container_remove_actor;
  iface->foreach = nbtk_container_foreach;
  iface->lower = nbtk_container_lower;
  iface->raise = nbtk_container_raise;
  iface->sort_depth_order = nbtk_container_sort_depth_order;
  iface->child_meta_type = NBTK_TYPE_TABLE_CHILD;
}


static void
nbtk_table_set_property (GObject       *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkTable *table = NBTK_TABLE (gobject);

  switch (prop_id)
    {
    case PROP_COL_SPACING:
      nbtk_table_set_col_spacing (table, g_value_get_int (value));
      break;

    case PROP_ROW_SPACING:
      nbtk_table_set_row_spacing (table, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_table_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkTablePrivate *priv = NBTK_TABLE (gobject)->priv;

  switch (prop_id)
    {
    case PROP_COL_SPACING:
      g_value_set_int (value, priv->col_spacing);
      break;

    case PROP_ROW_SPACING:
      g_value_set_int (value, priv->row_spacing);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_table_finalize (GObject *gobject)
{
  /* NbtkTablePrivate *priv = NBTK_TABLE (gobject)->priv; */

  G_OBJECT_CLASS (nbtk_table_parent_class)->finalize (gobject);
}

static void
nbtk_table_dispose (GObject *gobject)
{
  NbtkTablePrivate *priv = NBTK_TABLE (gobject)->priv;

  g_slist_foreach (priv->children, (GFunc) g_object_unref, NULL);
  g_slist_free (priv->children);
  priv->children = NULL;

  G_OBJECT_CLASS (nbtk_table_parent_class)->dispose (gobject);
}

static void
nbtk_table_allocate (ClutterActor          *self,
                     const ClutterActorBox *box,
                     gboolean               absolute_origin_changed)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (self);
  GSList *list;
  ClutterUnit col_width, row_height;

  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->allocate (self, box, absolute_origin_changed);

  col_width = (box->x2 - box->x1) / priv->n_cols;
  row_height = (box->y2 - box->y1) / priv->n_rows;

  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col;
      ClutterChildMeta *meta;
      ClutterActor *child;
      ClutterActorBox childbox;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);
      g_object_get (meta, "column", &col, "row", &row, NULL);


      childbox.x1 = (col_width * col);
      childbox.x2 = (childbox.x1 + col_width);

      childbox.y1 = (row_height * row);
      childbox.y2 = (childbox.y1 + row_height);

      clutter_actor_allocate (child, &childbox, absolute_origin_changed);
    }
}

static void
nbtk_table_paint (ClutterActor *self)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (self);
  GSList *list;

  for (list = priv->children; list; list = g_slist_next (list))
    {
      clutter_actor_paint (CLUTTER_ACTOR (list->data));
    }

}

static void
nbtk_table_pick (ClutterActor       *self,
                 const ClutterColor *color)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (self);
  GSList *list;

  for (list = priv->children; list; list = g_slist_next (list))
    {
      if (CLUTTER_ACTOR_IS_VISIBLE (list->data))
        clutter_actor_paint (CLUTTER_ACTOR (list->data));
    }

}

static void
nbtk_table_class_init (NbtkTableClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  /* NbtkWidgetClass *nbtk_widget_class = NBTK_WIDGET_CLASS (klass); */

  g_type_class_add_private (klass, sizeof (NbtkTablePrivate));

  gobject_class->set_property = nbtk_table_set_property;
  gobject_class->get_property = nbtk_table_get_property;
  gobject_class->dispose = nbtk_table_dispose;
  gobject_class->finalize = nbtk_table_finalize;

/*
  actor_class->table_press_event = nbtk_table_table_press;
  actor_class->table_release_event = nbtk_table_table_release;
  actor_class->enter_event = nbtk_table_enter;
  actor_class->leave_event = nbtk_table_leave;
*/
  actor_class->paint = nbtk_table_paint;
  actor_class->pick = nbtk_table_pick;
  actor_class->allocate = nbtk_table_allocate;
}

static void
nbtk_table_init (NbtkTable *table)
{
  table->priv = NBTK_TABLE_GET_PRIVATE (table);

  table->priv->n_cols = 0;
  table->priv->n_rows = 0;
}

NbtkWidget*
nbtk_table_new (void)
{
  return g_object_new (NBTK_TYPE_TABLE, NULL);
}

void
nbtk_table_set_col_spacing (NbtkTable *table,
                            gint       spacing)
{
  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (spacing <= 0);

  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

void
nbtk_table_set_row_spacing (NbtkTable *table,
                            gint       spacing)
{
  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (spacing <= 0);

  /* XXX: not yet implemented */
  g_warning ("%s() not yet implemented", __FUNCTION__);
}

void
nbtk_table_add_widget (NbtkTable *table,
                       NbtkWidget *widget,
                       gint row,
                       gint column)
{
  NbtkTablePrivate *priv;
  ClutterChildMeta *child;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (NBTK_IS_WIDGET (widget));
  g_return_if_fail (row >= 0);
  g_return_if_fail (column >= 0);

  priv = NBTK_TABLE_GET_PRIVATE (table);

  clutter_container_add_actor (CLUTTER_CONTAINER (table),
                               CLUTTER_ACTOR (widget));
  child = clutter_container_get_child_meta (CLUTTER_CONTAINER (table),
                                            CLUTTER_ACTOR (widget));
  g_object_set (child, "row", row, "column", column, NULL);

  priv->n_cols = MAX (priv->n_cols, column + 1);
  priv->n_rows = MAX (priv->n_rows, row + 1);

  priv->children = g_slist_prepend (priv->children, widget);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
}
