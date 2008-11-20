/* nbtk-button.c: Plain button actor
 *
 * Copyright 2007 OpenedHand
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
#include "nbtk-texture-frame.h"
#include "nbtk-texture-cache.h"

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
  ClutterActor *icon;
  ClutterTimeline *timeline;
  ClutterEffectTemplate *press_tmpl;

  guint8 old_opacity;

  guint is_pressed : 1;
  guint is_hover : 1;

  ClutterActor     *bg_image;
  ClutterActor     *old_bg;
  ClutterColor     *bg_color;
};

static guint button_signals[LAST_SIGNAL] = { 0, };


G_DEFINE_TYPE (NbtkButton, nbtk_button, NBTK_TYPE_WIDGET)

static void
style_changed_completed_effect (ClutterActor *actor, NbtkButton *button)
{
  NbtkButtonPrivate *priv = NBTK_BUTTON_GET_PRIVATE (button);

  if (priv->old_bg)
    g_object_unref (priv->old_bg);
  priv->old_bg = NULL;
}

static void
nbtk_button_style_changed (NbtkWidget *button)
{
  ClutterColor *real_color;
  gchar *bg_url = NULL;
  NbtkButtonPrivate *priv = NBTK_BUTTON_GET_PRIVATE (button);

  /* update the label styling */
  if (priv->label)
    {
      nbtk_stylable_get (NBTK_STYLABLE (button),
                         "color", &real_color,
                         NULL);
      g_object_set (G_OBJECT (priv->label),
                    "color", real_color,
                    NULL);
      clutter_color_free (real_color);
    }

  if (priv->bg_color)
    {
      clutter_color_free (priv->bg_color);
      priv->bg_color = NULL;
    }

  /* cache these values for use in the paint function */
  nbtk_stylable_get (NBTK_STYLABLE (button),
                    "background-color", &priv->bg_color,
                    "background-image", &bg_url,
                    NULL);

  if (bg_url)
    {
      NbtkTextureCache *cache;
      ClutterActor *texture;
      const gchar *pseudo_class;

      if (priv->bg_image)
        priv->old_bg = priv->bg_image;

      cache = nbtk_texture_cache_get_default ();

      /* the TextureFrame doesn't work with texture clones as it only
       * references the texture data */
      texture = nbtk_texture_cache_get_texture (cache, bg_url, FALSE);

      priv->bg_image = nbtk_texture_frame_new (CLUTTER_TEXTURE (texture),
                                               0, 0, 0, 0);
      g_object_unref (texture);
      clutter_actor_set_parent (CLUTTER_ACTOR (priv->bg_image),
                                CLUTTER_ACTOR (button));
      g_free (bg_url);

      if (G_UNLIKELY (!priv->press_tmpl))
        {
          priv->timeline = clutter_timeline_new_for_duration (200);
          priv->press_tmpl = clutter_effect_template_new (priv->timeline,
                                                          clutter_sine_inc_func);
          clutter_effect_template_set_timeline_clone (priv->press_tmpl, FALSE);
        }

      if (clutter_timeline_is_playing (priv->timeline))
        {
          clutter_timeline_stop (priv->timeline);
        }

      /* start a fade if we're not changing to pressed ("active") state */
      pseudo_class = nbtk_stylable_get_pseudo_class (NBTK_STYLABLE (button));
      if (g_strcmp0 ("active", pseudo_class))
        {
          if (priv->old_bg)
            clutter_effect_fade (priv->press_tmpl, priv->old_bg,
                                 0x00,
                                 (ClutterEffectCompleteFunc) style_changed_completed_effect,
                                 button);
        }
      else
        {
          /* remove the old image to perform instant transition when pressed */
          clutter_container_remove (CLUTTER_CONTAINER (button), priv->old_bg,
                                    NULL);
          g_object_unref (priv->old_bg);
          priv->old_bg = NULL;
        }

    }
  else
    {
      if (priv->bg_image)
        {
          g_object_unref (priv->bg_image);
          priv->bg_image = NULL;
        }
    }

  /* queue a relayout, which also calls redraw */
  clutter_actor_queue_relayout (CLUTTER_ACTOR (button));

 
  if (NBTK_WIDGET_CLASS (nbtk_button_parent_class)->style_changed)
    NBTK_WIDGET_CLASS (nbtk_button_parent_class)->style_changed (button);
}

static void
nbtk_button_paint (ClutterActor *actor)
{
  NbtkButton *button = NBTK_BUTTON (actor);
  NbtkButtonPrivate *priv = button->priv;

  if (priv->bg_image)
    {
      clutter_actor_paint (priv->bg_image);

      if (priv->old_bg)
        clutter_actor_paint (priv->old_bg);
    }
  else
    {
      ClutterActorBox allocation = { 0, };
      ClutterColor *bg_color = priv->bg_color;
      guint w, h;


      if (bg_color)
        {
          bg_color->alpha = clutter_actor_get_paint_opacity (actor)
                          * bg_color->alpha
                          / 255;

          clutter_actor_get_allocation_box (actor, &allocation);
 
          w = CLUTTER_UNITS_TO_DEVICE (allocation.x2 - allocation.x1);
          h = CLUTTER_UNITS_TO_DEVICE (allocation.y2 - allocation.y1);

          cogl_color (bg_color);
          cogl_rectangle (0, 0, w, h);
        }
    }

  if (CLUTTER_ACTOR_CLASS (nbtk_button_parent_class)->paint)
    CLUTTER_ACTOR_CLASS (nbtk_button_parent_class)->paint (CLUTTER_ACTOR (button));
}



static void
nbtk_button_real_pressed (NbtkButton *button)
{
  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "active");
}

static void
nbtk_button_real_released (NbtkButton *button)
{
  NbtkButtonPrivate *priv = button->priv;

  if (!priv->is_hover)
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "normal");
  else
    nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "hover");

}

static void
nbtk_button_construct_child (NbtkButton *button)
{
  NbtkButtonPrivate *priv = button->priv;
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

  priv->label = label;

  if (priv->bg_image)
    clutter_container_add_actor (CLUTTER_CONTAINER (priv->bg_image), label);
  else
    clutter_container_add_actor (CLUTTER_CONTAINER (button), label);

}

static gboolean
nbtk_button_button_press (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  
  if (event->button == 1)
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

  button->priv->is_hover = 0;


  if (button->priv->is_pressed)
    {
      NbtkButtonClass *klass = NBTK_BUTTON_GET_CLASS (button);

      clutter_ungrab_pointer ();

      button->priv->is_pressed = FALSE;

      if (klass->released)
        klass->released (button);
    }

  nbtk_widget_set_style_pseudo_class (NBTK_WIDGET (button), "");

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
      if (clutter_timeline_is_playing (priv->timeline))
        {
          clutter_timeline_stop (priv->timeline);
        }

      g_object_unref (priv->press_tmpl);
      g_object_unref (priv->timeline);

      priv->press_tmpl = NULL;
      priv->timeline = NULL;
    }

  G_OBJECT_CLASS (nbtk_button_parent_class)->dispose (gobject);
}

static void
nbtk_button_allocate (ClutterActor          *self,
                      const ClutterActorBox *box,
                      gboolean               absolute_origin_changed)
{
  NbtkButtonPrivate *priv = NBTK_BUTTON_GET_PRIVATE (self);

  if (priv->bg_image)
    {
      clutter_actor_set_size (priv->bg_image,
                              CLUTTER_UNITS_TO_DEVICE (box->x2 - box->x1),
                              CLUTTER_UNITS_TO_DEVICE (box->y2 - box->y1));
      clutter_actor_set_position (priv->bg_image,
                                  CLUTTER_UNITS_TO_DEVICE (box->x1),
                                  CLUTTER_UNITS_TO_DEVICE (box->y1));
    }

  if (priv->old_bg)
    {
      clutter_actor_set_size (priv->old_bg,
                              CLUTTER_UNITS_TO_DEVICE (box->x2 - box->x1),
                              CLUTTER_UNITS_TO_DEVICE (box->y2 - box->y1));
      clutter_actor_set_position (priv->bg_image,
                                  CLUTTER_UNITS_TO_DEVICE (box->x1),
                                  CLUTTER_UNITS_TO_DEVICE (box->y1));
    }

  CLUTTER_ACTOR_CLASS (nbtk_button_parent_class)->allocate (self, box, absolute_origin_changed);
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
  actor_class->allocate = nbtk_button_allocate;
  
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

/**
 * nbtk_button_new:
 *
 * Create a new button
 *
 * Returns: a new #NbtkButton
 */
NbtkWidget *
nbtk_button_new (void)
{
  return g_object_new (NBTK_TYPE_BUTTON, NULL);
}

/**
 * nbtk_button_new_with_label:
 * @text: text to set the label to
 *
 * Create a new #NbtkButton with the specified label
 *
 * Returns: a new #NbtkButton
 */
NbtkWidget *
nbtk_button_new_with_label (const gchar *text)
{
  return g_object_new (NBTK_TYPE_BUTTON, "label", text, NULL);
}

/**
 * nbtk_button_get_label:
 * @button: a #NbtkButton
 *
 * Get the text displayed on the button
 *
 * Returns: the text for the button. This must not be freed by the application
 */
G_CONST_RETURN gchar *
nbtk_button_get_label (NbtkButton *button)
{
  g_return_val_if_fail (NBTK_IS_BUTTON (button), NULL);

  return button->priv->text;
}

/**
 * nbtk_button_set_label:
 * @button: a #Nbtkbutton
 * @text: text to set the label to
 *
 * Sets the text displayed on the button
 */
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

/**
 * nbtk_button_set_icon_from_file:
 * @button: a #NbtkButton
 * @filename: path to an image to set
 *
 * Sets the icon of the button to the image specified in @filename.
 * If @filename is NULL, the current icon is removed.
 */
void
nbtk_button_set_icon_from_file (NbtkButton *button,
                                gchar      *filename)
{
  NbtkButtonPrivate *priv;
  GError *err = NULL;
  ClutterActor *icon;

  g_return_if_fail (NBTK_IS_BUTTON (button));

  priv = button->priv;

  if (filename == NULL)
    {
      if (priv->icon)
        clutter_container_remove_actor (CLUTTER_CONTAINER (button),
                                        priv->icon);
    }

  icon = clutter_texture_new_from_file (filename, &err);

  if (err)
    {
      g_warning ("Failed to load icon from file. %s", err->message);
      g_error_free (err);
      return;
    }


  /* remove the label if present */
  if (priv->label)
    clutter_container_remove_actor (CLUTTER_CONTAINER (button),
                                    priv->label);
  g_free (priv->text);

  clutter_container_add_actor (CLUTTER_CONTAINER (button), icon);
}
