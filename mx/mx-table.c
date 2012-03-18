/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-table.c: Table layout widget
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

/**
 * SECTION:mx-table
 * @short_description: A multi-child layout container based on rows
 * and columns
 *
 * #MxTable is a mult-child layout container based on a table arrangement
 * with rows and columns. #MxTable adds several child properties to its
 * children which control their position and size in the table.
 *
 * While other layouts (like #MxGrid) allow you to achieve 
 * table-like effects, #MxTable is the only layout which allows 
 * you to precisely (and easily) place elements at particular grid coordinates,
 * via mx_table_add_actor().
 *
 * <figure id="mx-table">
 *   <title>#MxTable, 3 rows by 3 columns</title>
 *   <para>Notice how rectangles have only been placed in a few of 
 *   the cells inside the table: this would be very difficult to do 
 *   with any other layout, without having layouts within layouts within 
 *   layouts...</para>
 *   <graphic fileref="MxTable.png" format="PNG"/>
 * </figure>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-table.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "mx-enum-types.h"
#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-table-child.h"
#include "mx-stylable.h"
#include "mx-focusable.h"

enum
{
  PROP_0,

  PROP_PADDING,
  PROP_COLUMN_SPACING,
  PROP_ROW_SPACING,

  PROP_ROW_COUNT,
  PROP_COL_COUNT,
};

#define MX_TABLE_GET_PRIVATE(obj)    \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_TABLE, MxTablePrivate))

typedef struct
{
  guint expand : 1;
  guint shrink : 1;
  guint is_visible : 1;

  gfloat min_size;
  gfloat pref_size;
  gfloat final_size;

} DimensionData;

struct _MxTablePrivate
{
  GList *children;

  guint   ignore_css_col_spacing : 1;
  guint   ignore_css_row_spacing : 1;
  gint    col_spacing;
  gint    row_spacing;

  gint    visible_rows;
  gint    visible_cols;

  gint    n_rows;
  gint    n_cols;

  gint    active_row;
  gint    active_col;

  GArray *columns;
  GArray *rows;

  MxFocusable *last_focus;
};

static void mx_container_iface_init (ClutterContainerIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);
static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxTable, mx_table, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                mx_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init));


static ClutterActor*
mx_table_find_actor_at (MxTable *table,
                        int      row,
                        int      column)
{
  MxTablePrivate *priv;
  GList *l;

  priv = table->priv;

  for (l = priv->children; l; l = g_list_next (l))
    {
      MxTableChild *child;
      ClutterActor *actor_child = CLUTTER_ACTOR (l->data);

      child = (MxTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (table),
                                                                 actor_child);

      if ((row >= child->row && row <= child->row + (child->row_span - 1)) &&
          (column >= child->col && column <= child->col + (child->col_span - 1)))
        return actor_child;
    }

  return NULL;
}

static MxFocusable*
mx_table_move_focus (MxFocusable      *focusable,
                     MxFocusDirection  direction,
                     MxFocusable      *from)
{
  MxTablePrivate *priv = MX_TABLE (focusable)->priv;
  MxTable *table = MX_TABLE (focusable);
  GList *l, *childlink;
  MxTableChild *child_meta;
  ClutterActor *child_actor;
  MxFocusable *focused;
  gint row, column;
  ClutterActor *found;

  /* find the current focus */
  childlink = g_list_find (priv->children, from);

  if (!childlink)
    return NULL;

  priv->last_focus = from;

  child_actor = CLUTTER_ACTOR (childlink->data);
  child_meta = (MxTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (focusable),
                                                                  child_actor);

  /* find the next widget to focus */
  switch (direction)
    {
    case MX_FOCUS_DIRECTION_NEXT:

      for (l = childlink->next; l; l = g_list_next (l))
        {
          if (MX_IS_FOCUSABLE (l->data))
            {
              focused = mx_focusable_accept_focus (MX_FOCUSABLE (l->data),
                                                   MX_FOCUS_HINT_FIRST);

              if (focused)
                return focused;
            }
        }

      /* no next widgets to focus */
      return NULL;

    case MX_FOCUS_DIRECTION_PREVIOUS:

      for (l = g_list_previous (childlink); l; l = g_list_previous (l))
        {
          if (MX_IS_FOCUSABLE (l->data))
            {
              focused = mx_focusable_accept_focus (MX_FOCUSABLE (l->data),
                                                   MX_FOCUS_HINT_LAST);

              if (focused)
                return focused;
            }
        }

      /* no widget found in the previous position */
      return NULL;

    case MX_FOCUS_DIRECTION_UP:
      /* move focus up */

      row = child_meta->row - 1;
      column = child_meta->col;

      focused = NULL;

      while (!focused && row >= 0)
        {
          found = mx_table_find_actor_at (table, row, column);

          if (found)
            {
              if (MX_IS_FOCUSABLE (found))
                {
                  focused = mx_focusable_accept_focus (MX_FOCUSABLE (found),
                                                       MX_FOCUS_HINT_FIRST);
                  if (focused)
                    break;
                }

              child_meta = (MxTableChild *) clutter_container_get_child_meta
                (CLUTTER_CONTAINER (focusable), found);

              /* row might not be the top row if @found is a spanned actor */
              row = child_meta->row - 1;
            }
          else
            row --;
        }

      return focused;


    case MX_FOCUS_DIRECTION_DOWN:
      /* move focus down */

      row = child_meta->row + child_meta->row_span;
      column = child_meta->col;

      focused = NULL;

      while (!focused && row < priv->n_rows)
        {
          found = mx_table_find_actor_at (table, row, column);

          if (found)
            {
              if (MX_IS_FOCUSABLE (found))
                {
                  focused = mx_focusable_accept_focus (MX_FOCUSABLE (found),
                                                       MX_FOCUS_HINT_FIRST);
                  if (focused)
                    break;
                }

              child_meta = (MxTableChild *) clutter_container_get_child_meta
                (CLUTTER_CONTAINER (focusable), found);

              row = child_meta->row + child_meta->row_span;
            }
          else
            row ++;
        }

      return focused;


    case MX_FOCUS_DIRECTION_LEFT:
      /* move focus left */

      row = child_meta->row;
      column = child_meta->col - 1;

      focused = NULL;

      while (!focused && column >= 0)
        {
          found = mx_table_find_actor_at (table, row, column);

          if (found)
            {
              if (MX_IS_FOCUSABLE (found))
                {
                  focused = mx_focusable_accept_focus (MX_FOCUSABLE (found),
                                                       MX_FOCUS_HINT_FIRST);
                  if (focused)
                    break;
                }

              child_meta = (MxTableChild *) clutter_container_get_child_meta
                (CLUTTER_CONTAINER (focusable), found);

              /* col might not be the first column if @found is a spanned actor */
              column = child_meta->col - 1;
            }
          else
            column --;
        }

      return focused;


    case MX_FOCUS_DIRECTION_RIGHT:
      /* move focus right */

      row = child_meta->row;
      column = child_meta->col + child_meta->col_span;

      focused = NULL;

      while (!focused && column < priv->n_cols)
        {
          found = mx_table_find_actor_at (table, row, column);

          if (found)
            {
              if (MX_IS_FOCUSABLE (found))
                {
                  focused = mx_focusable_accept_focus (MX_FOCUSABLE (found),
                                                       MX_FOCUS_HINT_FIRST);
                  if (focused)
                    break;
                }

              child_meta = (MxTableChild *) clutter_container_get_child_meta
                (CLUTTER_CONTAINER (focusable), found);

              column = child_meta->col + child_meta->col_span;
            }
          else
            column ++;
        }

      return focused;

    default:
      break;
    }

  return NULL;
}

static MxFocusable*
mx_table_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxTablePrivate *priv = MX_TABLE (focusable)->priv;
  MxFocusable *return_focusable;
  GList* list, *l;

  return_focusable = NULL;

  /* find the first/last focusable widget */
  switch (hint)
    {
    case MX_FOCUS_HINT_LAST:
      list = g_list_reverse (g_list_copy (priv->children));
      break;

    case MX_FOCUS_HINT_PRIOR:
      if (priv->last_focus)
        {
          list = g_list_copy (g_list_find (priv->children, priv->last_focus));
          if (list)
            break;
        }
      /* This intentionally runs into the next switch case */

    default:
    case MX_FOCUS_HINT_FIRST:
      list = g_list_copy (priv->children);
      break;
    }

  for (l = list; l; l = g_list_next (l))
    {
      if (MX_IS_FOCUSABLE (l->data))
        {
          return_focusable = mx_focusable_accept_focus (MX_FOCUSABLE (l->data),
                                                        hint);
          if (return_focusable)
            break;
        }
    }

  g_list_free (list);

  return return_focusable;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->move_focus = mx_table_move_focus;
  iface->accept_focus = mx_table_accept_focus;
}

/*
 * ClutterContainer Implementation
 */
static void
mx_container_add_actor (ClutterContainer *container,
                        ClutterActor     *actor)
{
  MxTablePrivate *priv = MX_TABLE (container)->priv;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  priv->children = g_list_append (priv->children, actor);

  /* default position of the actor is 0, 0 */
  _mx_table_update_row_col (MX_TABLE (container), 0, 0);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
mx_container_remove_actor (ClutterContainer *container,
                           ClutterActor     *actor)
{
  MxTablePrivate *priv = MX_TABLE (container)->priv;
  gint rows, cols;

  GList *item = NULL;
  MxTableChild *meta;
  GList *l;

  item = g_list_find (priv->children, actor);

  if (item == NULL)
    {
      g_warning ("Widget of type '%s' is not a child of container of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  g_object_ref (actor);

  if ((ClutterActor *)priv->last_focus == actor)
    priv->last_focus = NULL;

  priv->children = g_list_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  /* update row/column count */
  rows = 0;
  cols = 0;
  for (l = priv->children; l; l = l->next)
    {
      ClutterActor *child = CLUTTER_ACTOR (l->data);
      meta = (MxTableChild *) clutter_container_get_child_meta (container,
                                                                  child);
      rows = MAX (rows, meta->row + meta->row_span);
      cols = MAX (cols, meta->col + meta->col_span);
    }
  priv->n_rows = rows;
  priv->n_cols = cols;

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);
}

static void
mx_container_foreach (ClutterContainer *container,
                      ClutterCallback   callback,
                      gpointer          callback_data)
{
  MxTablePrivate *priv = MX_TABLE (container)->priv;

  g_list_foreach (priv->children, (GFunc) callback, callback_data);
}

static void
mx_container_lower (ClutterContainer *container,
                    ClutterActor     *actor,
                    ClutterActor     *sibling)
{
  gint i;
  GList *c, *position, *actor_link = NULL;

  MxTablePrivate *priv = MX_TABLE (container)->priv;

  if (priv->children && (priv->children->data == actor))
    return;

  position = priv->children;
  for (c = priv->children, i = 0; c; c = c->next, i++)
    {
      if (c->data == actor)
        actor_link = c;
      if (c->data == sibling)
        position = c;
    }

  if (!actor_link)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  priv->children = g_list_delete_link (priv->children, actor_link);
  priv->children = g_list_insert_before (priv->children, position, actor);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
mx_container_raise (ClutterContainer *container,
                    ClutterActor     *actor,
                    ClutterActor     *sibling)
{
  gint i;
  GList *c, *actor_link = NULL;

  gint position = -1;
  MxTablePrivate *priv = MX_TABLE (container)->priv;

  for (c = priv->children, i = 0; c; c = c->next, i++)
    {
      if (c->data == actor)
        actor_link = c;
      if (c->data == sibling)
        position = i;
    }

  if (!actor_link)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (container)));
      return;
    }

  if (!actor_link->next)
    return;

  priv->children = g_list_delete_link (priv->children, actor_link);
  priv->children = g_list_insert (priv->children, actor, position);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static gint
mx_table_depth_sort_cb (gconstpointer a,
                        gconstpointer b)
{
  gfloat depth_a = clutter_actor_get_depth ((ClutterActor *)a);
  gfloat depth_b = clutter_actor_get_depth ((ClutterActor *)b);

  if (depth_a < depth_b)
    return -1;
  else if (depth_a > depth_b)
    return 1;
  else
    return 0;
}

static void
mx_container_sort_depth_order (ClutterContainer *container)
{
  MxTablePrivate *priv = MX_TABLE (container)->priv;

  priv->children = g_list_sort (priv->children, mx_table_depth_sort_cb);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
mx_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = mx_container_add_actor;
  iface->remove = mx_container_remove_actor;
  iface->foreach = mx_container_foreach;
  iface->lower = mx_container_lower;
  iface->raise = mx_container_raise;
  iface->sort_depth_order = mx_container_sort_depth_order;
  iface->child_meta_type = MX_TYPE_TABLE_CHILD;
}

/* MxStylable implementation */
static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (is_initialized == FALSE))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("x-mx-column-spacing",
                                 "Column Spacing",
                                 "The size of the column spacing",
                                 0, G_MAXUINT, 0,
                                 MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_TABLE, pspec);

      pspec = g_param_spec_uint ("x-mx-row-spacing",
                                 "Row Spacing",
                                 "The size of the row spacing",
                                 0, G_MAXUINT, 0,
                                 MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_TABLE, pspec);
    }
}

/* MxTable Class Implementation */

static void
mx_table_set_property (GObject      *gobject,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  MxTable *table = MX_TABLE (gobject);

  switch (prop_id)
    {
    case PROP_COLUMN_SPACING:
      mx_table_set_column_spacing (table, g_value_get_int (value));
      break;

    case PROP_ROW_SPACING:
      mx_table_set_row_spacing (table, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_table_get_property (GObject    *gobject,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  MxTablePrivate *priv = MX_TABLE (gobject)->priv;

  switch (prop_id)
    {
    case PROP_COLUMN_SPACING:
      g_value_set_int (value, priv->col_spacing);
      break;

    case PROP_ROW_SPACING:
      g_value_set_int (value, priv->row_spacing);
      break;

    case PROP_COL_COUNT:
      g_value_set_int (value, priv->n_cols);
      break;

    case PROP_ROW_COUNT:
      g_value_set_int (value, priv->n_rows);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_table_finalize (GObject *gobject)
{
  MxTablePrivate *priv = MX_TABLE (gobject)->priv;

  g_array_free (priv->columns, TRUE);
  g_array_free (priv->rows, TRUE);

  G_OBJECT_CLASS (mx_table_parent_class)->finalize (gobject);
}

static void
mx_table_dispose (GObject *gobject)
{
  MxTablePrivate *priv = MX_TABLE (gobject)->priv;

  /* destroy the children
   * clutter_actor_destroy() will call clutter_container_remove() which will
   * remove the children from the internal list
   */
  while (priv->children)
    clutter_actor_destroy (CLUTTER_ACTOR (priv->children->data));

  G_OBJECT_CLASS (mx_table_parent_class)->dispose (gobject);
}

#define CLAMP_TO_PIXEL(x) ((float)((int)(x)))

static void
mx_table_calculate_col_widths (MxTable *table,
                               gint     for_width)
{
  gint i;
  MxTablePrivate *priv = table->priv;
  DimensionData *columns;
  GList *l;
  MxPadding padding;

  g_array_set_size (priv->columns, 0);
  g_array_set_size (priv->columns, priv->n_cols);
  columns = &g_array_index (priv->columns, DimensionData, 0);


  /* take off the padding values to calculate the allocatable width */
  mx_widget_get_padding (MX_WIDGET (table), &padding);

  for_width -= (int)(padding.left + padding.right);

  /* Reset all the visible attributes for the columns */
  priv->visible_cols = 0;
  for (i = 0; i < priv->n_cols; i++)
    columns[i].is_visible = FALSE;

  /* STAGE ONE: calculate column widths for non-spanned children */
  for (l = priv->children; l; l = g_list_next (l))
    {
      MxTableChild *meta;
      ClutterActor *child;
      DimensionData *col;
      gfloat c_min, c_pref;

      child = CLUTTER_ACTOR (l->data);

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      meta = (MxTableChild *)
        clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

      if (meta->col_span > 1)
        continue;

      col = &columns[meta->col];

      /* If this child is visible, then its column is visible */
      if (!col->is_visible)
        {
          col->is_visible = TRUE;
          priv->visible_cols++;
        }

      clutter_actor_get_preferred_width (child, -1, &c_min, &c_pref);

      col->min_size = MAX (col->min_size, c_min);
      col->final_size = col->pref_size = MAX (col->pref_size, c_pref);
      col->expand = MAX (col->expand, meta->x_expand);
    }

  /* STAGE TWO: take spanning children into account */
  for (l = priv->children; l; l = g_list_next (l))
    {
      MxTableChild *meta;
      ClutterActor *child;
      DimensionData *col;
      gfloat c_min, c_pref;
      gfloat min_width, pref_width;
      gint start_col, end_col;
      gint n_expand;

      child = CLUTTER_ACTOR (l->data);

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      meta = (MxTableChild *)
        clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

      if (meta->col_span < 2)
        continue;

      col = &columns[meta->col];
      start_col = meta->col;
      end_col = meta->col + meta->col_span - 1;

      clutter_actor_get_preferred_width (child, -1, &c_min, &c_pref);


      /* check there is enough room for this actor */
      min_width = 0;
      pref_width = 0;
      n_expand = 0;
      for (i = start_col; i <= end_col; i++)
        {
          min_width += columns[i].min_size;
          pref_width += columns[i].pref_size;

          if (columns[i].expand)
            {
              n_expand++;
            }

          /* If this child is visible, then the columns it spans
             are also visible */
          if (!col->is_visible)
            {
              col->is_visible = TRUE;
              priv->visible_cols++;
            }
        }
      min_width += priv->col_spacing * (meta->col_span - 1);
      pref_width += priv->col_spacing * (meta->col_span - 1);


      /* see mx_table_calculate_row_heights() for comments */
      /* (1) */
      if (c_min > min_width)
        {

          /* (2) */
          /* we can start from preferred width and decrease */
          if (pref_width > c_min)
            {
              for (i = start_col; i <= end_col; i++)
                {
                  columns[i].final_size = columns[i].pref_size;
                }

              while (pref_width > c_min)
                {
                  for (i = start_col; i <= end_col; i++)
                    {
                      if (columns[i].final_size > columns[i].min_size)
                        {
                          columns[i].final_size--;
                          pref_width--;
                        }
                    }
                }
              for (i = start_col; i <= end_col; i++)
                {
                  columns[i].min_size = columns[i].final_size;
                }

            }
          else
            {
              /* (3) */
              /* we can expand from preferred size */
              gfloat expand_by;

              expand_by = c_pref - pref_width;

              for (i = start_col; i <= end_col; i++)
                {
                  if (n_expand)
                    {
                      if (columns[i].expand)
                        columns[i].min_size =
                          columns[i].pref_size + expand_by / n_expand;
                    }
                  else
                    {
                      columns[i].min_size =
                        columns[i].pref_size + expand_by / meta->col_span;
                    }

                }
            }
        }


    }


  /* calculate final widths */
  if (for_width >= 0)
    {
      gfloat min_width, pref_width;
      gint n_expand;

      min_width = 0;
      pref_width = 0;
      n_expand = 0;
      for (i = 0; i < priv->n_cols; i++)
        {
          pref_width += columns[i].pref_size;
          min_width += columns[i].min_size;
          if (columns[i].expand)
            n_expand++;
        }
      pref_width += priv->col_spacing * (priv->n_cols - 1);
      min_width += priv->col_spacing * (priv->n_cols - 1);

      if (for_width <= min_width)
        {
          /* erk, we can't shrink this! */
          for (i = 0; i < priv->n_cols; i++)
            {
              columns[i].final_size = columns[i].min_size;
            }
          return;
        }

      if (for_width == pref_width)
        {
          /* perfect! */
          for (i = 0; i < priv->n_cols; i++)
            {
              columns[i].final_size = columns[i].pref_size;
            }
          return;
        }

      /* for_width is between min_width and pref_width */
      if (for_width < pref_width && for_width > min_width)
        {
          gfloat width;

          /* shrink columns until they reach min_width */

          /* start with all columns at preferred size */
          for (i = 0; i < priv->n_cols; i++)
            {
              columns[i].final_size = columns[i].pref_size;
            }
          width = pref_width;

          while (width > for_width)
            {
              for (i = 0; i < priv->n_cols; i++)
                {
                  if (columns[i].final_size > columns[i].min_size)
                    {
                      columns[i].final_size--;
                      width--;
                    }
                }
            }

          return;
        }

      /* expand columns */
      if (for_width > pref_width)
        {
          gfloat extra_width = for_width - pref_width;
          gint remaining;

          if (n_expand)
            remaining = (gint) extra_width % n_expand;
          else
            remaining = (gint) extra_width % priv->n_cols;

          for (i = 0; i < priv->n_cols; i++)
            {
              if (columns[i].expand)
                {
                  if (n_expand)
                    {
                      columns[i].final_size =
                        columns[i].pref_size + (extra_width / n_expand);
                    }
                  else
                    {
                      columns[i].final_size =
                        columns[i].pref_size + (extra_width / priv->n_cols);
                    }
                }
              else
                columns[i].final_size = columns[i].pref_size;
            }

          /* distribute the remainder among children */
          i = 0;
          while (remaining)
            {
              columns[i].final_size++;
              i++;
              remaining--;
            }
        }
    }

}

static void
mx_table_calculate_row_heights (MxTable *table,
                                gint     for_height)
{
  MxTablePrivate *priv = MX_TABLE (table)->priv;
  GList *l;
  gint i;
  DimensionData *rows, *columns;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (table), &padding);

  /* take padding off available height */
  for_height -= (int)(padding.top + padding.bottom);

  g_array_set_size (priv->rows, 0);
  g_array_set_size (priv->rows, priv->n_rows);
  rows = &g_array_index (priv->rows, DimensionData, 0);

  columns = &g_array_index (priv->columns, DimensionData, 0);

  /* Reset the visible rows */
  priv->visible_rows = 0;
  for (i = 0; i < priv->n_rows; i++)
    rows[i].is_visible = FALSE;

  /* STAGE ONE: calculate row heights for non-spanned children */
  for (l = priv->children; l; l = g_list_next (l))
    {
      MxTableChild *meta;
      ClutterActor *child;
      DimensionData *row;
      gfloat c_min, c_pref;

      child = CLUTTER_ACTOR (l->data);

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      meta = (MxTableChild *)
        clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

      if (meta->row_span > 1)
        continue;

      row = &rows[meta->row];

      /* If this child is visible, then its row is visible */
      if (!row->is_visible)
        {
          row->is_visible = TRUE;
          priv->visible_rows++;
        }

      clutter_actor_get_preferred_height (child, columns[meta->col].final_size,
                                          &c_min, &c_pref);

      row->min_size = MAX (row->min_size, c_min);
      row->final_size = row->pref_size = MAX (row->pref_size, c_pref);
      row->expand = MAX (row->expand, meta->y_expand);
    }



  /* STAGE TWO: take spanning children into account */
  for (l = priv->children; l; l = g_list_next (l))
    {
      MxTableChild *meta;
      ClutterActor *child;
      gfloat c_min, c_pref;
      gfloat min_height, pref_height;
      gint start_row, end_row;
      gint n_expand;

      child = CLUTTER_ACTOR (l->data);

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      meta = (MxTableChild *)
        clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

      if (meta->row_span < 2)
        continue;

      start_row = meta->row;
      end_row = meta->row + meta->row_span - 1;

      clutter_actor_get_preferred_height (child, columns[meta->col].final_size,
                                         &c_min, &c_pref);


      /* check there is enough room for this actor */
      min_height = 0;
      pref_height = 0;
      n_expand = 0;
      for (i = start_row; i <= end_row; i++)
        {
          min_height += rows[i].min_size;
          pref_height += rows[i].pref_size;

          if (rows[i].expand)
            {
              n_expand++;
            }

          /* If this actor is visible, then all the rows is spans are visible */
          if (!rows[i].is_visible)
            {
              rows[i].is_visible = TRUE;
              priv->visible_rows++;
            }
        }
      min_height += priv->row_spacing * (meta->row_span - 1);
      pref_height += priv->row_spacing * (meta->row_span - 1);

      /* 1) If the minimum height of the rows spanned is less than the minimum
       * height of the child that is spanning them, then we must increase the
       * minimum height of the rows spanned.
       *
       * 2) If the preferred height of the spanned rows is more that the minimum
       * height of the spanning child, then we can start at this size and
       * decrease each row evenly.
       *
       * 3) If the preferred height of the rows is more than the minimum height
       * of the spanned child, then we can start at the preferred height and
       * expand.
       */
      /* (1) */
      if (c_min > min_height)
        {

          /* (2) */
          /* we can start from preferred height and decrease */
          if (pref_height > c_min)
            {
              for (i = start_row; i <= end_row; i++)
                {
                  rows[i].final_size = rows[i].pref_size;
                }

              while (pref_height > c_min)
                {
                  for (i = start_row; i <= end_row; i++)
                    {
                      if (rows[i].final_size > rows[i].min_size)
                        {
                          rows[i].final_size--;
                          pref_height--;
                        }
                    }
                }
              for (i = start_row; i <= end_row; i++)
                {
                  rows[i].min_size = rows[i].final_size;
                }

            }
          else
            {
              /* (3) */
              /* we can expand from preferred size */
              gfloat expand_by;

              expand_by = c_pref - pref_height;

              for (i = start_row; i <= end_row; i++)
                {
                  if (n_expand)
                    {
                      if (rows[i].expand)
                        rows[i].min_size =
                          rows[i].pref_size + expand_by / n_expand;
                    }
                  else
                    {
                      rows[i].min_size =
                        rows[i].pref_size + expand_by / meta->row_span;
                    }

                }
            }
        }

    }


  /* calculate final heights */
  if (for_height >= 0)
    {
      gfloat min_height, pref_height;
      gint n_expand;

      min_height = 0;
      pref_height = 0;
      n_expand = 0;
      for (i = 0; i < priv->n_rows; i++)
        {
          pref_height += rows[i].pref_size;
          min_height += rows[i].min_size;
          if (rows[i].expand)
            n_expand++;
        }
      pref_height += priv->row_spacing * (priv->n_rows - 1);
      min_height += priv->row_spacing * (priv->n_rows - 1);

      if (for_height <= min_height)
        {
          /* erk, we can't shrink this! */
          for (i = 0; i < priv->n_rows; i++)
            {
              rows[i].final_size = rows[i].min_size;
            }
          return;
        }

      if (for_height == pref_height)
        {
          /* perfect! */
          for (i = 0; i < priv->n_rows; i++)
            {
              rows[i].final_size = rows[i].pref_size;
            }
          return;
        }

      /* for_height is between min_height and pref_height */
      if (for_height < pref_height && for_height > min_height)
        {
          gfloat height;

          /* shrink rows until they reach min_height */

          /* start with all rows at preferred size */
          for (i = 0; i < priv->n_rows; i++)
            {
              rows[i].final_size = rows[i].pref_size;
            }
          height = pref_height;

          while (height > for_height)
            {
              for (i = 0; i < priv->n_rows; i++)
                {
                  if (rows[i].final_size > rows[i].min_size)
                    {
                      rows[i].final_size--;
                      height--;
                    }
                }
            }

          return;
        }

      /* expand rows */
      if (for_height > pref_height)
        {
          gfloat extra_height = for_height - pref_height;
          gint remaining;

          if (n_expand)
            remaining = (gint) extra_height % n_expand;
          else
            remaining = (gint) extra_height % priv->n_rows;

          for (i = 0; i < priv->n_rows; i++)
            {
              if (rows[i].expand)
                {
                  if (n_expand)
                    {
                      rows[i].final_size =
                        rows[i].pref_size + (extra_height / n_expand);
                    }
                  else
                    {
                      rows[i].final_size =
                        rows[i].pref_size + (extra_height / priv->n_rows);
                    }
                }
              else
                rows[i].final_size = rows[i].pref_size;
            }

          /* distribute the remainder among children */
          i = 0;
          while (remaining)
            {
              rows[i].final_size++;
              i++;
              remaining--;
            }
        }
    }

}

static void
mx_table_calculate_dimensions (MxTable *table,
                               gfloat for_width,
                               gfloat for_height)
{
  mx_table_calculate_col_widths (table, for_width);
  mx_table_calculate_row_heights (table, for_height);
}

static void
mx_table_preferred_allocate (ClutterActor          *self,
                             const ClutterActorBox *box,
                             gboolean               flags)
{
  GList *list;
  gint row_spacing, col_spacing;
  gint i;
  MxTable *table;
  MxTablePrivate *priv;
  MxPadding padding;
  DimensionData *rows, *columns;

  table = MX_TABLE (self);
  priv = MX_TABLE (self)->priv;

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  col_spacing = (priv->col_spacing);
  row_spacing = (priv->row_spacing);

  mx_table_calculate_dimensions (table, box->x2 - box->x1, box->y2 - box->y1);

  rows = &g_array_index (priv->rows, DimensionData, 0);
  columns = &g_array_index (priv->columns, DimensionData, 0);

  for (list = priv->children; list; list = g_list_next (list))
    {
      gint row, col, row_span, col_span;
      gint col_width, row_height;
      MxTableChild *meta;
      ClutterActor *child;
      ClutterActorBox childbox;
      gint child_x, child_y;
      gdouble x_align_d, y_align_d;
      gboolean x_fill, y_fill;
      MxAlign x_align, y_align;

      child = CLUTTER_ACTOR (list->data);

      meta = (MxTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      if (!CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      /* get child properties */
      col = meta->col;
      row = meta->row;
      row_span = meta->row_span;
      col_span = meta->col_span;
      x_align_d = meta->x_align;
      y_align_d = meta->y_align;
      x_fill = meta->x_fill;
      y_fill = meta->y_fill;

      /* Convert to MxAlign */
      if (x_align_d < 1.0 / 3.0)
        x_align = MX_ALIGN_START;
      else if (x_align_d > 2.0 / 3.0)
        x_align = MX_ALIGN_END;
      else
        x_align = MX_ALIGN_MIDDLE;
      if (y_align_d < 1.0 / 3.0)
        y_align = MX_ALIGN_START;
      else if (y_align_d > 2.0 / 3.0)
        y_align = MX_ALIGN_END;
      else
        y_align = MX_ALIGN_MIDDLE;

      /* initialise the width and height */
      col_width = columns[col].final_size;
      row_height = rows[row].final_size;

      /* Add the widths of the spanned columns:
       *
       * First check that we have a non-zero span. Then we loop over each of
       * the columns that we're spanning but we stop short if we go past the
       * number of columns in the table. This is necessary to avoid accessing
       * uninitialised memory. We add the spacing in here too since we only
       * want to add as much spacing as times we successfully span.
       */
      if (col + col_span > priv->n_cols)
        g_warning ("MxTable: col-span exceeds number of columns");
      if (row + row_span > priv->n_rows)
        g_warning ("MxTable: row-span exceeds number of rows");

      if (col_span > 1)
        {
          for (i = col + 1; i < col + col_span && i < priv->n_cols; i++)
            {
              col_width += columns[i].final_size;
              col_width += col_spacing;
            }
        }

      /* add the height of the spanned rows */
      if (row_span > 1)
        {
          for (i = row + 1; i < row + row_span && i < priv->n_rows; i++)
            {
              row_height += rows[i].final_size;
              row_height += row_spacing;
            }
        }

      /* calculate child x */
      child_x = (int) padding.left;
      for (i = 0; i < col; i++)
        {
          if (columns[i].is_visible)
            {
              child_x += columns[i].final_size;
              child_x += col_spacing;
            }
        }

      /* calculate child y */
      child_y = (int) padding.top;
      for (i = 0; i < row; i++)
        {
          if (rows[i].is_visible)
            {
              child_y += rows[i].final_size;
              child_y += row_spacing;
            }
        }

      /* set up childbox */
      childbox.x1 = (float) child_x;
      childbox.x2 = (float) MAX (0, child_x + col_width);

      childbox.y1 = (float) child_y;
      childbox.y2 = (float) MAX (0, child_y + row_height);


      mx_allocate_align_fill (child, &childbox, x_align, y_align, x_fill, y_fill);

      clutter_actor_allocate (child, &childbox, flags);
    }
}

static void
mx_table_allocate (ClutterActor          *self,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  MxTablePrivate *priv = MX_TABLE (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_table_parent_class)->allocate (self, box, flags);

  if (priv->n_cols < 1 || priv->n_rows < 1)
    {
      return;
    };

  mx_table_preferred_allocate (self, box, flags);
}

static void
mx_table_get_preferred_width (ClutterActor *self,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  gfloat total_min_width, total_pref_width;
  MxTablePrivate *priv = MX_TABLE (self)->priv;
  gint i;
  MxPadding padding;
  DimensionData *columns;

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  if (priv->n_cols < 1)
    {
      *min_width_p = 0;
      *natural_width_p = 0;
      return;
    }

  mx_table_calculate_dimensions (MX_TABLE (self), -1, for_height);

  columns = &g_array_index (priv->columns, DimensionData, 0);

  total_min_width = padding.left + padding.right
                    + (priv->visible_cols - 1) * (float) priv->col_spacing;
  total_pref_width = total_min_width;

  for (i = 0; i < priv->n_cols; i++)
    {
      total_min_width += columns[i].min_size;
      total_pref_width += columns[i].pref_size;
    }

  if (min_width_p)
    *min_width_p = total_min_width;
  if (natural_width_p)
    *natural_width_p = total_pref_width;
}

static void
mx_table_get_preferred_height (ClutterActor *self,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  MxTablePrivate *priv = MX_TABLE (self)->priv;
  gint i, total_min_height, total_pref_height;
  MxPadding padding;
  DimensionData *rows;

  if (priv->n_rows < 1)
    {
      *min_height_p = 0;
      *natural_height_p = 0;
      return;
    }

  /* use min_widths to help allocation of height-for-width widgets */
  mx_table_calculate_dimensions (MX_TABLE (self), for_width, -1);

  rows = &g_array_index (priv->rows, DimensionData, 0);

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  /* start off with padding plus row spacing */
  total_min_height = padding.top + padding.bottom + (priv->visible_rows - 1) *
                     (float)(priv->row_spacing);

  total_pref_height = total_min_height;

  for (i = 0; i < priv->n_rows; i++)
    {
      total_min_height += rows[i].min_size;
      total_pref_height += rows[i].pref_size;
    }

  if (min_height_p)
    *min_height_p = total_min_height;
  if (natural_height_p)
    *natural_height_p = total_pref_height;
}

static void
mx_table_paint (ClutterActor *self)
{
  MxTablePrivate *priv = MX_TABLE (self)->priv;
  GList *list;

  /* make sure the background gets painted first */
  CLUTTER_ACTOR_CLASS (mx_table_parent_class)->paint (self);

  for (list = priv->children; list; list = g_list_next (list))
    {
      ClutterActor *child = CLUTTER_ACTOR (list->data);
      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }

  if (_mx_debug (MX_DEBUG_LAYOUT))
    {
      int i;
      float width, height;
      gfloat pos = 0;
      DimensionData *rows, *cols;

      rows = &g_array_index (priv->rows, DimensionData, 0);
      cols = &g_array_index (priv->columns, DimensionData, 0);

      clutter_actor_get_size (self, &width, &height);

      cogl_set_source_color4f (0.0, 0.0, 1.0, 0.7);

      for (i = 0; i < priv->n_rows; i++)
        {
          cogl_rectangle (0, pos, 10, pos + rows[i].final_size);

          pos += rows[i].final_size + priv->row_spacing;
        }


      cogl_set_source_color4f (1.0, 0.0, 0.0, 0.7);

      pos = 0;
      for (i = 0; i < priv->n_rows; i++)
        {
          cogl_rectangle (pos, 0, pos + cols[i].final_size, 10);

          pos += cols[i].final_size + priv->col_spacing;
        }


    }
}

static void
mx_table_pick (ClutterActor       *self,
               const ClutterColor *color)
{
  MxTablePrivate *priv = MX_TABLE (self)->priv;
  GList *list;

  /* Chain up so we get a bounding box painted (if we are reactive) */
  CLUTTER_ACTOR_CLASS (mx_table_parent_class)->pick (self, color);

  for (list = priv->children; list; list = g_list_next (list))
    {
      if (CLUTTER_ACTOR_IS_VISIBLE (list->data))
        clutter_actor_paint (CLUTTER_ACTOR (list->data));
    }
}

static void
mx_table_show_all (ClutterActor *table)
{
  MxTablePrivate *priv = MX_TABLE (table)->priv;
  GList *l;

  for (l = priv->children; l; l = l->next)
    clutter_actor_show_all (CLUTTER_ACTOR (l->data));

  clutter_actor_show (table);
}

static void
mx_table_hide_all (ClutterActor *table)
{
  MxTablePrivate *priv = MX_TABLE (table)->priv;
  GList *l;

  clutter_actor_hide (table);

  for (l = priv->children; l; l = l->next)
    clutter_actor_hide_all (CLUTTER_ACTOR (l->data));
}

static void
mx_table_class_init (MxTableClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  /* MxWidgetClass *mx_widget_class = MX_WIDGET_CLASS (klass); */

  g_type_class_add_private (klass, sizeof (MxTablePrivate));

  gobject_class->set_property = mx_table_set_property;
  gobject_class->get_property = mx_table_get_property;
  gobject_class->dispose = mx_table_dispose;
  gobject_class->finalize = mx_table_finalize;

  actor_class->paint = mx_table_paint;
  actor_class->pick = mx_table_pick;
  actor_class->allocate = mx_table_allocate;
  actor_class->get_preferred_width = mx_table_get_preferred_width;
  actor_class->get_preferred_height = mx_table_get_preferred_height;
  actor_class->show_all = mx_table_show_all;
  actor_class->hide_all = mx_table_hide_all;


  pspec = g_param_spec_int ("column-spacing",
                            "Column Spacing",
                            "Spacing between columns",
                            0, G_MAXINT, 0,
                            MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_COLUMN_SPACING,
                                   pspec);

  pspec = g_param_spec_int ("row-spacing",
                            "Row Spacing",
                            "Spacing between row",
                            0, G_MAXINT, 0,
                            MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_ROW_SPACING,
                                   pspec);

  pspec = g_param_spec_int ("row-count",
                            "Row Count",
                            "The number of rows in the table",
                            0, G_MAXINT, 0,
                            MX_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_ROW_COUNT,
                                   pspec);

  pspec = g_param_spec_int ("column-count",
                            "Column Count",
                            "The number of columns in the table",
                            0, G_MAXINT, 0,
                            MX_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_COL_COUNT,
                                   pspec);
}

static void
mx_table_style_changed (MxWidget *widget,
                        gpointer  userdata)
{
  MxTable *table = MX_TABLE (widget);
  MxTablePrivate *priv = table->priv;
  guint row_spacing, col_spacing;

  mx_stylable_get (MX_STYLABLE (widget),
                   "x-mx-column-spacing", &col_spacing,
                   "x-mx-row-spacing", &row_spacing,
                   NULL);

  if (!priv->ignore_css_col_spacing)
    priv->col_spacing = col_spacing;

  if (!priv->ignore_css_row_spacing)
    priv->row_spacing = row_spacing;
}

static void
mx_table_init (MxTable *table)
{
  table->priv = MX_TABLE_GET_PRIVATE (table);

  table->priv->n_cols = 0;
  table->priv->n_rows = 0;

  table->priv->columns = g_array_new (FALSE, TRUE, sizeof (DimensionData));
  table->priv->rows = g_array_new (FALSE, TRUE, sizeof (DimensionData));

  g_signal_connect (table, "style-changed",
                    G_CALLBACK (mx_table_style_changed), NULL);
}

/* used by MxTableChild to update row/column count */
void _mx_table_update_row_col (MxTable *table,
                               gint     row,
                               gint     col)
{
  if (col > -1)
    table->priv->n_cols = MAX (table->priv->n_cols, col + 1);

  if (row > -1)
    table->priv->n_rows = MAX (table->priv->n_rows, row + 1);

}

/*** Public Functions ***/

/**
 * mx_table_new:
 *
 * Create a new #MxTable
 *
 * Returns: a new #MxTable
 */
ClutterActor *
mx_table_new (void)
{
  return g_object_new (MX_TYPE_TABLE, NULL);
}

/**
 * mx_table_set_column_spacing
 * @table: a #MxTable
 * @spacing: spacing in pixels
 *
 * Sets the amount of spacing between columns.
 */
void
mx_table_set_column_spacing (MxTable *table,
                             gint     spacing)
{
  MxTablePrivate *priv;

  g_return_if_fail (MX_IS_TABLE (table));
  g_return_if_fail (spacing >= 0);

  priv = MX_TABLE (table)->priv;

  if (priv->col_spacing != spacing)
    {
      priv->col_spacing = spacing;

      priv->ignore_css_col_spacing = TRUE;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));

      g_object_notify (G_OBJECT (table), "column-spacing");
    }
}

/**
 * mx_table_set_row_spacing
 * @table: a #MxTable
 * @spacing: spacing in pixels
 *
 * Sets the amount of spacing between rows.
 */
void
mx_table_set_row_spacing (MxTable *table,
                          gint     spacing)
{
  MxTablePrivate *priv;

  g_return_if_fail (MX_IS_TABLE (table));
  g_return_if_fail (spacing >= 0);

  priv = MX_TABLE (table)->priv;

  if (priv->row_spacing != spacing)
    {
      priv->row_spacing = spacing;

      priv->ignore_css_row_spacing = TRUE;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));

      g_object_notify (G_OBJECT (table), "row-spacing");
    }
}

/**
 * mx_table_get_row_spacing
 * @table: a #MxTable
 *
 * Gets the amount of spacing between rows.
 *
 * Returns: the spacing between rows in device units
 */
gint
mx_table_get_row_spacing (MxTable *table)
{
  MxTablePrivate *priv;

  g_return_val_if_fail (MX_IS_TABLE (table), -1);
  priv = MX_TABLE (table)->priv;

  return priv->row_spacing;
}

/**
 * mx_table_get_column_spacing
 * @table: a #MxTable
 *
 * Gets the amount of spacing between columns.
 *
 * Returns: the spacing between columns in device units
 */
gint
mx_table_get_column_spacing (MxTable *table)
{
  MxTablePrivate *priv;

  g_return_val_if_fail (MX_IS_TABLE (table), -1);
  priv = MX_TABLE (table)->priv;

  return priv->col_spacing;
}

/**
 * mx_table_add_actor:
 * @table: a #MxTable
 * @actor: the child to insert
 * @row: the row to place the child into
 * @column: the column to place the child into
 *
 * Add an actor at the specified row and column
 *
 * Note, column and rows numbers start from zero
 */
void
mx_table_add_actor (MxTable      *table,
                    ClutterActor *actor,
                    gint          row,
                    gint          column)
{
  MxTableChild *meta;
  ClutterContainer *container;

  g_return_if_fail (MX_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));
  g_return_if_fail (row >= -1);
  g_return_if_fail (column >= -1);

  if (row < 0)
    row = table->priv->n_rows + 1;

  if (column < 0)
    column = table->priv->n_cols + 1;

  container = CLUTTER_CONTAINER (table);
  clutter_container_add_actor (container, actor);

  meta = (MxTableChild *) clutter_container_get_child_meta (container, actor);
  meta->row = row;
  meta->col = column;
  _mx_table_update_row_col (table, row, column);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
}

/**
 * mx_table_add_actor_with_properties
 * @table: a #MxTable
 * @actor: the child #ClutterActor
 * @row: the row to place the child into
 * @column: the column to place the child into
 * @first_property_name: name of the first property to set
 * @...: value for the first property, followed optionally by more name/value pairs terminated with NULL.
 *
 * Add an actor into at the specified row and column, with additional child
 * properties to set.
 */
void
mx_table_add_actor_with_properties (MxTable      *table,
                                    ClutterActor *actor,
                                    gint          row,
                                    gint          column,
                                    const gchar  *first_property_name,
                                    ...)
{
  va_list args;
  MxTableChild *meta;
  ClutterContainer *container;

  g_return_if_fail (MX_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));
  g_return_if_fail (row >= -1);
  g_return_if_fail (column >= -1);
  g_return_if_fail (first_property_name != NULL);

  if (row < 0)
    row = table->priv->n_rows + 1;

  if (column < 0)
    column = table->priv->n_cols + 1;

  container = (ClutterContainer *) table;
  clutter_container_add_actor (container, actor);

  meta = (MxTableChild *) clutter_container_get_child_meta (container, actor);
  meta->row = row;
  meta->col = column;
  _mx_table_update_row_col (table, row, column);

  va_start (args, first_property_name);
  g_object_set_valist ((GObject*) meta, first_property_name, args);
  va_end (args);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
}

/**
 * mx_table_get_row_count:
 * @table: A #MxTable
 *
 * Retrieve the current number rows in the @table
 *
 * Returns: the number of rows
 */
gint
mx_table_get_row_count (MxTable *table)
{
  g_return_val_if_fail (MX_IS_TABLE (table), -1);

  return MX_TABLE (table)->priv->n_rows;
}

/**
 * mx_table_get_column_count:
 * @table: A #MxTable
 *
 * Retrieve the current number of columns in @table
 *
 * Returns: the number of columns
 */
gint
mx_table_get_column_count (MxTable *table)
{
  g_return_val_if_fail (MX_IS_TABLE (table), -1);

  return MX_TABLE (table)->priv->n_cols;
}
