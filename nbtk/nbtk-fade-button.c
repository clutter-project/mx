/*
 * nbtk-fade-button.c
 *
 * Copyright (c) 2009 Intel Corporation.
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
 */

#include "nbtk-fade-button.h"

G_DEFINE_TYPE (NbtkFadeButton, nbtk_fade_button, NBTK_TYPE_BUTTON)

#define NBTK_FADE_BUTTON_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_FADE_BUTTON, NbtkFadeButtonPrivate))

struct _NbtkFadeButtonPrivate
{
  ClutterActor *old_bg;
};

static void
nbtk_button_fade_transition (NbtkButton *button, ClutterActor *old_bg)
{
  NbtkFadeButtonPrivate *priv = ((NbtkFadeButton*) button)->priv;
  const gchar *pseudo_class;

  if (priv->old_bg)
    clutter_actor_unparent (priv->old_bg);

  priv->old_bg = old_bg;
  if (old_bg)
    clutter_actor_set_parent (old_bg, CLUTTER_ACTOR (button));

  pseudo_class = nbtk_stylable_get_pseudo_class (NBTK_STYLABLE (button));
  if (old_bg && g_strcmp0 ("active", pseudo_class))
    {
      gint duration = 0;
      g_object_get (button, "transition-duration", &duration, NULL);
      clutter_actor_animate (old_bg, CLUTTER_LINEAR,
                             duration,
                             "opacity", 0,
                             NULL);
    }
}

static void
nbtk_fade_button_draw_background (NbtkWidget         *self,
                                  ClutterActor       *background,
                                  const ClutterColor *color)
{
  NbtkFadeButtonPrivate *priv = NBTK_FADE_BUTTON (self)->priv;
  NbtkWidgetClass *parent_class;

  parent_class = NBTK_WIDGET_CLASS (nbtk_fade_button_parent_class);
  parent_class->draw_background (self, background, color);

  if (priv->old_bg)
    clutter_actor_paint (priv->old_bg);
}

static void
nbtk_fade_button_allocate (ClutterActor          *self,
                           const ClutterActorBox *box,
                           gboolean               origin_changed)
{
  NbtkFadeButtonPrivate *priv = NBTK_FADE_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_fade_button_parent_class)->allocate (self,
                                                                 box,
                                                                 origin_changed);

  if (priv->old_bg)
    {
      ClutterActorBox frame_box = {
          0, 0, box->x2 - box->x1, box->y2 - box->y1
      };

      clutter_actor_allocate (priv->old_bg,
                              &frame_box,
                              origin_changed);
    }
}

static void
nbtk_fade_button_class_init (NbtkFadeButtonClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  NbtkWidgetClass *widget_class = NBTK_WIDGET_CLASS (klass);
  NbtkButtonClass *button_class = NBTK_BUTTON_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkFadeButtonPrivate));

  actor_class->allocate = nbtk_fade_button_allocate;

  widget_class->draw_background = nbtk_fade_button_draw_background;
  button_class->transition = nbtk_button_fade_transition;
}

static void
nbtk_fade_button_init (NbtkFadeButton *self)
{
  self->priv = NBTK_FADE_BUTTON_GET_PRIVATE (self);
}

/**
 * nbtk_fade_button_new:
 *
 * Create a new button that fades between states.
 *
 * Returns: A newly allocated #NbtkFadeButton
 */
NbtkWidget*
nbtk_fade_button_new (void)
{
  return g_object_new (NBTK_TYPE_FADE_BUTTON, NULL);
}

/**
 * nbtk_fade_button_new_with_label:
 * @text: text to set as the label of the button
 *
 * Create a new fade button and set the label to the text specified.
 *
 * Returns: a newly allocated #NbtkFadeButton
 */
NbtkWidget*
nbtk_fade_button_new_with_label (const gchar *text)
{
  return g_object_new (NBTK_TYPE_FADE_BUTTON,
                       "label", text,
                       NULL);
}
