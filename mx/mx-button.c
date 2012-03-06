/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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
  PROP_ICON_NAME,
  PROP_ICON_SIZE,
  PROP_IS_TOGGLE,
  PROP_TOGGLED,
  PROP_ACTION,
  PROP_ICON_POSITION,
  PROP_ICON_VISIBLE,
  PROP_LABEL_VISIBLE
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
  gchar            *icon_name;
  gchar            *style_icon_name;
  guint             icon_size;
  guint             style_icon_size;

  guint8            old_opacity;

  guint             is_pressed : 1;
  guint             is_toggle : 1;
  guint             is_toggled : 1;

  ClutterAnimation *animation;

  ClutterActor *content_image;

  MxAction   *action;
  MxPosition  icon_position;
  guint       icon_visible : 1;
  guint       label_visible : 1;

  ClutterActor *hbox;
  ClutterActor *icon;
  ClutterActor *label;

  GBinding *action_label_binding;
  GBinding *action_icon_binding;
};

static guint button_signals[LAST_SIGNAL] = { 0, };

static void mx_button_update_contents (MxButton *self);

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

      pspec = g_param_spec_string ("x-mx-icon-name",
                                   "Icon name",
                                   "Named icon to place inside the button",
                                   NULL,
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);

      pspec = g_param_spec_uint ("x-mx-icon-size",
                                 "Icon size",
                                 "Size to use for icon",
                                 1, G_MAXINT, 48,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_BUTTON, pspec);
    }
}

static MxFocusable*
mx_button_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  mx_stylable_style_pseudo_class_add (MX_STYLABLE (focusable), "focus");

  clutter_actor_grab_key_focus (CLUTTER_ACTOR (focusable));

  return focusable;
}

static MxFocusable*
mx_button_move_focus (MxFocusable      *focusable,
                      MxFocusDirection  direction,
                      MxFocusable      *from)
{
  /* check if focus is being moved from us */
  if (focusable == from)
    mx_stylable_style_pseudo_class_remove (MX_STYLABLE (focusable), "focus");

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
  MxButtonPrivate *priv = button->priv;

  mx_stylable_apply_clutter_text_attributes (MX_STYLABLE (button),
                                             CLUTTER_TEXT (priv->label));
}


static void
mx_button_style_changed (MxWidget *widget)
{
  MxButton *button = MX_BUTTON (widget);
  MxButtonPrivate *priv = button->priv;
  MxBorderImage *content_image = NULL;

  /* update the label styling */
  mx_button_update_label_style (button);

  g_free (priv->style_icon_name);
  mx_stylable_get (MX_STYLABLE (widget),
                   "x-mx-content-image", &content_image,
                   "x-mx-icon-name", &priv->style_icon_name,
                   "x-mx-icon-size", &priv->style_icon_size,
                   NULL);

  if (content_image && content_image->uri)
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
  else
    {
      /* remove any previous content image */
      if (priv->content_image)
        {
          clutter_actor_unparent (priv->content_image);
          priv->content_image = NULL;
        }

      if (content_image)
        g_boxed_free (MX_TYPE_BORDER_IMAGE, content_image);
    }

  if (priv->icon_size == 0)
    mx_icon_set_icon_size (MX_ICON (priv->icon), priv->style_icon_size);

  if (priv->style_icon_name && !priv->icon_name)
    {
      mx_icon_set_icon_name (MX_ICON (priv->icon), priv->style_icon_name);
      mx_button_update_contents (button);
    }
}


static void
mx_button_push (MxButton           *button,
                ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (button);

  mx_widget_hide_tooltip (widget);

  button->priv->is_pressed = TRUE;

  //clutter_grab_pointer (CLUTTER_ACTOR (button));

  mx_stylable_style_pseudo_class_add (MX_STYLABLE (button), "active");

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

  //clutter_ungrab_pointer ();

  if (button->priv->is_toggle)
    {
      mx_button_set_toggled (button, !button->priv->is_toggled);
    }

  button->priv->is_pressed = FALSE;



  /* activate any associated action */
  if (priv->action)
    g_signal_emit_by_name (priv->action, "activated", 0);


  g_signal_emit (button, button_signals[CLICKED], 0);

  mx_widget_long_press_cancel (widget);

  mx_stylable_style_pseudo_class_remove (MX_STYLABLE (button), "active");
}

static gboolean
mx_button_button_press (ClutterActor       *actor,
                        ClutterButtonEvent *event)
{

  if (mx_widget_get_disabled (MX_WIDGET (actor)))
    return TRUE;

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
  if (mx_widget_get_disabled (MX_WIDGET (actor)))
    return TRUE;

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
  if (event->keyval == CLUTTER_KEY_Return ||
      event->keyval == CLUTTER_KEY_KP_Enter ||
      event->keyval == CLUTTER_KEY_ISO_Enter ||
      event->keyval == CLUTTER_KEY_space)
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
  if (event->keyval == CLUTTER_KEY_Return ||
      event->keyval == CLUTTER_KEY_KP_Enter ||
      event->keyval == CLUTTER_KEY_ISO_Enter ||
      event->keyval == CLUTTER_KEY_space)
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
  MxWidget *widget = MX_WIDGET (actor);

  if (event->source != actor)
    return FALSE;

  /* check if the widget is disabled */
  if (mx_widget_get_disabled (MX_WIDGET (actor)))
    return FALSE;


  mx_stylable_style_pseudo_class_add (MX_STYLABLE (widget), "hover");

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

  /* hide the tooltip */
  if (mx_widget_get_tooltip_text (widget))
    mx_widget_hide_tooltip (widget);

  /* check if the widget is disabled */
  if (mx_widget_get_disabled (MX_WIDGET (actor)))
    return FALSE;

  if (button->priv->is_pressed)
    {
      //clutter_ungrab_pointer ();

      mx_widget_long_press_cancel (widget);
      mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "active");
      button->priv->is_pressed = FALSE;
    }

  mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "hover");

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

    case PROP_ICON_NAME:
      mx_button_set_icon_name (button, g_value_get_string (value));
      break;

    case PROP_ICON_SIZE:
      mx_button_set_icon_size (button, g_value_get_uint (value));
      break;

    case PROP_IS_TOGGLE:
      mx_button_set_is_toggle (button, g_value_get_boolean (value));
      break;

    case PROP_TOGGLED:
      mx_button_set_toggled (button, g_value_get_boolean (value));
      break;

    case PROP_ACTION:
      mx_button_set_action (button, g_value_get_object (value));
      break;

    case PROP_ICON_POSITION:
      mx_button_set_icon_position (button, g_value_get_enum (value));
      break;

    case PROP_ICON_VISIBLE:
      mx_button_set_icon_visible (button, g_value_get_boolean (value));
      break;

    case PROP_LABEL_VISIBLE:
      mx_button_set_label_visible (button, g_value_get_boolean (value));
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

    case PROP_ICON_NAME:
      g_value_set_string (value, priv->icon_name ?
                          priv->icon_name : priv->style_icon_name);
      break;

    case PROP_ICON_SIZE:
      g_value_set_uint (value, priv->icon_size ?
                        priv->icon_size : priv->style_icon_size);
      break;

    case PROP_IS_TOGGLE:
      g_value_set_boolean (value, priv->is_toggle);
      break;

    case PROP_TOGGLED:
      g_value_set_boolean (value, priv->is_toggled);
      break;

    case PROP_ACTION:
      g_value_set_object (value, priv->action);
      break;

    case PROP_ICON_POSITION:
      g_value_set_enum (value, priv->icon_position);
      break;

    case PROP_ICON_VISIBLE:
      g_value_set_boolean (value, priv->icon_visible);
      break;

    case PROP_LABEL_VISIBLE:
      g_value_set_boolean (value, priv->label_visible);
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
  g_free (priv->icon_name);
  g_free (priv->style_icon_name);

  G_OBJECT_CLASS (mx_button_parent_class)->finalize (gobject);
}

static void
mx_button_dispose (GObject *gobject)
{
  MxButtonPrivate *priv = MX_BUTTON (gobject)->priv;

  if (priv->content_image)
    {
      clutter_actor_unparent (priv->content_image);
      priv->content_image = NULL;
    }

  if (priv->action)
    {
      g_object_unref (priv->action);
      priv->action = NULL;
    }

  if (priv->hbox)
    {
      g_object_unref (priv->hbox);
      priv->hbox = NULL;
    }

  G_OBJECT_CLASS (mx_button_parent_class)->dispose (gobject);
}

static void
mx_button_map (ClutterActor *self)
{
  MxButtonPrivate *priv = MX_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->map (self);

  if (priv->content_image)
    clutter_actor_map (priv->content_image);
}

static void
mx_button_unmap (ClutterActor *self)
{
  MxButtonPrivate *priv = MX_BUTTON (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_button_parent_class)->unmap (self);

  if (priv->content_image)
    clutter_actor_unmap (priv->content_image);
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
  else
    CLUTTER_ACTOR_CLASS (mx_button_parent_class)->paint (actor);
}

static void
mx_button_update_contents (MxButton *self)
{
  MxButtonPrivate *priv = self->priv;
  ClutterActor *child;
  gboolean icon_visible, label_visible;
  const gchar *text;

  /* If the icon doesn't have a name set, treat it as
   * not-visible.
   */
  if (priv->icon_visible && mx_icon_get_icon_name (MX_ICON (priv->icon)))
    icon_visible = TRUE;
  else
    icon_visible = FALSE;

  text = clutter_text_get_text (CLUTTER_TEXT (priv->label));

  if (priv->label_visible && text && (strcmp (text, "") != 0))
    label_visible = TRUE;
  else
    label_visible = FALSE;

  /* replace any custom content */
  child = mx_bin_get_child (MX_BIN (self));

  if (child != priv->hbox)
    mx_bin_set_child (MX_BIN (self), priv->hbox);

  /* Handle the simple cases first */
  if (!icon_visible && !label_visible)
    {
      clutter_actor_hide (priv->hbox);
      return;
    }

  /* ensure the hbox is visible */
  clutter_actor_show (priv->hbox);

  if (icon_visible && !label_visible)
    {
      clutter_actor_show (priv->icon);
      clutter_actor_hide (priv->label);
      clutter_actor_lower_bottom (priv->icon);
      return;
    }

  if (!icon_visible && label_visible)
    {
      clutter_actor_hide (priv->icon);
      clutter_actor_show (priv->label);
      clutter_actor_lower_bottom (priv->label);
      return;
    }

  /* Both the icon and text are visible, handle this case */
  clutter_actor_show (priv->icon);
  clutter_actor_show (priv->label);

  switch (priv->icon_position)
    {
    case MX_POSITION_TOP:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_VERTICAL);
      clutter_actor_lower_bottom (priv->icon);

      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->label, "x-align", MX_ALIGN_MIDDLE,
                                   "y-align", MX_ALIGN_END, NULL);
      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->icon, "x-align", MX_ALIGN_MIDDLE,
                                   "y-align", MX_ALIGN_START, NULL);
      break;

    case MX_POSITION_RIGHT:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_HORIZONTAL);
      clutter_actor_raise_top (priv->icon);

      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->label, "x-align", MX_ALIGN_START,
                                   "y-align", MX_ALIGN_MIDDLE, NULL);
      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->icon, "x-align", MX_ALIGN_END,
                                   "y-align", MX_ALIGN_MIDDLE, NULL);
      break;

    case MX_POSITION_BOTTOM:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_VERTICAL);
      clutter_actor_raise_top (priv->icon);

      mx_box_layout_child_set_x_align (MX_BOX_LAYOUT (priv->hbox),
                                       priv->label, MX_ALIGN_MIDDLE);
      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->label, "x-align", MX_ALIGN_MIDDLE,
                                   "y-align", MX_ALIGN_START, NULL);
      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->icon, "x-align", MX_ALIGN_MIDDLE,
                                   "y-align", MX_ALIGN_END, NULL);
      break;

    case MX_POSITION_LEFT:
      mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->hbox),
                                     MX_ORIENTATION_HORIZONTAL);
      clutter_actor_lower_bottom (priv->icon);

      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->label, "x-align", MX_ALIGN_END,
                                   "y-align", MX_ALIGN_MIDDLE, NULL);
      clutter_container_child_set (CLUTTER_CONTAINER (priv->hbox),
                                   priv->icon, "x-align", MX_ALIGN_START,
                                   "y-align", MX_ALIGN_MIDDLE, NULL);
      break;
    }
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
                               NULL,
                               MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

  pspec = g_param_spec_string ("icon-name",
                               "Icon name",
                               "Icon name of the button",
                               NULL, MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ICON_NAME, pspec);

  pspec = g_param_spec_uint ("icon-size",
                             "Icon size",
                             "The size to use for the button icon (in pixels)",
                             0, G_MAXUINT, 0,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ICON_SIZE, pspec);

  pspec = g_param_spec_boolean ("is-toggle",
                                "Is Toggle",
                                "Enable or disable toggling",
                                FALSE, MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_IS_TOGGLE, pspec);

  pspec = g_param_spec_boolean ("toggled",
                                "Toggled",
                                "Indicates if a toggle button is \"on\""
                                " or \"off\"",
                                FALSE, MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TOGGLED, pspec);

  pspec = g_param_spec_object ("action", "Action", "Associated action",
                               MX_TYPE_ACTION,
                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (gobject_class, PROP_ACTION, pspec);

  pspec = g_param_spec_enum ("icon-position", "Icon position",
                             "The position of the icon, relative to the text",
                             MX_TYPE_POSITION, MX_POSITION_LEFT,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (gobject_class, PROP_ICON_POSITION, pspec);

  pspec = g_param_spec_boolean ("icon-visible", "Icon visible",
                                "Whether to show the icon",
                                TRUE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (gobject_class, PROP_ICON_VISIBLE, pspec);

  pspec = g_param_spec_boolean ("label-visible", "Label visible",
                                "Whether to show the label",
                                TRUE,
                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (gobject_class, PROP_LABEL_VISIBLE, pspec);

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
  MxButtonPrivate *priv;

  priv = button->priv = MX_BUTTON_GET_PRIVATE (button);

  clutter_actor_set_reactive ((ClutterActor *) button, TRUE);

  g_signal_connect (button, "style-changed",
                    G_CALLBACK (mx_button_style_changed), NULL);

  priv->icon_visible = TRUE;
  priv->label_visible = TRUE;
  priv->icon_position = MX_POSITION_LEFT;

  /* take an extra reference to the hbox */
  priv->hbox = g_object_ref (mx_box_layout_new ());
  mx_bin_set_child (MX_BIN (button), priv->hbox);

  priv->icon = mx_icon_new ();
  priv->label = g_object_new (CLUTTER_TYPE_TEXT,
                              "line-alignment", PANGO_ALIGN_CENTER,
                              "ellipsize", PANGO_ELLIPSIZE_END,
                              NULL);

  clutter_container_add (CLUTTER_CONTAINER (priv->hbox), priv->icon,
                         priv->label, NULL);

  mx_box_layout_child_set_expand (MX_BOX_LAYOUT (priv->hbox),
                                  priv->label, TRUE);
  mx_box_layout_child_set_y_fill (MX_BOX_LAYOUT (priv->hbox),
                                  priv->label, FALSE);
  mx_box_layout_child_set_x_fill (MX_BOX_LAYOUT (priv->hbox),
                                  priv->label, FALSE);

  mx_box_layout_child_set_expand (MX_BOX_LAYOUT (priv->hbox),
                                  priv->icon, TRUE);
  mx_box_layout_child_set_y_fill (MX_BOX_LAYOUT (priv->hbox),
                                  priv->icon, FALSE);
  mx_box_layout_child_set_x_fill (MX_BOX_LAYOUT (priv->hbox),
                                  priv->icon, FALSE);

  mx_button_update_contents (button);
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
const gchar *
mx_button_get_label (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), NULL);

  return button->priv->text;
}

/**
 * mx_button_set_label:
 * @button: a #MxButton
 * @text: text to set the label to
 *
 * Sets the text displayed on the button
 */
void
mx_button_set_label (MxButton    *button,
                     const gchar *text)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;

  g_free (priv->text);

  if (text)
    priv->text = g_strdup (text);
  else
    priv->text = g_strdup ("");

  clutter_text_set_text (CLUTTER_TEXT (priv->label), priv->text);

  mx_button_update_contents (button);

  g_object_notify (G_OBJECT (button), "label");
}

/**
 * mx_button_get_is_toggle:
 * @button: a #MxButton
 *
 * Get the toggle mode status of the button.
 *
 * Returns: #TRUE if toggle mode is set, otherwise #FALSE
 */
gboolean
mx_button_get_is_toggle (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);

  return button->priv->is_toggle;
}

/**
 * mx_button_set_is_toggle:
 * @button: a #MxButton
 * @toggle: #TRUE or #FALSE
 *
 * Enables or disables toggle mode for the button. In toggle mode, the active
 * state will be "toggled" when the user clicks the button.
 */
void
mx_button_set_is_toggle (MxButton *button,
                         gboolean  toggle)
{
  g_return_if_fail (MX_IS_BUTTON (button));

  button->priv->is_toggle = toggle;

  g_object_notify (G_OBJECT (button), "is-toggle");
}

/**
 * mx_button_get_toggled:
 * @button: a #MxButton
 *
 * Get the state of the button that is in toggle mode.
 *
 * Returns: #TRUE if the button is toggled, or #FALSE if not
 */
gboolean
mx_button_get_toggled (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);

  return button->priv->is_toggled;
}

/**
 * mx_button_set_toggled:
 * @button: a #MxButton
 * @toggled: #TRUE or #FALSE
 *
 * Sets the toggled state of the button. This is only really useful if the
 * button has #toggle-mode mode set to #TRUE.
 */
void
mx_button_set_toggled (MxButton *button,
                       gboolean  toggled)
{
  g_return_if_fail (MX_IS_BUTTON (button));

  if (button->priv->is_toggled != toggled)
    {
      button->priv->is_toggled = toggled;

      if (toggled)
        mx_stylable_style_pseudo_class_add (MX_STYLABLE (button), "checked");
      else
        mx_stylable_style_pseudo_class_remove (MX_STYLABLE (button), "checked");

      g_object_notify (G_OBJECT (button), "toggled");
    }
}

/**
 * mx_button_set_action:
 * @button: A #MxButton
 * @action: A #MxAction
 *
 * Sets @action as the action for @button. @Button will take its label and
 * icon from @action.
 *
 * Since: 1.2
 */
void
mx_button_set_action (MxButton *button,
                      MxAction *action)
{
  MxButtonPrivate *priv;
  const gchar *display_name;

  g_return_if_fail (MX_IS_BUTTON (button));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = button->priv;

  if (priv->action)
    g_object_unref (priv->action);

  if (priv->action_label_binding)
    g_object_unref (priv->action_label_binding);

  if (priv->action_icon_binding)
    g_object_unref (priv->action_icon_binding);

  priv->action = g_object_ref_sink (action);

  display_name = mx_action_get_display_name (action);

  mx_icon_set_icon_name (MX_ICON (priv->icon), mx_action_get_icon (action));
  clutter_text_set_text (CLUTTER_TEXT (priv->label),
                         (display_name) ? display_name : "");

  /* bind action properties to button properties */
  priv->action_label_binding = g_object_bind_property (action, "display-name",
                                                       priv->label, "text", 0);

  priv->action_icon_binding = g_object_bind_property (action, "icon",
                                                      priv->icon, "icon-name",
                                                      0);

  mx_button_update_contents (button);
}

/**
 * mx_button_get_action:
 * @button: A #MxButton
 *
 * Retrieves the #MxAction associated with @button.
 *
 * Returns: (transfer none): A #MxAction
 *
 * Since: 1.2
 */
MxAction *
mx_button_get_action (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), NULL);

  return button->priv->action;
}

/**
 * mx_button_set_icon_position:
 * @button: A #MxButton
 * @position: A #MxPosition
 *
 * Sets the icon position, relative to the text on the button.
 *
 * Since: 1.2
 */
void
mx_button_set_icon_position (MxButton   *button,
                             MxPosition  position)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;
  if (priv->icon_position != position)
    {
      priv->icon_position = position;

      mx_button_update_contents (button);

      g_object_notify (G_OBJECT (button), "icon-position");
    }
}

/**
 * mx_button_get_icon_position:
 * @button: A #MxButton
 *
 * Retrieves the icon's relative position to the text.
 *
 * Returns: A #MxPosition
 *
 * Since: 1.2
 */
MxPosition
mx_button_get_icon_position (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), MX_POSITION_LEFT);

  return button->priv->icon_position;
}

/**
 * mx_button_set_icon_visible:
 * @button: A #MxButton
 * @visible: %TRUE if the icon should be visible
 *
 * Sets the visibility of the icon associated with the button's action.
 *
 * Since: 1.2
 */
void
mx_button_set_icon_visible (MxButton *button,
                            gboolean  visible)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;
  if (priv->icon_visible != visible)
    {
      priv->icon_visible = visible;

      mx_button_update_contents (button);

      g_object_notify (G_OBJECT (button), "icon-visible");
    }
}

/**
 * mx_button_get_icon_visible:
 * @button: A #MxButton
 *
 * Retrieves the visibility of the icon associated with the button's action.
 *
 * Returns: %TRUE if the icon is visible, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_button_get_icon_visible (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);
  return button->priv->icon_visible;
}

/**
 * mx_button_set_label_visible:
 * @button: A #MxButton
 * @visible: %TRUE if the text should be visible
 *
 * Sets the visibility of the text associated with the button's action.
 *
 * Since: 1.2
 */
void
mx_button_set_label_visible (MxButton *button,
                            gboolean  visible)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;

  if (priv->label_visible != visible)
    {
      priv->label_visible = visible;

      mx_button_update_contents (button);

      g_object_notify (G_OBJECT (button), "label-visible");
    }
}

/**
 * mx_button_get_label_visible:
 * @button: A #MxButton
 *
 * Retrieves the visibility of the text associated with the button's action.
 *
 * Returns: %TRUE if the text is visible, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_button_get_label_visible (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), FALSE);

  return button->priv->label_visible;
}

/**
 * mx_button_get_icon_name:
 * @button: a #MxButton
 *
 * Get the icon-name being used on the button.
 *
 * Returns: the icon-name. This must not be freed by the application. %NULL if
 *   no icon has been set
 *
 * Since: 1.2
 */
const gchar *
mx_button_get_icon_name (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), NULL);

  return button->priv->icon_name ?
    button->priv->icon_name : button->priv->style_icon_name;
}

/**
 * mx_button_set_icon_name:
 * @button: a #MxButton
 * @icon_name: (allow-none): icon-name to use on the button
 *
 * Sets the icon-name used to display an icon on the button. Setting %NULL
 * will remove the icon name, or resort to the icon-name set in the current
 * style. Setting an icon name overrides any icon set in the style.
 *
 * Since: 1.2
 */
void
mx_button_set_icon_name (MxButton    *button,
                         const gchar *icon_name)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;

  g_free (priv->icon_name);
  priv->icon_name = g_strdup (icon_name);

  mx_icon_set_icon_name (MX_ICON (priv->icon), icon_name ?
                         icon_name : priv->style_icon_name);
  mx_button_update_contents (button);

  g_object_notify (G_OBJECT (button), "icon-name");
}

/**
 * mx_button_get_icon_size:
 * @button: a #MxButton
 *
 * Retrieves the icon-size being used for the displayed icon inside the button.
 *
 * Returns: The icon-size being used for the button icon, in pixels
 *
 * Since: 1.2
 */
guint
mx_button_get_icon_size (MxButton *button)
{
  g_return_val_if_fail (MX_IS_BUTTON (button), 0);

  return button->priv->icon_size ?
    button->priv->icon_size : button->priv->style_icon_size;
}

/**
 * mx_button_set_icon_size:
 * @button: a #MxButton
 *
 * Sets the icon-size to use for the icon displayed inside the button. This will
 * override the icon-size set in the style. Setting a value of %0 resets to the
 * size from the style.
 *
 * Since: 1.2
 */
void
mx_button_set_icon_size (MxButton *button,
                         guint     icon_size)
{
  MxButtonPrivate *priv;

  g_return_if_fail (MX_IS_BUTTON (button));

  priv = button->priv;

  if (priv->icon_size != icon_size)
    {
      priv->icon_size = icon_size;

      mx_icon_set_icon_size (MX_ICON (priv->icon), icon_size ?
                             icon_size : priv->style_icon_size);

      g_object_notify (G_OBJECT (button), "icon-size");
    }
}
