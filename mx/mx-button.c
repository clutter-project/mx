/*
 * mx-button.c: Plain button actor
 *
 * Copyright 2007 OpenedHand
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
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 *
 */

/**
 * SECTION:mx-button
 * @short_description: Button widget
 *
 * A button widget with support for either a text label or icon, toggle mode
 * and transitions effects between states.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include "mx-button.h"

#include "mx-marshal.h"
#include "mx-stylable.h"
#include "mx-style.h"
#include "mx-texture-frame.h"
#include "mx-texture-cache.h"
#include "mx-private.h"

enum
{
  PROP_0,

  PROP_LABEL,
  PROP_TOGGLE,
  PROP_ACTIVE,
  PROP_TRANSITION
};

enum
{
  CLICKED,

  LAST_SIGNAL
};

#define MX_BUTTON_GET_PRIVATE(obj)    \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_BUTTON, MxButtonPrivate))

struct _MxButtonPrivate
{
  gchar            *text;

  ClutterActor     *old_bg;
  gboolean          old_bg_parented; /* TRUE if we have adopted old_bg */

  guint8            old_opacity;

  guint             is_pressed : 1;
  guint             is_hover : 1;
  guint             is_checked : 1;
  guint             is_toggle : 1;

  gint              transition_duration;

  ClutterAnimation *animation;

  gint              spacing;
};

static guint button_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxButton, mx_button, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init));

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      ClutterColor bg_color = { 0xcc, 0xcc, 0xcc, 0x00 };
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_int ("border-spacing",
                                "Border Spacing",
                                "Spacing between internal elements",
                                0, G_MAXINT, 6,
                                G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);


      is_initialized = TRUE;

      pspec = clutter_param_spec_color ("background-color",
                                        "Background Color",
                                        "The background color of an actor",
                                        &bg_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);
    }
}

static void
mx_button_update_label_style (MxButton *button)
{
  ClutterColor *real_color = NULL;
  gchar *font_string = NULL;
  gchar *font_name = NULL;
  gint font_size = 0;
  ClutterActor *label;

  label = mx_bin_get_child ((MxBin*) button);

  /* check the child is really a label */
  if (!CLUTTER_IS_TEXT (label))
    return;

  mx_stylable_get (MX_STYLABLE (button),
                   "color", &real_color,
                   "font-family", &font_name,
                   "font-size", &font_size,
                   NULL);

  if (font_name || font_size)
    {
      if (font_name && font_size)
        font_string = g_strdup_printf ("%s %dpx", font_name, font_size);
      else
        {
          if (font_size)
            font_string = g_strdup_printf ("%dpx", font_size);
          else
            font_string = font_name;
        }

      clutter_text_set_font_name (CLUTTER_TEXT (label), font_string);

      if (font_string != font_name)
        g_free (font_string);
    }

  g_free (font_name);

  if (real_color)
    {
      clutter_text_set_color (CLUTTER_TEXT (label), real_color);
      clutter_color_free (real_color);
    }
}

static void
mx_button_dispose_old_bg (MxButton *button)
{
  MxButtonPrivate *priv = button->priv;

  if (priv->old_bg)
    {
      if (priv->old_bg_parented)
        {
          clutter_actor_unparent (priv->old_bg);
          priv->old_bg_parented = FALSE;
        }
      g_object_unref (priv->old_bg);
      priv->old_bg = NULL;
    }
}

static void
mx_button_stylable_changed (MxStylable *stylable)
{
  MxButton *button = MX_BUTTON (stylable);
  ClutterActor *bg_image;

  mx_button_dispose_old_bg (button);

  bg_image = mx_widget_get_border_image ((MxWidget*) button);
  if (bg_image)
    button->priv->old_bg = g_object_ref (bg_image);
}

static void
mx_animation_completed (ClutterAnimation *animation,
                        MxButton         *button)
{
  mx_button_dispose_old_bg (button);
}

static void
mx_button_style_changed (MxWidget *widget)
{
  MxButton *button = MX_BUTTON (widget);
  MxButtonPrivate *priv = button->priv;
  MxButtonClass *button_class = MX_BUTTON_GET_CLASS (button);

  /* get the spacing value */
  mx_stylable_get (MX_STYLABLE (widget),
                   "border-spacing", &priv->spacing,
                   NULL);

  /* update the label styling */
  mx_button_update_label_style (button);

  /* run a transition if applicable */
  if (button_class->transition)
    {
      button_class->transition (button, priv->old_bg);
    }
  else
    {
      if (priv->old_bg &&
          (!mx_widget_get_style_pseudo_class (widget)))
        {
          ClutterAnimation *animation;
          if (!clutter_actor_get_parent (priv->old_bg))
            {
              clutter_actor_set_parent (priv->old_bg, (ClutterActor*) widget);
              priv->old_bg_parented = TRUE;
            }
          if (priv->transition_duration > 0)
            {
              animation = clutter_actor_animate (priv->old_bg,
                                                 CLUTTER_LINEAR,
                                                 priv->transition_duration,
                                                 "opacity", 0,
                                                 NULL);
              g_signal_connect (animation, "completed",
                                G_CALLBACK (mx_animation_completed), button);
            }
          else
            {
              mx_button_dispose_old_bg (button);
            }

        }
    }
}

static void
mx_button_real_pressed (MxButton *button)
{
  mx_widget_set_style_pseudo_class ((MxWidget*) button, "active");
}

static void
mx_button_real_released (MxButton *button)
{
  MxButtonPrivate *priv = button->priv;

  if (priv->is_checked)
    mx_widget_set_style_pseudo_class ((MxWidget*) button, "checked");
  else if (!priv->is_hover)
    mx_widget_set_style_pseudo_class ((MxWidget*) button, NULL);
  else
    mx_widget_set_style_pseudo_class ((MxWidget*) button, "hover");

}

static gboolean
mx_button_button_press (ClutterActor       *actor,
                        ClutterButtonEvent *event)
{
  mx_widget_hide_tooltip (MX_WIDGET (actor));

  if (event->button == 1)
    {
      MxButton *button = MX_BUTTON (actor);
      MxButtonClass *klass = MX_BUTTON_GET_CLASS (button);

      button->priv->is_pressed = TRUE;

      clutter_grab_pointer (actor);

      if (klass->pressed)
        klass->pressed (button);

      return TRUE;
    }

  return FALSE;
}

static gboolean
mx_button_button_release (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  if (event->button == 1)
    {
      MxButton *button = MX_BUTTON (actor);
      MxButtonClass *klass = MX_BUTTON_GET_CLASS (button);

      if (!button->priv->is_pressed)
        return FALSE;

      clutter_ungrab_pointer ();

      if (button->priv->is_toggle)
        {
          mx_button_set_checked (button, !button->priv->is_checked);
        }

      button->priv->is_pressed = FALSE;

      if (klass->released)
        klass->released (button);

      g_signal_emit (button, button_signals[CLICKED], 0);

      return TRUE;
    }

  return FALSE;
}

static gboolean
mx_button_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxButton *button = MX_BUTTON (actor);

  if (!button->priv->is_checked)
    mx_widget_set_style_pseudo_class ((MxWidget*) button, "hover");

  button->priv->is_hover = 1;

  return CLUTTER_ACTOR_CLASS (mx_button_parent_class)->enter_event (actor, event);
}

static gboolean
mx_button_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxButton *button = MX_BUTTON (actor);

  button->priv->is_hover = 0;

  if (button->priv->is_pressed)
    {
      MxButtonClass *klass = MX_BUTTON_GET_CLASS (button);

      clutter_ungrab_pointer ();

      button->priv->is_pressed = FALSE;

      if (klass->released)
        klass->released (button);
    }

  if (button->priv->is_checked)
    mx_widget_set_style_pseudo_class ((MxWidget*) button, "checked");
  else
    mx_widget_set_style_pseudo_class ((MxWidget*) button, NULL);

  return CLUTTER_ACTOR_CLASS (mx_button_parent_class)->leave_event (actor, event);
}

static void
mx_button_set_property (GObject      *gobject,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxButton *button = MX_BUTTON (gobject);
  MxButtonPrivate *priv = MX_BUTTON (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      mx_button_set_label (button, g_value_get_string (value));
      break;
    case PROP_TOGGLE:
      mx_button_set_toggle_mode (button, g_value_get_boolean (value));
      break;
    case PROP_ACTIVE:
      mx_button_set_checked (button, g_value_get_boolean (value));
      break;
    case PROP_TRANSITION:
      priv->transition_duration = g_value_get_int (value);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_button_get_property (GObject    *gobject,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxButtonPrivate *priv = MX_BUTTON (gobject)->priv;

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, priv->text);
      break;
    case PROP_TOGGLE:
      g_value_set_boolean (value, priv->is_toggle);
      break;
    case PROP_ACTIVE:
      g_value_set_boolean (value, priv->is_checked);
      break;
    case PROP_TRANSITION:
      g_value_set_int (value, priv->transition_duration);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_button_finalize (GObject *gobject)
{
  MxButtonPrivate *priv = MX_BUTTON (gobject)->priv;

  g_free (priv->text);

  G_OBJECT_CLASS (mx_button_parent_class)->finalize (gobject);
}

static void
mx_button_dispose (GObject *gobject)
{
  mx_button_dispose_old_bg (MX_BUTTON (gobject));

  G_OBJECT_CLASS (mx_button_parent_class)->dispose (gobject);
}

static void
mx_button_map (ClutterActor *self)
{
  MxButtonPrivate *priv = MX_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->map (self);

  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_map (priv->old_bg);
}

static void
mx_button_unmap (ClutterActor *self)
{
  MxButtonPrivate *priv = MX_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->unmap (self);

  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_unmap (priv->old_bg);
}

static void
mx_button_draw_background (MxWidget           *widget,
                           ClutterActor       *background,
                           const ClutterColor *color)
{
  MxButtonPrivate *priv;

  MX_WIDGET_CLASS (mx_button_parent_class)->draw_background (widget, background, color);

  priv = MX_BUTTON (widget)->priv;

  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_paint (priv->old_bg);
}

static void
mx_button_class_init (MxButtonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxButtonPrivate));

  klass->pressed = mx_button_real_pressed;
  klass->released = mx_button_real_released;

  gobject_class->set_property = mx_button_set_property;
  gobject_class->get_property = mx_button_get_property;
  gobject_class->dispose = mx_button_dispose;
  gobject_class->finalize = mx_button_finalize;

  actor_class->button_press_event = mx_button_button_press;
  actor_class->button_release_event = mx_button_button_release;
  actor_class->enter_event = mx_button_enter;
  actor_class->leave_event = mx_button_leave;

  actor_class->map = mx_button_map;
  actor_class->unmap = mx_button_unmap;

  widget_class->draw_background = mx_button_draw_background;

  pspec = g_param_spec_string ("label",
                               "Label",
                               "Label of the button",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

  pspec = g_param_spec_boolean ("toggle-mode",
                                "Toggle Mode",
                                "Enable or disable toggling",
                                FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TOGGLE, pspec);

  pspec = g_param_spec_boolean ("checked",
                                "Checked",
                                "Indicates if a toggle button is \"on\""
                                " or \"off\"",
                                FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ACTIVE, pspec);

  pspec = g_param_spec_int ("transition-duration",
                            "Transition Duration",
                            "Duration of the state transition effect",
                            0, G_MAXINT, 120, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TRANSITION, pspec);


  /**
   * MxButton::clicked:
   * @button: the object that received the signal
   *
   * Emitted when the user activates the button, either with a mouse press and
   * release or with the keyboard.
   */

  button_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxButtonClass, clicked),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
mx_button_init (MxButton *button)
{
  button->priv = MX_BUTTON_GET_PRIVATE (button);
  button->priv->transition_duration = 120;
  button->priv->spacing = 6;

  clutter_actor_set_reactive ((ClutterActor *) button, TRUE);

  g_signal_connect (button, "style-changed",
                    G_CALLBACK (mx_button_style_changed), NULL);

  g_signal_connect (button, "stylable-changed",
                    G_CALLBACK (mx_button_stylable_changed), NULL);
}

/**
 * mx_button_new:
 *
 * Create a new button
 *
 * Returns: a new #MxButton
 */
MxWidget *
mx_button_new (void)
{
  return g_object_new (MX_TYPE_BUTTON, NULL);
}

/**
 * mx_button_new_with_label:
 * @text: text to set the label to
 *
 * Create a new #MxButton with the specified label
 *
 * Returns: a new #MxButton
 */
MxWidget *
mx_button_new_with_label (const gchar *text)
{
  return g_object_new (MX_TYPE_BUTTON, "label", text, NULL);
}

/**
 * mx_button_get_label:
 * @button: a #MxButton
 *
 * Get the text displayed on the button
 *
 * Returns: the text for the button. This must not be freed by the application
 */
G_CONST_RETURN gchar *
mx_button_get_label (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), NULL);

  return button->priv->text;
}

/**
 * mx_button_set_label:
 * @button: a #Mxbutton
 * @text: text to set the label to
 *
 * Sets the text displayed on the button
 */
void
mx_button_set_label (MxButton    *button,
                     const gchar *text)
{
  MxButtonPrivate *priv;
  ClutterActor *label;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;

  g_free (priv->text);

  if (text)
    priv->text = g_strdup (text);
  else
    priv->text = g_strdup ("");

  label = mx_bin_get_child ((MxBin*) button);

  if (label && CLUTTER_IS_TEXT (label))
    {
      clutter_text_set_text (CLUTTER_TEXT (label), priv->text);
    }
  else
    {
      label = g_object_new (CLUTTER_TYPE_TEXT,
                            "text", priv->text,
                            "line-alignment", PANGO_ALIGN_CENTER,
                            "ellipsize", PANGO_ELLIPSIZE_END,
                            NULL);
      mx_bin_set_child ((MxBin*) button, label);
    }

  mx_stylable_changed ((MxStylable*) button);

  g_object_notify (G_OBJECT (button), "label");
}

/**
 * mx_button_get_toggle_mode:
 * @button: a #MxButton
 *
 * Get the toggle mode status of the button.
 *
 * Returns: #TRUE if toggle mode is set, otherwise #FALSE
 */
gboolean
mx_button_get_toggle_mode (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);

  return button->priv->is_toggle;
}

/**
 * mx_button_set_toggle_mode:
 * @button: a #Mxbutton
 * @toggle: #TRUE or #FALSE
 *
 * Enables or disables toggle mode for the button. In toggle mode, the active
 * state will be "toggled" when the user clicks the button.
 */
void
mx_button_set_toggle_mode (MxButton *button,
                           gboolean  toggle)
{
  g_return_if_fail (MX_IS_BUTTON (button));

  button->priv->is_toggle = toggle;

  g_object_notify (G_OBJECT (button), "toggle-mode");
}

/**
 * mx_button_get_checked:
 * @button: a #MxButton
 *
 * Get the state of the button that is in toggle mode.
 *
 * Returns: #TRUE if the button is checked, or #FALSE if not
 */
gboolean
mx_button_get_checked (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);

  return button->priv->is_checked;
}

/**
 * mx_button_set_checked:
 * @button: a #Mxbutton
 * @checked: #TRUE or #FALSE
 *
 * Sets the pressed state of the button. This is only really useful if the
 * button has #toggle-mode mode set to #TRUE.
 */
void
mx_button_set_checked (MxButton *button,
                       gboolean  checked)
{
  g_return_if_fail (MX_IS_BUTTON (button));

  if (button->priv->is_checked != checked)
    {
      button->priv->is_checked = checked;

      if (checked)
        mx_widget_set_style_pseudo_class ((MxWidget*) button, "checked");
      else
      if (button->priv->is_hover)
        mx_widget_set_style_pseudo_class ((MxWidget*) button, "hover");
      else
        mx_widget_set_style_pseudo_class ((MxWidget*) button, NULL);
    }

  g_object_notify (G_OBJECT (button), "checked");
}
