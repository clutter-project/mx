#include "nbtk-table-child.h"
#include "nbtk-private.h"
#include <nbtk/nbtk-widget.h>
#include <nbtk/nbtk-table.h>

/*
 * ClutterChildMeta Implementation
 */

enum {
  CHILD_PROP_0,

  CHILD_PROP_COL,
  CHILD_PROP_COL_OLD,
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
    case CHILD_PROP_KEEP_RATIO:
      g_warning ("The \"keep-aspect-ratio\" property of NbtkTableChild is"
                 " deprecated. Please implement this feature using the child's"
                 " preferred width and height mechanism");
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

static NbtkTableChild*
get_child_meta (NbtkTable    *table,
                 ClutterActor *child)
{
  NbtkTableChild *meta;

  meta = (NbtkTableChild*) clutter_container_get_child_meta (CLUTTER_CONTAINER (table), child);

  return meta;
}

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

void
nbtk_table_child_set_col_span (NbtkTable    *table,
                               ClutterActor *child,
                               gint          span)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));
  g_return_if_fail (span < 1);

  meta = get_child_meta (table, child);

  meta->col_span = span;

  clutter_actor_queue_relayout (child);
}

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
void
nbtk_table_child_set_row_span (NbtkTable    *table,
                               ClutterActor *child,
                               gint          span)
{
  NbtkTableChild *meta;

  g_return_if_fail (NBTK_IS_TABLE (table));
  g_return_if_fail (CLUTTER_IS_ACTOR (child));
  g_return_if_fail (span < 1);

  meta = get_child_meta (table, child);

  meta->row_span = span;

  clutter_actor_queue_relayout (child);
}

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

void
nbtk_table_child_set_x_align (NbtkTable *table,
                              ClutterActor *child,
                              NbtkAlign align)
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

NbtkAlign
nbtk_table_child_get_y_align (NbtkTable *table,
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

void
nbtk_table_child_set_y_align (NbtkTable *table,
                              ClutterActor *child,
                              NbtkAlign align)
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
