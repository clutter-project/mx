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
 *             Thomas Wood <thomas.wood@intel.com>
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
#include <clutter/clutter-keysyms.h>

#include "mx-button.h"

#include "mx-marshal.h"
#include "mx-stylable.h"
#include "mx-style.h"
#include "mx-texture-frame.h"
#include "mx-texture-cache.h"
#include "mx-private.h"
#include "mx-enum-types.h"
#include "mx-focusable.h"

enum
{
  PROP_0,

  PROP_LABEL,
  PROP_TOGGLE,
  PROP_ACTIVE
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

  ClutterAnimation *animation;

  ClutterActor *content_image;
};

static guint button_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_iface_init (MxStylableIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxButton, mx_button, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init));

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      ClutterColor bg_color = { 0xcc, 0xcc, 0xcc, 0x00 };
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = clutter_param_spec_color ("background-color",
                                        "Background Color",
                                        "The background color of an actor",
                                        &bg_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);

      pspec = g_param_spec_boxed ("x-mx-content-image",
                                   "Content Image",
                                   "Image used as the button",
                                   MX_TYPE_BORDER_IMAGE,
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);
    }
}

static MxFocusable*
mx_button_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  /* FIXME: pretend hover is the same as focus for now */
  mx_stylable_set_style_pseudo_class (MX_STYLABLE (focusable), "hover");
  MX_BUTTON (focusable)->priv->is_hover = TRUE;

  clutter_actor_grab_key_focus (CLUTTER_ACTOR (focusable));

  return focusable;
}

static MxFocusable*
mx_button_move_focus (MxFocusable *focusable,
                      MxDirection  direction,
                      MxFocusable *from)
{
  /* check if focus is being moved from us */
  if (focusable == from)
    {
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (focusable), "");
      MX_BUTTON (focusable)->priv->is_hover = FALSE;
    }

  return NULL;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_button_accept_focus;
  iface->move_focus = mx_button_move_focus;
}

static void
mx_button_update_label_style (MxButton *button)
{
  ClutterActor *label;

  label = mx_bin_get_child ((MxBin*) button);

  /* check the child is really a label */
  if (!CLUTTER_IS_TEXT (label))
    return;

  mx_stylable_apply_clutter_text_attributes (MX_STYLABLE (button),
                                             CLUTTER_TEXT (label));
}

static void
mx_button_dispose_old_bg (MxButton *button)
{
  MxButtonPrivate *priv = button->priv;

  if (priv->content_image)
    {
      clutter_actor_unparent (priv->content_image);
      priv->content_image = NULL;
    }

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
  MxBorderImage *content_image = NULL;

  /* update the label styling */
  mx_button_update_label_style (button);

  mx_stylable_get (MX_STYLABLE (widget), "x-mx-content-image", &content_image,
                   NULL);

  if (content_image)
    {
      GError *err = NULL;

      if (priv->content_image)
        {
          clutter_actor_unparent (priv->content_image);
        }

      priv->content_image = clutter_texture_new_from_file (content_image->uri,
                                                           &err);

      if (priv->content_image)
        clutter_actor_set_parent (priv->content_image, CLUTTER_ACTOR (widget));

      if (err)
        {
          g_warning ("Could not load content image: %s", err->message);

          g_error_free (err);
        }

      g_boxed_free (MX_TYPE_BORDER_IMAGE, content_image);

      return;
    }

  /* run a transition if applicable */
  if (priv->old_bg &&
      (!mx_stylable_get_style_pseudo_class (MX_STYLABLE (widget))))
    {
      ClutterAnimation *animation;

      if (!clutter_actor_get_parent (priv->old_bg))
        {
          clutter_actor_set_parent (priv->old_bg, (ClutterActor*) widget);
          priv->old_bg_parented = TRUE;
        }

      animation = clutter_actor_animate (priv->old_bg,
                                         CLUTTER_LINEAR,
                                         120,
                                         "opacity", 0,
                                         NULL);
      g_signal_connect (animation, "completed",
                        G_CALLBACK (mx_animation_completed), button);
    }
}


static void
mx_button_push (MxButton           *button,
                ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (button);

  mx_widget_hide_tooltip (widget);

  button->priv->is_pressed = TRUE;

  clutter_grab_pointer (CLUTTER_ACTOR (button));

  mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), "active");

  if (event)
    mx_widget_long_press_query (widget, event);
}

static void
mx_button_pull (MxButton *button)
{
  MxWidget *widget = MX_WIDGET (button);
  MxButtonPrivate *priv = button->priv;

  if (!button->priv->is_pressed)
    return;

  clutter_ungrab_pointer ();

  if (button->priv->is_toggle)
    {
      mx_button_set_checked (button, !button->priv->is_checked);
    }

  button->priv->is_pressed = FALSE;

  g_signal_emit (button, button_signals[CLICKED], 0);

  mx_widget_long_press_cancel (widget);

  if (priv->is_checked)
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), "checked");
  else if (!priv->is_hover)
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), NULL);
  else
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), "hover");
}

static gboolean
mx_button_button_press (ClutterActor       *actor,
                        ClutterButtonEvent *event)
{
  if (event->button == 1)
    {
      mx_button_push (MX_BUTTON (actor), event);

      return TRUE;
    }

  return TRUE;
}

static gboolean
mx_button_button_release (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  if (event->button == 1)
    {
      mx_button_pull (MX_BUTTON (actor));

      return TRUE;
    }

  return TRUE;
}

static gboolean
mx_button_key_press (ClutterActor    *actor,
                     ClutterKeyEvent *event)
{
  if (event->keyval == CLUTTER_Return || event->keyval == CLUTTER_space)
    {
      mx_button_push (MX_BUTTON (actor), NULL);

      return TRUE;
    }

  return FALSE;
}

static gboolean
mx_button_key_release (ClutterActor    *actor,
                       ClutterKeyEvent *event)
{
  if (event->keyval == CLUTTER_Return || event->keyval == CLUTTER_space)
    {
      mx_button_pull (MX_BUTTON (actor));

      return TRUE;
    }

  return FALSE;
}

static gboolean
mx_button_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxButton *button = MX_BUTTON (actor);
  MxWidget *widget = MX_WIDGET (actor);

  if (event->source != actor)
    return FALSE;

  if (!button->priv->is_checked)
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "hover");

  button->priv->is_hover = 1;

  if (mx_widget_get_has_tooltip (widget))
    mx_widget_show_tooltip (widget);

  return FALSE;
}

static gboolean
mx_button_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxButton *button = MX_BUTTON (actor);
  MxWidget *widget = MX_WIDGET (actor);

  if (event->source != actor)
    return FALSE;

  button->priv->is_hover = 0;

  if (button->priv->is_pressed)
    {
      clutter_ungrab_pointer ();

      button->priv->is_pressed = FALSE;
    }

  if (button->priv->is_checked)
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "checked");
  else
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), NULL);

  if (mx_widget_get_has_tooltip (widget))
    mx_widget_hide_tooltip (widget);

  return FALSE;
}

static void
mx_button_set_property (GObject      *gobject,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxButton *button = MX_BUTTON (gobject);

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

  if (priv->content_image)
    clutter_actor_map (priv->content_image);

  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_map (priv->old_bg);
}

static void
mx_button_unmap (ClutterActor *self)
{
  MxButtonPrivate *priv = MX_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->unmap (self);

  if (priv->content_image)
    clutter_actor_map (priv->content_image);

  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_unmap (priv->old_bg);
}

static void
mx_button_paint_background (MxWidget           *widget,
                            ClutterActor       *background,
                            const ClutterColor *color)
{
  MxButtonPrivate *priv;

  priv = MX_BUTTON (widget)->priv;

  if (priv->content_image)
    {
      clutter_actor_paint (priv->content_image);

      return;
    }

  MX_WIDGET_CLASS (mx_button_parent_class)->paint_background (widget,
                                                              background,
                                                              color);


  if (priv->old_bg && priv->old_bg_parented)
    clutter_actor_paint (priv->old_bg);
}

static void
mx_button_allocate (ClutterActor           *actor,
                    const ClutterActorBox  *box,
                    ClutterAllocationFlags  flags)
{
  MxButtonPrivate *priv = MX_BUTTON (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->allocate (actor, box, flags);

  if (priv->content_image)
    {
      ClutterActorBox childbox;

      childbox.x1 = 0;
      childbox.y1 = 0;
      childbox.x2 = (box->x2 - box->x1);
      childbox.y2 = (box->y2 - box->y1);

      clutter_actor_allocate (priv->content_image, &childbox, flags);
    }

  mx_bin_allocate_child (MX_BIN (actor), box, flags);
}

static void
mx_button_get_preferred_width (ClutterActor *actor,
                               gfloat        for_height,
                               gfloat       *min_width,
                               gfloat       *pref_width)
{
  MxButtonPrivate *priv = MX_BUTTON (actor)->priv;

  if (priv->content_image)
    {
      clutter_actor_get_preferred_width (priv->content_image, for_height,
                                         min_width, pref_width);
      return;
    }

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->get_preferred_width (actor,
                                                                     for_height,
                                                                     min_width,
                                                                     pref_width);
}

static void
mx_button_get_preferred_height (ClutterActor *actor,
                                gfloat        for_width,
                                gfloat       *min_height,
                                gfloat       *pref_height)
{
  MxButtonPrivate *priv = MX_BUTTON (actor)->priv;

  if (priv->content_image)
    {
      clutter_actor_get_preferred_height (priv->content_image, for_width,
                                          min_height, pref_height);
      return;
    }

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->get_preferred_height (actor,
                                                                      for_width,
                                                                      min_height,
                                                                      pref_height);
}

static void
mx_button_paint (ClutterActor *actor)
{
  MxButtonPrivate *priv = MX_BUTTON (actor)->priv;

  if (priv->content_image)
    clutter_actor_paint (priv->content_image);

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->paint (actor);
}

static void
mx_button_class_init (MxButtonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxButtonPrivate));

  gobject_class->set_property = mx_button_set_property;
  gobject_class->get_property = mx_button_get_property;
  gobject_class->dispose = mx_button_dispose;
  gobject_class->finalize = mx_button_finalize;

  actor_class->button_press_event = mx_button_button_press;
  actor_class->button_release_event = mx_button_button_release;
  actor_class->key_press_event = mx_button_key_press;
  actor_class->key_release_event = mx_button_key_release;
  actor_class->enter_event = mx_button_enter;
  actor_class->leave_event = mx_button_leave;
  actor_class->get_preferred_width = mx_button_get_preferred_width;
  actor_class->get_preferred_height = mx_button_get_preferred_height;
  actor_class->paint = mx_button_paint;

  actor_class->map = mx_button_map;
  actor_class->unmap = mx_button_unmap;
  actor_class->allocate = mx_button_allocate;

  widget_class->paint_background = mx_button_paint_background;

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
ClutterActor *
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
ClutterActor *
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
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), "checked");
      else
      if (button->priv->is_hover)
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), "hover");
      else
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (button), NULL);

      g_object_notify (G_OBJECT (button), "checked");
    }
}
