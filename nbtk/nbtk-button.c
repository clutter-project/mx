/* nbtk-button.c: Plain button actor
 *
 * Copyright (C) 2007 OpenedHand
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
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/clutter-container.h>

#include "nbtk-button.h"

#include "nbtk-marshal.h"
#include "nbtk-stylable.h"

enum
{
  PROP_0,

  PROP_LABEL
};

enum
{
  CLICKED,

  LAST_SIGNAL
};

#define NBTK_BUTTON_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_BUTTON, NbtkButtonPrivate))

struct _NbtkButtonPrivate
{
  gchar *text;

  ClutterActor *label;
  ClutterTimeline *timeline;
  ClutterEffectTemplate *press_tmpl;

  guint8 old_opacity;

  guint is_pressed : 1;
  guint is_hover : 1;
};

static guint button_signals[LAST_SIGNAL] = { 0, };


G_DEFINE_TYPE (NbtkButton, nbtk_button, NBTK_TYPE_ACTOR)

static void
nbtk_button_style_changed (NbtkWidget *button)
{
  ClutterColor *real_color;

  /* update the label styling */
  nbtk_stylable_get (NBTK_STYLABLE (button),
                     "color", &real_color,
                     NULL);
  g_object_set (G_OBJECT (NBTK_BUTTON_GET_PRIVATE (button)->label), "color", real_color, NULL);
  clutter_color_free (real_color);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (button));
 
  if (NBTK_WIDGET_CLASS (nbtk_button_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_button_parent_class)->style_changed (button);
}

static void
nbtk_button_paint (ClutterActor *actor)
{
  NbtkButton *button = NBTK_BUTTON (actor);
  NbtkButtonPrivate *priv = button->priv;

  cogl_push_matrix ();

  /*
  if (priv->texture)
    clutter_actor_paint (priv->texture);
  else
  */
    {
      ClutterActorBox allocation = { 0, };
      ClutterColor *bg_color;
      guint w, h;

      nbtk_stylable_get (NBTK_STYLABLE (button), "background-color", &bg_color, NULL);

      bg_color->alpha = clutter_actor_get_paint_opacity (actor)
                      * bg_color->alpha
                      / 255;

      clutter_actor_get_allocation_box (actor, &allocation);

      w = CLUTTER_UNITS_TO_DEVICE (allocation.x2 - allocation.x1);
      h = CLUTTER_UNITS_TO_DEVICE (allocation.y2 - allocation.y1);

      cogl_color (bg_color);
      cogl_rectangle (0, 0, w, h);
      clutter_color_free (bg_color);
    }

  if (priv->label && CLUTTER_ACTOR_IS_VISIBLE (priv->label))
    clutter_actor_paint (priv->label);

  cogl_pop_matrix ();
}



static void
nbtk_button_real_pressed (NbtkButton *button)
{
  NbtkButtonPrivate *priv = button->priv;
  ClutterActor *actor = CLUTTER_ACTOR (button);

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "active");

  if (G_UNLIKELY (!priv->press_tmpl))
    {
      priv->timeline = clutter_timeline_new_for_duration (250);
      priv->press_tmpl = clutter_effect_template_new (priv->timeline,
                                                      clutter_sine_inc_func);
      clutter_effect_template_set_timeline_clone (priv->press_tmpl, FALSE);
    }

  if (clutter_timeline_is_playing (priv->timeline))
    {
      clutter_timeline_stop (priv->timeline);
      clutter_actor_set_opacity (actor, priv->old_opacity);
    }

  priv->old_opacity = clutter_actor_get_opacity (actor);

  clutter_effect_fade (priv->press_tmpl, actor,
                       0x44,
                       NULL, NULL);
}

static void
nbtk_button_real_released (NbtkButton *button)
{
  NbtkButtonPrivate *priv = button->priv;
  ClutterActor *actor = CLUTTER_ACTOR (button);

  if (!priv->is_hover)
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "normal");
  else
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "hover");

  if (G_UNLIKELY (!priv->press_tmpl))
    {
      priv->timeline = clutter_timeline_new_for_duration (250);
      priv->press_tmpl = clutter_effect_template_new (priv->timeline,
                                                      clutter_sine_inc_func);
      clutter_effect_template_set_timeline_clone (priv->press_tmpl, FALSE);
    }

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_stop (priv->timeline);

  clutter_effect_fade (priv->press_tmpl, actor,
                       priv->old_opacity,
                       NULL, NULL);
}

static void
nbtk_button_construct_child (NbtkButton *button)
{
  NbtkButtonPrivate *priv = button->priv;
  ClutterColor *real_color;
  ClutterActor *label;

  if (!priv->text)
    return;

  label = g_object_new (CLUTTER_TYPE_LABEL,
                        "text", priv->text,
                        "alignment", PANGO_ALIGN_CENTER,
                        "ellipsize", PANGO_ELLIPSIZE_MIDDLE,
                        "use-markup", TRUE,
                        "wrap", FALSE,
                        NULL);

  nbtk_stylable_get (NBTK_STYLABLE (button),
                     "color", &real_color,
                     NULL);

  priv->label = label;

  g_object_set (G_OBJECT (label), "color", real_color, NULL);
  clutter_color_free (real_color);

  clutter_actor_show (label);
  clutter_container_add_actor (CLUTTER_CONTAINER (button), label);
}

static gboolean
nbtk_button_button_press (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  
  if (event->button == 1 &&
      event->click_count == 1)
    {
      NbtkButton *button = NBTK_BUTTON (actor);
      NbtkButtonClass *klass = NBTK_BUTTON_GET_CLASS (button);

      button->priv->is_pressed = TRUE;

      clutter_grab_pointer (actor);

      if (klass->pressed)
        klass->pressed (button);

      return TRUE;
    }

  return FALSE;
}

static gboolean
nbtk_button_button_release (ClutterActor       *actor,
                            ClutterButtonEvent *event)
{
  if (event->button == 1)
    {
      NbtkButton *button = NBTK_BUTTON (actor);
      NbtkButtonClass *klass = NBTK_BUTTON_GET_CLASS (button);

      if (!button->priv->is_pressed)
        return FALSE;

      clutter_ungrab_pointer ();

      button->priv->is_pressed = FALSE;

      if (klass->released)
        klass->released (button);

      g_signal_emit (button, button_signals[CLICKED], 0);

      return TRUE;
    }

  return FALSE;
}

static gboolean
nbtk_button_enter (ClutterActor         *actor,
                   ClutterCrossingEvent *event)
{
  NbtkButton *button = NBTK_BUTTON (actor);

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "hover");

  button->priv->is_hover = 1;

  return FALSE;
}

static gboolean
nbtk_button_leave (ClutterActor         *actor,
                   ClutterCrossingEvent *event)
{
  NbtkButton *button = NBTK_BUTTON (actor);

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "");

  button->priv->is_hover = 0;

  if (button->priv->is_pressed)
    {
      NbtkButtonClass *klass = NBTK_BUTTON_GET_CLASS (button);

      clutter_ungrab_pointer ();

      button->priv->is_pressed = FALSE;

      if (klass->released)
        klass->released (button);
    }

  return FALSE;
}

static void
nbtk_button_set_property (GObject      *gobject,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  NbtkButton *button = NBTK_BUTTON (gobject);

  switch (prop_id)
    {
    case PROP_LABEL:
      nbtk_button_set_label (button, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_button_get_property (GObject    *gobject,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  NbtkButtonPrivate *priv = NBTK_BUTTON (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, priv->text);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
nbtk_button_finalize (GObject *gobject)
{
  NbtkButtonPrivate *priv = NBTK_BUTTON (gobject)->priv;

  g_free (priv->text);

  G_OBJECT_CLASS (nbtk_button_parent_class)->finalize (gobject);
}

static void
nbtk_button_dispose (GObject *gobject)
{
  NbtkButtonPrivate *priv = NBTK_BUTTON (gobject)->priv;

  if (priv->press_tmpl)
    {
      g_object_unref (priv->press_tmpl);
      g_object_unref (priv->timeline);

      priv->press_tmpl = NULL;
      priv->timeline = NULL;
    }

  G_OBJECT_CLASS (nbtk_button_parent_class)->dispose (gobject);
}

static void
nbtk_button_class_init (NbtkButtonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *nbtk_widget_class = NBTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkButtonPrivate));

  klass->pressed = nbtk_button_real_pressed;
  klass->released = nbtk_button_real_released;

  gobject_class->set_property = nbtk_button_set_property;
  gobject_class->get_property = nbtk_button_get_property;
  gobject_class->dispose = nbtk_button_dispose;
  gobject_class->finalize = nbtk_button_finalize;

  actor_class->button_press_event = nbtk_button_button_press;
  actor_class->button_release_event = nbtk_button_button_release;
  actor_class->enter_event = nbtk_button_enter;
  actor_class->leave_event = nbtk_button_leave;
  actor_class->paint = nbtk_button_paint;
  
  nbtk_widget_class->style_changed = nbtk_button_style_changed;

  g_object_class_install_property (gobject_class,
                                   PROP_LABEL,
                                   g_param_spec_string ("label",
                                                        "Label",
                                                        "Label of the button",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  button_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkButtonClass, clicked),
                  NULL, NULL,
                  _nbtk_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
nbtk_button_init (NbtkButton *button)
{
  button->priv = NBTK_BUTTON_GET_PRIVATE (button);
}

ClutterActor *
nbtk_button_new (void)
{
  return g_object_new (NBTK_TYPE_BUTTON, NULL);
}

ClutterActor *
nbtk_button_new_with_label (const gchar *text)
{
  return g_object_new (NBTK_TYPE_BUTTON, "label", text, NULL);
}

G_CONST_RETURN gchar *
nbtk_button_get_label (NbtkButton *button)
{
  g_return_val_if_fail (NBTK_IS_BUTTON (button), NULL);

  return button->priv->text;
}

void
nbtk_button_set_label (NbtkButton  *button,
                       const gchar *text)
{
  NbtkButtonPrivate *priv;

  g_return_if_fail (NBTK_IS_BUTTON (button));

  priv = button->priv;

  g_free (priv->text);
  priv->text = g_strdup (text);

  nbtk_button_construct_child (button);

  g_object_notify (G_OBJECT (button), "label");
}
