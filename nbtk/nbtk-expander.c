/*
 * nbtk-expander.c: Expander Widget
 *
 * Copyright 2009 Intel Corporation.
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#include "nbtk-expander.h"
#include "nbtk-private.h"

G_DEFINE_TYPE (NbtkExpander, nbtk_expander, NBTK_TYPE_BIN)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_EXPANDER, NbtkExpanderPrivate))

enum {
  PROP_0,

  PROP_EXPANDED
};

struct _NbtkExpanderPrivate {
  ClutterActor *label;
  ClutterUnit   spacing;

  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
  gdouble progress;

  gboolean expanded : 1;
};

static void
nbtk_expander_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (object)->priv;

  switch (property_id) {
    case PROP_EXPANDED:
      g_value_set_boolean (value, priv->expanded);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
nbtk_expander_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_EXPANDED:
      nbtk_expander_set_expanded ((NbtkExpander *) object,
                                  g_value_get_boolean (value));
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
nbtk_expander_dispose (GObject *object)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (object)->priv;

  if (priv->label)
    {
      clutter_actor_unparent (priv->label);
      priv->label = NULL;
    }

  if (priv->timeline)
    {
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }

  if (priv->alpha)
    {
      g_object_unref (priv->alpha);
      priv->alpha = NULL;
    }

  G_OBJECT_CLASS (nbtk_expander_parent_class)->dispose (object);
}

static void
nbtk_expander_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_expander_parent_class)->finalize (object);
}

static void
timeline_complete (ClutterTimeline *timeline,
                   ClutterActor    *expander)
{
  guchar opacity;
  ClutterActor *child;
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (expander)->priv;

  child = nbtk_bin_get_child (NBTK_BIN (expander));

  if (!child)
    return;

  /* if we are "opening" */
  if (clutter_timeline_get_direction (priv->timeline)
      != CLUTTER_TIMELINE_FORWARD)
    return;

  /* we can't do an animation if there is already one in progress,
   * because we cannot reliably get the actors true opacity
   */
  if (clutter_actor_get_animation (child))
    {
      clutter_actor_show (child);
      return;
    }


  opacity = clutter_actor_get_opacity (child);
  clutter_actor_set_opacity (child, 0);

  clutter_actor_show (child);
  clutter_actor_animate (child, CLUTTER_EASE_IN_SINE, 100,
                         "opacity", opacity,
                         NULL);
}

static void
new_frame (ClutterTimeline *timeline,
           gint             frame_num,
           ClutterActor    *expander)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (expander)->priv;

  priv->progress = clutter_alpha_get_alpha (priv->alpha);

  clutter_actor_queue_relayout (expander);
}

static void
nbtk_expander_toggle_expanded (NbtkExpander *expander)
{
  NbtkExpanderPrivate *priv = expander->priv;
  ClutterActor *child;

  priv->expanded = !priv->expanded;

  child = nbtk_bin_get_child (NBTK_BIN (expander));

  if (!child)
    return;


  if (!priv->expanded)
    {
      clutter_actor_hide (child);
      clutter_timeline_set_direction (priv->timeline,
                                      CLUTTER_TIMELINE_BACKWARD);
    }
  else
    {
      clutter_timeline_set_direction (priv->timeline,
                                      CLUTTER_TIMELINE_FORWARD);
    }


  if (!clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_rewind (priv->timeline);

  clutter_timeline_start (priv->timeline);
}

static gboolean
nbtk_expander_button_release (ClutterActor       *actor,
                              ClutterButtonEvent *event)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;

  nbtk_expander_toggle_expanded (NBTK_EXPANDER (actor));

  return FALSE;
}

static void
nbtk_expander_get_preferred_width (ClutterActor *actor,
                                   ClutterUnit   for_height,
                                   ClutterUnit  *min_width,
                                   ClutterUnit  *pref_width)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActor *child;
  NbtkPadding padding;
  ClutterUnit min_child_w, pref_child_w, min_label_w, pref_label_w;

  child = nbtk_bin_get_child (NBTK_BIN (actor));
  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  if (child)
    {
      clutter_actor_get_preferred_width (child,
                                         -1,
                                         &min_child_w,
                                         &pref_child_w);
    }
  else
    {
      min_child_w = 0;
      pref_child_w = 0;
    }

  clutter_actor_get_preferred_width (priv->label,
                                     -1, &min_label_w, &pref_label_w);

  if (min_width)
    *min_width = padding.left
                 + MAX (min_child_w, min_label_w)
                 + padding.right;

  if (pref_width)
    *pref_width = padding.left
                  + MAX (pref_child_w, pref_label_w)
                  + padding.right;
}

static void
nbtk_expander_get_preferred_height (ClutterActor *actor,
                                    ClutterUnit   for_width,
                                    ClutterUnit  *min_height,
                                    ClutterUnit  *pref_height)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActor *child;
  NbtkPadding padding;
  ClutterUnit min_child_h, pref_child_h, min_label_h, pref_label_h;
  ClutterUnit available_w;

  child = nbtk_bin_get_child (NBTK_BIN (actor));

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);
  available_w = for_width - padding.left - padding.right;

  if (child)
    {
      clutter_actor_get_preferred_height (child,
                                          available_w,
                                          &min_child_h,
                                          &pref_child_h);
      min_child_h += priv->spacing;
      pref_child_h += priv->spacing;

      min_child_h *= priv->progress;
      pref_child_h *= priv->progress;
    }
  else
    {
      min_child_h = 0;
      pref_child_h = 0;
    }

  clutter_actor_get_preferred_height (priv->label,
                                      available_w,
                                      &min_label_h,
                                      &pref_label_h);

  if (min_height)
    *min_height = padding.top
                  + min_child_h + min_label_h
                  + padding.bottom;

  if (pref_height)
    *pref_height = padding.top
                   + pref_child_h + pref_label_h
                   + padding.bottom;

}

static void
nbtk_expander_allocate (ClutterActor          *actor,
                        const ClutterActorBox *box,
                        gboolean               origin_changed)
{
  ClutterActor *child;
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActorBox child_box;
  NbtkPadding padding;
  ClutterUnit label_w, label_h;
  ClutterUnit child_h, child_w;
  ClutterActorClass *parent_parent;
  ClutterUnit available_w, available_h, min_w, min_h, full_h;
  ClutterRequestMode request;

  /* skip NbtkBin allocate */
  parent_parent = g_type_class_peek_parent (nbtk_expander_parent_class);
  parent_parent->allocate (actor, box, origin_changed);

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  available_w = (box->x2 - box->x1) - padding.left - padding.right;
  available_h = (box->y2 - box->y1) - padding.top - padding.bottom;

  /* visual height of the expander - used for allocating the background */
  full_h = padding.top + padding.bottom;

  /* label */
  min_h = 0;
  min_w = 0;

  clutter_actor_get_preferred_width (priv->label, available_h, &min_w, &label_w);
  label_w = CLAMP (label_w, min_w, available_w);

  clutter_actor_get_preferred_height (priv->label, label_w, &min_h, &label_h);
  label_h = CLAMP (label_h, min_h, available_h);

  child_box.x1 = padding.left;
  child_box.x2 = child_box.x1 + label_w;
  child_box.y1 = padding.top;
  child_box.y2 = child_box.y2 + label_h;
  clutter_actor_allocate (priv->label, &child_box, origin_changed);

  full_h += label_h;

  /* remove label height and spacing for child calculations */
  available_h -= label_h - priv->spacing;

  /* child */
  child = nbtk_bin_get_child (NBTK_BIN (actor));
  if (child && CLUTTER_ACTOR_IS_VISIBLE (child))
    {
      request = CLUTTER_REQUEST_HEIGHT_FOR_WIDTH;
      g_object_get (G_OBJECT (child), "request-mode", &request, NULL);

      min_h = 0;
      min_w = 0;

      if (request == CLUTTER_REQUEST_HEIGHT_FOR_WIDTH)
        {
          clutter_actor_get_preferred_width (child, available_h, &min_w, &child_w);
          child_w = CLAMP (child_w, min_w, available_w);

          clutter_actor_get_preferred_height (child, child_w, &min_h, &child_h);
          child_h = CLAMP (child_h, min_h, available_h);
        }
      else if (request == CLUTTER_REQUEST_WIDTH_FOR_HEIGHT)
        {
          clutter_actor_get_preferred_height (child, available_w, &min_h, &child_h);
          child_h = CLAMP (child_h, min_h, available_h);

          clutter_actor_get_preferred_width (child, child_h, &min_w, &child_w);
          child_w = CLAMP (child_w, min_w, available_w);
        }

      child_box.x1 = padding.left;
      child_box.x2 = child_box.x1 + child_w;
      child_box.y1 = padding.top + priv->spacing + label_h;
      child_box.y2 = child_box.y2 + child_h;
      clutter_actor_allocate (child, &child_box, origin_changed);

      full_h += priv->spacing + child_h;
    }

}

static void
nbtk_expander_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)->paint (actor);

  clutter_actor_paint (NBTK_EXPANDER (actor)->priv->label);
}

static void
nbtk_expander_class_init (NbtkExpanderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkExpanderPrivate));

  object_class->get_property = nbtk_expander_get_property;
  object_class->set_property = nbtk_expander_set_property;
  object_class->dispose = nbtk_expander_dispose;
  object_class->finalize = nbtk_expander_finalize;

  actor_class->button_release_event = nbtk_expander_button_release;
  actor_class->allocate = nbtk_expander_allocate;
  actor_class->get_preferred_width = nbtk_expander_get_preferred_width;
  actor_class->get_preferred_height = nbtk_expander_get_preferred_height;
  actor_class->paint = nbtk_expander_paint;

  pspec = g_param_spec_boolean ("expanded",
                                "Expanded",
                                "Indicates that the expander is open or closed",
                                TRUE,
                                NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class,
                                   PROP_EXPANDED,
                                   pspec);
}

static void
nbtk_expander_init (NbtkExpander *self)
{
  NbtkExpanderPrivate *priv = self->priv = GET_PRIVATE (self);

  priv->label = clutter_text_new ();
  clutter_actor_set_parent (priv->label, (ClutterActor *) self);

  /* TODO: make this a style property */
  priv->spacing = CLUTTER_UNITS_FROM_INT (6);

  priv->progress = 1.0;
  priv->expanded = 1;

  priv->timeline = clutter_timeline_new_for_duration (250);
  clutter_timeline_set_direction (priv->timeline, CLUTTER_TIMELINE_BACKWARD);
  g_signal_connect (priv->timeline, "new-frame", G_CALLBACK (new_frame), self);
  g_signal_connect (priv->timeline, "completed", G_CALLBACK (timeline_complete), self);

  priv->alpha = clutter_alpha_new_full (priv->timeline, CLUTTER_EASE_IN_SINE);
}

NbtkWidget*
nbtk_expander_new (void)
{
  return g_object_new (NBTK_TYPE_EXPANDER, NULL);
}

void
nbtk_expander_set_label (NbtkExpander *expander,
                         const gchar *label)
{
  g_return_if_fail (NBTK_IS_EXPANDER (expander));
  g_return_if_fail (label != NULL);

  clutter_text_set_text (CLUTTER_TEXT (expander->priv->label), label);
}

void
nbtk_expander_set_expanded (NbtkExpander *expander,
                            gboolean      expanded)
{
  g_return_if_fail (NBTK_IS_EXPANDER (expander));

  if (expander->priv->expanded != expanded)
    {
      nbtk_expander_toggle_expanded (expander);

      g_object_notify ((GObject *) expander, "expanded");
    }
}

gboolean
nbtk_expander_get_expanded (NbtkExpander *expander)
{
  g_return_if_fail (NBTK_IS_EXPANDER (expander));

  return expander->priv->expanded;
}
