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

/**
 * SECTION:nbtk-table
 * @short_description: A multi-child layout container based on rows
 * and columns
 *
 * #NbtkTable is a mult-child layout container based on a table arrangement
 * with rows and columns. #NbtkTable adds several child properties to it's
 * children that control their position and size in the table.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-table.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "nbtk-private.h"
#include "nbtk.h"

enum
{
  PROP_0,

  PROP_PADDING,

  PROP_COL_SPACING,
  PROP_ROW_SPACING,

  PROP_HOMOGENEOUS,
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

  gint active_row;
  gint active_col;

  GArray *min_widths_u;
  GArray *pref_widths_u;
  GArray *min_heights_u;
  GArray *pref_heights_u;

  GArray *has_expand_cols;
  GArray *has_expand_rows;

  GArray *col_widths;
  GArray *min_heights;

  guint homogeneous : 1;
};

static void nbtk_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkTable, nbtk_table, NBTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                nbtk_container_iface_init));


/*
 * ClutterChildMeta Implementation
 */

enum {
  CHILD_PROP_0,

  CHILD_PROP_COL,
  CHILD_PROP_ROW,
  CHILD_PROP_COL_SPAN,
  CHILD_PROP_ROW_SPAN,
  CHILD_PROP_KEEP_RATIO,
  CHILD_PROP_X_EXPAND,
  CHILD_PROP_Y_EXPAND,
  CHILD_PROP_X_ALIGN,
  CHILD_PROP_Y_ALIGN,
  CHILD_PROP_X_FILL,
  CHILD_PROP_Y_FILL,
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
    case CHILD_PROP_COL:
      child->col = g_value_get_int (value);
      table->priv->n_cols = MAX (table->priv->n_cols, child->col + 1);
      clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
      break;
    case CHILD_PROP_ROW:
      child->row = g_value_get_int (value);
      table->priv->n_rows = MAX (table->priv->n_rows, child->row + 1);
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
    case CHILD_PROP_KEEP_RATIO:
      child->keep_ratio = g_value_get_boolean (value);
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
    case CHILD_PROP_COL_SPAN:
      g_value_set_int (value, child->col_span);
      break;
    case CHILD_PROP_ROW_SPAN:
      g_value_set_int (value, child->row_span);
      break;
    case CHILD_PROP_KEEP_RATIO:
      g_value_set_boolean (value, child->keep_ratio);
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

  pspec = g_param_spec_boolean ("keep-aspect-ratio",
                                "Keep Aspect Ratio",
                                "Whether the container should attempt to "
                                "preserve the child's aspect ratio when "
                                "allocating it's size",
                                FALSE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_KEEP_RATIO, pspec);

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
}

/*
 * ClutterContainer Implementation
 */
static void
nbtk_container_add_actor (ClutterContainer *container,
                          ClutterActor     *actor)
{
  NbtkTablePrivate *priv = NBTK_TABLE (container)->priv;
  guint             dnd_threshold;

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));


  priv->children = g_slist_append (priv->children, actor);

  dnd_threshold = nbtk_widget_get_dnd_threshold (NBTK_WIDGET (container));

  if (dnd_threshold > 0)
    nbtk_widget_setup_child_dnd (NBTK_WIDGET (container), actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);
}

static void
nbtk_container_remove_actor (ClutterContainer *container,
                             ClutterActor     *actor)
{
  NbtkTablePrivate *priv = NBTK_TABLE (container)->priv;

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

  nbtk_widget_undo_child_dnd (NBTK_WIDGET (container), actor);

  priv->children = g_slist_delete_link (priv->children, item);
  clutter_actor_unparent (actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-removed", actor);

  g_object_unref (actor);
}

static void
nbtk_container_foreach (ClutterContainer *container,
                        ClutterCallback   callback,
                        gpointer          callback_data)
{
  NbtkTablePrivate *priv = NBTK_TABLE (container)->priv;

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

/* NbtkTable Class Implementation */

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

    case PROP_HOMOGENEOUS:
      if (table->priv->homogeneous != g_value_get_boolean (value))
        {
          table->priv->homogeneous = g_value_get_boolean (value);
          clutter_actor_queue_relayout ((ClutterActor *)gobject);
        }
      break;

    case PROP_PADDING:
      nbtk_table_set_padding (table, g_value_get_boxed (value));
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
  NbtkPadding padding;

  switch (prop_id)
    {
    case PROP_COL_SPACING:
      g_value_set_int (value, priv->col_spacing);
      break;

    case PROP_ROW_SPACING:
      g_value_set_int (value, priv->row_spacing);
      break;

    case PROP_HOMOGENEOUS:
      g_value_set_boolean (value, priv->homogeneous);
      break;

    case PROP_PADDING:
      nbtk_table_get_padding (NBTK_TABLE (gobject), &padding);
      g_value_set_boxed (value, &padding);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_table_finalize (GObject *gobject)
{
  NbtkTablePrivate *priv = NBTK_TABLE (gobject)->priv;

  g_array_free (priv->min_widths_u, TRUE);
  g_array_free (priv->pref_widths_u, TRUE);

  g_array_free (priv->min_heights_u, TRUE);
  g_array_free (priv->pref_heights_u, TRUE);

  g_array_free (priv->has_expand_cols, TRUE);
  g_array_free (priv->has_expand_rows, TRUE);

  g_array_free (priv->col_widths, TRUE);
  g_array_free (priv->min_heights, TRUE);

  G_OBJECT_CLASS (nbtk_table_parent_class)->finalize (gobject);
}

static void
nbtk_table_dispose (GObject *gobject)
{
  NbtkTablePrivate *priv = NBTK_TABLE (gobject)->priv;
  GSList *l, *next;

  for (l = priv->children; l;)
    {
      next = l->next;
      clutter_container_remove_actor ((ClutterContainer *)gobject,
                                      CLUTTER_ACTOR (l->data));
      l = next;
    }

  G_OBJECT_CLASS (nbtk_table_parent_class)->dispose (gobject);
}

#define CLAMP_TO_PIXEL(x) CLUTTER_UNITS_FROM_INT (CLUTTER_UNITS_TO_INT (x))

/* Utility function to modify a child allocation box with respect to the
 * x/y-fill child properties. Expects childbox to contain the available
 * allocation space.
 */
static void
nbtk_table_allocate_fill (ClutterActor *child,
                          ClutterActorBox *childbox,
                          gdouble x_align,
                          gdouble y_align,
                          gboolean x_fill,
                          gboolean y_fill)
{
  if (!x_fill)
    {
      ClutterUnit width, max_width;
      max_width = childbox->x2 - childbox->x1;
      clutter_actor_get_preferred_width (child, -1, NULL, &width);
      if (width < max_width)
        {
          childbox->x1 += CLAMP_TO_PIXEL((max_width - width) * x_align);
          childbox->x2 = childbox->x1 + width;
        }
    }

  if (!y_fill)
    {
      ClutterUnit height, max_height;
      max_height = childbox->y2 - childbox->y1;
      clutter_actor_get_preferred_height (child, -1, NULL, &height);
      if (height < max_height)
        {
          childbox->y1 += CLAMP_TO_PIXEL ((max_height - height) * y_align);
          childbox->y2 = childbox->y1 + height;
        }
    }
}

static void
nbtk_table_homogeneous_allocate (ClutterActor          *self,
                                const ClutterActorBox *box,
                                gboolean               absolute_origin_changed)
{
  GSList *list;
  ClutterUnit col_width, row_height;
  gint row_spacing, col_spacing;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  NbtkPadding padding;

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  col_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->col_spacing);
  row_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->row_spacing);

  col_width = (box->x2 - box->x1
               - padding.left - padding.right
               - (col_spacing * (priv->n_cols - 1)))
            / priv->n_cols;
  row_height = (box->y2 - box->y1
                - padding.top - padding.bottom
                - (row_spacing * (priv->n_rows - 1)))
             / priv->n_rows;

  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, row_span, col_span;
      gboolean keep_ratio;
      NbtkTableChild *meta;
      ClutterActor *child;
      ClutterActorBox childbox;
      gdouble x_align, y_align;
      gboolean x_fill, y_fill;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      /* get child properties */
      col = meta->col;
      row = meta->row;
      row_span = meta->row_span;
      col_span = meta->col_span;
      keep_ratio = meta->keep_ratio;
      x_align = meta->x_align;
      y_align = meta->y_align;
      x_fill = meta->x_fill;
      y_fill = meta->y_fill;

      childbox.x1 = padding.left + (col_width + col_spacing) * col;
      childbox.x2 = childbox.x1 + (col_width * col_span) + (col_spacing * (col_span - 1));

      childbox.y1 = padding.top + (row_height + row_spacing) * row;
      childbox.y2 = childbox.y1 + (row_height * row_span) + (row_spacing * (row_span - 1));

      nbtk_table_allocate_fill (child, &childbox, x_align, y_align, x_fill, y_fill);

      if (keep_ratio)
        {
          ClutterUnit w, h;
          gint new_width;
          gint new_height;
          gint center_offset;

          clutter_actor_get_sizeu (child, &w, &h);

          new_height = ((gdouble) h / w)  * ((gdouble) childbox.x2 - childbox.x1);
          new_width = ((gdouble) w / h)  * ((gdouble) childbox.y2 - childbox.y1);


          if (new_height > row_height)
            {
              /* center for new width */
              center_offset = ((childbox.x2 - childbox.x1) - new_width) * x_align;
              childbox.x1 = childbox.x1 + center_offset;
              childbox.x2 = childbox.x1 + new_width;
            }
          else
            {
              /* center for new height */
              center_offset = ((childbox.y2 - childbox.y1) - new_height) * y_align;
              childbox.y1 = childbox.y1 + center_offset;
              childbox.y2 = childbox.y1 + new_height;
            }

        }

      clutter_actor_allocate (child, &childbox, absolute_origin_changed);
    }

}


static gint *
nbtk_table_calculate_col_widths (NbtkTable *table, gint for_width)
{
  gint total_min_width, i;
  NbtkTablePrivate *priv = table->priv;
  gboolean *has_expand_cols;
  gint extra_col_width, n_expanded_cols = 0, expanded_cols = 0;
  gint *col_widths;
  GSList *list;
  NbtkPadding padding;

  g_array_set_size (priv->has_expand_cols, 0);
  g_array_set_size (priv->has_expand_cols, priv->n_cols);
  has_expand_cols = (gboolean *)priv->has_expand_cols->data;

  g_array_set_size (priv->col_widths, 0);
  g_array_set_size (priv->col_widths, priv->n_cols);
  col_widths = (gint *)priv->col_widths->data;

  /* take off the padding values to calculate the allocatable width */
  nbtk_widget_get_padding (NBTK_WIDGET (table), &padding);

  for_width -= CLUTTER_UNITS_TO_INT (padding.left + padding.right);

  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col;
      ClutterUnit w_min, w_pref;
      gboolean x_expand;
      NbtkTableChild *meta;
      ClutterActor *child;
      gint col_span, row_span;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

      /* get child properties */
      col = meta->col;
      row = meta->row;
      x_expand = meta->x_expand;
      col_span = meta->col_span;
      row_span = meta->row_span;

      if (x_expand)
        has_expand_cols[col] = TRUE;

      clutter_actor_get_preferred_width (child, -1, &w_min, &w_pref);
      if (col_span == 1 && w_pref > col_widths[col])
        col_widths[col] = CLUTTER_UNITS_TO_INT (w_pref);

    }

  total_min_width = priv->col_spacing * (priv->n_cols - 1);
  for (i = 0; i < priv->n_cols; i++)
    total_min_width += col_widths[i];

  /* calculate the remaining space and distribute it evenly onto all rows/cols
   * with the x/y expand property set. */
  for (i = 0; i < priv->n_cols; i++)
    if (has_expand_cols[i])
      {
        expanded_cols += col_widths[i];
        n_expanded_cols++;
      }

  /* for_width - total_min_width */
  extra_col_width = for_width - total_min_width;
  if (extra_col_width)
    for (i = 0; i < priv->n_cols; i++)
      if (has_expand_cols[i])
       {
          if (extra_col_width < 0)
            col_widths[i] =
              MAX (0,
                   col_widths[i]
                  + (extra_col_width * (col_widths[i] / (float) expanded_cols)));
          else
            col_widths[i] += extra_col_width / n_expanded_cols;
        }

  return col_widths;
}

static void
nbtk_table_preferred_allocate (ClutterActor          *self,
                               const ClutterActorBox *box,
                               gboolean               absolute_origin_changed)
{
  GSList *list;
  gint row_spacing, col_spacing, extra_row_height;
  gint total_min_height, i, table_width, table_height;
  gint *min_widths, *min_heights;
  gint n_expanded_rows = 0;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  gboolean *has_expand_rows;
  gint expanded_rows = 0;
  NbtkPadding padding;

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  col_spacing = (priv->col_spacing);
  row_spacing = (priv->row_spacing);

  g_array_set_size (priv->has_expand_rows, 0);
  g_array_set_size (priv->has_expand_rows, priv->n_rows);
  has_expand_rows = (gboolean *)priv->has_expand_rows->data;

  g_array_set_size (priv->min_heights, 0);
  g_array_set_size (priv->min_heights, priv->n_rows);
  min_heights = (gboolean *)priv->min_heights->data;

  table_height = CLUTTER_UNITS_TO_INT (box->y2 - box->y1
                                       - padding.top
                                       - padding.bottom);
  table_width = CLUTTER_UNITS_TO_INT (box->x2 - box->x1
                                      - padding.right
                                      - padding.left);

  min_widths = nbtk_table_calculate_col_widths (NBTK_TABLE (self),
                                                CLUTTER_UNITS_TO_INT (box->x2 - box->x1));

  /* calculate minimum row widths and column heights */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, cell_width;
      ClutterUnit h_min, h_pref;
      gboolean x_expand, y_expand;
      NbtkTableChild *meta;
      ClutterActor *child;
      gint col_span, row_span;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      /* get child properties */
      col = meta->col;
      row = meta->row;
      x_expand = meta->x_expand;
      y_expand = meta->y_expand;
      col_span = meta->col_span;
      row_span = meta->row_span;

      if (y_expand)
        has_expand_rows[row] = TRUE;

      /* calculate the cell width by including any spanned columns */
      cell_width = 0;
      for (i = 0; i < col_span; i++)
        cell_width += min_widths[col + i];

      clutter_actor_get_preferred_height (child, cell_width, &h_min, &h_pref);

      if (row_span == 1 && h_pref > min_heights[row])
        min_heights[row] = CLUTTER_UNITS_TO_INT (h_pref);
    }

  total_min_height = row_spacing * (priv->n_rows - 1);
  for (i = 0; i < priv->n_rows; i++)
    total_min_height += min_heights[i];

  /* calculate the remaining space and distribute it evenly onto all rows/cols
   * with the x/y expand property set. */
  for (i = 0; i < priv->n_rows; i++)
    if (has_expand_rows[i])
      {
        expanded_rows += min_heights[i];
        n_expanded_rows++;
      }

  extra_row_height = table_height - total_min_height;

  /* distribute the extra space amongst columns/rows with expand set to TRUE
   *
   * extra space is allocated evenly when extra space is > 0 and
   * proportionally when extra space < 0.
   */
  if (extra_row_height)
    for (i = 0; i < priv->n_rows; i++)
      if (has_expand_rows[i])
        {
          if (expanded_rows < 0)
            min_heights[i] =
              MAX (0,
                   min_heights[i]
                   + (extra_row_height * (min_heights[i] / (float) expanded_rows)));
          else
            min_heights[i] += extra_row_height / n_expanded_rows;
        }

  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, row_span, col_span;
      gint col_width, row_height;
      gboolean keep_ratio;
      NbtkTableChild *meta;
      ClutterActor *child;
      ClutterActorBox childbox;
      gint child_x, child_y;
      gdouble x_align, y_align;
      gboolean x_fill, y_fill;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      /* get child properties */
      col = meta->col;
      row = meta->row;
      row_span = meta->row_span;
      col_span = meta->col_span;
      keep_ratio = meta->keep_ratio;
      x_align = meta->x_align;
      y_align = meta->y_align;
      x_fill = meta->x_fill;
      y_fill = meta->y_fill;


      /* initialise the width and height */
      col_width = min_widths[col];
      row_height = min_heights[row];

      /* Add the widths of the spanned columns:
       *
       * First check that we have a non-zero span. Then we loop over each of
       * the columns that we're spanning but we stop short if we go past the
       * number of columns in the table. This is necessary to avoid accessing
       * uninitialised memory. We add the spacing in here too since we only
       * want to add as much spacing as times we successfully span.
       */
      if (col_span > 1)
        {
          for (i = col + 1; i < col + col_span && i < priv->n_cols; i++)
            {
              col_width += min_widths[i];
              col_width += col_spacing;
            }
        }

      /* add the height of the spanned rows */
      if (row_span > 1)
        {
          for (i = row + 1; i < row + row_span && i < priv->n_rows; i++)
            {
              row_height += min_heights[i];
              row_height += row_spacing;
            }
        }

      /* calculate child x */
      child_x = CLUTTER_UNITS_TO_INT (padding.left)
              + col_spacing * col;
      for (i = 0; i < col; i++)
        child_x += min_widths[i];

      /* calculate child y */
      child_y = CLUTTER_UNITS_TO_INT (padding.top)
              + row_spacing * row;
      for (i = 0; i < row; i++)
        child_y += min_heights[i];

      /* set up childbox */
      childbox.x1 = CLUTTER_UNITS_FROM_INT (child_x);
      childbox.x2 = CLUTTER_UNITS_FROM_INT (MAX (0, child_x + col_width));

      childbox.y1 = CLUTTER_UNITS_FROM_INT (child_y);
      childbox.y2 = CLUTTER_UNITS_FROM_INT (MAX (0, child_y + row_height));


      nbtk_table_allocate_fill (child, &childbox, x_align, y_align, x_fill, y_fill);

      if (keep_ratio)
        {
          guint w, h;
          guint new_width;
          guint new_height;
          gint center_offset;

          clutter_actor_get_size (child, &w, &h);

          new_height = (h / (gdouble) w)  * (gdouble) col_width;
          new_width = (w / (gdouble) h)  * (gdouble) row_height;


          if (new_height > row_height)
            {
              /* apply new width */
              center_offset = (CLUTTER_UNITS_TO_INT (childbox.x2 - childbox.x1) - new_width) * x_align;
              childbox.x1 = childbox.x1 + CLUTTER_UNITS_FROM_INT (center_offset);
              childbox.x2 = childbox.x1 + CLUTTER_UNITS_FROM_INT (new_width);
            }
          else
            {
              /* apply new height */
              center_offset = (CLUTTER_UNITS_TO_INT (childbox.y2 - childbox.y1) - new_height) * y_align;
              childbox.y1 = childbox.y1 + CLUTTER_UNITS_FROM_INT (center_offset);
              childbox.y2 = childbox.y1 + CLUTTER_UNITS_FROM_INT (new_height);
            }
        }

      clutter_actor_allocate (child, &childbox, absolute_origin_changed);
    }
}

static void
nbtk_table_allocate (ClutterActor          *self,
                     const ClutterActorBox *box,
                     gboolean               absolute_origin_changed)
{
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->allocate (self, box, absolute_origin_changed);

  if (priv->n_cols < 1 || priv->n_rows < 1)
    {
      return;
    };

  if (priv->homogeneous)
    nbtk_table_homogeneous_allocate (self, box, absolute_origin_changed);
  else
    nbtk_table_preferred_allocate (self, box, absolute_origin_changed);
}

static void
nbtk_table_get_preferred_width (ClutterActor *self,
                                ClutterUnit   for_height,
                                ClutterUnit  *min_width_p,
                                ClutterUnit  *natural_width_p)
{
  ClutterUnit *min_widths, *pref_widths;
  ClutterUnit total_min_width, total_pref_width;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  GSList *list;
  gint i;
  NbtkPadding padding;

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  if (priv->n_cols < 1)
  {
    *min_width_p = 0;
    *natural_width_p = 0;
    return;
  }

  /* Setting size to zero and then what we want it to be causes a clear if
   * clear flag is set (which it should be.)
   */
  g_array_set_size (priv->min_widths_u, 0);
  g_array_set_size (priv->pref_widths_u, 0);
  g_array_set_size (priv->min_widths_u, priv->n_cols);
  g_array_set_size (priv->pref_widths_u, priv->n_cols);

  min_widths = (ClutterUnit *)priv->min_widths_u->data;
  pref_widths = (ClutterUnit *)priv->pref_widths_u->data;

  /* calculate minimum row widths */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint col;
      ClutterUnit w_min, w_pref;
      NbtkTableChild *meta;
      ClutterActor *child;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      /* get child properties */
      col = meta->col;

      clutter_actor_get_preferred_width (child, -1, &w_min, &w_pref);

      if (w_min > min_widths[col])
        min_widths[col] = w_min;
      if (w_pref > pref_widths[col])
        pref_widths[col] = w_pref;
    }

  total_min_width = padding.left
                  + padding.right
                  + (priv->n_cols - 1)
                  * CLUTTER_UNITS_FROM_DEVICE (priv->col_spacing);
  total_pref_width = total_min_width;

  for (i = 0; i < priv->n_cols; i++)
    {
      total_min_width += min_widths[i];
      total_pref_width += pref_widths[i];
    }

  if (min_width_p)
    *min_width_p = total_min_width;
  if (natural_width_p)
    *natural_width_p = total_pref_width;
}

static void
nbtk_table_get_preferred_height (ClutterActor *self,
                                 ClutterUnit   for_width,
                                 ClutterUnit  *min_height_p,
                                 ClutterUnit  *natural_height_p)
{
  ClutterUnit *min_heights, *pref_heights;
  ClutterUnit total_min_height, total_pref_height;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  GSList *list;
  gint i;
  gint *min_widths;
  NbtkPadding padding;

  if (priv->n_rows < 1)
  {
    *min_height_p = 0;
    *natural_height_p = 0;
    return;
  }

  /* Setting size to zero and then what we want it to be causes a clear if
   * clear flag is set (which it should be.)
   */
  g_array_set_size (priv->min_heights_u, 0);
  g_array_set_size (priv->pref_heights_u, 0);
  g_array_set_size (priv->min_heights_u, priv->n_rows);
  g_array_set_size (priv->pref_heights_u, priv->n_rows);

  /* use min_widths to help allocation of height-for-width widgets */
  min_widths = nbtk_table_calculate_col_widths (NBTK_TABLE (self), for_width);

  min_heights = (ClutterUnit *)priv->min_heights_u->data;
  pref_heights = (ClutterUnit *)priv->pref_heights_u->data;

  /* calculate minimum row heights */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, col_span, cell_width;
      ClutterUnit min, pref;
      NbtkTableChild *meta;
      ClutterActor *child;

      child = CLUTTER_ACTOR (list->data);

      meta = (NbtkTableChild *) clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      /* get child properties */
      row = meta->row;
      col = meta->col;
      col_span = meta->col_span;

      cell_width = 0;
      for (i = 0; i < col_span; i++)
        cell_width += min_widths[col + i];


      clutter_actor_get_preferred_height (child,
              CLUTTER_UNITS_FROM_INT (cell_width), &min, &pref);

      if (min > min_heights[row])
        min_heights[row] = min;
      if (pref > pref_heights[row])
        pref_heights[row] = pref;
    }

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  /* start off with padding plus row spacing */
  total_min_height = padding.top + padding.bottom + (priv->n_rows - 1) *
                     CLUTTER_UNITS_FROM_DEVICE (priv->row_spacing);

  total_pref_height = total_min_height;

  for (i = 0; i < priv->n_rows; i++)
    {
      total_min_height += min_heights[i];
      total_pref_height += pref_heights[i];
    }

  if (min_height_p)
    *min_height_p = total_min_height;
  if (natural_height_p)
    *natural_height_p = total_pref_height;
}

static void
nbtk_table_paint (ClutterActor *self)
{
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  GSList *list;

  /* make sure the background gets painted first */
  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->paint (self);

  for (list = priv->children; list; list = g_slist_next (list))
    {
      ClutterActor *child = CLUTTER_ACTOR (list->data);
      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }
}

static void
nbtk_table_pick (ClutterActor       *self,
                 const ClutterColor *color)
{
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  GSList *list;

  /* Chain up so we get a bounding box painted (if we are reactive) */
  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->pick (self, color);

  for (list = priv->children; list; list = g_slist_next (list))
    {
      if (CLUTTER_ACTOR_IS_VISIBLE (list->data))
        clutter_actor_paint (CLUTTER_ACTOR (list->data));
    }
}

static void
nbtk_table_dnd_dropped (NbtkWidget   *actor,
			ClutterActor *dragged,
			ClutterActor *icon,
			gint          x,
			gint          y)
{
  ClutterActor *parent;
  gboolean keep_ratio = FALSE;

  g_object_ref (dragged);
  parent = clutter_actor_get_parent (dragged);

  if (NBTK_IS_TABLE (parent))
    {
      NbtkTableChild *meta;

      meta = NBTK_TABLE_CHILD (
		clutter_container_get_child_meta (CLUTTER_CONTAINER (parent),
						  dragged));

      /*
       * Must do it like this, as meta->keep_ratio is a 1 bit field, and
       * we can only pass actual TRUE/FALSE values into the property setter.
       */
      if (meta->keep_ratio)
	keep_ratio = TRUE;
    }

  clutter_container_remove_actor (CLUTTER_CONTAINER (parent), dragged);
  nbtk_table_insert_actor_at_position (NBTK_TABLE (actor), dragged, x, y);

  clutter_container_child_set (CLUTTER_CONTAINER (actor), dragged,
			       "keep-aspect-ratio", keep_ratio,
                               NULL);

  g_object_unref (dragged);
}

static void
nbtk_table_show_all (ClutterActor *table)
{
  NbtkTablePrivate *priv = NBTK_TABLE (table)->priv;
  GSList *l;

  for (l = priv->children; l; l = l->next)
    clutter_actor_show_all (CLUTTER_ACTOR (l->data));

  clutter_actor_show (table);
}

static void
nbtk_table_hide_all (ClutterActor *table)
{
  NbtkTablePrivate *priv = NBTK_TABLE (table)->priv;
  GSList *l;

  clutter_actor_hide (table);

  for (l = priv->children; l; l = l->next)
    clutter_actor_hide_all (CLUTTER_ACTOR (l->data));
}

static void
nbtk_table_class_init (NbtkTableClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);

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
  actor_class->get_preferred_width = nbtk_table_get_preferred_width;
  actor_class->get_preferred_height = nbtk_table_get_preferred_height;
  actor_class->show_all = nbtk_table_show_all;
  actor_class->hide_all = nbtk_table_hide_all;

  widget_class->dnd_dropped = nbtk_table_dnd_dropped;

  pspec = g_param_spec_boolean ("homogeneous",
                                "Homogeneous",
                                "Homogeneous rows and columns",
                                TRUE,
                                NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_HOMOGENEOUS,
                                   pspec);

  pspec = g_param_spec_int ("col-spacing",
                            "Column Spacing",
                            "Spacing between columns",
                            0, G_MAXINT, 0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_COL_SPACING,
                                   pspec);

  pspec = g_param_spec_int ("row-spacing",
                            "Row Spacing",
                            "Spacing between row",
                            0, G_MAXINT, 0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_ROW_SPACING,
                                   pspec);

  pspec = g_param_spec_boxed ("padding",
                              "Padding",
                              "Padding of the table",
                              NBTK_TYPE_PADDING,
                              NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_PADDING,
                                   pspec);
}

static void
nbtk_table_init (NbtkTable *table)
{
  table->priv = NBTK_TABLE_GET_PRIVATE (table);

  table->priv->n_cols = 0;
  table->priv->n_rows = 0;

  table->priv->min_widths_u = g_array_new (FALSE,
                                           TRUE,
                                           sizeof (ClutterUnit));
  table->priv->pref_widths_u = g_array_new (FALSE,
                                            TRUE,
                                            sizeof (ClutterUnit));
  table->priv->min_heights_u = g_array_new (FALSE,
                                            TRUE,
                                            sizeof (ClutterUnit));
  table->priv->pref_heights_u = g_array_new (FALSE,
                                             TRUE,
                                             sizeof (ClutterUnit));

  table->priv->has_expand_cols = g_array_new (FALSE,
                                              TRUE,
                                              sizeof (gboolean));
  table->priv->has_expand_rows = g_array_new (FALSE,
                                              TRUE,
                                              sizeof (gboolean));

  table->priv->col_widths = g_array_new (FALSE,
                                         TRUE,
                                         sizeof (gint));
  table->priv->min_heights = g_array_new (FALSE,
                                          TRUE,
                                          sizeof (gint));
}

/*** Public Functions ***/

/**
 * nbtk_table_new:
 *
 * Create a new #NbtkTable
 *
 * Returns: a new #NbtkTable
 */
NbtkWidget*
nbtk_table_new (void)
{
  return g_object_new (NBTK_TYPE_TABLE, NULL);
}

/**
 * nbtk_table_set_col_spacing
 * @table: a #NbtkTable
 * @spacing: spacing in pixels
 *
 * Sets the amount of spacing between columns.
 */
void
nbtk_table_set_col_spacing (NbtkTable *table,
                            gint       spacing)
{
  NbtkTablePrivate *priv;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (spacing >= 0);

  priv = NBTK_TABLE (table)->priv;

  priv->col_spacing = spacing;
}

/**
 * nbtk_table_set_row_spacing
 * @table: a #NbtkTable
 * @spacing: spacing in pixels
 *
 * Sets the amount of spacing between rows.
 */
void
nbtk_table_set_row_spacing (NbtkTable *table,
                            gint       spacing)
{
  NbtkTablePrivate *priv;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (spacing >= 0);

  priv = NBTK_TABLE (table)->priv;

  priv->row_spacing = spacing;
}

/**
 * nbtk_table_get_row_spacing
 * @table: a #NbtkTable
 *
 * Gets the amount of spacing between rows.
 *
 * Returns: the spacing between rows in device units
 */
gint
nbtk_table_get_row_spacing (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE (table)->priv;

  return priv->row_spacing;
}

/**
 * nbtk_table_get_col_spacing
 * @table: a #NbtkTable
 *
 * Gets the amount of spacing between columns.
 *
 * Returns: the spacing between columns in device units
 */
gint
nbtk_table_get_col_spacing (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE (table)->priv;

  return priv->col_spacing;
}

void
nbtk_table_add_actor (NbtkTable   *table,
                     ClutterActor *actor,
                     gint          row,
                     gint          column)
{
  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));
  g_return_if_fail (row >= 0);
  g_return_if_fail (column >= 0);

  clutter_container_add_actor (CLUTTER_CONTAINER (table), actor);
  clutter_container_child_set (CLUTTER_CONTAINER (table), actor,
                               "row", row,
                               "column", column,
                               NULL);

}

void
nbtk_table_add_actor_full (NbtkTable            *table,
                           ClutterActor         *actor,
                           gint                  row,
                           gint                  column,
                           gint                  rowspan,
                           gint                  colspan,
                           NbtkTableChildOptions options,
                           gdouble               xalign,
                           gdouble               yalign)
{
  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));
  g_return_if_fail (row >= 0);
  g_return_if_fail (column >= 0);
  g_return_if_fail (rowspan >= 1);
  g_return_if_fail (colspan >= 1);
  g_return_if_fail ((xalign >= 0) && (xalign <= 1.0));
  g_return_if_fail ((yalign >= 0) && (yalign <= 1.0));

  nbtk_table_add_actor (table, actor, row, column);
  clutter_container_child_set (CLUTTER_CONTAINER (table),
                               actor,
                               "row-span", rowspan,
                               "col-span", colspan,
                               "keep-aspect-ratio",
                               options & NBTK_KEEP_ASPECT_RATIO,
                               "x-expand", (options & NBTK_X_EXPAND) ?
                                 TRUE : FALSE,
                               "y-expand", (options & NBTK_Y_EXPAND) ?
                                 TRUE : FALSE,
                               "x-align", xalign,
                               "y-align", yalign,
                               "x-fill", (options & NBTK_X_FILL) ?
                                 TRUE : FALSE,
                               "y-fill", (options & NBTK_Y_FILL) ?
                                 TRUE : FALSE,
                               NULL);
}

/**
 * nbtk_table_add_widget:
 * @table: a #NbtkTable
 * @widget: a #NbtkWidget
 * @row: row to insert the widget in
 * @column: column to insert the widget in
 *
 * Add a widget to the table at the specified row and column.
 *
 * Note: row and column numberings start from 0 in the top left corner.
 * Therefore, the top left most cell is at column 0, row 0.
 */
void
nbtk_table_add_widget (NbtkTable  *table,
                       NbtkWidget *widget,
                       gint        row,
                       gint        column)
{
  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (NBTK_IS_WIDGET (widget));
  g_return_if_fail (row >= 0);
  g_return_if_fail (column >= 0);

  nbtk_table_add_actor (table, CLUTTER_ACTOR (widget), row, column);
}

/**
 * nbtk_table_add_widget_full:
 * @table: a #NbtkTable
 * @widget: a #NbtkWidget
 * @row: row to insert the widget in
 * @column: column to insert the widget in
 * @rowspan: rows to span the widget over
 * @colspan: columns to span the widget over
 * @options: a #NbtkTableChildOptions
 * @xalign: horizontal alignment of the widget
 * @yalign: vertical alignment of the widget
 *
 * Convenience function to add a widget to the table and set all of its
 * child properties simultaneously.
 *
 * See nbtk_table_add_widget().
 */
void
nbtk_table_add_widget_full (NbtkTable            *table,
                            NbtkWidget           *widget,
                            gint                  row,
                            gint                  column,
                            gint                  rowspan,
                            gint                  colspan,
                            NbtkTableChildOptions options,
                            gdouble               xalign,
                            gdouble               yalign)
{
  g_return_if_fail (NBTK_IS_WIDGET (widget));

  nbtk_table_add_actor_full (table,
                             CLUTTER_ACTOR (widget),
                             row,
                             column,
                             rowspan,
                             colspan,
                             options,
                             xalign,
                             yalign);
}

/**
 * nbtk_table_set_widget_colspan:
 * @table: a #NbtkTable
 * @widget: a #NbtkWidget
 * @colspan: The number of columns to span
 *
 * Set the number of columns a widget should span, starting with the current
 * column and moving right. For example, a widget placed in column 1 with
 * colspan set to 3 will occupy columns 1, 2 and 3.
 */
void
nbtk_table_set_widget_colspan (NbtkTable *table,
                               NbtkWidget *widget,
                               gint colspan)
{
  ClutterChildMeta *meta;

  g_return_if_fail (NBTK_TABLE (table));
  g_return_if_fail (NBTK_WIDGET (widget));
  g_return_if_fail (colspan >= 1);

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (table),
                                           CLUTTER_ACTOR (widget));
  g_object_set (meta, "col-span", colspan, NULL);

}

/**
 * nbtk_table_set_widget_rowspan:
 * @table: a #NbtkTable
 * @widget: a #NbtkWidget
 * @rowspan: The number of rows to span
 *
 * Set the number of rows a widget should span, starting with the current
 * row and moving down. For example, a widget placed in row 1 with rowspan
 * set to 3 will occupy rows 1, 2 and 3.
 */
void
nbtk_table_set_widget_rowspan (NbtkTable *table,
                               NbtkWidget *widget,
                               gint rowspan)
{
  ClutterChildMeta *meta;

  g_return_if_fail (NBTK_TABLE (table));
  g_return_if_fail (NBTK_WIDGET (widget));
  g_return_if_fail (rowspan >= 1);

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (table),
                                           CLUTTER_ACTOR (widget));
  g_object_set (meta, "row-span", rowspan, NULL);
}

/**
 * nbtk_table_insert_actor_at_position
 * @table: a #NbtkTable
 * @actor: a #ClutterActor
 * @x: screen coordinate of the insertion
 * @y: screen coordiante of the insertion
 *
 * Set the number of rows a widget should span, starting with the current
 * row and moving down. For example, a widget placed in row 1 with rowspan
 * set to 3 will occupy rows 1, 2 and 3.
 */
void
nbtk_table_insert_actor_at_position (NbtkTable *table,
				     ClutterActor *actor, gint x, gint y)
{
  NbtkTablePrivate *priv;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));

  priv = NBTK_TABLE (table)->priv;

  /*
   * FIXME -- this is just a stub; do something sensible with the coords
   */
  nbtk_table_add_actor (NBTK_TABLE (table), actor, priv->n_rows, 0);
}

void
nbtk_table_set_padding (NbtkTable         *table,
                        const NbtkPadding *padding)
{
  g_return_if_fail (NBTK_IS_TABLE (table));

  g_warning ("%s is deprecated. Please set padding using a stylesheet.", __FUNCTION__);
}

void
nbtk_table_get_padding (NbtkTable   *table,
                        NbtkPadding *padding)
{
  g_return_if_fail (NBTK_TABLE (table));
  g_return_if_fail (padding != NULL);

  g_warning ("%s is deprecated. Please use nbtk_widget_get_padding",
             __FUNCTION__);

  nbtk_widget_get_padding ((NbtkWidget*) table, padding);
}
