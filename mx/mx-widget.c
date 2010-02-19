/*
 * mx-widget.c: Base class for Mx actors
 *
 * Copyright 2007 OpenedHand
 * Copyright 2008, 2009, 2010 Intel Corporation.
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
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <clutter/clutter.h>

#include "mx-widget.h"

#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-stylable.h"
#include "mx-texture-cache.h"
#include "mx-texture-frame.h"
#include "mx-tooltip.h"
#include "mx-enum-types.h"

#define LONG_PRESS_TIMOUT 500

/*
 * Forward declaration for sake of MxWidgetChild
 */
struct _MxWidgetPrivate
{
  MxPadding     border;
  MxPadding     padding;

  MxStyle      *style;
  gchar        *pseudo_class;
  gchar        *style_class;

  ClutterActor *border_image;
  ClutterActor *old_border_image;
  ClutterActor *background_image;
  ClutterColor *bg_color;

  gboolean      has_tooltip : 1;
  gboolean      is_style_dirty : 1;
  gboolean      is_hovered : 1;

  MxTooltip    *tooltip;
  MxPopup      *popup;

  guint         long_press_source;
};

/**
 * SECTION:mx-widget
 * @short_description: Base class for stylable actors
 *
 * #MxWidget is a simple abstract class on top of #ClutterActor. It
 * provides basic themeing properties.
 *
 * Actors in the Mx library should subclass #MxWidget if they plan
 * to obey to a certain #MxStyle.
 */

enum
{
  PROP_0,

  PROP_STYLE,
  PROP_STYLE_CLASS,
  PROP_STYLE_PSEUDO_CLASS,

  PROP_HAS_TOOLTIP,
  PROP_TOOLTIP_TEXT
};

enum
{
  LONG_PRESS,

  LAST_SIGNAL
};

static guint widget_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_iface_init (MxStylableIface *iface);


G_DEFINE_ABSTRACT_TYPE_WITH_CODE (MxWidget, mx_widget, CLUTTER_TYPE_ACTOR,
                                  G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                         mx_stylable_iface_init));

#define MX_WIDGET_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_WIDGET, MxWidgetPrivate))

static void
mx_widget_set_property (GObject      *gobject,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxWidget *actor = MX_WIDGET (gobject);

  switch (prop_id)
    {
    case PROP_STYLE:
      mx_stylable_set_style (MX_STYLABLE (actor),
                             g_value_get_object (value));
      break;

    case PROP_STYLE_PSEUDO_CLASS:
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (actor), g_value_get_string (value));
      break;

    case PROP_STYLE_CLASS:
      mx_stylable_set_style_class (MX_STYLABLE (actor), g_value_get_string (value));
      break;

    case PROP_HAS_TOOLTIP:
      mx_widget_set_has_tooltip (actor, g_value_get_boolean (value));
      break;

    case PROP_TOOLTIP_TEXT:
      mx_widget_set_tooltip_text (actor, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_widget_get_property (GObject    *gobject,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxWidget *actor = MX_WIDGET (gobject);
  MxWidgetPrivate *priv = actor->priv;

  switch (prop_id)
    {
    case PROP_STYLE:
      g_value_set_object (value, priv->style);
      break;

    case PROP_STYLE_PSEUDO_CLASS:
      g_value_set_string (value, priv->pseudo_class);
      break;

    case PROP_STYLE_CLASS:
      g_value_set_string (value, priv->style_class);
      break;

    case PROP_HAS_TOOLTIP:
      g_value_set_boolean (value, priv->has_tooltip);
      break;

    case PROP_TOOLTIP_TEXT:
      g_value_set_string (value, mx_widget_get_tooltip_text (actor));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_widget_dispose (GObject *gobject)
{
  MxWidget *actor = MX_WIDGET (gobject);
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;

  if (priv->style)
    {
      g_object_unref (priv->style);
      priv->style = NULL;
    }

  if (priv->border_image)
    {
      clutter_actor_unparent (priv->border_image);
      priv->border_image = NULL;
    }

  if (priv->old_border_image)
    {
      clutter_actor_unparent (priv->old_border_image);
      priv->old_border_image = NULL;
    }

  if (priv->background_image)
    {
      clutter_actor_unparent (priv->background_image);
      priv->background_image = NULL;
    }

  if (priv->tooltip)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->tooltip));
      priv->tooltip = NULL;
    }

  if (priv->popup)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->popup));
      priv->popup = NULL;
    }

  G_OBJECT_CLASS (mx_widget_parent_class)->dispose (gobject);
}

static void
mx_widget_finalize (GObject *gobject)
{
  MxWidgetPrivate *priv = MX_WIDGET (gobject)->priv;

  g_free (priv->style_class);
  g_free (priv->pseudo_class);

  clutter_color_free (priv->bg_color);

  G_OBJECT_CLASS (mx_widget_parent_class)->finalize (gobject);
}

static void
mx_widget_allocate (ClutterActor          *actor,
                    const ClutterActorBox *box,
                    ClutterAllocationFlags flags)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;
  ClutterActorClass *klass;
  ClutterGeometry area;
  ClutterVertex in_v, out_v;

  klass = CLUTTER_ACTOR_CLASS (mx_widget_parent_class);
  klass->allocate (actor, box, flags);

  /* update tooltip position */
  if (priv->tooltip)
    {
      in_v.x = in_v.y = in_v.z = 0;
      clutter_actor_apply_transform_to_point (actor, &in_v, &out_v);
      area.x = out_v.x;
      area.y = out_v.y;

      in_v.x = box->x2 - box->x1;
      in_v.y = box->y2 - box->y1;
      clutter_actor_apply_transform_to_point (actor, &in_v, &out_v);
      area.width = out_v.x - area.x;
      area.height = out_v.y - area.y;

      mx_tooltip_set_tip_area (priv->tooltip, &area);
    }



  if (priv->border_image)
    {
      ClutterActorBox frame_box = {
        0,
        0,
        box->x2 - box->x1,
        box->y2 - box->y1
      };

      clutter_actor_allocate (CLUTTER_ACTOR (priv->border_image),
                              &frame_box,
                              flags);

      if (priv->old_border_image)
        clutter_actor_allocate (priv->old_border_image, &frame_box, flags);
    }

  if (priv->background_image)
    {
      ClutterActorBox frame_box = {
        0, 0, box->x2 - box->x1, box->y2 - box->y1
      };
      gfloat w, h;

      clutter_actor_get_size (CLUTTER_ACTOR (priv->background_image), &w, &h);

      /* scale the background into the allocated bounds */
      if (w > frame_box.x2 || h > frame_box.y2)
        {
          gint new_h, new_w, offset;
          gint box_w, box_h;

          box_w = (int) frame_box.x2;
          box_h = (int) frame_box.y2;

          /* scale to fit */
          new_h = (int)((h / w) * ((gfloat) box_w));
          new_w = (int)((w / h) * ((gfloat) box_h));

          if (new_h > box_h)
            {
              /* center for new width */
              offset = ((box_w) - new_w) * 0.5;
              frame_box.x1 = offset;
              frame_box.x2 = offset + new_w;

              frame_box.y2 = box_h;
            }
          else
            {
              /* center for new height */
              offset = ((box_h) - new_h) * 0.5;
              frame_box.y1 = offset;
              frame_box.y2 = offset + new_h;

              frame_box.x2 = box_w;
            }

        }
      else
        {
          /* center the background on the widget */
          frame_box.x1 = (int)(((box->x2 - box->x1) / 2) - (w / 2));
          frame_box.y1 = (int)(((box->y2 - box->y1) / 2) - (h / 2));
          frame_box.x2 = frame_box.x1 + w;
          frame_box.y2 = frame_box.y1 + h;
        }

      clutter_actor_allocate (CLUTTER_ACTOR (priv->background_image),
                              &frame_box,
                              flags);
    }

  if (priv->tooltip)
    clutter_actor_allocate_preferred_size (CLUTTER_ACTOR (priv->tooltip),
                                           flags);
  if (priv->popup)
    clutter_actor_allocate_preferred_size (CLUTTER_ACTOR (priv->popup),
                                           flags);
}

static void
mx_widget_real_paint_background (MxWidget           *self,
                                 ClutterActor       *background,
                                 const ClutterColor *color)
{
  /* Default implementation just draws the background
   * colour and the image on top
   */
  if (color && color->alpha != 0)
    {
      ClutterActor *actor = CLUTTER_ACTOR (self);
      ClutterActorBox allocation = { 0, };
      ClutterColor bg_color = *color;
      gfloat w, h;

      bg_color.alpha = clutter_actor_get_paint_opacity (actor)
                       * bg_color.alpha
                       / 255;

      clutter_actor_get_allocation_box (actor, &allocation);

      w = allocation.x2 - allocation.x1;
      h = allocation.y2 - allocation.y1;

      cogl_set_source_color4ub (bg_color.red,
                                bg_color.green,
                                bg_color.blue,
                                bg_color.alpha);
      cogl_rectangle (0, 0, w, h);
    }

  if (background)
    clutter_actor_paint (background);

  if (self->priv->old_border_image)
    clutter_actor_paint (self->priv->old_border_image);
}

static void
mx_widget_paint (ClutterActor *self)
{
  MxWidgetPrivate *priv = MX_WIDGET (self)->priv;
  MxWidgetClass *klass = MX_WIDGET_GET_CLASS (self);

  klass->paint_background (MX_WIDGET (self),
                           priv->border_image,
                           priv->bg_color);

  if (priv->background_image != NULL)
    clutter_actor_paint (priv->background_image);

  if (priv->tooltip)
    clutter_actor_paint (CLUTTER_ACTOR (priv->tooltip));

  if (priv->popup)
    clutter_actor_paint (CLUTTER_ACTOR (priv->popup));
}

static void
mx_widget_pick (ClutterActor *self, const ClutterColor *color)
{
  MxWidgetPrivate *priv = MX_WIDGET (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->pick (self, color);

  if (priv->popup)
    clutter_actor_paint (CLUTTER_ACTOR (priv->popup));
}

static void
mx_widget_parent_set (ClutterActor *widget,
                      ClutterActor *old_parent)
{
  ClutterActorClass *parent_class;
  ClutterActor *new_parent;

  parent_class = CLUTTER_ACTOR_CLASS (mx_widget_parent_class);
  if (parent_class->parent_set)
    parent_class->parent_set (widget, old_parent);

  new_parent = clutter_actor_get_parent (widget);

  /* don't send the style changed signal if we no longer have a parent actor */
  if (new_parent)
    mx_stylable_changed ((MxStylable*) widget);
}

static void
mx_widget_map (ClutterActor *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->map (actor);

  mx_widget_ensure_style ((MxWidget*) actor);

  if (priv->border_image)
    clutter_actor_map (priv->border_image);

  if (priv->background_image)
    clutter_actor_map (priv->background_image);

  if (priv->tooltip)
    clutter_actor_map (CLUTTER_ACTOR (priv->tooltip));

  if (priv->popup)
    clutter_actor_map (CLUTTER_ACTOR (priv->popup));
}

static void
mx_widget_unmap (ClutterActor *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->unmap (actor);

  if (priv->border_image)
    clutter_actor_unmap (priv->border_image);

  if (priv->background_image)
    clutter_actor_unmap (priv->background_image);

  if (priv->tooltip)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->tooltip));

  if (priv->popup)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->popup));
}

static void
old_background_faded_cb (ClutterAnimation *animation,
                         MxWidgetPrivate  *priv)
{
  if (priv->old_border_image)
    {
      clutter_actor_unparent (priv->old_border_image);
      priv->old_border_image = NULL;
    }
}

static void
mx_widget_style_changed (MxStylable *self)
{
  MxWidgetPrivate *priv = MX_WIDGET (self)->priv;
  MxBorderImage *border_image = NULL, *background_image = NULL;
  MxTextureCache *texture_cache;
  ClutterTexture *texture;
  gchar *bg_file;
  MxPadding *padding = NULL;
  gboolean relayout_needed = FALSE;
  gboolean has_changed = FALSE;
  ClutterColor *color;
  guint duration;

  /* cache these values for use in the paint function */
  mx_stylable_get (self,
                   "background-color", &color,
                   "background-image", &background_image,
                   "border-image", &border_image,
                   "padding", &padding,
                   "x-mx-border-image-transition-duration", &duration,
                   NULL);

  if (color)
    {
      if (priv->bg_color && clutter_color_equal (color, priv->bg_color))
        {
          /* color is the same ... */
          clutter_color_free (color);
        }
      else
        {
          clutter_color_free (priv->bg_color);
          priv->bg_color = color;
          has_changed = TRUE;
        }
    }
  else
  if (priv->bg_color)
    {
      clutter_color_free (priv->bg_color);
      priv->bg_color = NULL;
      has_changed = TRUE;
    }



  if (padding)
    {
      if (priv->padding.top != padding->top ||
          priv->padding.left != padding->left ||
          priv->padding.right != padding->right ||
          priv->padding.bottom != padding->bottom)
        {
          /* Padding changed. Need to relayout. */
          has_changed = TRUE;
          relayout_needed = TRUE;
        }

      priv->padding = *padding;
      g_boxed_free (MX_TYPE_PADDING, padding);
    }

  if (priv->border_image)
    {
      if (duration == 0)
        {
          clutter_actor_unparent (priv->border_image);
        }
      else
        {
          if (priv->old_border_image)
            clutter_actor_unparent (priv->old_border_image);

          priv->old_border_image = priv->border_image;
          clutter_actor_animate (priv->old_border_image,
                                 CLUTTER_LINEAR,
                                 duration,
                                 "opacity", 0x0,
                                 "signal::completed", old_background_faded_cb,
                                 priv,
                                 NULL);
        }

      priv->border_image = NULL;
    }

  if (priv->background_image)
    {
      clutter_actor_unparent (priv->background_image);
      priv->background_image = NULL;
    }

  texture_cache = mx_texture_cache_get_default ();

  /* Check if the URL is actually present, not garbage in the property */
  if (border_image && border_image->uri)
    {
      gint border_left, border_right, border_top, border_bottom;
      gint width, height;

      /* `border-image' takes precedence over `background-image'.
       * Firefox lets the background-image shine thru when border-image has
       * alpha an channel, maybe that would be an option for the future. */
      texture = mx_texture_cache_get_texture (texture_cache,
                                              border_image->uri);

      clutter_texture_get_base_size (CLUTTER_TEXTURE (texture),
                                     &width, &height);

      border_left = border_image->left;
      border_top = border_image->top;
      border_right = border_image->right;
      border_bottom = border_image->bottom;

      priv->border_image = mx_texture_frame_new (texture,
                                                 border_top,
                                                 border_right,
                                                 border_bottom,
                                                 border_left);
      clutter_actor_set_parent (priv->border_image, CLUTTER_ACTOR (self));

      has_changed = TRUE;
      relayout_needed = TRUE;
    }

  /* Might get a border-image: none */
  if (border_image)
    {
      g_boxed_free (MX_TYPE_BORDER_IMAGE, border_image);
    }

  if (background_image)
    {
      bg_file = background_image->uri;
      if (bg_file != NULL &&
          strcmp (bg_file, "none"))
        {
          texture = mx_texture_cache_get_texture (texture_cache, bg_file);
          priv->background_image = (ClutterActor*) texture;

          if (priv->background_image != NULL)
            {
              clutter_actor_set_parent (priv->background_image,
                                        CLUTTER_ACTOR (self));
            }
          else
            g_warning ("Could not load %s", bg_file);

          has_changed = TRUE;
          relayout_needed = TRUE;
        }
      g_boxed_free (MX_TYPE_BORDER_IMAGE, background_image);
    }

  /* If there are any properties above that need to cause a relayout thay
   * should set this flag.
   */
  if (has_changed)
    {
      if (relayout_needed)
        clutter_actor_queue_relayout ((ClutterActor *) self);
      else
        clutter_actor_queue_redraw ((ClutterActor *) self);
    }

  priv->is_style_dirty = FALSE;
}

static void
mx_widget_stylable_child_notify (ClutterActor *actor,
                                 gpointer      user_data)
{
  if (MX_IS_STYLABLE (actor))
    mx_stylable_changed ((MxStylable*) actor);
}

static void
mx_widget_stylable_changed (MxStylable *stylable)
{

  MX_WIDGET (stylable)->priv->is_style_dirty = TRUE;

  /* update the style only if we are mapped */
  if (!CLUTTER_ACTOR_IS_MAPPED ((ClutterActor *) stylable))
    return;

  g_signal_emit_by_name (stylable, "style-changed", 0);


  if (CLUTTER_IS_CONTAINER (stylable))
    {
      /* notify our children that their parent stylable has changed */
      clutter_container_foreach ((ClutterContainer *) stylable,
                                 mx_widget_stylable_child_notify,
                                 NULL);
    }
}

static gboolean
mx_widget_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);
  MxWidgetPrivate *priv = widget->priv;

  if (event->source != actor)
    return FALSE;

  mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "hover");
  priv->is_hovered = TRUE;

  if (priv->has_tooltip)
    mx_widget_show_tooltip (widget);

  return FALSE;
}

static gboolean
mx_widget_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);
  MxWidgetPrivate *priv = widget->priv;

  if (event->source != actor)
    return FALSE;

  mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "");
  priv->is_hovered = FALSE;

  if (priv->has_tooltip)
    mx_tooltip_hide (priv->tooltip);

  return FALSE;
}

static gboolean
mx_widget_emit_long_press (MxWidget *widget)
{
  gboolean result;

  g_signal_emit (widget, widget_signals[LONG_PRESS], 0,
                 0.0, 0.0, MX_LONG_PRESS_ACTION, &result);
  widget->priv->long_press_source = 0;

  return FALSE;
}

/**
 * mx_widget_long_press_query:
 * @widget: An MxWidget
 * @event: the event used to determine whether to run a long-press
 *
 * Emit the long-press query signal and start a long-press timeout if required.
 */
void
mx_widget_long_press_query (MxWidget           *widget,
                            ClutterButtonEvent *event)
{
  MxWidgetPrivate *priv = widget->priv;
  gboolean query_result = FALSE;

  g_signal_emit (widget, widget_signals[LONG_PRESS], 0, event->x,
                 event->y, MX_LONG_PRESS_QUERY, &query_result);

  if (query_result)
    priv->long_press_source = g_timeout_add (LONG_PRESS_TIMOUT,
                                             (GSourceFunc) mx_widget_emit_long_press,
                                             widget);
}

/**
 * mx_widget_long_press_cancel:
 * @widget: An MxWidget
 *
 * Cancel a long-press timeout if one is running and emit the signal to notify
 * that the long-press has been cancelled.
 */
void
mx_widget_long_press_cancel (MxWidget *widget)
{
  MxWidgetPrivate *priv = widget->priv;

  if (priv->long_press_source)
    {
      gboolean result;

      g_source_remove (priv->long_press_source);
      priv->long_press_source = 0;
      g_signal_emit (widget, widget_signals[LONG_PRESS], 0,
                     0.0, 0.0, MX_LONG_PRESS_CANCEL, &result);
    }
}

static gboolean
mx_widget_button_press (ClutterActor       *actor,
                        ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  if (event->button == 1)
    mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "active");

  mx_widget_long_press_query (widget, event);

  return TRUE;
}

static gboolean
mx_widget_button_release (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  if (event->button == 1)
    {
      if (widget->priv->is_hovered)
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "hover");
      else
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (widget), "");
    }

  mx_widget_long_press_cancel (widget);

  return TRUE;
}

static void
mx_widget_hide (ClutterActor *actor)
{
  MxWidget *widget = (MxWidget *) actor;

  /* hide the tooltip, if there is one */
  if (widget->priv->tooltip)
    mx_tooltip_hide (widget->priv->tooltip);

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->hide (actor);
}



static void
mx_widget_class_init (MxWidgetClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxWidgetPrivate));

  gobject_class->set_property = mx_widget_set_property;
  gobject_class->get_property = mx_widget_get_property;
  gobject_class->dispose = mx_widget_dispose;
  gobject_class->finalize = mx_widget_finalize;

  actor_class->allocate = mx_widget_allocate;
  actor_class->paint = mx_widget_paint;
  actor_class->pick = mx_widget_pick;
  actor_class->parent_set = mx_widget_parent_set;
  actor_class->map = mx_widget_map;
  actor_class->unmap = mx_widget_unmap;

  actor_class->enter_event = mx_widget_enter;
  actor_class->leave_event = mx_widget_leave;
  actor_class->button_press_event = mx_widget_button_press;
  actor_class->button_release_event = mx_widget_button_release;

  actor_class->hide = mx_widget_hide;

  klass->paint_background = mx_widget_real_paint_background;

  /* stylable interface properties */
  g_object_class_override_property (gobject_class, PROP_STYLE, "style");
  g_object_class_override_property (gobject_class, PROP_STYLE_CLASS,
                                    "style-class");
  g_object_class_override_property (gobject_class, PROP_STYLE_PSEUDO_CLASS,
                                    "style-pseudo-class");

  /**
   * MxWidget:has-tooltip:
   *
   * Determines whether the widget has a tooltip. If set to TRUE, causes the
   * widget to monitor enter and leave events (i.e. sets the widget reactive).
   */
  pspec = g_param_spec_boolean ("has-tooltip",
                                "Has Tooltip",
                                "Determines whether the widget has a tooltip",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_HAS_TOOLTIP,
                                   pspec);


  /**
   * MxWidget:tooltip-text:
   *
   * text displayed on the tooltip
   */
  pspec = g_param_spec_string ("tooltip-text",
                               "Tooltip Text",
                               "Text displayed on the tooltip",
                               "",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TOOLTIP_TEXT, pspec);


  /**
   * MxWidget::long-press:
   * @widget: the object that received the signal
   *
   * Emitted when the user holds a mouse button down for a longer period.
   */

  widget_signals[LONG_PRESS] =
    g_signal_new ("long-press",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxWidgetClass, long_press),
                  NULL, NULL,
                  _mx_marshal_BOOL__FLOAT_FLOAT_ENUM,
                  G_TYPE_BOOLEAN, 3, G_TYPE_FLOAT, G_TYPE_FLOAT,
                  MX_TYPE_LONG_PRESS_ACTION);

}

static MxStyle *
mx_widget_get_style (MxStylable *stylable)
{
  MxWidgetPrivate *priv = MX_WIDGET (stylable)->priv;

  return priv->style;
}

static void
mx_style_changed_cb (MxStyle    *style,
                     MxStylable *stylable)
{
  mx_stylable_changed (stylable);
}


static void
mx_widget_set_style (MxStylable *stylable,
                     MxStyle    *style)
{
  MxWidgetPrivate *priv = MX_WIDGET (stylable)->priv;

  if (priv->style)
    g_object_unref (priv->style);

  priv->style = g_object_ref_sink (style);

  g_signal_connect (priv->style,
                    "changed",
                    G_CALLBACK (mx_style_changed_cb),
                    stylable);
}

static void
_mx_widget_set_style_class (MxStylable  *actor,
                            const gchar *style_class)
{
  MxWidgetPrivate *priv;

  g_return_if_fail (MX_IS_WIDGET (actor));

  priv = MX_WIDGET (actor)->priv;

  if (g_strcmp0 (style_class, priv->style_class))
    {
      g_free (priv->style_class);
      priv->style_class = g_strdup (style_class);

      mx_stylable_changed (actor);

      g_object_notify (G_OBJECT (actor), "style-class");
    }
}

static void
_mx_stylable_set_style_pseudo_class (MxStylable  *actor,
                                   const gchar *pseudo_class)
{
  MxWidgetPrivate *priv;

  g_return_if_fail (MX_IS_WIDGET (actor));

  priv = MX_WIDGET (actor)->priv;

  if (g_strcmp0 (pseudo_class, priv->pseudo_class))
    {
      g_free (priv->pseudo_class);
      priv->pseudo_class = g_strdup (pseudo_class);

      mx_stylable_changed (actor);

      g_object_notify (G_OBJECT (actor), "style-pseudo-class");
    }
}

static const gchar*
_mx_stylable_get_style_pseudo_class (MxStylable *actor)
{
  return ((MxWidget *) actor)->priv->pseudo_class;
}


static const gchar*
_mx_widget_get_style_class (MxStylable *actor)
{
  return ((MxWidget *) actor)->priv->style_class;
}


static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;
      ClutterColor color = { 0x00, 0x00, 0x00, 0xff };
      ClutterColor bg_color = { 0xff, 0xff, 0xff, 0x00 };

      is_initialized = TRUE;

      pspec = clutter_param_spec_color ("background-color",
                                        "Background Color",
                                        "The background color of an actor",
                                        &bg_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = clutter_param_spec_color ("color",
                                        "Text Color",
                                        "The color of the text of an actor",
                                        &color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_boxed ("background-image",
                                  "Background Image",
                                  "Background image filename",
                                  MX_TYPE_BORDER_IMAGE,
                                  G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_string ("font-family",
                                   "Font Family",
                                   "Name of the font to use",
                                   "Sans",
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_int ("font-size",
                                "Font Size",
                                "Size of the font to use in pixels",
                                0, G_MAXINT, 12,
                                G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_enum ("font-weight",
                                 "Font Weight",
                                 "Font Weight",
                                 MX_TYPE_FONT_WEIGHT,
                                 MX_WEIGHT_NORMAL,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_boxed ("border-image",
                                  "Border image",
                                  "9-slice image to use for drawing borders and background",
                                  MX_TYPE_BORDER_IMAGE,
                                  G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_boxed ("padding",
                                  "Padding",
                                  "Padding between the widget's borders "
                                  "and its content",
                                  MX_TYPE_PADDING,
                                  G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_uint ("x-mx-border-image-transition-duration",
                                 "background transition duration",
                                 "Length of the cross fade when changing images"
                                 " in milliseconds",
                                 0, G_MAXUINT, 0,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      /*
      pspec = g_param_spec_uint ("x-mx-transition-duration",
                                 "transition duration",
                                 "Length of transition in milliseconds",
                                 0, G_MAXUINT, 0,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);

      pspec = g_param_spec_string ("x-mx-transition-property",
                                   "transition property",
                                   "Property to apply the transition too",
                                   "",
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_WIDGET, pspec);
      */

      iface->style_changed = mx_widget_style_changed;
      iface->stylable_changed = mx_widget_stylable_changed;

      iface->get_style = mx_widget_get_style;
      iface->set_style = mx_widget_set_style;

      iface->get_style_pseudo_class = _mx_stylable_get_style_pseudo_class;
      iface->set_style_pseudo_class = _mx_stylable_set_style_pseudo_class;
      iface->get_style_class = _mx_widget_get_style_class;
      iface->set_style_class = _mx_widget_set_style_class;
    }
}


static void
mx_widget_name_notify (MxWidget   *widget,
                       GParamSpec *pspec,
                       gpointer    data)
{
  mx_stylable_changed ((MxStylable*) widget);
}

static void
mx_widget_init (MxWidget *actor)
{
  MxWidgetPrivate *priv;

  actor->priv = priv = MX_WIDGET_GET_PRIVATE (actor);

  /* connect style changed */
  g_signal_connect (actor, "notify::name", G_CALLBACK (mx_widget_name_notify), NULL);

  /* set the default style */
  mx_widget_set_style (MX_STYLABLE (actor), mx_style_get_default ());

}

/**
 * mx_widget_ensure_style:
 * @widget: A #MxWidget
 *
 * Ensures that @widget has read its style information.
 *
 */
void
mx_widget_ensure_style (MxWidget *widget)
{
  g_return_if_fail (MX_IS_WIDGET (widget));

  if (widget->priv->is_style_dirty)
    {
      g_signal_emit_by_name (widget, "style-changed", 0);
    }
}


/**
 * mx_widget_get_border_image:
 * @actor: A #MxWidget
 *
 * Get the texture used as the border image. This is set using the
 * "border-image" CSS property. This function should normally only be used
 * by subclasses.
 *
 * Returns: #ClutterActor
 */
ClutterActor *
mx_widget_get_border_image (MxWidget *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;
  return priv->border_image;
}

/**
 * mx_widget_get_background_image:
 * @actor: A #MxWidget
 *
 * Get the texture used as the background image. This is set using the
 * "background-image" CSS property. This function should normally only be used
 * by subclasses.
 *
 * Returns: a #ClutterActor
 */
ClutterActor *
mx_widget_get_background_image (MxWidget *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;
  return priv->background_image;
}

/**
 * mx_widget_get_padding:
 * @widget: A #MxWidget
 * @padding: A pointer to an #MxPadding to fill
 *
 * Gets the padding of the widget, set using the "padding" CSS property. This
 * function should normally only be used by subclasses.
 *
 */
void
mx_widget_get_padding (MxWidget  *widget,
                       MxPadding *padding)
{
  g_return_if_fail (MX_IS_WIDGET (widget));
  g_return_if_fail (padding != NULL);

  *padding = widget->priv->padding;
}

/**
 * mx_widget_set_has_tooltip:
 * @widget: A #MxWidget
 * @has_tooltip: #TRUE if the widget should display a tooltip
 *
 * Enables tooltip support on the #MxWidget.
 *
 * Note that setting has-tooltip to #TRUE will cause the widget to be set
 * reactive. If you no longer need tooltip support and do not need the widget
 * to be reactive, you need to set ClutterActor::reactive to FALSE.
 *
 */
void
mx_widget_set_has_tooltip (MxWidget *widget,
                           gboolean  has_tooltip)
{
  MxWidgetPrivate *priv;
  ClutterActor *actor;

  g_return_if_fail (MX_IS_WIDGET (widget));

  actor = CLUTTER_ACTOR (widget);

  priv = widget->priv;

  priv->has_tooltip = has_tooltip;

  if (has_tooltip)
    {
      clutter_actor_set_reactive (actor, TRUE);

      if (!priv->tooltip)
        {
          priv->tooltip = g_object_new (MX_TYPE_TOOLTIP, NULL);
          clutter_actor_set_parent (CLUTTER_ACTOR (priv->tooltip), actor);
        }
    }
  else
    {
      if (priv->tooltip)
        {
          clutter_actor_unparent (CLUTTER_ACTOR (priv->tooltip));
          priv->tooltip = NULL;
        }
    }
}

/**
 * mx_widget_get_has_tooltip:
 * @widget: A #MxWidget
 *
 * Returns the current value of the has-tooltip property. See
 * mx_tooltip_set_has_tooltip() for more information.
 *
 * Returns: current value of has-tooltip on @widget
 */
gboolean
mx_widget_get_has_tooltip (MxWidget *widget)
{
  g_return_val_if_fail (MX_IS_WIDGET (widget), FALSE);

  return widget->priv->has_tooltip;
}

/**
 * mx_widget_set_tooltip_text:
 * @widget: A #MxWidget
 * @text: text to set as the tooltip
 *
 * Set the tooltip text of the widget. This will set MxWidget::has-tooltip to
 * #TRUE. A value of #NULL will unset the tooltip and set has-tooltip to #FALSE.
 *
 */
void
mx_widget_set_tooltip_text (MxWidget    *widget,
                            const gchar *text)
{
  MxWidgetPrivate *priv;

  g_return_if_fail (MX_IS_WIDGET (widget));

  priv = widget->priv;

  if (text == NULL)
    mx_widget_set_has_tooltip (widget, FALSE);
  else
    mx_widget_set_has_tooltip (widget, TRUE);

  if (priv->tooltip)
    mx_tooltip_set_label (priv->tooltip, text);
}

/**
 * mx_widget_get_tooltip_text:
 * @widget: A #MxWidget
 *
 * Get the current tooltip string
 *
 * Returns: The current tooltip string, owned by the #MxWidget
 */
const gchar*
mx_widget_get_tooltip_text (MxWidget *widget)
{
  MxWidgetPrivate *priv;

  g_return_val_if_fail (MX_IS_WIDGET (widget), NULL);
  priv = widget->priv;

  if (!priv->has_tooltip)
    return NULL;

  return mx_tooltip_get_label (widget->priv->tooltip);
}

/**
 * mx_widget_show_tooltip:
 * @widget: A #MxWidget
 *
 * Show the tooltip for @widget
 *
 */
void
mx_widget_show_tooltip (MxWidget *widget)
{
  gfloat x, y, width, height;
  ClutterGeometry area;

  g_return_if_fail (MX_IS_WIDGET (widget));

  /* XXX not necceary, but first allocate transform is wrong */

  clutter_actor_get_transformed_position ((ClutterActor*) widget,
                                          &x, &y);

  clutter_actor_get_size ((ClutterActor*) widget, &width, &height);

  area.x = x;
  area.y = y;
  area.width = width;
  area.height = height;


  if (widget->priv->tooltip)
    {
      mx_tooltip_set_tip_area (widget->priv->tooltip, &area);
      mx_tooltip_show (widget->priv->tooltip);
    }
}

/**
 * mx_widget_hide_tooltip:
 * @widget: A #MxWidget
 *
 * Hide the tooltip for @widget
 *
 */
void
mx_widget_hide_tooltip (MxWidget *widget)
{
  g_return_if_fail (MX_IS_WIDGET (widget));

  if (widget->priv->tooltip)
    mx_tooltip_hide (widget->priv->tooltip);
}

/**
 * mx_widget_paint_background:
 * @widget: a #MxWidget
 *
 * Invokes #MxWidget::paint_background() using the default background
 * image and/or color from the @widget style
 *
 * This function should be used by subclasses of #MxWidget that override
 * the paint() virtual function and cannot chain up
 */
void
mx_widget_paint_background (MxWidget *self)
{
  MxWidgetPrivate *priv;
  MxWidgetClass *klass;

  g_return_if_fail (MX_IS_WIDGET (self));

  priv = self->priv;

  klass = MX_WIDGET_GET_CLASS (self);
  klass->paint_background (MX_WIDGET (self),
                          priv->border_image,
                          priv->bg_color);

}

void
mx_widget_get_available_area (MxWidget              *widget,
                              const ClutterActorBox *allocation,
                              ClutterActorBox       *area)
{
  MxWidgetPrivate *priv = widget->priv;

  area->x1 = priv->padding.left;
  area->y1 = priv->padding.top;

  area->x2 = allocation->x2 - allocation->x1 - priv->padding.right;
  area->y2 = allocation->y2 - allocation->y1 - priv->padding.bottom;
}

void
mx_widget_set_popup (MxWidget *widget,
                     MxPopup  *popup)
{
  MxWidgetPrivate *priv = widget->priv;

  if (priv->popup)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->popup));
      priv->popup = NULL;
    }

  if (popup)
    {
      ClutterActor *popup_actor = CLUTTER_ACTOR (popup);

      priv->popup = popup;
      clutter_actor_set_parent (popup_actor, CLUTTER_ACTOR (widget));
      clutter_actor_hide (popup_actor);
      clutter_actor_set_reactive (popup_actor, FALSE);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (widget));
}

MxPopup *
mx_widget_get_popup (MxWidget *widget)
{
  return widget->priv->popup;
}

static gboolean
mx_widget_popup_disconnect_anim (MxWidget *widget)
{
  ClutterActor *popup_actor;
  ClutterAnimation *animation;

  MxWidgetPrivate *priv = widget->priv;

  if (!priv->popup)
    return FALSE;

  popup_actor = CLUTTER_ACTOR (priv->popup);

  if (!(animation = clutter_actor_get_animation (popup_actor)))
    return FALSE;

  return g_signal_handlers_disconnect_by_func (animation,
                                               clutter_actor_hide,
                                               popup_actor) ?
    TRUE : FALSE;
}

void
mx_widget_show_popup (MxWidget *widget, gfloat x, gfloat y)
{
  ClutterActor *popup_actor;
  MxWidgetPrivate *priv = widget->priv;

  if (!priv->popup || CLUTTER_ACTOR_IS_REACTIVE (priv->popup))
    return;

  popup_actor = CLUTTER_ACTOR (priv->popup);

  clutter_actor_set_position (popup_actor, x, y);

  if (!mx_widget_popup_disconnect_anim (widget))
    {
      clutter_actor_show (popup_actor);
      clutter_actor_set_opacity (popup_actor, 0x00);
    }

  clutter_actor_set_reactive (popup_actor, TRUE);
  clutter_actor_animate (popup_actor, CLUTTER_EASE_OUT_CUBIC, 150,
                         "opacity", (guchar)0xff,
                         NULL);
}

void
mx_widget_hide_popup (MxWidget *widget)
{
  ClutterActor *popup_actor;
  MxWidgetPrivate *priv = widget->priv;

  if (!priv->popup)
    return;

  popup_actor = CLUTTER_ACTOR (priv->popup);

  if (CLUTTER_ACTOR_IS_REACTIVE (popup_actor))
    {
      clutter_actor_set_reactive (popup_actor, FALSE);
      clutter_actor_animate (popup_actor, CLUTTER_LINEAR, 250,
                             "opacity", (guchar) 0,
                             "signal-swapped::completed", clutter_actor_hide,
                             popup_actor,
                             NULL);
    }
}

