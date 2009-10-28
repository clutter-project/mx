/*
 * mx-grid.c: Reflowing grid layout container for mx.
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
 *
 * Written by: Øyvind Kolås <pippin@linux.intel.com>
 * Ported to mx by: Robert Staudinger <robsta@openedhand.com>
 * Scrollable support added by: Thomas Wood <thomas.wood@intel.com>
 *
 */

/* TODO:
 *
 * - Better names for properties.
 * - Caching layouted positions? (perhaps needed for huge collections)
 * - More comments / overall concept on how the layouting is done.
 * - Allow more layout directions than just row major / column major.
 */
/**
 * SECTION:mx-grid
 * @short_description: a flow layout container
 *
 * #MxGrid is a layout container that arranges its children by placing them
 * in a single line and wrapping round to a new line when the edge of the
 * container is reached.
 */

#include <string.h>

#include "mx-scrollable.h"
#include "mx-grid.h"
#include "mx-stylable.h"

typedef struct _MxGridActorData MxGridActorData;

static void mx_grid_dispose             (GObject *object);
static void mx_grid_finalize            (GObject *object);

static void mx_grid_set_property        (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);
static void mx_grid_get_property        (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec);

static void clutter_container_iface_init  (ClutterContainerIface *iface);

static void mx_grid_real_add            (ClutterContainer *container,
                                         ClutterActor     *actor);
static void mx_grid_real_remove         (ClutterContainer *container,
                                         ClutterActor     *actor);
static void mx_grid_real_foreach        (ClutterContainer *container,
                                         ClutterCallback   callback,
                                         gpointer          user_data);
static void mx_grid_real_raise          (ClutterContainer *container,
                                         ClutterActor     *actor,
                                         ClutterActor     *sibling);
static void mx_grid_real_lower          (ClutterContainer *container,
                                         ClutterActor     *actor,
                                         ClutterActor     *sibling);
static void
mx_grid_real_sort_depth_order (ClutterContainer *container);

static void
mx_grid_free_actor_data (gpointer data);

static void mx_grid_paint (ClutterActor *actor);

static void mx_grid_pick (ClutterActor       *actor,
                          const ClutterColor *color);

static void
mx_grid_get_preferred_width (ClutterActor *self,
                             gfloat        for_height,
                             gfloat       *min_width_p,
                             gfloat       *natural_width_p);

static void
mx_grid_get_preferred_height (ClutterActor *self,
                              gfloat        for_width,
                              gfloat       *min_height_p,
                              gfloat       *natural_height_p);

static void mx_grid_allocate (ClutterActor          *self,
                              const ClutterActorBox *box,
                              ClutterAllocationFlags flags);

static void
mx_grid_do_allocate (ClutterActor          *self,
                     const ClutterActorBox *box,
                     ClutterAllocationFlags flags,
                     gboolean               calculate_extents_only,
                     gfloat                *actual_width,
                     gfloat                *actual_height);

static void scrollable_interface_init (MxScrollableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (MxGrid, mx_grid,
                         MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SCROLLABLE,
                                                scrollable_interface_init));

#define MX_GRID_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_GRID, \
                                MxGridPrivate))

struct _MxGridPrivate
{
  GHashTable   *hash_table;
  GList        *list;

  gboolean      homogenous_rows;
  gboolean      homogenous_columns;
  gboolean      end_align;
  gfloat        column_gap, row_gap;
  gdouble       valign, halign;

  gboolean      column_major;

  gboolean      first_of_batch;
  gfloat        a_current_sum, a_wrap;
  gfloat        max_extent_a;
  gfloat        max_extent_b;

  gint          max_stride;

  MxAdjustment *hadjustment;
  MxAdjustment *vadjustment;

  gboolean      allocate_hidden;
};

enum
{
  PROP_0,
  PROP_HOMOGENOUS_ROWS,
  PROP_HOMOGENOUS_COLUMNS,
  PROP_ROW_GAP,
  PROP_COLUMN_GAP,
  PROP_VALIGN,
  PROP_HALIGN,
  PROP_END_ALIGN,
  PROP_COLUMN_MAJOR,
  PROP_HADJUST,
  PROP_VADJUST,
  PROP_MAX_STRIDE,
  PROP_ALLOCATE_HIDDEN
};

struct _MxGridActorData
{
  gboolean xpos_set,   ypos_set;
  gfloat   xpos,       ypos;
  gfloat   pref_width, pref_height;
};

static void
ensure_children_are_visible (MxGrid *grid)
{
  MxGridPrivate *priv = grid->priv;
  GList *child_item;
  gfloat x, y;
  ClutterActorBox grid_b;

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  clutter_actor_get_allocation_box ((ClutterActor *) grid, &grid_b);
  grid_b.x2 = (grid_b.x2 - grid_b.x1) + x;
  grid_b.x1 = 0;
  grid_b.y2 = (grid_b.y2 - grid_b.y1) + y;
  grid_b.y1 = 0;

  for (child_item = priv->list;
       child_item != NULL;
       child_item = child_item->next)
    {
      ClutterActor *child = child_item->data;
      ClutterActorBox child_b;

      g_assert (child != NULL);

      /* check if the child is "on screen" */
      clutter_actor_get_allocation_box (child, &child_b);

      if ((child_b.x1 < grid_b.x2)
          && (child_b.x2 > grid_b.x1)
          && (child_b.y1 < grid_b.y2)
          && (child_b.y2 > grid_b.y1))
        {
          clutter_actor_show (child);
        } else {
          clutter_actor_hide (child);
        }
    }
}

/* scrollable interface */
static void
adjustment_value_notify_cb (MxAdjustment *adjustment,
                            GParamSpec   *pspec,
                            MxGrid       *grid)
{
  MxGridPrivate *priv = grid->priv;

  /* Mark children as visible if they are inside the visible area */
  if (priv->allocate_hidden)
    ensure_children_are_visible (grid);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (grid));
}

static void
scrollable_set_adjustments (MxScrollable *scrollable,
                            MxAdjustment *hadjustment,
                            MxAdjustment *vadjustment)
{
  MxGridPrivate *priv = MX_GRID (scrollable)->priv;

  if (hadjustment != priv->hadjustment)
    {
      if (priv->hadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->hadjustment,
                                                adjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->hadjustment);
        }

      if (hadjustment)
        {
          g_object_ref (hadjustment);
          g_signal_connect (hadjustment, "notify::value",
                            G_CALLBACK (adjustment_value_notify_cb),
                            scrollable);
        }

      priv->hadjustment = hadjustment;
    }

  if (vadjustment != priv->vadjustment)
    {
      if (priv->vadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->vadjustment,
                                                adjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->vadjustment);
        }

      if (vadjustment)
        {
          g_object_ref (vadjustment);
          g_signal_connect (vadjustment, "notify::value",
                            G_CALLBACK (adjustment_value_notify_cb),
                            scrollable);
        }

      priv->vadjustment = vadjustment;
    }
}

static void
scrollable_get_adjustments (MxScrollable  *scrollable,
                            MxAdjustment **hadjustment,
                            MxAdjustment **vadjustment)
{
  MxGridPrivate *priv;

  g_return_if_fail (MX_IS_GRID (scrollable));

  priv = ((MxGrid *) scrollable)->priv;

  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          MxAdjustment *adjustment;

          /* create an initial adjustment. this is filled with correct values
           * as soon as allocate() is called */

          adjustment = mx_adjustment_new (0.0,
                                          0.0,
                                          1.0,
                                          1.0,
                                          1.0,
                                          1.0);

          scrollable_set_adjustments (scrollable,
                                      adjustment,
                                      priv->vadjustment);

          *hadjustment = adjustment;
        }
    }

  if (vadjustment)
    {
      if (priv->vadjustment)
        *vadjustment = priv->vadjustment;
      else
        {
          MxAdjustment *adjustment;

          /* create an initial adjustment. this is filled with correct values
           * as soon as allocate() is called */

          adjustment = mx_adjustment_new (0.0,
                                          0.0,
                                          1.0,
                                          1.0,
                                          1.0,
                                          1.0);

          scrollable_set_adjustments (scrollable,
                                      priv->hadjustment,
                                      adjustment);

          *vadjustment = adjustment;
        }
    }
}

static void
scrollable_interface_init (MxScrollableInterface *iface)
{
  iface->set_adjustments = scrollable_set_adjustments;
  iface->get_adjustments = scrollable_get_adjustments;
}

static void
mx_grid_apply_transform (ClutterActor *a,
                         CoglMatrix   *m)
{
  MxGridPrivate *priv = MX_GRID (a)->priv;
  gdouble x, y;

  CLUTTER_ACTOR_CLASS (mx_grid_parent_class)->apply_transform (a, m);

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  cogl_matrix_translate (m , (int) -x, (int) -y, 0);
}

static void
mx_grid_class_init (MxGridClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  ClutterActorClass *actor_class = (ClutterActorClass *) klass;

  GParamSpec *pspec;

  gobject_class->dispose = mx_grid_dispose;
  gobject_class->finalize = mx_grid_finalize;

  gobject_class->set_property = mx_grid_set_property;
  gobject_class->get_property = mx_grid_get_property;

  actor_class->paint                = mx_grid_paint;
  actor_class->pick                 = mx_grid_pick;
  actor_class->get_preferred_width  = mx_grid_get_preferred_width;
  actor_class->get_preferred_height = mx_grid_get_preferred_height;
  actor_class->allocate             = mx_grid_allocate;
  actor_class->apply_transform      = mx_grid_apply_transform;

  g_type_class_add_private (klass, sizeof (MxGridPrivate));


  pspec = g_param_spec_float ("row-gap",
                              "Row gap",
                              "gap between rows in the layout",
                              0, G_MAXFLOAT,
                              0,
                              G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_ROW_GAP, pspec);

  pspec = g_param_spec_float ("column-gap",
                              "Column gap",
                              "gap between columns in the layout",
                              0, G_MAXFLOAT,
                              0,
                              G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_COLUMN_GAP, pspec);


  pspec = g_param_spec_boolean ("homogenous-rows",
                                "homogenous rows",
                                "Should all rows have the same height?",
                                FALSE,
                                G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_HOMOGENOUS_ROWS, pspec);

  pspec = g_param_spec_boolean ("homogenous-columns",
                                "homogenous columns",
                                "Should all columns have the same height?",
                                FALSE,
                                G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class,
                                   PROP_HOMOGENOUS_COLUMNS,
                                   pspec);

  pspec = g_param_spec_boolean ("column-major",
                                "column-major",
                                "Do a column filling first instead of row"
                                " filling first",
                                FALSE,
                                G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_COLUMN_MAJOR, pspec);

  pspec = g_param_spec_boolean ("end-align",
                                "end-align",
                                "Right/bottom aligned rows/columns",
                                FALSE,
                                G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_END_ALIGN, pspec);


  pspec = g_param_spec_double ("valign",
                               "Vertical align",
                               "Vertical alignment of items within cells",
                               0.0, 1.0, 0.0,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class,PROP_VALIGN, pspec);

  pspec = g_param_spec_double ("halign",
                               "Horizontal align",
                               "Horizontal alignment of items within cells",
                               0.0, 1.0, 0.0,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_HALIGN, pspec);

  pspec = g_param_spec_int ("max-stride",
                            "Maximum stride",
                            "Maximum number of rows or columns, depending"
                            " on orientation",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_MAX_STRIDE, pspec);

  g_object_class_override_property (gobject_class,
                                    PROP_HADJUST,
                                    "hadjustment");

  g_object_class_override_property (gobject_class,
                                    PROP_VADJUST,
                                    "vadjustment");

  pspec = g_param_spec_boolean ("allocate-hidden",
                                "Allocate children that are not visible",
                                "This flag determines whether hidden children"
                                " should be treated allocated or whether they"
                                " should be skipped from the allocation and"
                                " treated as if they are not present",
                                FALSE,
                                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_ALLOCATE_HIDDEN, pspec);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add              = mx_grid_real_add;
  iface->remove           = mx_grid_real_remove;
  iface->foreach          = mx_grid_real_foreach;
  iface->raise            = mx_grid_real_raise;
  iface->lower            = mx_grid_real_lower;
  iface->sort_depth_order = mx_grid_real_sort_depth_order;
}

static void
mx_grid_init (MxGrid *self)
{
  MxGridPrivate *priv;

  self->priv = priv = MX_GRID_GET_PRIVATE (self);

  /* do not unref in the hashtable, the reference is for now kept by the list
   * (double bookkeeping sucks)
   */
  priv->hash_table
    = g_hash_table_new_full (g_direct_hash,
                             g_direct_equal,
                             NULL,
                             mx_grid_free_actor_data);

  /* allocate_hidden defaults to FALSE. This is the original MxGrid
   * behaviour
   */
  priv->allocate_hidden = FALSE;
}

static void
mx_grid_dispose (GObject *object)
{
  MxGrid *self = (MxGrid *) object;
  MxGridPrivate *priv;

  priv = self->priv;

  /* Destroy all of the children. This will cause them to be removed
     from the container and unparented */
  clutter_container_foreach (CLUTTER_CONTAINER (object),
                             (ClutterCallback) clutter_actor_destroy,
                             NULL);

  G_OBJECT_CLASS (mx_grid_parent_class)->dispose (object);
}

static void
mx_grid_finalize (GObject *object)
{
  MxGrid *self = (MxGrid *) object;
  MxGridPrivate *priv = self->priv;

  g_hash_table_destroy (priv->hash_table);

  G_OBJECT_CLASS (mx_grid_parent_class)->finalize (object);
}


void
mx_grid_set_end_align (MxGrid  *self,
                       gboolean value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->end_align = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gboolean
mx_grid_get_end_align (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->end_align;
}

void
mx_grid_set_homogenous_rows (MxGrid  *self,
                             gboolean value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->homogenous_rows = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gboolean
mx_grid_get_homogenous_rows (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->homogenous_rows;
}


void
mx_grid_set_homogenous_columns (MxGrid  *self,
                                gboolean value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->homogenous_columns = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}


gboolean
mx_grid_get_homogenous_columns (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->homogenous_columns;
}


void
mx_grid_set_column_major (MxGrid  *self,
                          gboolean value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->column_major = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gboolean
mx_grid_get_column_major (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->column_major;
}

void
mx_grid_set_column_gap (MxGrid *self,
                        gfloat  value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->column_gap = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gfloat
mx_grid_get_column_gap (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->column_gap;
}



void
mx_grid_set_row_gap (MxGrid *self,
                     gfloat  value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->row_gap = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gfloat
mx_grid_get_row_gap (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->row_gap;
}


void
mx_grid_set_valign (MxGrid *self,
                    gdouble value)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->valign = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gdouble
mx_grid_get_valign (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->valign;
}



void
mx_grid_set_halign (MxGrid *self,
                    gdouble value)

{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  priv->halign = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gdouble
mx_grid_get_halign (MxGrid *self)
{
  MxGridPrivate *priv = MX_GRID_GET_PRIVATE (self);
  return priv->halign;
}

void
mx_grid_set_max_stride (MxGrid *self,
                        gint    value)
{
  g_return_if_fail (MX_IS_GRID (self));

  self->priv->max_stride = value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}

gint
mx_grid_get_max_stride (MxGrid *self)
{
  g_return_val_if_fail (MX_IS_GRID (self), 0);

  return self->priv->max_stride;
}

static void
mx_grid_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  MxGrid *grid = MX_GRID (object);

  MxGridPrivate *priv;

  priv = MX_GRID_GET_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_END_ALIGN:
      mx_grid_set_end_align (grid, g_value_get_boolean (value));
      break;
    case PROP_HOMOGENOUS_ROWS:
      mx_grid_set_homogenous_rows (grid, g_value_get_boolean (value));
      break;
    case PROP_HOMOGENOUS_COLUMNS:
      mx_grid_set_homogenous_columns (grid, g_value_get_boolean (value));
      break;
    case PROP_COLUMN_MAJOR:
      mx_grid_set_column_major (grid, g_value_get_boolean (value));
      break;
    case PROP_COLUMN_GAP:
      mx_grid_set_column_gap (grid, g_value_get_float (value));
      break;
    case PROP_ROW_GAP:
      mx_grid_set_row_gap (grid, g_value_get_float (value));
      break;
    case PROP_VALIGN:
      mx_grid_set_valign (grid, g_value_get_double (value));
      break;
    case PROP_HALIGN:
      mx_grid_set_halign (grid, g_value_get_double (value));
      break;
    case PROP_HADJUST:
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  g_value_get_object (value),
                                  priv->vadjustment);
      break;
    case PROP_VADJUST:
      scrollable_set_adjustments (MX_SCROLLABLE (object),
                                  priv->hadjustment,
                                  g_value_get_object (value));
      break;
    case PROP_MAX_STRIDE:
      mx_grid_set_max_stride (grid, g_value_get_int (value));
      break;
    case PROP_ALLOCATE_HIDDEN:
      priv->allocate_hidden = g_value_get_boolean (value);
      clutter_actor_queue_relayout ((ClutterActor *) object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
mx_grid_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  MxAdjustment *adjustment;
  MxGrid *grid = MX_GRID (object);

  MxGridPrivate *priv;

  priv = MX_GRID_GET_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_HOMOGENOUS_ROWS:
      g_value_set_boolean (value, mx_grid_get_homogenous_rows (grid));
      break;
    case PROP_HOMOGENOUS_COLUMNS:
      g_value_set_boolean (value, mx_grid_get_homogenous_columns (grid));
      break;
    case PROP_END_ALIGN:
      g_value_set_boolean (value, mx_grid_get_end_align (grid));
      break;
    case PROP_COLUMN_MAJOR:
      g_value_set_boolean (value, mx_grid_get_column_major (grid));
      break;
    case PROP_COLUMN_GAP:
      g_value_set_float (value, mx_grid_get_column_gap (grid));
      break;
    case PROP_ROW_GAP:
      g_value_set_float (value, mx_grid_get_row_gap (grid));
      break;
    case PROP_VALIGN:
      g_value_set_double (value, mx_grid_get_valign (grid));
      break;
    case PROP_HALIGN:
      g_value_set_double (value, mx_grid_get_halign (grid));
      break;
    case PROP_HADJUST:
      scrollable_get_adjustments (MX_SCROLLABLE (grid), &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;
    case PROP_VADJUST:
      scrollable_get_adjustments (MX_SCROLLABLE (grid), NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;
    case PROP_MAX_STRIDE:
      g_value_set_int (value, mx_grid_get_max_stride (grid));
      break;
    case PROP_ALLOCATE_HIDDEN:
      g_value_set_boolean (value, priv->allocate_hidden);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


static void
mx_grid_free_actor_data (gpointer data)
{
  g_slice_free (MxGridActorData, data);
}

ClutterActor *
mx_grid_new (void)
{
  MxWidget *self = g_object_new (MX_TYPE_GRID, NULL);

  return self;
}

static void
mx_grid_real_add (ClutterContainer *container,
                  ClutterActor     *actor)
{
  MxGridPrivate *priv;
  MxGridActorData *data;

  g_return_if_fail (MX_IS_GRID (container));

  priv = MX_GRID (container)->priv;

  g_object_ref (actor);

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  data = g_slice_alloc0 (sizeof (MxGridActorData));

  priv->list = g_list_append (priv->list, actor);
  g_hash_table_insert (priv->hash_table, actor, data);

  g_signal_emit_by_name (container, "actor-added", actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (container));

  g_object_unref (actor);
}

static void
mx_grid_real_remove (ClutterContainer *container,
                     ClutterActor     *actor)
{
  MxGrid *layout = MX_GRID (container);
  MxGridPrivate *priv = layout->priv;

  g_object_ref (actor);

  if (g_hash_table_remove (priv->hash_table, actor))
    {
      clutter_actor_unparent (actor);

      clutter_actor_queue_relayout (CLUTTER_ACTOR (layout));

      g_signal_emit_by_name (container, "actor-removed", actor);

      if (!priv->allocate_hidden && !CLUTTER_ACTOR_IS_VISIBLE (actor))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (layout));
    }
  priv->list = g_list_remove (priv->list, actor);

  g_object_unref (actor);
}

static void
mx_grid_real_foreach (ClutterContainer *container,
                      ClutterCallback   callback,
                      gpointer          user_data)
{
  MxGrid *layout = MX_GRID (container);
  MxGridPrivate *priv = layout->priv;

  g_list_foreach (priv->list, (GFunc) callback, user_data);
}

static void
mx_grid_real_raise (ClutterContainer *container,
                    ClutterActor     *actor,
                    ClutterActor     *sibling)
{
  /* STUB */
}

static void
mx_grid_real_lower (ClutterContainer *container,
                    ClutterActor     *actor,
                    ClutterActor     *sibling)
{
  /* STUB */
}

static void
mx_grid_real_sort_depth_order (ClutterContainer *container)
{
  /* STUB */
}

static void
mx_grid_paint (ClutterActor *actor)
{
  MxGrid *layout = (MxGrid *) actor;
  MxGridPrivate *priv = layout->priv;
  GList *child_item;
  gfloat x, y;
  ClutterActorBox grid_b;

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  CLUTTER_ACTOR_CLASS (mx_grid_parent_class)->paint (actor);

  clutter_actor_get_allocation_box (actor, &grid_b);
  grid_b.x2 = (grid_b.x2 - grid_b.x1) + x;
  grid_b.x1 = x;
  grid_b.y2 = (grid_b.y2 - grid_b.y1) + y;
  grid_b.y1 = y;

  for (child_item = priv->list;
       child_item != NULL;
       child_item = child_item->next)
    {
      ClutterActor *child = child_item->data;
      ClutterActorBox child_b;

      g_assert (child != NULL);

      /* ensure the child is "on screen" */
      clutter_actor_get_allocation_box (CLUTTER_ACTOR (child), &child_b);

      if ((child_b.x1 < grid_b.x2)
          && (child_b.x2 > grid_b.x1)
          && (child_b.y1 < grid_b.y2)
          && (child_b.y2 > grid_b.y1)
          && CLUTTER_ACTOR_IS_VISIBLE (child))
        {
          clutter_actor_paint (child);
        }
    }
}

static void
mx_grid_pick (ClutterActor       *actor,
              const ClutterColor *color)
{
  MxGrid *layout = (MxGrid *) actor;
  MxGridPrivate *priv = layout->priv;
  GList *child_item;
  gfloat x, y;
  ClutterActorBox grid_b;

  if (priv->hadjustment)
    x = mx_adjustment_get_value (priv->hadjustment);
  else
    x = 0;

  if (priv->vadjustment)
    y = mx_adjustment_get_value (priv->vadjustment);
  else
    y = 0;

  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (mx_grid_parent_class)->pick (actor, color);


  clutter_actor_get_allocation_box (actor, &grid_b);
  grid_b.x2 = (grid_b.x2 - grid_b.x1) + x;
  grid_b.x1 = x;
  grid_b.y2 = (grid_b.y2 - grid_b.y1) + y;
  grid_b.y1 = y;

  for (child_item = priv->list;
       child_item != NULL;
       child_item = child_item->next)
    {
      ClutterActor *child = child_item->data;
      ClutterActorBox child_b;

      g_assert (child != NULL);

      /* ensure the child is "on screen" */
      clutter_actor_get_allocation_box (CLUTTER_ACTOR (child), &child_b);

      if ((child_b.x1 < grid_b.x2)
          && (child_b.x2 > grid_b.x1)
          && (child_b.y1 < grid_b.y2)
          && (child_b.y2 > grid_b.y1)
          && CLUTTER_ACTOR_IS_VISIBLE (child))
        {
          clutter_actor_paint (child);
        }
    }
}

static void
mx_grid_get_preferred_width (ClutterActor *self,
                             gfloat        for_height,
                             gfloat       *min_width_p,
                             gfloat       *natural_width_p)
{
  gfloat actual_width, actual_height;
  ClutterActorBox box;

  box.x1 = 0;
  box.y1 = 0;
  box.x2 = G_MAXFLOAT;
  box.y2 = for_height;

  mx_grid_do_allocate (self, &box, FALSE,
                       TRUE, &actual_width, &actual_height);

  if (min_width_p)
    *min_width_p = actual_width;
  if (natural_width_p)
    *natural_width_p = actual_width;
}

static void
mx_grid_get_preferred_height (ClutterActor *self,
                              gfloat        for_width,
                              gfloat       *min_height_p,
                              gfloat       *natural_height_p)
{
  gfloat actual_width, actual_height;
  ClutterActorBox box;

  box.x1 = 0;
  box.y1 = 0;
  box.x2 = for_width;
  box.y2 = G_MAXFLOAT;

  mx_grid_do_allocate (self, &box, FALSE,
                       TRUE, &actual_width, &actual_height);

  if (min_height_p)
    *min_height_p = actual_height;
  if (natural_height_p)
    *natural_height_p = actual_height;
}

static gfloat
compute_row_height (GList         *siblings,
                    gfloat         best_yet,
                    gfloat         current_a,
                    MxGridPrivate *priv)
{
  GList *l;

  gboolean homogenous_a;
  gboolean homogenous_b;
  gfloat gap;

  if (priv->column_major)
    {
      homogenous_b = priv->homogenous_columns;
      homogenous_a = priv->homogenous_rows;
      gap          = priv->row_gap;
    }
  else
    {
      homogenous_a = priv->homogenous_columns;
      homogenous_b = priv->homogenous_rows;
      gap          = priv->column_gap;
    }

  for (l = siblings; l != NULL; l = l->next)
    {
      ClutterActor *child = l->data;
      gfloat natural_width, natural_height;

      /* each child will get as much space as they require */
      clutter_actor_get_preferred_size (CLUTTER_ACTOR (child),
                                        NULL, NULL,
                                        &natural_width, &natural_height);

      if (priv->column_major)
        {
          gfloat temp = natural_height;
          natural_height = natural_width;
          natural_width = temp;
        }

      /* if the primary axis is homogenous, each additional item is the same
       * width */
      if (homogenous_a)
        natural_width = priv->max_extent_a;

      if (natural_height > best_yet)
        best_yet = natural_height;

      /* if the child is overflowing, we wrap to next line */
      if (current_a + natural_width + gap > priv->a_wrap)
        {
          return best_yet;
        }
      current_a += natural_width + gap;
    }
  return best_yet;
}




static gfloat
compute_row_start (GList         *siblings,
                   gfloat         start_x,
                   MxGridPrivate *priv)
{
  gfloat current_a = start_x;
  GList *l;

  gboolean homogenous_a;
  gboolean homogenous_b;
  gfloat gap;

  if (priv->column_major)
    {
      homogenous_b = priv->homogenous_columns;
      homogenous_a = priv->homogenous_rows;
      gap          = priv->row_gap;
    }
  else
    {
      homogenous_a = priv->homogenous_columns;
      homogenous_b = priv->homogenous_rows;
      gap          = priv->column_gap;
    }

  for (l = siblings; l != NULL; l = l->next)
    {
      ClutterActor *child = l->data;
      gfloat natural_width, natural_height;

      /* each child will get as much space as they require */
      clutter_actor_get_preferred_size (CLUTTER_ACTOR (child),
                                        NULL, NULL,
                                        &natural_width, &natural_height);


      if (priv->column_major)
        natural_width = natural_height;

      /* if the primary axis is homogenous, each additional item is the same width */
      if (homogenous_a)
        natural_width = priv->max_extent_a;

      /* if the child is overflowing, we wrap to next line */
      if (current_a + natural_width + gap > priv->a_wrap)
        {
          if (current_a == start_x)
            return start_x;
          return (priv->a_wrap - current_a);
        }
      current_a += natural_width + gap;
    }
  return (priv->a_wrap - current_a);
}

static void
mx_grid_do_allocate (ClutterActor          *self,
                     const ClutterActorBox *box,
                     ClutterAllocationFlags flags,
                     gboolean               calculate_extents_only,
                     gfloat                *actual_width,
                     gfloat                *actual_height)
{
  MxGrid *layout = (MxGrid *) self;
  MxGridPrivate *priv = layout->priv;
  MxPadding padding;

  gfloat current_a;
  gfloat current_b;
  gfloat next_b;
  gfloat agap;
  gfloat bgap;

  gboolean homogenous_a;
  gboolean homogenous_b;
  gdouble aalign;
  gdouble balign;
  int current_stride;

  mx_widget_get_padding (MX_WIDGET (self), &padding);

  if (actual_width)
    *actual_width = 0;

  if (actual_height)
    *actual_height = 0;

  current_a = current_b = next_b = 0;

  GList *iter;

  if (priv->column_major)
    {
      priv->a_wrap = box->y2 - box->y1 - padding.top - padding.bottom;
      homogenous_b = priv->homogenous_columns;
      homogenous_a = priv->homogenous_rows;
      aalign = priv->valign;
      balign = priv->halign;
      agap          = priv->row_gap;
      bgap          = priv->column_gap;
    }
  else
    {
      priv->a_wrap = box->x2 - box->x1 - padding.left - padding.right;
      homogenous_a = priv->homogenous_columns;
      homogenous_b = priv->homogenous_rows;
      aalign = priv->halign;
      balign = priv->valign;
      agap          = priv->column_gap;
      bgap          = priv->row_gap;
    }

  priv->max_extent_a = 0;
  priv->max_extent_b = 0;

  priv->first_of_batch = TRUE;

  if (homogenous_a ||
      homogenous_b)
    {
      for (iter = priv->list; iter; iter = iter->next)
        {
          ClutterActor *child = iter->data;
          gfloat natural_width;
          gfloat natural_height;

          if (!priv->allocate_hidden && !CLUTTER_ACTOR_IS_VISIBLE (child))
            continue;

          /* each child will get as much space as they require */
          clutter_actor_get_preferred_size (CLUTTER_ACTOR (child),
                                            NULL, NULL,
                                            &natural_width, &natural_height);
          if (natural_width > priv->max_extent_a)
            priv->max_extent_a = natural_width;
          if (natural_height > priv->max_extent_b)
            priv->max_extent_b = natural_width;
        }
    }

  if (priv->column_major)
    {
      gfloat temp = priv->max_extent_a;
      priv->max_extent_a = priv->max_extent_b;
      priv->max_extent_b = temp;
    }

  current_stride = 0;
  for (iter = priv->list; iter; iter=iter->next)
    {
      ClutterActor *child = iter->data;
      gfloat natural_a;
      gfloat natural_b;

      if (!priv->allocate_hidden && !CLUTTER_ACTOR_IS_VISIBLE (child))
        continue;

      /* each child will get as much space as they require */
      clutter_actor_get_preferred_size (CLUTTER_ACTOR (child),
                                        NULL, NULL,
                                        &natural_a, &natural_b);

      if (priv->column_major) /* swap axes around if column is major */
        {
          gfloat temp = natural_a;
          natural_a = natural_b;
          natural_b = temp;
        }

      /* if the child is overflowing, or the max-stride has been reached,
       * we wrap to next line */
      current_stride++;
      if ((priv->max_stride > 0 && current_stride > priv->max_stride)
          || (current_a + natural_a > priv->a_wrap
              || (homogenous_a && current_a + priv->max_extent_a > priv->a_wrap)))
        {
          current_b = next_b + bgap;
          current_a = 0;
          next_b = current_b + bgap;
          priv->first_of_batch = TRUE;
          current_stride = 1;
        }

      if (priv->end_align &&
          priv->first_of_batch)
        {
          current_a = compute_row_start (iter, current_a, priv);
          priv->first_of_batch = FALSE;
        }

      if (next_b-current_b < natural_b)
        next_b = current_b + natural_b;

      {
        gfloat row_height;
        ClutterActorBox child_box;

        if (homogenous_b)
          {
            row_height = priv->max_extent_b;
          }
        else
          {
            row_height = compute_row_height (iter, next_b-current_b,
                                             current_a, priv);
          }

        if (homogenous_a)
          {
            child_box.x1 = current_a + (priv->max_extent_a-natural_a) * aalign;
            child_box.x2 = child_box.x1 + natural_a;

          }
        else
          {
            child_box.x1 = current_a;
            child_box.x2 = child_box.x1 + natural_a;
          }

        child_box.y1 = current_b + (row_height-natural_b) * balign;
        child_box.y2 = child_box.y1 + natural_b;


        if (priv->column_major)
          {
            gfloat temp = child_box.x1;
            child_box.x1 = child_box.y1;
            child_box.y1 = temp;

            temp = child_box.x2;
            child_box.x2 = child_box.y2;
            child_box.y2 = temp;
          }

        /* account for padding and pixel-align */
        child_box.x1 = (int)(child_box.x1 + padding.left);
        child_box.y1 = (int)(child_box.y1 + padding.top);
        child_box.x2 = (int)(child_box.x2 + padding.left);
        child_box.y2 = (int)(child_box.y2 + padding.top);

        /* update the allocation */
        if (!calculate_extents_only)
          clutter_actor_allocate (CLUTTER_ACTOR (child),
                                  &child_box,
                                  flags);

        /* update extents */
        if (actual_width && (child_box.x2 + padding.right) > *actual_width)
          *actual_width = child_box.x2 + padding.right;

        if (actual_height && (child_box.y2 + padding.bottom) > *actual_height)
          *actual_height = child_box.y2 + padding.bottom;

        if (homogenous_a)
          {
            current_a += priv->max_extent_a + agap;
          }
        else
          {
            current_a += natural_a + agap;
          }
      }
    }
}

static void
mx_grid_allocate (ClutterActor          *self,
                  const ClutterActorBox *box,
                  ClutterAllocationFlags flags)
{
  MxGridPrivate *priv = MX_GRID (self)->priv;
  ClutterActorBox alloc_box = *box;

  /* chain up here to preserve the allocated size
   *
   * (we ignore the height of the allocation if we have a vadjustment set,
   *  or the width of the allocation if an column_major is set and an
   *  hadjustment is set)
   */
  CLUTTER_ACTOR_CLASS (mx_grid_parent_class)
  ->allocate (self, box, flags);


  /* only update vadjustment - we don't really want horizontal scrolling */
  if (priv->vadjustment && !priv->column_major)
    {
      gdouble prev_value;
      gfloat height;

      /* get preferred height for this width */
      mx_grid_do_allocate (self,
                           box,
                           flags,
                           TRUE,
                           NULL,
                           &height);
      /* set our allocated height to be the preferred height, since we will be
       * scrolling
       */
      alloc_box.y2 = alloc_box.y1 + height;

      g_object_set (G_OBJECT (priv->vadjustment),
                    "lower", 0.0,
                    "upper", height,
                    "page-size", box->y2 - box->y1,
                    "step-increment", (box->y2 - box->y1) / 6,
                    "page-increment", box->y2 - box->y1,
                    NULL);

      if (priv->hadjustment)
        {
          g_object_set (G_OBJECT (priv->hadjustment),
                        "lower", 0.0,
                        "upper", 0.0,
                        NULL);;
        }

      prev_value = mx_adjustment_get_value (priv->vadjustment);
      mx_adjustment_set_value (priv->vadjustment, prev_value);
    }
  if (priv->hadjustment && priv->column_major)
    {
      gdouble prev_value;
      gfloat width;

      /* get preferred width for this height */
      mx_grid_do_allocate (self,
                           box,
                           flags,
                           TRUE,
                           &width,
                           NULL);
      /* set our allocated height to be the preferred height, since we will be
       * scrolling
       */
      alloc_box.x2 = alloc_box.x1 + width;

      g_object_set (G_OBJECT (priv->hadjustment),
                    "lower", 0.0,
                    "upper", width,
                    "page-size", box->x2 - box->x1,
                    "step-increment", (box->x2 - box->x1) / 6,
                    "page-increment", box->x2 - box->x1,
                    NULL);

      if (priv->vadjustment)
        {
          g_object_set (G_OBJECT (priv->vadjustment),
                        "lower", 0.0,
                        "upper", 0.0,
                        NULL);;
        }

      prev_value = mx_adjustment_get_value (priv->hadjustment);
      mx_adjustment_set_value (priv->hadjustment, prev_value);
    }


  mx_grid_do_allocate (self,
                       &alloc_box,
                       flags,
                       FALSE,
                       NULL,
                       NULL);

  if (priv->allocate_hidden)
    {
      ensure_children_are_visible ((MxGrid *) self);
    }
}


