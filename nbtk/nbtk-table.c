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

#include "nbtk-table.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <clutter/clutter.h>
#include <clutter/clutter-container.h>

#include "nbtk-stylable.h"
#include "nbtk-private.h"

enum
{
  PROP_0,

  PROP_COL_SPACING,
  PROP_ROW_SPACING,

  PROP_ACTIVE_ROW,
  PROP_ACTIVE_COL,

  PROP_HOMOGENEOUS
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

  gboolean homogeneous;

  ClutterColor *bg_color;
  ClutterColor *active_color;
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
  CHILD_PROP_KEEP_RATIO
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
  gint col_span;
  gint row_span;
  gboolean keep_ratio;
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
    case CHILD_PROP_COL_SPAN:
      child->col_span = g_value_get_int (value);
      break;
    case CHILD_PROP_ROW_SPAN:
      child->row_span = g_value_get_int (value);
      break;
    case CHILD_PROP_KEEP_RATIO:
      child->keep_ratio = g_value_get_boolean (value);
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
                                "Whether the container should attempt to preserve the child's"
                                "aspect ratio when allocating it's size",
                                FALSE,
                                NBTK_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, CHILD_PROP_KEEP_RATIO, pspec);
}

static void
nbtk_table_child_init (NbtkTableChild *self)
{
  self->col_span = 1;
  self->row_span = 1;
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

/* NbtkTable Class Implementation */

static void
nbtk_table_style_changed (NbtkWidget *table)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (table);

  /* cache these values for use in the paint function */
  nbtk_stylable_get (NBTK_STYLABLE (table),
                    "background-color", &priv->bg_color,
                    "color", &priv->active_color,
                    NULL);
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

    case PROP_ACTIVE_COL:
      nbtk_table_set_active_col (table, g_value_get_int (value));
      break;

    case PROP_ACTIVE_ROW:
      nbtk_table_set_active_row (table, g_value_get_int (value));
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

    case PROP_ACTIVE_COL:
      g_value_set_int (value, priv->active_col);
      break;

    case PROP_ACTIVE_ROW:
      g_value_set_int (value, priv->active_row);
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

  g_slist_foreach (priv->children, (GFunc) clutter_actor_unparent, NULL);
  g_slist_free (priv->children);
  priv->children = NULL;

  G_OBJECT_CLASS (nbtk_table_parent_class)->dispose (gobject);
}

static void
nbtk_table_homogeneous_allocate (ClutterActor          *self,
                                const ClutterActorBox *box,
                                gboolean               absolute_origin_changed)
{
  GSList *list;
  ClutterUnit col_width, row_height;
  gint row_spacing, col_spacing;
  NbtkPadding padding = { 0, };
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (self);

  col_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->col_spacing);
  row_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->row_spacing);

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  col_width = (box->x2 - box->x1 - padding.left - padding.right - (col_spacing * (priv->n_cols - 1))) / priv->n_cols;
  row_height = (box->y2 - box->y1 - padding.top - padding.bottom - (row_spacing * (priv->n_rows - 1))) / priv->n_rows;

  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, row_span, col_span;
      gboolean keep_ratio;
      ClutterChildMeta *meta;
      ClutterActor *child;
      ClutterActorBox childbox;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);
      g_object_get (meta, "column", &col, "row", &row,
                    "row-span", &row_span, "col-span", &col_span,
                    "keep-aspect-ratio", &keep_ratio, NULL);

      childbox.x1 = padding.left + (col_width + col_spacing) * col;
      childbox.x2 = childbox.x1 + (col_width * col_span) + (col_spacing * (col_span - 1));

      childbox.y1 = padding.top + (row_height + row_spacing) * row;
      childbox.y2 = childbox.y1 + (row_height * row_span) + (row_spacing * (row_span - 1));

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
              center_offset = ((childbox.x2 - childbox.x1) - new_width) / 2;
              childbox.x1 = childbox.x1 + center_offset;
              childbox.x2 = childbox.x1 + new_width;
            }
          else
            {
              /* center for new height */
              center_offset = ((childbox.y2 - childbox.y1) - new_height) / 2;
              childbox.y1 = childbox.y1 + center_offset;
              childbox.y2 = childbox.y1 + new_height;
            }

        }

      clutter_actor_allocate (child, &childbox, absolute_origin_changed);
    }

}

static void
nbtk_table_preferred_allocate (ClutterActor          *self,
                               const ClutterActorBox *box,
                               gboolean               absolute_origin_changed)
{
  GSList *list;
  gint row_spacing, col_spacing, extra_col_width, extra_row_height;
  gint total_min_width, total_min_height, i, table_width, table_height;
  gint *min_widths, *min_heights;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  NbtkPadding padding = { 0, };

  col_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->col_spacing);
  row_spacing = CLUTTER_UNITS_FROM_DEVICE (priv->row_spacing);

  min_widths = g_new0 (gint, priv->n_cols);
  min_heights = g_new0 (gint, priv->n_rows);

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  table_height = box->y2 - box->y1 - padding.top - padding.bottom;
  table_width = box->x2 - box->x1 - padding.right - padding.left;

  /* calculate minimum row widths and column heights */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col;
      gint h_min, h_pref, w_min, w_pref;
      ClutterChildMeta *meta;
      ClutterActor *child;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      g_object_get (meta, "column", &col, "row", &row, NULL);

      clutter_actor_get_preferred_size (child, &w_min, &h_min, &w_pref, &h_pref);
      if (w_pref > min_widths[col])
        min_widths[col] = w_pref;

      if (h_pref > min_heights[row])
        min_heights[row] = h_pref;
    }

  total_min_width = col_spacing * (priv->n_cols - 1);
  for (i = 0; i < priv->n_cols; i++)
    total_min_width += min_widths[i];

  total_min_height = row_spacing * (priv->n_rows - 1);
  for (i = 0; i < priv->n_rows; i++)
    total_min_height += min_heights[i];

  /* calculate the remaining space and distribute it evenly onto all rows/cols */
  extra_col_width = (table_width - total_min_width) / priv->n_cols;
  extra_row_height =  (table_height - total_min_height) / priv->n_rows;

  for (i = 0; i < priv->n_cols; i++)
    min_widths[i] += extra_col_width;

  for (i = 0; i < priv->n_rows; i++)
    min_heights[i] += extra_row_height;


  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row, col, row_span, col_span;
      gint col_width, row_height;
      gboolean keep_ratio;
      ClutterChildMeta *meta;
      ClutterActor *child;
      ClutterActorBox childbox;
      gint child_x, child_y;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);
      g_object_get (meta, "column", &col, "row", &row,
                    "row-span", &row_span, "col-span", &col_span,
                    "keep-aspect-ratio", &keep_ratio, NULL);

      /* initialise the width and height */
      col_width = min_widths[col];
      row_height = min_heights[row];

      /* add the widths of the spanned columns */
      for (i = col + 1; i < col + col_span; i++)
        {
          col_width += min_widths[i];
        }
      col_width += col_spacing * (col_span - 1);

      /* add the height of the spanned rows */
      for (i = row + 1; i < row + row_span; i++)
        {
          row_height += min_heights[i];
        }
      row_height += row_spacing * (row_span - 1);


      /* calculate child x */
      child_x = padding.left + col_spacing * col;
      for (i = 0; i < col; i++)
        child_x += min_widths[i];

      /* calculate child y */
      child_y = padding.top + row_spacing * row;
      for (i = 0; i < row; i++)
        child_y += min_heights[i];

      /* set up childbox */
      childbox.x1 = child_x;
      childbox.x2 = child_x + col_width;

      childbox.y1 = child_y;
      childbox.y2 = child_y + row_height;

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
              center_offset = ((childbox.x2 - childbox.x1) - new_width) / 2;
              childbox.x1 = childbox.x1 + center_offset;
              childbox.x2 = childbox.x1 + new_width;
            }
          else
            {
              /* center for new height */
              center_offset = ((childbox.y2 - childbox.y1) - new_height) / 2;
              childbox.y1 = childbox.y1 + center_offset;
              childbox.y2 = childbox.y1 + new_height;
            }
        }

      clutter_actor_allocate (child, &childbox, absolute_origin_changed);

    }

  g_free (min_widths);
  g_free (min_heights);
}

static void
nbtk_table_allocate (ClutterActor          *self,
                     const ClutterActorBox *box,
                     gboolean               absolute_origin_changed)
{
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->allocate (self, box, absolute_origin_changed);

  g_return_if_fail (priv->n_cols != 0 || priv->n_rows != 0);

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
  ClutterUnit *min_widths;
  ClutterUnit total_min_width;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  NbtkPadding padding = { 0, };
  GSList *list;
  gint i;

  min_widths = g_new0 (ClutterUnit, priv->n_cols);

  /* calculate minimum row widths */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint col;
      gint w_min, w_pref;
      ClutterChildMeta *meta;
      ClutterActor *child;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      g_object_get (meta, "column", &col, NULL);

      clutter_actor_get_preferred_width (child, -1, &w_min, &w_pref);
      if (w_pref > min_widths[col])
        min_widths[col] = w_pref;

    }

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  total_min_width = padding.left + padding.right
                    + (priv->n_cols - 1) * CLUTTER_UNITS_FROM_DEVICE (priv->col_spacing);
  for (i = 0; i < priv->n_cols; i++)
    total_min_width += min_widths[i];

  g_free (min_widths);

  if (min_width_p)
    *min_width_p = total_min_width;
  if (natural_width_p)
    *natural_width_p = total_min_width;
}

static void
nbtk_table_get_preferred_height (ClutterActor *self,
                                 ClutterUnit   for_width,
                                 ClutterUnit  *min_height_p,
                                 ClutterUnit  *natural_height_p)
{
  ClutterUnit *min_heights;
  ClutterUnit total_min_height;
  NbtkTablePrivate *priv = NBTK_TABLE (self)->priv;
  NbtkPadding padding = { 0, };
  GSList *list;
  gint i;

  min_heights = g_new0 (ClutterUnit, priv->n_rows);

  /* calculate minimum row heights */
  for (list = priv->children; list; list = g_slist_next (list))
    {
      gint row;
      gint min, pref;
      ClutterChildMeta *meta;
      ClutterActor *child;

      child = CLUTTER_ACTOR (list->data);

      meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

      g_object_get (meta, "row", &row, NULL);

      clutter_actor_get_preferred_height (child, -1, &min, &pref);
      if (pref > min_heights[row])
        min_heights[row] = pref;
    }

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

  total_min_height = padding.top + padding.bottom
                     + (priv->n_rows - 1) * CLUTTER_UNITS_FROM_DEVICE (priv->row_spacing);

  for (i = 0; i < priv->n_rows; i++)
    total_min_height += min_heights[i];

  g_free (min_heights);

  if (min_height_p)
    *min_height_p = total_min_height;
  if (natural_height_p)
    *natural_height_p = total_min_height;
}

static void
nbtk_table_paint (ClutterActor *self)
{
  NbtkTablePrivate *priv = NBTK_TABLE_GET_PRIVATE (self);
  GSList *list;
  NbtkPadding padding = { 0, };
  gint p_left, p_right, p_top, p_bottom;

  nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);
  p_left = CLUTTER_UNITS_TO_INT (padding.left);
  p_right = CLUTTER_UNITS_TO_INT (padding.right);
  p_top = CLUTTER_UNITS_TO_INT (padding.top);
  p_bottom = CLUTTER_UNITS_TO_INT (padding.bottom);

  if (priv->bg_color)
    {
      ClutterActorBox allocation = { 0, };
      ClutterColor bg_color = *priv->bg_color;
      guint w, h;

      bg_color.alpha = clutter_actor_get_paint_opacity (self)
                       * bg_color.alpha / 255;

      clutter_actor_get_allocation_box (self, &allocation);

      w = CLUTTER_UNITS_TO_DEVICE (allocation.x2 - allocation.x1);
      h = CLUTTER_UNITS_TO_DEVICE (allocation.y2 - allocation.y1);

      cogl_color (&bg_color);
      cogl_rectangle (0, 0, w, h);
    }

  if (priv->active_col >= 0)
    {
      gint col_width;
      guint w, h;

      clutter_actor_get_size (self, &w, &h);

      col_width = (w - p_left - p_right) / priv->n_cols;

      cogl_color (priv->active_color);
      priv->active_color->alpha = clutter_actor_get_paint_opacity (self)
                                 * priv->active_color->alpha / 255;
      
      cogl_rectangle (p_left + col_width * priv->active_col,
                      p_top,
                      col_width,
                      h - p_bottom - p_top);
    }

  if (priv->active_row >= 0)
    {
      gint row_height;
      guint w, h;

      clutter_actor_get_size (self, &w, &h);

      row_height = (h - p_top - p_bottom) / priv->n_rows;

      cogl_color (priv->active_color);
      priv->active_color->alpha = clutter_actor_get_paint_opacity (self)
                                  * priv->active_color->alpha / 255;
      
      cogl_rectangle (p_left,
                      row_height * priv->active_row,
                      w - p_left - p_right,
                      row_height);
    }


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

  /* Chain up so we get a bounding box painted (if we are reactive) */
  CLUTTER_ACTOR_CLASS (nbtk_table_parent_class)->pick (self, color);

  for (list = priv->children; list; list = g_slist_next (list))
    {
      if (CLUTTER_ACTOR_IS_VISIBLE (list->data))
        clutter_actor_paint (CLUTTER_ACTOR (list->data));
    }
}

static void
nbtk_table_class_init (NbtkTableClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *nbtk_widget_class = NBTK_WIDGET_CLASS (klass);

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

  nbtk_widget_class->style_changed = nbtk_table_style_changed;

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

  pspec = g_param_spec_int ("active-row",
                            "Active Row",
                            "Row to highlight",
                            0, G_MAXINT, 0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_ACTIVE_ROW,
                                   pspec);

  pspec = g_param_spec_int ("active-col",
                            "Active Column",
                            "Column to highlight",
                            0, G_MAXINT, 0,
                            NBTK_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_ACTIVE_ROW,
                                   pspec);
}

static void
nbtk_table_init (NbtkTable *table)
{
  table->priv = NBTK_TABLE_GET_PRIVATE (table);

  table->priv->n_cols = 0;
  table->priv->n_rows = 0;

  table->priv->active_row = -1;
  table->priv->active_col = -1;
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

  priv = NBTK_TABLE_GET_PRIVATE (table);

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

  priv = NBTK_TABLE_GET_PRIVATE (table);

  priv->row_spacing = spacing;
}

/**
 * nbtk_table_get_row_spacing
 * @table: a #NbtkTable
 *
 * Gets the amount of spacing between rows.
 */
gint
nbtk_table_get_row_spacing (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE_GET_PRIVATE (table);

  return priv->row_spacing;
}

/**
 * nbtk_table_get_col_spacing
 * @table: a #NbtkTable
 *
 * Gets the amount of spacing between columns.
 */
gint
nbtk_table_get_col_spacing (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE_GET_PRIVATE (table);

  return priv->col_spacing;
}

/**
 * nbtk_table_set_active_col:
 * @table: a #NbtkTable
 * @column: column number to set as active
 *
 * Sets the active column in the table. An active column is hilighted with the
 * active color. A value of -1 removes the active column.
 */
void
nbtk_table_set_active_col (NbtkTable *table,
                           gint       column)
{
  NbtkTablePrivate *priv;

  g_return_if_fail (NBTK_IS_TABLE (table));

  priv = NBTK_TABLE_GET_PRIVATE (table);

  g_return_if_fail (column >= -1 && column <= priv->n_cols);

  priv->active_col = column;
}

/**
 * nbtk_table_set_active_row:
 * @table: a #NbtkTable
 * @row: row number to set as active
 *
 * Sets the active row in the table. An active row is hilighted with the active
 * color. A value of -1 removes the active row.
 */
void
nbtk_table_set_active_row (NbtkTable *table,
                           gint       row)
{
  NbtkTablePrivate *priv;

  g_return_if_fail (NBTK_IS_TABLE (table));

  priv = NBTK_TABLE_GET_PRIVATE (table);

  g_return_if_fail (row >= -1 && row <= priv->n_rows);

  priv->active_row = row;
}

/**
 * nbtk_table_get_active_row:
 * @table: a #NbtkTable
 *
 * Gets the active row in the table.
 */
gint
nbtk_table_get_active_row (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE_GET_PRIVATE (table);

  return priv->active_row;
}

/**
 * nbtk_table_get_active_col:
 * @table: a #NbtkTable
 *
 * Gets the active column in the table.
 */
gint
nbtk_table_get_active_col (NbtkTable *table)
{
  NbtkTablePrivate *priv;

  g_return_val_if_fail (NBTK_IS_TABLE (table), -1);
  priv = NBTK_TABLE_GET_PRIVATE (table);

  return priv->active_col;
}


void
nbtk_table_add_actor (NbtkTable   *table,
                     ClutterActor *actor,
                     gint          row,
                     gint          column)
{
  NbtkTablePrivate *priv;
  ClutterChildMeta *child;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));
  g_return_if_fail (row >= 0);
  g_return_if_fail (column >= 0);

  priv = NBTK_TABLE_GET_PRIVATE (table);

  clutter_container_add_actor (CLUTTER_CONTAINER (table), actor);
  child = clutter_container_get_child_meta (CLUTTER_CONTAINER (table), actor);

  g_object_set (child, "row", row, "column", column, NULL);

  priv->n_cols = MAX (priv->n_cols, column + 1);
  priv->n_rows = MAX (priv->n_rows, row + 1);

  priv->children = g_slist_append (priv->children, actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (table));
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
