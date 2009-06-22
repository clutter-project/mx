/*
 * nbtk-table-child.h: Table child implementation
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
 * Written by: Thomas Wood  <thomas.wood@intel.com>
 *
 */

#include "nbtk-table-child.h"
#include "nbtk-private.h"
#include <nbtk/nbtk-widget.h>
#include <nbtk/nbtk-table.h>

/*
 * ClutterChildMeta Implementation
 */

/**
 * SECTION:nbtk-table-child
 * @short_description: The child property store for #NbtkTable
 *
 * The #ClutterChildMeta implementation for the #NbtkTable container widget.
 *
 */

enum {
  CHILD_PROP_0,

  CHILD_PROP_COL,
  CHILD_PROP_COL_OLD,
  CHILD_PROP_ROW,
  CHILD_PROP_COL_SPAN,
  CHILD_PROP_ROW_SPAN,
  CHILD_PROP_X_EXPAND,
  CHILD_PROP_Y_EXPAND,
  CHILD_PROP_X_ALIGN,
  CHILD_PROP_Y_ALIGN,
  CHILD_PROP_X_FILL,
  CHILD_PROP_Y_FILL,
  CHILD_PROP_ALLOCATE_HIDDEN,
};

G_DEFINE_TYPE (NbtkTableChild, nbtk_table_child, NBTK_TYPE_WIDGET_CHILD);

static void
table_child_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkTableChild *child = NBTK_TABLE_CHILD (gobject);
  NbtkTable *table = NBTK_TABLE (CLUTTER_CHILD_META(gobject)->container);

  switch (prop_id)
    {
    case CHILD_PROP_COL_OLD:
      g_warning ("The \"column\" property of NbtkTableChild is deprecated."
                 " Please use \"col\" instead.");
    case CHILD_PROP_COL:
      child->col = g_value_get_int (value);
      _nbtk_table_update_row_col (table, -1, child->col);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_ROW:
      child->row = g_value_get_int (value);
      _nbtk_table_update_row_col (table, child->row, -1);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_COL_SPAN:
      child->col_span = g_value_get_int (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_ROW_SPAN:
      child->row_span = g_value_get_int (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_X_EXPAND:
      child->x_expand = g_value_get_boolean (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_Y_EXPAND:
      child->y_expand = g_value_get_boolean (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_X_ALIGN:
      child->x_align = g_value_get_double (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_Y_ALIGN:
      child->y_align = g_value_get_double (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_X_FILL:
      child->x_fill = g_value_get_boolean (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_Y_FILL:
      child->y_fill = g_value_get_boolean (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_ALLOCATE_HIDDEN:
      child->allocate_hidden = g_value_get_boolean (value);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
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
    case CHILD_PROP_COL_OLD:
      g_warning ("The \"column\" property of NbtkTableChild is deprecated."
                 " Please use \"col\" instead.");
    case CHILD_PROP_COL:
      g_value_set_int (value, child->col);
      break;
    case CHILD_PROP_ROW:
      g_value_set_int (value, child->row);
      break;
    case CHILD_PROP_COL_SPAN:
      g_value_set_int (value, child->col_span);
      break;
    case CHILD_PROP_ROW_SPAN:
      g_value_set_int (value, child->row_span);
      break;
    case CHILD_PROP_X_EXPAND:
      g_value_set_boolean (value, child->x_expand);
      break;
    case CHILD_PROP_Y_EXPAND:
      g_value_set_boolean (value, child->y_expand);
      break;
    case CHILD_PROP_X_ALIGN:
      g_value_set_double (value, child->x_align);
      break;
    case CHILD_PROP_Y_ALIGN:
      g_value_set_double (value, child->y_align);
      break;
    case CHILD_PROP_X_FILL:
      g_value_set_boolean (value, child->x_fill);
      break;
    case CHILD_PROP_Y_FILL:
      g_value_set_boolean (value, child->y_fill);
      break;
    case CHILD_PROP_ALLOCATE_HIDDEN:
      g_value_set_boolean (value, child->allocate_hidden);
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
                            NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_COL_OLD, pspec);

  pspec = g_param_spec_int ("col",
                            "Column Number",
                            "The column the widget resides in",
                            0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_COL, pspec);

  pspec = g_param_spec_int ("row",
                            "Row Number",
                            "The row the widget resides in",
                            0, G_MAXINT,
                            0,
                            NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_ROW, pspec);

  pspec = g_param_spec_int ("row-span",
                            "Row Span",
                            "The number of rows the widget should span",
                            1, G_MAXINT,
                            1,
                            NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_ROW_SPAN, pspec);

  pspec = g_param_spec_int ("col-span",
                            "Column Span",
                            "The number of columns the widget should span",
                            1, G_MAXINT,
                            1,
                            NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_COL_SPAN, pspec);

  pspec = g_param_spec_boolean ("x-expand",
                                "X Expand",
                                "Whether the child should receive priority "
                                "when the container is allocating spare space "
                                "on the horizontal axis",
                                TRUE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_X_EXPAND, pspec);

  pspec = g_param_spec_boolean ("y-expand",
                                "Y Expand",
                                "Whether the child should receive priority "
                                "when the container is allocating spare space "
                                "on the vertical axis",
                                TRUE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_Y_EXPAND, pspec);

  pspec = g_param_spec_double ("x-align",
                               "X Alignment",
                               "X alignment of the widget within the cell",
                               0, 1,
                               0.5,
                               NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_X_ALIGN, pspec);

  pspec = g_param_spec_double ("y-align",
                               "Y Alignment",
                               "Y alignment of the widget within the cell",
                               0, 1,
                               0.5,
                               NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_Y_ALIGN, pspec);

  pspec = g_param_spec_boolean ("x-fill",
                                "X Fill",
                                "Whether the child should be allocated its "
                                "entire available space, or whether it should "
                                "be squashed and aligned.",
                                TRUE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_X_FILL, pspec);

  pspec = g_param_spec_boolean ("y-fill",
                                "Y Fill",
                                "Whether the child should be allocated its "
                                "entire available space, or whether it should "
                                "be squashed and aligned.",
                                TRUE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_Y_FILL, pspec);

  pspec = g_param_spec_boolean ("allocate-hidden",
                                "Allocate Hidden",
                                "Whether the child should be allocate even "
                                "if it is hidden",
                                TRUE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_ALLOCATE_HIDDEN, pspec);
}

static void
nbtk_table_child_init (NbtkTableChild *self)
{
  self->col_span = 1;
  self->row_span = 1;

  self->x_align = 0.5;
  self->y_align = 0.5;

  self->x_expand = TRUE;
  self->y_expand = TRUE;

  self->x_fill = TRUE;
  self->y_fill = TRUE;

  self->allocate_hidden = TRUE;
}

static NbtkTableChild*
get_child_meta (NbtkTable    *table,
                 ClutterActor *child)
{
  NbtkTableChild *meta;

  meta = (NbtkTableChild*) clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

  return meta;
}

/**
 * nbtk_table_child_get_col_span:
 * @table: an #NbtkTable
 * @child: a #ClutterActor
 *
 * Get the column span of the child. Defaults to 1.
 *
 * Returns: the column span of the child
 */
gint
nbtk_table_child_get_col_span (NbtkTable    *table,
                               ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  return meta->col_span;
}

/**
 * nbtk_table_child_set_col_span:
 * @table: An #NbtkTable
 * @child: An #ClutterActor
 * @span: The number of columns to span
 *
 * Set the column span of the child.
 *
 */
void
nbtk_table_child_set_col_span (NbtkTable    *table,
                               ClutterActor *child,
                               gint          span)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));
  g_return_if_fail (span > 1);

  meta = get_child_meta (table, child);

  meta->col_span = span;

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_get_row_span:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the row span of the child. Defaults to 1.
 *
 * Returns: the row span of the child
 */
gint
nbtk_table_child_get_row_span (NbtkTable    *table,
                               ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  return meta->row_span;
}

/**
 * nbtk_table_child_set_row_span:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @span: the number of rows to span
 *
 * Set the row span of the child.
 *
 */
void
nbtk_table_child_set_row_span (NbtkTable    *table,
                               ClutterActor *child,
                               gint          span)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));
  g_return_if_fail (span > 1);

  meta = get_child_meta (table, child);

  meta->row_span = span;

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_get_x_fill:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the x-fill state of the child
 *
 * Returns: #TRUE if the child is set to x-fill
 */
gboolean
nbtk_table_child_get_x_fill (NbtkTable    *table,
                             ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  return meta->x_fill;
}

/**
 * nbtk_table_child_set_x_fill:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @fill: the fill state
 *
 * Set the fill state of the child on the x-axis. This will cause the child to
 * be allocated the maximum available space.
 *
 */
void
nbtk_table_child_set_x_fill (NbtkTable    *table,
                             ClutterActor *child,
                             gboolean      fill)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  meta->x_fill = fill;

  clutter_actor_queue_relayout (child);
}


/**
 * nbtk_table_child_get_y_fill:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the y-fill state of the child
 *
 * Returns: #TRUE if the child is set to y-fill
 */
gboolean
nbtk_table_child_get_y_fill (NbtkTable    *table,
                             ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  return meta->y_fill;
}

/**
 * nbtk_table_child_set_y_fill:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @fill: the fill state
 *
 * Set the fill state of the child on the y-axis. This will cause the child to
 * be allocated the maximum available space.
 *
 */
void
nbtk_table_child_set_y_fill (NbtkTable    *table,
                             ClutterActor *child,
                             gboolean      fill)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  meta->y_fill = fill;

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_get_col_expand:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the column expand property of the child
 *
 * Returns: #TRUE if the child is set to column expand
 */
gboolean
nbtk_table_child_get_col_expand (NbtkTable    *table,
                                 ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  return meta->x_expand;
}

/**
 * nbtk_table_child_set_row_expand:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @expand: the new value of the row expand child property
 *
 * Set row expand on the child. This causes the row which the child
 * resides in to be allocated any extra space if the allocation of the table is
 * larger than the preferred size.
 *
 */
void
nbtk_table_child_set_row_expand (NbtkTable    *table,
                                 ClutterActor *child,
                                 gboolean      expand)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  meta->y_expand = expand;

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_set_col_expand:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @expand: the new value of the column expand child property
 *
 * Set column expand on the child. This causes the column which the child
 * resides in to be allocated any extra space if the allocation of the table is
 * larger than the preferred size.
 *
 */
void
nbtk_table_child_set_col_expand (NbtkTable    *table,
                                 ClutterActor *child,
                                 gboolean      expand)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  meta->x_expand = expand;

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_get_row_expand:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the row expand property of the child.
 *
 * Returns: #TRUE if the child is set to row expand
 */
gboolean
nbtk_table_child_get_row_expand (NbtkTable    *table,
                                 ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  if (meta->x_align == 0.0)
      return NBTK_ALIGN_START;
  else if (meta->x_align == 1.0)
      return NBTK_ALIGN_END;
  else
      return NBTK_ALIGN_MIDDLE;
}

/**
 * nbtk_table_child_get_x_align:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the x-align value of the child
 *
 * Returns: An #NbtkAlign value
 */
NbtkAlign
nbtk_table_child_get_x_align (NbtkTable    *table,
                              ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  if (meta->x_align == 0.0)
    return NBTK_ALIGN_START;
  else if (meta->x_align == 1.0)
    return NBTK_ALIGN_END;
  else
    return NBTK_ALIGN_MIDDLE;

}

/**
 * nbtk_table_child_set_x_align:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @align: A #NbtkAlign value
 *
 * Set the alignment of the child within its cell. This will only have an effect
 * if the the x-fill property is FALSE.
 *
 */
void
nbtk_table_child_set_x_align (NbtkTable    *table,
                              ClutterActor *child,
                              NbtkAlign     align)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  switch (align)
    {
    case NBTK_ALIGN_START:
      meta->x_align = 0.0;
      break;
    case NBTK_ALIGN_MIDDLE:
      meta->x_align = 0.5;
      break;
    case NBTK_ALIGN_END:
      meta->x_align = 1.0;
      break;
    }

  clutter_actor_queue_relayout (child);
}

/**
 * nbtk_table_child_get_y_align:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 *
 * Get the y-align value of the child
 *
 * Returns: An #NbtkAlign value
 */
NbtkAlign
nbtk_table_child_get_y_align (NbtkTable    *table,
                              ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), 0);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), 0);

  meta = get_child_meta (table, child);

  if (meta->y_align == 0.0)
    return NBTK_ALIGN_START;
  else if (meta->y_align == 1.0)
    return NBTK_ALIGN_END;
  else
    return NBTK_ALIGN_MIDDLE;

}

/**
 * nbtk_table_child_set_y_align:
 * @table: A #NbtkTable
 * @child: A #ClutterActor
 * @align: A #NbtkAlign value
 *
 * Set the value of the y-align property. This will only have an effect if
 * y-fill value is set to FALSE.
 *
 */
void
nbtk_table_child_set_y_align (NbtkTable    *table,
                              ClutterActor *child,
                              NbtkAlign     align)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  switch (align)
    {
    case NBTK_ALIGN_START:
      meta->y_align = 0.0;
      break;
    case NBTK_ALIGN_MIDDLE:
      meta->y_align = 0.5;
      break;
    case NBTK_ALIGN_END:
      meta->y_align = 1.0;
      break;
    }

  clutter_actor_queue_relayout (child);
}

void
nbtk_table_child_set_allocate_hidden (NbtkTable    *table,
                                      ClutterActor *child,
                                      gboolean      value)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));

  meta = get_child_meta (table, child);

  if (meta->allocate_hidden != value)
    {
      meta->allocate_hidden = value;

      clutter_actor_queue_relayout (child);

      g_object_notify (G_OBJECT (meta), "allocate-hidden");
    }
}

gboolean
nbtk_table_child_get_allocate_hidden (NbtkTable    *table,
                                      ClutterActor *child)
{
  NbtkTableChild *meta;

  g_return_val_if_fail (NBTK_IS_TABLE (table), TRUE);
  g_return_val_if_fail (CLUTTER_IS_ACTOR (child), TRUE);

  meta = get_child_meta (table, child);

  return meta->allocate_hidden;
}
