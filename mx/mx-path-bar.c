/*
 * mx-path-bar.c: A path bar actor
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-path-bar.h"
#include "mx-path-bar-button.h"
#include "mx-stylable.h"
#include "mx-focusable.h"
#include "mx-texture-frame.h"
#include "mx-private.h"

enum
{
  PROP_0,

  PROP_EDITABLE,
  PROP_CLEAR_ON_CHANGE,
  PROP_LEVEL,
  PROP_ENTRY
};

static void mx_stylable_iface_init (MxStylableIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxPathBar, mx_path_bar, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init))

#define PATH_BAR_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_PATH_BAR, MxPathBarPrivate))

struct _MxPathBarPrivate
{
  GList        *crumbs;
  gint          current_level;
  gint          overlap;

  gboolean      editable;
  gboolean      clear_on_change;
  ClutterActor *entry;
};

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_int ("x-mx-overlap",
                                "Overlap",
                                "Overlap between buttons.",
                                0, G_MAXINT, 0,
                                G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_PATH_BAR, pspec);
    }
}

static MxFocusable *
mx_path_bar_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxFocusable *focus_widget;
  MxPathBarPrivate *priv = MX_PATH_BAR (focusable)->priv;

  if (!priv->editable && !priv->current_level)
    return NULL;

  if (priv->current_level)
    {
      if (hint == MX_FOCUS_HINT_LAST)
        {
          if (priv->editable)
            focus_widget = MX_FOCUSABLE (priv->entry);
          else
            focus_widget =
              MX_FOCUSABLE (g_list_nth_data (priv->crumbs,
                                             priv->current_level - 1));
        }
      else
        focus_widget = MX_FOCUSABLE (priv->crumbs->data);
    }
  else
    focus_widget = MX_FOCUSABLE (priv->entry);

  return mx_focusable_accept_focus (focus_widget, hint);
}

static MxFocusable *
mx_path_bar_move_focus (MxFocusable      *focusable,
                        MxFocusDirection  direction,
                        MxFocusable      *from)
{
  gint i;
  GList *c, *last;
  MxFocusable *focus_widget;
  MxPathBarPrivate *priv = MX_PATH_BAR (focusable)->priv;

  if (direction == MX_FOCUS_DIRECTION_UP ||
      direction == MX_FOCUS_DIRECTION_DOWN ||
      direction == MX_FOCUS_DIRECTION_OUT)
    return NULL;

  last = NULL;
  focus_widget = NULL;
  for (i = 0, c = priv->crumbs;
       c && (i < priv->current_level);
       c = c->next, i++)
    {
      MxFocusable *crumb = c->data;

      if (crumb == from)
        {
          switch (direction)
            {
            case MX_FOCUS_DIRECTION_LEFT:
            case MX_FOCUS_DIRECTION_PREVIOUS:
              if (!last)
                return NULL;
              focus_widget = (MxFocusable *)last->data;
              break;

            case MX_FOCUS_DIRECTION_RIGHT:
            case MX_FOCUS_DIRECTION_NEXT:
              if (c->next)
                focus_widget = (MxFocusable *)c->next->data;
              else if (priv->editable)
                focus_widget = (MxFocusable *)priv->entry;
              else
                return NULL;
              break;

            default:
              return NULL;
            }
        }
      last = c;
    }

  if (from == (MxFocusable *)priv->entry)
    {
      switch (direction)
        {
        case MX_FOCUS_DIRECTION_LEFT:
        case MX_FOCUS_DIRECTION_PREVIOUS:
          if (!last)
            return NULL;
          else
            focus_widget = (MxFocusable *)last->data;
          break;

        default:
          return NULL;
        }
    }

  if (!focus_widget)
    {
      if (!priv->editable)
        return NULL;

      focus_widget = (MxFocusable *)priv->entry;
    }

  return mx_focusable_accept_focus (focus_widget, MX_FOCUS_HINT_FIRST);
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_path_bar_accept_focus;
  iface->move_focus = mx_path_bar_move_focus;
}

static void
mx_path_bar_style_changed_cb (MxWidget *self, MxStyleChangedFlags flags)
{
  MxPathBarPrivate *priv = MX_PATH_BAR (self)->priv;
  gint overlap;
  GList *c;

  mx_stylable_get (MX_STYLABLE (self),
                   "x-mx-overlap", &overlap,
                   NULL);

  if (overlap != priv->overlap)
    {
      priv->overlap = overlap;
      clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
    }

  /* Inform our private children */
  for (c = priv->crumbs; c; c = c->next)
    mx_stylable_style_changed (MX_STYLABLE (c->data), flags);

  if (priv->entry)
    mx_stylable_style_changed (MX_STYLABLE (priv->entry), flags);
}

static void
mx_path_bar_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  MxPathBar *self = MX_PATH_BAR (object);

  switch (property_id)
    {
    case PROP_EDITABLE:
      g_value_set_boolean (value, mx_path_bar_get_editable (self));
      break;

    case PROP_LEVEL:
      g_value_set_int (value, mx_path_bar_get_level (self));
      break;

    case PROP_CLEAR_ON_CHANGE:
      g_value_set_boolean (value, mx_path_bar_get_clear_on_change (self));
      break;

    case PROP_ENTRY:
      g_value_set_object (value, mx_path_bar_get_entry (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_path_bar_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  MxPathBar *self = MX_PATH_BAR (object);

  switch (property_id)
    {
    case PROP_EDITABLE:
      mx_path_bar_set_editable (self, g_value_get_boolean (value));
      break;

    case PROP_CLEAR_ON_CHANGE:
      mx_path_bar_set_clear_on_change (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_path_bar_dispose (GObject *object)
{
  MxPathBarPrivate *priv = MX_PATH_BAR (object)->priv;

  while (priv->crumbs)
    {
      clutter_actor_unparent ((ClutterActor *)priv->crumbs->data);
      priv->crumbs = g_list_delete_link (priv->crumbs, priv->crumbs);
    }

  if (priv->entry)
    {
      clutter_actor_unparent (priv->entry);
      priv->entry = NULL;
      priv->editable = FALSE;
    }

  G_OBJECT_CLASS (mx_path_bar_parent_class)->dispose (object);
}

static void
mx_path_bar_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_path_bar_parent_class)->finalize (object);
}

static void
mx_path_bar_get_preferred_width (ClutterActor *actor,
                                 gfloat        for_height,
                                 gfloat       *min_width_p,
                                 gfloat       *nat_width_p)
{
  GList *c;
  MxPadding padding;
  gfloat min_width, nat_width;

  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  min_width = nat_width = 0;
  for (c = priv->crumbs; c; c = c->next)
    {
      gfloat cmin_width, cnat_width;
      ClutterActor *crumb = c->data;

      clutter_actor_get_preferred_width (crumb,
                                         for_height,
                                         &cmin_width,
                                         &cnat_width);
      min_width += cmin_width;
      nat_width += cnat_width;

      if (c != priv->crumbs)
        {
          min_width -= MIN (priv->overlap, cmin_width);
          nat_width -= MIN (priv->overlap, cnat_width);
        }
    }

  if (priv->entry)
    {
      gfloat emin_width, enat_width;

      clutter_actor_get_preferred_width (priv->entry,
                                         for_height,
                                         &emin_width,
                                         &enat_width);

      min_width += emin_width;
      nat_width += enat_width;

      if (priv->crumbs)
        {
          min_width -= MIN (min_width, priv->overlap);
          nat_width -= MIN (nat_width, priv->overlap);
        }
    }

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (min_width_p)
    *min_width_p = min_width + padding.left + padding.right;
  if (nat_width_p)
    *nat_width_p = nat_width + padding.left + padding.right;
}

static void
mx_path_bar_get_preferred_height (ClutterActor *actor,
                                  gfloat        for_width,
                                  gfloat       *min_height_p,
                                  gfloat       *nat_height_p)
{
  GList *c;
  MxPadding padding;
  gfloat min_height, nat_height;

  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  min_height = nat_height = 0;
  for (c = priv->crumbs; c; c = c->next)
    {
      gfloat cmin_height, cnat_height;
      ClutterActor *crumb = c->data;

      clutter_actor_get_preferred_height (crumb,
                                          -1,
                                          &cmin_height,
                                          &cnat_height);

      if (cmin_height > min_height)
        min_height = cmin_height;
      if (cnat_height > nat_height)
        nat_height = cnat_height;
    }

  if (priv->entry)
    {
      gfloat emin_height, enat_height;

      clutter_actor_get_preferred_height (priv->entry,
                                          -1,
                                          &emin_height,
                                          &enat_height);

      if (emin_height > min_height)
        min_height = emin_height;
      if (enat_height > nat_height)
        nat_height = enat_height;
    }

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  min_height += padding.top + padding.bottom;
  nat_height += padding.top + padding.bottom;

  /* Check if the border-image is taller than our height.
   * If so, say we want this larger size instead - this is to
   * avoid stretching the border-image vertically.
   */
  if (priv->crumbs)
    {
      ClutterActor *border =
        mx_widget_get_border_image (MX_WIDGET (priv->crumbs->data));
      if (border)
        {
          ClutterTexture *texture =
            mx_texture_frame_get_parent_texture (MX_TEXTURE_FRAME (border));

          if (texture)
            {
              gint border_height;

              clutter_texture_get_base_size (texture, NULL, &border_height);
              if (border_height > nat_height)
                nat_height = border_height;
              if (border_height > min_height)
                min_height = border_height;
            }
        }
    }

  if (min_height_p)
    *min_height_p = min_height;
  if (nat_height_p)
    *nat_height_p = nat_height;
}

static void
mx_path_bar_allocate (ClutterActor           *actor,
                      const ClutterActorBox  *box,
                      ClutterAllocationFlags  flags)
{
  GList *c;
  gint n_crumbs;
  MxPadding padding;
  gboolean allocate_pref;
  ClutterActorBox child_box;
  gfloat min_width, nat_width, extra_space;

  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_path_bar_parent_class)->allocate (actor, box, flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  child_box.x1 = padding.left;
  child_box.y1 = padding.top;
  child_box.y2 = box->y2 - box->y1 - padding.bottom;

  /* Get our minimum/natural width so we can decide how to
   * squash actors if necessary
   */
  clutter_actor_get_preferred_width (actor,
                                     box->y2 - box->y1,
                                     &min_width,
                                     &nat_width);

  /* If we have less than our natural width, find out how
   * much extra space above the minimum width we have to
   * play with.
   */
  if ((box->x2 - box->x1) < nat_width)
    {
      extra_space = box->x2 - box->x1 - min_width;
      if (extra_space < 0)
        extra_space = 0;
      allocate_pref = FALSE;
    }
  else
    {
      extra_space = 0;
      allocate_pref = TRUE;
    }

  /* Allocate crumbs */
  n_crumbs = g_list_length (priv->crumbs);
  for (c = priv->crumbs; c; c = c->next)
    {
      gfloat cmin_width, cnat_width;
      ClutterActor *crumb = c->data;

      clutter_actor_get_preferred_width (crumb,
                                         child_box.y2 - child_box.y1,
                                         &cmin_width,
                                         &cnat_width);

      if (!allocate_pref)
        {
          /* Allocate a fair share of extra space, but don't allocate
           * over the natural width.
           */
          child_box.x2 = child_box.x1 +
            MIN (cmin_width + extra_space / n_crumbs, cnat_width);

          n_crumbs --;
          if (extra_space >= (child_box.x2 - child_box.x1 - cmin_width))
            extra_space -= (child_box.x2 - child_box.x1 - cmin_width);
          else
            extra_space = 0;
        }
      else
        child_box.x2 = child_box.x1 + cnat_width;

      /* If this is the last crumb, give it all extra space */
      if (!priv->entry && !c->next &&
          (box->x2 - box->x1 - padding.right) > (child_box.x2 - child_box.x1))
        child_box.x2 = box->x2 - box->x1 - padding.right;

      clutter_actor_allocate (crumb, &child_box, flags);
      child_box.x1 = child_box.x2 - MIN (priv->overlap,
                                         (child_box.x2 - child_box.x1));
    }

  /* Allocate the entry the rest of the space */
  if (priv->editable)
    {
      child_box.x2 = box->x2 - box->x1 - padding.right;
      clutter_actor_allocate (priv->entry, &child_box, flags);
    }
}

static void
mx_path_bar_map (ClutterActor *actor)
{
  GList *c;
  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_path_bar_parent_class)->map (actor);

  for (c = priv->crumbs; c; c = c->next)
    clutter_actor_map (CLUTTER_ACTOR (c->data));

  if (priv->entry)
    clutter_actor_map (priv->entry);
}

static void
mx_path_bar_unmap (ClutterActor *actor)
{
  GList *c;
  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_path_bar_parent_class)->unmap (actor);

  for (c = priv->crumbs; c; c = c->next)
    clutter_actor_unmap (CLUTTER_ACTOR (c->data));

  if (priv->entry)
    clutter_actor_unmap (priv->entry);
}

static void
mx_path_bar_paint (ClutterActor *actor)
{
  GList *c;
  MxPathBarPrivate *priv = MX_PATH_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_path_bar_parent_class)->paint (actor);

  if (priv->entry)
    clutter_actor_paint (priv->entry);

  for (c = g_list_last (priv->crumbs); c; c = c->prev)
    clutter_actor_paint (CLUTTER_ACTOR (c->data));
}

static void
mx_path_bar_pick (ClutterActor       *actor,
                  const ClutterColor *color)
{
  mx_path_bar_paint (actor);
}

static void
mx_path_bar_apply_style (MxWidget *widget,
                         MxStyle  *style)
{
  MxPathBarPrivate *priv = MX_PATH_BAR (widget)->priv;
  GList *c;

  for (c = priv->crumbs; c; c = c->next)
    mx_stylable_set_style (MX_STYLABLE (c->data), style);

  if (priv->entry != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->entry), style);
}

static void
mx_path_bar_class_init (MxPathBarClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxPathBarPrivate));

  object_class->get_property = mx_path_bar_get_property;
  object_class->set_property = mx_path_bar_set_property;
  object_class->dispose = mx_path_bar_dispose;
  object_class->finalize = mx_path_bar_finalize;

  actor_class->get_preferred_width = mx_path_bar_get_preferred_width;
  actor_class->get_preferred_height = mx_path_bar_get_preferred_height;
  actor_class->allocate = mx_path_bar_allocate;
  actor_class->map = mx_path_bar_map;
  actor_class->unmap = mx_path_bar_unmap;
  actor_class->paint = mx_path_bar_paint;
  actor_class->pick = mx_path_bar_pick;

  widget_class->apply_style = mx_path_bar_apply_style;

  pspec = g_param_spec_boolean ("editable",
                                "Editable",
                                "Enable or disable editing",
                                FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_EDITABLE, pspec);

  pspec = g_param_spec_boolean ("clear-on-change",
                                "Clear on level change",
                                "Whether to clear the entry "
                                "when changing levels",
                                FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLEAR_ON_CHANGE, pspec);

  pspec = g_param_spec_int ("level",
                            "Level",
                            "Depth of the path bar",
                            -1, G_MAXINT, 0, G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_LEVEL, pspec);

  pspec = g_param_spec_object ("entry",
                               "Entry",
                               "The MxEntry inside the path bar",
                               MX_TYPE_ENTRY,
                               MX_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_ENTRY, pspec);
}

static void
mx_path_bar_init (MxPathBar *self)
{
  self->priv = PATH_BAR_PRIVATE (self);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_path_bar_style_changed_cb), NULL);
}

ClutterActor *
mx_path_bar_new (void)
{
  return g_object_new (MX_TYPE_PATH_BAR, NULL);
}

static void
mx_path_bar_crumb_clicked_cb (ClutterActor *crumb,
                              MxPathBar    *self)
{
  gint i;
  GList *c;

  MxPathBarPrivate *priv = self->priv;

  if (priv->clear_on_change)
    mx_path_bar_set_text (self, "");

  for (c = priv->crumbs, i = 1; c; c = c->next, i++)
    {
      if (c->data == crumb)
        {
          while (priv->current_level > i)
            mx_path_bar_pop (self);
          break;
        }
    }
}

static void
mx_path_bar_reset_last_crumb (MxPathBar *bar)
{
  MxPathBarPrivate *priv = bar->priv;
  ClutterActor *last_crumb =
    g_list_nth_data (priv->crumbs, priv->current_level -1);

  if (last_crumb)
    mx_stylable_set_style_class (MX_STYLABLE (last_crumb),
                                 priv->editable ? NULL : "End");
}

gint
mx_path_bar_push (MxPathBar *bar, const gchar *name)
{
  ClutterActor *crumb;
  MxPathBarPrivate *priv;

  g_return_val_if_fail (MX_IS_PATH_BAR (bar), -1);

  priv = bar->priv;

  if (priv->clear_on_change)
    mx_path_bar_set_text (bar, "");

  crumb = mx_path_bar_button_new (name);
  clutter_actor_set_parent (crumb, CLUTTER_ACTOR (bar));

  priv->crumbs = g_list_insert (priv->crumbs, crumb, priv->current_level);

  if (!priv->entry)
    {
      if (priv->current_level)
        {
          ClutterActor *old_last_crumb =
            g_list_nth_data (priv->crumbs, priv->current_level - 1);

          mx_stylable_set_style_class (MX_STYLABLE (old_last_crumb), NULL);
        }

      mx_stylable_set_style_class (MX_STYLABLE (crumb), "End");
    }

  priv->current_level ++;

  g_signal_connect (crumb, "clicked",
                    G_CALLBACK (mx_path_bar_crumb_clicked_cb), bar);

  clutter_actor_animate (crumb, CLUTTER_EASE_OUT_QUAD, 150,
                         "transition", 1.0,
                         NULL);
  clutter_actor_queue_relayout (CLUTTER_ACTOR (bar));

  g_object_notify (G_OBJECT (bar), "level");

  return priv->current_level;
}

static void
mx_path_bar_pop_completed_cb (ClutterAnimation *animation,
                              ClutterActor     *crumb)
{
  MxPathBar *self = MX_PATH_BAR (clutter_actor_get_parent (crumb));
  MxPathBarPrivate *priv = self->priv;

  priv->crumbs = g_list_remove (priv->crumbs, crumb);
  clutter_actor_unparent (crumb);
}

gint
mx_path_bar_pop (MxPathBar *bar)
{
  ClutterActor *crumb;
  MxPathBarPrivate *priv;

  g_return_val_if_fail (MX_IS_PATH_BAR (bar), -1);

  priv = bar->priv;

  if (priv->clear_on_change)
    mx_path_bar_set_text (bar, "");

  if (priv->current_level == 0)
    return 0;

  crumb = g_list_nth_data (priv->crumbs, priv->current_level - 1);
  clutter_actor_animate (crumb, CLUTTER_EASE_IN_QUAD, 150,
                         "transition", 0.0,
                         "signal-after::completed",
                           mx_path_bar_pop_completed_cb, crumb,
                         NULL);

  priv->current_level --;
  mx_path_bar_reset_last_crumb (bar);
  g_object_notify (G_OBJECT (bar), "level");

  return priv->current_level;
}

gint
mx_path_bar_get_level (MxPathBar *bar)
{
  g_return_val_if_fail (MX_IS_PATH_BAR (bar), -1);

  return bar->priv->current_level;
}

/**
 * mx_path_bar_clear:
 * @bar: An #MxPathBar
 *
 * Remove all the current buttons
 *
 */
void
mx_path_bar_clear (MxPathBar *bar)
{
  g_return_if_fail (MX_IS_PATH_BAR (bar));

  while (bar->priv->current_level)
    mx_path_bar_pop (bar);
}

/**
 * mx_path_bar_get_editable:
 * @bar: A #MxPathBar
 *
 * Get the value of the #MxPathBar:editable property.
 *
 * Returns: the current value of the "editable" property.
 */
gboolean
mx_path_bar_get_editable (MxPathBar *bar)
{
  g_return_val_if_fail (MX_IS_PATH_BAR (bar), FALSE);

  return bar->priv->editable;
}

static void
mx_path_bar_entry_faded_cb (ClutterAnimation *animation,
                            MxPathBar        *bar)
{
  MxPathBarPrivate *priv = bar->priv;

  clutter_actor_unparent (priv->entry);
  priv->entry = NULL;

  clutter_actor_queue_relayout (CLUTTER_ACTOR (bar));
}

/**
 * mx_path_bar_set_editable:
 * @bar: A #MxPathBar
 * @editable: #TRUE if the path bar should be editable
 *
 * Set the value of the #MxPathBar:editable property.
 *
 */
void
mx_path_bar_set_editable (MxPathBar *bar, gboolean editable)
{
  MxPathBarPrivate *priv;

  g_return_if_fail (MX_IS_PATH_BAR (bar));

  priv = bar->priv;
  if (priv->editable == editable)
    return;

  priv->editable = editable;
  if (!editable)
    {
      clutter_actor_animate (priv->entry, CLUTTER_EASE_OUT_QUAD, 150,
                             "opacity", 0x00,
                             "signal-after::completed",
                               mx_path_bar_entry_faded_cb, bar,
                             NULL);
    }
  else
    {
      if (priv->entry)
        {
          ClutterAnimation *anim = clutter_actor_get_animation (priv->entry);
          g_signal_handlers_disconnect_by_func (anim,
                                                mx_path_bar_entry_faded_cb,
                                                bar);
        }
      else
        {
          priv->entry = mx_entry_new ();
          clutter_actor_set_parent (priv->entry, CLUTTER_ACTOR (bar));
          if (CLUTTER_ACTOR_IS_VISIBLE (priv->entry))
            clutter_actor_set_opacity (priv->entry, 0x00);
        }

      clutter_actor_animate (priv->entry, CLUTTER_EASE_OUT_QUAD, 150,
                             "opacity", 0xff, NULL);
    }

  mx_path_bar_reset_last_crumb (bar);

  g_object_notify (G_OBJECT (bar), "editable");
  clutter_actor_queue_relayout (CLUTTER_ACTOR (bar));
}

const gchar *
mx_path_bar_get_label (MxPathBar *bar, gint level)
{
  ClutterActor *crumb;
  MxPathBarPrivate *priv;

  g_return_val_if_fail (MX_IS_PATH_BAR (bar), NULL);
  g_return_val_if_fail ((level > 0) && (level <= bar->priv->current_level),
                        NULL);

  priv = bar->priv;
  crumb = (ClutterActor *)g_list_nth_data (priv->crumbs, level - 1);

  if (crumb)
    return mx_button_get_label (MX_BUTTON (crumb));
  else
    return NULL;
}

/**
 * mx_path_bar_set_label:
 * @bar: A #MxPathBar
 * @level: A #gint
 * @label: A #gchar
 *
 * Set the text on the button specified by @level
 *
 */
void
mx_path_bar_set_label (MxPathBar   *bar,
                       gint         level,
                       const gchar *label)
{
  ClutterActor *crumb;
  MxPathBarPrivate *priv;

  g_return_if_fail (MX_IS_PATH_BAR (bar));
  g_return_if_fail ((level > 0) && (level <= bar->priv->current_level));

  priv = bar->priv;
  crumb = (ClutterActor *)g_list_nth_data (priv->crumbs, level - 1);

  if (crumb)
    mx_button_set_label (MX_BUTTON (crumb), label);
}

const gchar *
mx_path_bar_get_text (MxPathBar *bar)
{
  g_return_val_if_fail (MX_IS_PATH_BAR (bar), NULL);

  if (!bar->priv->editable)
    return NULL;

  return mx_entry_get_text (MX_ENTRY (bar->priv->entry));
}

/**
 * mx_path_bar_set_text:
 * @bar: A #MxPathBar
 * @text: string to set the editable text to.
 *
 * Set the text in the editable area of the #MxPathBar
 *
 */
void
mx_path_bar_set_text (MxPathBar *bar, const gchar *text)
{
  g_return_if_fail (MX_IS_PATH_BAR (bar));

  if (!bar->priv->editable)
    return;

  mx_entry_set_text (MX_ENTRY (bar->priv->entry), text);
}

/**
 * mx_path_bar_get_entry:
 * @bar: A #MxPathBar
 *
 * Get the MxEntry used as the editable area in the MxPathBar.
 *
 * Returns: (transfer none): MxEntry *
 */
MxEntry *
mx_path_bar_get_entry (MxPathBar *bar)
{
  g_return_val_if_fail (MX_IS_PATH_BAR (bar), NULL);

  return (MxEntry *)bar->priv->entry;
}

/**
 * mx_path_bar_get_clear_on_change:
 * @bar: A #MxPathBar
 *
 * Get the value of the #MxPathBar:clear-on-change property
 *
 * Returns: the value of the "clear-on-change" property
 */
gboolean
mx_path_bar_get_clear_on_change (MxPathBar *bar)
{
  g_return_val_if_fail (MX_IS_PATH_BAR (bar), FALSE);

  return bar->priv->clear_on_change;
}

/**
 * mx_path_bar_set_clear_on_change:
 * @bar: A #MxPathBar
 * @clear_on_change: the new value of the property
 *
 * Set theh value of the #MxPathBar:clear-on-change property
 *
 */
void
mx_path_bar_set_clear_on_change (MxPathBar *bar,
                                 gboolean   clear_on_change)
{
  g_return_if_fail (MX_IS_PATH_BAR (bar));

  if (bar->priv->clear_on_change != clear_on_change)
    {
      bar->priv->clear_on_change = clear_on_change;
      g_object_notify (G_OBJECT (bar), "clear-on-change");
    }
}
