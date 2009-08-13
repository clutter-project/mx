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

#include "nbtk-marshal.h"
#include "nbtk-expander.h"
#include "nbtk-private.h"
#include "nbtk-stylable.h"
#include "nbtk-icon.h"

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (NbtkExpander, nbtk_expander, NBTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))

static ClutterContainerIface *container_parent_class = NULL;

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_EXPANDER, NbtkExpanderPrivate))

enum {
  PROP_0,

  PROP_EXPANDED
};

enum
{
  EXPAND_COMPLETE,

  LAST_SIGNAL
};

struct _NbtkExpanderPrivate {
  ClutterActor *label;
  ClutterActor *arrow;
  gfloat   spacing;

  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
  gdouble progress;

  gboolean expanded : 1;
};

static guint expander_signals[LAST_SIGNAL] = { 0, };

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

  g_signal_emit (expander, expander_signals[EXPAND_COMPLETE], 0);

  child = nbtk_bin_get_child (NBTK_BIN (expander));

  if (!child)
    return;

  /* continue only if we are "opening" */
  if (!priv->expanded)
    return;

  /* we can't do an animation if there is already one in progress,
   * because we cannot get the actors original opacity */
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
  g_object_notify ((GObject *) expander, "expanded");

  if (priv->expanded)
    {
      clutter_actor_set_name (priv->arrow, "nbtk-expander-arrow-open");
      nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (expander), "active");
    }
  else
    {
      clutter_actor_set_name (priv->arrow, "nbtk-expander-arrow-closed");
      nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (expander), NULL);
    }

  child = nbtk_bin_get_child (NBTK_BIN (expander));

  if (!child)
    return;

  /* setup and start the expansion animation */
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
  nbtk_expander_toggle_expanded (NBTK_EXPANDER (actor));

  return FALSE;
}

static void
nbtk_expander_get_preferred_width (ClutterActor *actor,
                                   gfloat   for_height,
                                   gfloat  *min_width,
                                   gfloat  *pref_width)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActor *child;
  NbtkPadding padding;
  gfloat min_child_w, pref_child_w, min_label_w, pref_label_w, arrow_w;

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
  clutter_actor_get_preferred_width (priv->arrow,
                                     -1, NULL, &arrow_w);

  /* TODO: create a style property for this padding between arrow and label */
  if (arrow_w)
    arrow_w += 6.0f;

  if (min_width)
    *min_width = padding.left
                 + MAX (min_child_w, min_label_w + arrow_w)
                 + padding.right;

  if (pref_width)
    *pref_width = padding.left
                  + MAX (pref_child_w, pref_label_w + arrow_w)
                  + padding.right;
}

static void
nbtk_expander_get_preferred_height (ClutterActor *actor,
                                    gfloat   for_width,
                                    gfloat  *min_height,
                                    gfloat  *pref_height)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActor *child;
  NbtkPadding padding;
  gfloat min_child_h, pref_child_h, min_label_h, pref_label_h, arrow_h;
  gfloat available_w;

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

      /* allocate the space multiplied by the progress of the "expansion"
       * animation */
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

  clutter_actor_get_preferred_height (priv->arrow, -1, NULL, &arrow_h);

  min_label_h = MAX (min_label_h, arrow_h);
  pref_label_h = MAX (pref_label_h, arrow_h);

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
                        ClutterAllocationFlags flags)
{
  ClutterActor *child;
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActorBox child_box;
  NbtkPadding padding;
  gfloat label_w, label_h;
  gfloat child_h, child_w;
  ClutterActorClass *parent_parent;
  gfloat available_w, available_h, min_w, min_h, full_h, arrow_h, arrow_w;
  ClutterRequestMode request;

  /* skip NbtkBin allocate */
  parent_parent = g_type_class_peek_parent (nbtk_expander_parent_class);
  parent_parent->allocate (actor, box, flags);

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  available_w = (box->x2 - box->x1) - padding.left - padding.right;
  available_h = (box->y2 - box->y1) - padding.top - padding.bottom;

  /* visual height of the expander - used for allocating the background */
  full_h = padding.top + padding.bottom;

  /* arrow */
  clutter_actor_get_preferred_width (priv->arrow, -1, NULL, &arrow_w);
  arrow_w = MIN (arrow_w, available_w);

  clutter_actor_get_preferred_height (priv->arrow, -1, NULL, &arrow_h);
  arrow_h = MIN (arrow_h, available_h);

  child_box.x1 = padding.left;
  child_box.x2 = child_box.x1 + arrow_w;
  child_box.y1 = padding.top;
  child_box.y2 = child_box.y1 + arrow_h;
  clutter_actor_allocate (priv->arrow, &child_box, flags);

  /* label */
  min_h = 0;
  min_w = 0;

  clutter_actor_get_preferred_width (priv->label,
                                     available_h, &min_w, &label_w);
  label_w = CLAMP (label_w, min_w, available_w);

  clutter_actor_get_preferred_height (priv->label,
                                      label_w, &min_h, &label_h);
  label_h = CLAMP (label_h, min_h, available_h);

  /* TODO: make a style property for padding between arrow and label */
  child_box.x1 = padding.left + arrow_w + 6.0f;
  child_box.x2 = child_box.x1 + label_w;
  child_box.y1 = padding.top;
  child_box.y2 = child_box.y1 + label_h;
  clutter_actor_allocate (priv->label, &child_box, flags);

  full_h += MAX (label_h, arrow_h);

  /* remove label height and spacing for child calculations */
  available_h -= MAX (label_h, arrow_h) - priv->spacing;

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
      child_box.y1 = padding.top + priv->spacing + MAX (label_h, arrow_h);
      child_box.y2 = child_box.y2 + child_h;
      clutter_actor_allocate (child, &child_box, flags);

      full_h += priv->spacing + child_h;
    }

}

static void
nbtk_expander_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)->paint (actor);

  clutter_actor_paint (((NbtkExpander* ) actor)->priv->label);
  clutter_actor_paint (((NbtkExpander* ) actor)->priv->arrow);
}

static void
nbtk_expander_map (ClutterActor *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;
  ClutterActorClass *parent_parent_class = g_type_class_peek_parent (nbtk_expander_parent_class);

  CLUTTER_ACTOR_CLASS (parent_parent_class)->map (actor);

  clutter_actor_map (priv->label);
  clutter_actor_map (priv->arrow);
}

static void
nbtk_expander_unmap (ClutterActor *actor)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_expander_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->label);
  clutter_actor_unmap (priv->arrow);
}

static void
nbtk_expander_style_changed (NbtkWidget *widget)
{
  NbtkExpander *expander = NBTK_EXPANDER (widget);
  NbtkExpanderPrivate *priv = expander->priv;
  const gchar *pseudo_class;
  ClutterColor *color = NULL;
  gchar *font_name;
  gchar *font_string;
  gint font_size;

  pseudo_class = nbtk_widget_get_style_pseudo_class (widget);

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (expander->priv->arrow),
                                      pseudo_class);


  nbtk_stylable_get (NBTK_STYLABLE (widget),
                     "color", &color,
                     "font-family", &font_name,
                     "font-size", &font_size,
                     NULL);

  if (color)
    {
      clutter_text_set_color (CLUTTER_TEXT (priv->label), color);
      clutter_color_free (color);
    }

  if (font_name || font_size)
    {
      if (font_name && font_size)
        {
          font_string = g_strdup_printf ("%s %dpx", font_name, font_size);
          g_free (font_name);
        }
      else
        {
          if (font_size)
            font_string = g_strdup_printf ("%dpx", font_size);
          else
            font_string = font_name;
        }

      clutter_text_set_font_name (CLUTTER_TEXT (priv->label), font_string);
      g_free (font_string);
    }

}

static gboolean
nbtk_expander_enter (ClutterActor         *actor,
                     ClutterCrossingEvent *event)
{
  if (!NBTK_EXPANDER (actor)->priv->expanded)
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (actor), "hover");

  return FALSE;
}

static gboolean
nbtk_expander_leave (ClutterActor         *actor,
                     ClutterCrossingEvent *event)
{
  if (!NBTK_EXPANDER (actor)->priv->expanded)
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (actor), NULL);

  return FALSE;
}

static void
nbtk_expander_foreach (ClutterContainer *container,
                       ClutterCallback   callback,
                       gpointer          user_data)
{
  NbtkExpanderPrivate *priv = NBTK_EXPANDER (container)->priv;

  if (priv->expanded)
    callback (nbtk_bin_get_child ((NbtkBin*) container),
              user_data);
}

static void
nbtk_expander_add (ClutterContainer *container,
                   ClutterActor *actor)
{
  NbtkExpander *expander = NBTK_EXPANDER (container);
  NbtkExpanderPrivate *priv = expander->priv;

  /* Override the container add method so we can hide the actor if the
     expander is not expanded */

  /* chain up */
  container_parent_class->add (container, actor);

  if (!priv->expanded)
    {
      actor = nbtk_bin_get_child (NBTK_BIN (container));
      if (actor)
        clutter_actor_hide (actor);
    }
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  container_parent_class = g_type_interface_peek_parent (iface);

  iface->foreach = nbtk_expander_foreach;
  iface->add = nbtk_expander_add;
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
  actor_class->enter_event = nbtk_expander_enter;
  actor_class->leave_event = nbtk_expander_leave;
  actor_class->map = nbtk_expander_map;
  actor_class->unmap = nbtk_expander_unmap;

  pspec = g_param_spec_boolean ("expanded",
                                "Expanded",
                                "Indicates that the expander is open or closed",
                                FALSE,
                                NBTK_PARAM_READWRITE);
  g_object_class_install_property (object_class,
                                   PROP_EXPANDED,
                                   pspec);

  /**
   * NbtkExpander::expand-complete:
   * @expander: the object that received the signal
   *
   * Emitted after the expand animation finishes. Check the "expanded" property
   * of the #NbtkExpander to determine if the expander is expanded or not.
   */

  expander_signals[EXPAND_COMPLETE] =
    g_signal_new ("expand-complete",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkExpanderClass, expand_complete),
                  NULL, NULL,
                  _nbtk_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
nbtk_expander_init (NbtkExpander *self)
{
  NbtkExpanderPrivate *priv = self->priv = GET_PRIVATE (self);

  priv->label = clutter_text_new ();
  clutter_actor_set_parent (priv->label, (ClutterActor *) self);

  priv->arrow = (ClutterActor *) nbtk_icon_new ();
  clutter_actor_set_parent (priv->arrow, (ClutterActor *) self);
  clutter_actor_set_name (priv->arrow, "nbtk-expander-arrow-closed");

  /* TODO: make this a style property */
  priv->spacing = 10.0f;

  priv->timeline = clutter_timeline_new (250);
  g_signal_connect (priv->timeline, "new-frame", G_CALLBACK (new_frame), self);
  g_signal_connect (priv->timeline, "completed", G_CALLBACK (timeline_complete), self);

  priv->alpha = clutter_alpha_new_full (priv->timeline, CLUTTER_EASE_IN_SINE);
  g_object_ref_sink (priv->alpha);

  clutter_actor_set_reactive ((ClutterActor *) self, TRUE);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (nbtk_expander_style_changed), NULL);
}

/**
 * nbtk_expander_new:
 *
 * Creates a new #NbtkExpander
 *
 * Returns: the newly allocated #NbtkExpander
 */
NbtkWidget*
nbtk_expander_new (void)
{
  return g_object_new (NBTK_TYPE_EXPANDER, NULL);
}

/**
 * nbtk_expander_set_label:
 * @expander: A #NbtkExpander
 * @label: string to set as the expander label
 *
 * Sets the text displayed as the title of the expander
 */
void
nbtk_expander_set_label (NbtkExpander *expander,
                         const gchar *label)
{
  g_return_if_fail (NBTK_IS_EXPANDER (expander));
  g_return_if_fail (label != NULL);

  clutter_text_set_text (CLUTTER_TEXT (expander->priv->label), label);
}

/**
 * nbtk_expander_set_expanded:
 * @expander: A #NbtkExpander
 * @expanded: the state of the expander to set
 *
 * Set the state of the expander. This will cause the expander to open and
 * close if the state is changed.
 */
void
nbtk_expander_set_expanded (NbtkExpander *expander,
                            gboolean      expanded)
{
  g_return_if_fail (NBTK_IS_EXPANDER (expander));

  if (expander->priv->expanded != expanded)
    {
      nbtk_expander_toggle_expanded (expander);
    }
}

/**
 * nbtk_expander_get_expanded:
 * @expander: a #NbtkExpander
 *
 * Get the current state of the expander
 *
 * Returns: #TRUE if the expander is open, #FALSE if it is closed
 */
gboolean
nbtk_expander_get_expanded (NbtkExpander *expander)
{
  g_return_val_if_fail (NBTK_IS_EXPANDER (expander), FALSE);

  return expander->priv->expanded;
}
