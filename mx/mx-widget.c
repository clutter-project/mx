/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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
#include <libintl.h>

#include <clutter/clutter.h>

#include "mx-widget.h"

#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-stylable.h"
#include "mx-texture-cache.h"
#include "mx-texture-frame.h"
#include "mx-tooltip.h"
#include "mx-enum-types.h"
#include "mx-settings.h"

/*
 * Forward declaration for sake of MxWidgetChild
 */
struct _MxWidgetPrivate
{
  MxPadding     border;
  MxPadding     padding;

  MxStyle       *style;
  gchar         *pseudo_class;
  gchar         *style_class;
  MxBorderImage *mx_border_image;

  ClutterActor *border_image;
  ClutterActor *old_border_image;
  ClutterActor *background_image;
  ClutterColor *bg_color;

  guint         is_disabled : 1;
  guint         parent_disabled : 1;

  MxTooltip    *tooltip;
  MxMenu       *menu;

  guint         long_press_source;

  guint         tooltip_timeout;
  guint         tooltip_delay;
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

  PROP_TOOLTIP_TEXT,
  PROP_MENU,

  PROP_DISABLED,

  PROP_TOOLTIP_DELAY,

  LAST_PROP
};

static GParamSpec *widget_properties[LAST_PROP];

enum
{
  LONG_PRESS,

  LAST_SIGNAL
};

static guint widget_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_iface_init (MxStylableIface *iface);
static ClutterScriptableIface *parent_scriptable_iface = NULL;
static void scriptable_iface_init (ClutterScriptableIface *iface);

/* Length of time in milliseconds that the cursor must be held steady
   over a widget before the tooltip is displayed */
#define MX_WIDGET_TOOLTIP_TIMEOUT 500

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (MxWidget, mx_widget, CLUTTER_TYPE_ACTOR,
                                  G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                         mx_stylable_iface_init)
                                  G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_SCRIPTABLE,
                                                         scriptable_iface_init));

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

    case PROP_TOOLTIP_TEXT:
      mx_widget_set_tooltip_text (actor, g_value_get_string (value));
      break;

    case PROP_MENU:
      mx_widget_set_menu (actor, (MxMenu *)g_value_get_object (value));
      break;

    case PROP_DISABLED:
      mx_widget_set_disabled (actor, g_value_get_boolean (value));
      break;

    case PROP_TOOLTIP_DELAY:
      mx_widget_set_tooltip_delay (actor, g_value_get_int (value));
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

    case PROP_TOOLTIP_TEXT:
      g_value_set_string (value, mx_widget_get_tooltip_text (actor));
      break;

    case PROP_MENU:
      g_value_set_object (value, mx_widget_get_menu (actor));
      break;

    case PROP_DISABLED:
      g_value_set_boolean (value, mx_widget_get_disabled (actor));
      break;

    case PROP_TOOLTIP_DELAY:
      g_value_set_int (value, mx_widget_get_tooltip_delay (actor));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static gboolean
mx_widget_tooltip_timeout_cb (gpointer data)
{
  MxWidget *self = MX_WIDGET (data);
  MxWidgetPrivate *priv = self->priv;

  mx_widget_show_tooltip (self);

  priv->tooltip_timeout = 0;

  return FALSE;
}

static void
mx_widget_remove_tooltip_timeout (MxWidget *widget)
{
  MxWidgetPrivate *priv = widget->priv;

  if (priv->tooltip_timeout)
    {
      g_source_remove (priv->tooltip_timeout);
      priv->tooltip_timeout = 0;
    }
}

static void
mx_widget_set_tooltip_timeout (MxWidget *widget)
{
  MxWidgetPrivate *priv = widget->priv;

  /* Remove any existing tooltip timeout so that we can start again */
  mx_widget_remove_tooltip_timeout (widget);

  priv->tooltip_timeout =
    clutter_threads_add_timeout (mx_widget_get_tooltip_delay (widget),
                                 mx_widget_tooltip_timeout_cb,
                                 widget);
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
      g_object_remove_weak_pointer (G_OBJECT (priv->old_border_image),
                                    (gpointer)&priv->old_border_image);
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

  if (priv->menu)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->menu));
      priv->menu = NULL;
    }

  G_OBJECT_CLASS (mx_widget_parent_class)->dispose (gobject);
}

static void
mx_widget_finalize (GObject *gobject)
{
  MxWidgetPrivate *priv = MX_WIDGET (gobject)->priv;

  mx_widget_remove_tooltip_timeout (MX_WIDGET (gobject));

  g_free (priv->style_class);
  g_free (priv->pseudo_class);

  if (priv->mx_border_image)
    {
      g_boxed_free (MX_TYPE_BORDER_IMAGE, priv->mx_border_image);
      priv->mx_border_image = NULL;
    }

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
  ClutterActorBox frame_box = { 0, 0, box->x2 - box->x1, box->y2 - box->y1 };

  klass = CLUTTER_ACTOR_CLASS (mx_widget_parent_class);
  klass->allocate (actor, box, flags);

  /* update tooltip position */
  if (priv->tooltip)
    {
      ClutterVertex verts[4];
      ClutterGeometry area;
      gfloat x, y, x2, y2;
      gint i;

      clutter_actor_get_abs_allocation_vertices (actor, verts);

      x = y = G_MAXFLOAT;
      x2 = y2 = -G_MAXFLOAT;
      for (i = 0; i < G_N_ELEMENTS (verts); i++)
        {
          if (verts[i].x < x)
            x = verts[i].x;
          if (verts[i].x > x2)
            x2 = verts[i].x;
          if (verts[i].y < y)
            y = verts[i].y;
          if (verts[i].y > y2)
            y2 = verts[i].y;
        }

      area.x = x;
      area.y = y;
      area.width = x2 - x;
      area.height = y2 - y;

      mx_tooltip_set_tip_area (priv->tooltip, &area);
    }



  /* allocate border images */
  if (priv->border_image)
    clutter_actor_allocate (priv->border_image, &frame_box, flags);

  if (priv->old_border_image)
    clutter_actor_allocate (priv->old_border_image, &frame_box, flags);


  if (priv->background_image)
    {
      gfloat w, h;

      clutter_actor_get_preferred_size (CLUTTER_ACTOR (priv->background_image),
                                        NULL, NULL,
                                        &w, &h);

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
  if (priv->menu)
    clutter_actor_allocate_preferred_size (CLUTTER_ACTOR (priv->menu),
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

  if (priv->menu)
    clutter_actor_paint (CLUTTER_ACTOR (priv->menu));
}

static void
mx_widget_pick (ClutterActor *self, const ClutterColor *color)
{
  MxWidgetPrivate *priv = MX_WIDGET (self)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->pick (self, color);

  if (priv->menu)
    clutter_actor_paint (CLUTTER_ACTOR (priv->menu));
}

static void
mx_widget_map (ClutterActor *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->map (actor);

  if (priv->border_image)
    clutter_actor_map (priv->border_image);

  if (priv->old_border_image)
    clutter_actor_map (priv->old_border_image);

  if (priv->background_image)
    clutter_actor_map (priv->background_image);

  if (priv->tooltip)
    clutter_actor_map (CLUTTER_ACTOR (priv->tooltip));

  if (priv->menu)
    clutter_actor_map (CLUTTER_ACTOR (priv->menu));
}

static void
mx_widget_unmap (ClutterActor *actor)
{
  MxWidgetPrivate *priv = MX_WIDGET (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->unmap (actor);

  if (priv->border_image)
    clutter_actor_unmap (priv->border_image);

  if (priv->old_border_image)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->old_border_image));

  if (priv->background_image)
    clutter_actor_unmap (priv->background_image);

  if (priv->tooltip)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->tooltip));

  if (priv->menu)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->menu));
}

static void
old_background_faded_cb (ClutterAnimation *animation, ClutterActor *self)
{
  clutter_actor_unparent (self);
}

/* TODO: move to mx-types.c */
static gboolean
mx_border_image_equal (MxBorderImage *v1,
                       MxBorderImage *v2)
{
  if (v1 == v2)
    return FALSE;

  if (!v1 && v2)
    return TRUE;

  if (!v2 && v1)
    return TRUE;

  if (g_strcmp0 (v1->uri, v2->uri))
    return TRUE;

  if (v1->top != v2->top)
    return TRUE;

  if (v1->right != v2->right)
    return TRUE;

  if (v1->bottom != v2->bottom)
    return TRUE;

  if (v1->left != v2->left)
    return TRUE;

  return FALSE;
}

static void
mx_widget_style_changed (MxStylable *self, MxStyleChangedFlags flags)
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
  gboolean border_image_changed = FALSE;

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


  /* border-image property */

  /* check whether the border-image has changed */
  border_image_changed = mx_border_image_equal (priv->mx_border_image,
                                                border_image);

  /* remove the old border-image if it has changed */
  if (border_image_changed && priv->border_image)
    {
      if (duration == 0)
        {
          clutter_actor_unparent (priv->border_image);
        }
      else
        {
          if (priv->old_border_image)
            {
              g_object_remove_weak_pointer (G_OBJECT (priv->old_border_image),
                                            (gpointer)&priv->old_border_image);
              clutter_actor_unparent (priv->old_border_image);
            }

          priv->old_border_image = priv->border_image;
          g_object_add_weak_pointer (G_OBJECT (priv->old_border_image),
                                     (gpointer)&priv->old_border_image);
          clutter_actor_animate (priv->old_border_image,
                                 CLUTTER_LINEAR,
                                 duration,
                                 "opacity", 0x0,
                                 "signal-after::completed",
                                   old_background_faded_cb,
                                   priv->old_border_image,
                                 NULL);
        }

      priv->border_image = NULL;
    }
  texture_cache = mx_texture_cache_get_default ();

  /* apply the new border-image, as long as there is a valid URI */
  if (border_image_changed && border_image && border_image->uri)
    {
      gint border_left, border_right, border_top, border_bottom;
      gint width, height;

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

  /* if the border-image has changed, free the old one and store the new one */
  if (border_image_changed)
    {
      if (priv->mx_border_image)
        g_boxed_free (MX_TYPE_BORDER_IMAGE, priv->mx_border_image);

      priv->mx_border_image = border_image;
    }
  else
    {
      /* If it's not changed just free the one we've requested */
      if (border_image)
        g_boxed_free (MX_TYPE_BORDER_IMAGE, border_image);
    }

  /* background-image property */
  if (priv->background_image)
    {
      clutter_actor_unparent (priv->background_image);
      priv->background_image = NULL;
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
}

static gboolean
mx_widget_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  /* This is also expected to handle enter events from child actors
     because they will bubble up */

  mx_stylable_style_pseudo_class_add (MX_STYLABLE (widget), "hover");

  return FALSE;
}

static gboolean
mx_widget_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  /* If the leave event is simply moving to a child actor then we'll
     ignore it so that the actor will retain its hover state until the
     pointer moves to an unrelated actor. For this to work the actual
     leave event from the child needs to bubble up to this actor */
  if (event->related &&
      clutter_actor_contains (actor, event->related))
    return FALSE;

  mx_widget_hide_tooltip (widget);

  mx_widget_long_press_cancel (widget);
  mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "active");
  mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "hover");

  return FALSE;
}

static gboolean
mx_widget_motion (ClutterActor       *actor,
                  ClutterMotionEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);
  MxWidgetPrivate *priv = widget->priv;

  if (priv->tooltip && !CLUTTER_ACTOR_IS_VISIBLE (priv->tooltip))
    {
      /* If tooltips are in browse mode then display the tooltip immediately */
      if (mx_tooltip_is_in_browse_mode ())
        mx_widget_show_tooltip (widget);
      else
        mx_widget_set_tooltip_timeout (widget);
    }

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
  MxSettings *settings = mx_settings_get_default ();
  guint timeout;

  g_object_get (settings, "long-press-timeout", &timeout, NULL);

  g_signal_emit (widget, widget_signals[LONG_PRESS], 0, event->x,
                 event->y, MX_LONG_PRESS_QUERY, &query_result);

  if (query_result)
    priv->long_press_source = g_timeout_add (timeout,
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

/**
 * mx_widget_apply_style:
 * @widget: A #MxWidget
 * @style: A #MxStyle
 *
 * Used to implement how a new style instance should be applied in the widget.
 * For instance, setting style instance on stylable internal children.
 *
 * Since: 1.2
 */
void
mx_widget_apply_style (MxWidget *widget,
                       MxStyle  *style)
{
  MxWidgetClass *klass;

  g_return_if_fail (MX_IS_WIDGET (widget));
  g_return_if_fail (style != NULL);

  klass = MX_WIDGET_GET_CLASS (widget);

  if (klass->apply_style != NULL)
    klass->apply_style (widget, style);
}

static gboolean
mx_widget_button_press (ClutterActor       *actor,
                        ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  if (mx_widget_get_disabled (MX_WIDGET (actor)))
      return TRUE;

  if (event->button == 1)
    mx_stylable_style_pseudo_class_add (MX_STYLABLE (widget), "active");

  mx_widget_long_press_query (widget, event);

  return FALSE;
}

static gboolean
mx_widget_button_release (ClutterActor       *actor,
                          ClutterButtonEvent *event)
{
  MxWidget *widget = MX_WIDGET (actor);

  if (mx_widget_get_disabled (MX_WIDGET (actor)))
      return TRUE;

  if (event->button == 1)
    mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "active");

  mx_widget_long_press_cancel (widget);

  return FALSE;
}

static void
mx_widget_hide (ClutterActor *actor)
{
  MxWidget *widget = (MxWidget *) actor;

  /* hide the tooltip, if there is one */
  mx_widget_hide_tooltip (widget);

  CLUTTER_ACTOR_CLASS (mx_widget_parent_class)->hide (actor);
}

static void
_mx_widget_propagate_disabled (ClutterContainer *container,
                               gboolean          disabled)
{
  GList *c, *children;

  children = clutter_container_get_children (container);

  /* Recurse through the children and set the 'parent_disabled' flag. */
  for (c = children; c; c = c->next)
    {
      MxWidget *child = c->data;

      if (MX_IS_WIDGET (child))
        {
          MxWidgetPrivate *child_priv = child->priv;

          child_priv->parent_disabled = disabled;

          /* emit the "notify" signal for the "disabled" property */
          g_object_notify_by_pspec (G_OBJECT (container),
                                    widget_properties[PROP_DISABLED]);


          if (disabled)
            mx_stylable_style_pseudo_class_add (MX_STYLABLE (child),
                                                "disabled");
          else
            mx_stylable_style_pseudo_class_remove (MX_STYLABLE (child),
                                                   "disabled");

          /* If this child has already been disabled explicitly,
           * we don't need to recurse through its children to set
           * the 'parent_disabled' flag, as it'll already be set.
           */
          if (child_priv->is_disabled)
            continue;
        }

      if (CLUTTER_IS_CONTAINER (child))
        _mx_widget_propagate_disabled ((ClutterContainer *) child, disabled);
    }

  g_list_free (children);
}

static void
mx_widget_parent_set (ClutterActor *actor,
                      ClutterActor *old_parent)
{
  ClutterActor *parent;
  MxWidget *widget = (MxWidget *) actor;
  MxWidgetPrivate *priv = widget->priv;

  /* Make sure the disabled state cache remains valid */
  if (!priv->is_disabled)
    {
      gboolean disabled;

      parent = clutter_actor_get_parent (actor);
      while (parent && !MX_IS_WIDGET (parent))
        parent = clutter_actor_get_parent (parent);

      if (parent)
        {
          MxWidgetPrivate *parent_priv = ((MxWidget *) parent)->priv;
          disabled = parent_priv->is_disabled || parent_priv->parent_disabled;
        }
      else
        disabled = FALSE;

      if (disabled != priv->parent_disabled)
        {
          priv->parent_disabled = disabled;
          if (CLUTTER_IS_CONTAINER (widget))
            _mx_widget_propagate_disabled ((ClutterContainer *) widget,
                                           disabled);
        }
    }
}

static gboolean
mx_widget_get_paint_volume (ClutterActor       *actor,
                            ClutterPaintVolume *volume)
{
  /* the allocation of scrollable widgets cannot be used as the paint volume
   * because it does not account for any transformations applied during
   * scrolling */
  if (MX_IS_SCROLLABLE (actor))
    return FALSE;

  return clutter_paint_volume_set_from_allocation (volume, actor);
}

static void
mx_widget_class_init (MxWidgetClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxWidgetPrivate));

  gobject_class->set_property = mx_widget_set_property;
  gobject_class->get_property = mx_widget_get_property;
  gobject_class->dispose = mx_widget_dispose;
  gobject_class->finalize = mx_widget_finalize;

  actor_class->allocate = mx_widget_allocate;
  actor_class->paint = mx_widget_paint;
  actor_class->pick = mx_widget_pick;
  actor_class->map = mx_widget_map;
  actor_class->unmap = mx_widget_unmap;

  actor_class->enter_event = mx_widget_enter;
  actor_class->leave_event = mx_widget_leave;
  actor_class->motion_event = mx_widget_motion;
  actor_class->button_press_event = mx_widget_button_press;
  actor_class->button_release_event = mx_widget_button_release;

  actor_class->hide = mx_widget_hide;
  actor_class->parent_set = mx_widget_parent_set;

  actor_class->get_paint_volume = mx_widget_get_paint_volume;

  klass->paint_background = mx_widget_real_paint_background;

  /* stylable interface properties */
  g_object_class_override_property (gobject_class, PROP_STYLE, "style");
  widget_properties[PROP_STYLE] = g_object_class_find_property (gobject_class,
                                                                "style");
  g_object_class_override_property (gobject_class, PROP_STYLE_CLASS,
                                    "style-class");
  widget_properties[PROP_STYLE_CLASS] =
    g_object_class_find_property (gobject_class, "style-class");

  g_object_class_override_property (gobject_class, PROP_STYLE_PSEUDO_CLASS,
                                    "style-pseudo-class");
  widget_properties[PROP_STYLE_PSEUDO_CLASS] =
    g_object_class_find_property (gobject_class, "style-pseudo-class");

  /**
   * MxWidget:tooltip-text:
   *
   * text displayed on the tooltip
   */
  widget_properties[PROP_TOOLTIP_TEXT] =
    g_param_spec_string ("tooltip-text",
                         "Tooltip Text",
                         "Text displayed on the tooltip",
                         "",
                         MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (gobject_class, PROP_TOOLTIP_TEXT,
                                   widget_properties[PROP_TOOLTIP_TEXT]);

  /**
   * MxWidget:menu:
   *
   * #MxMenu associated with the widget.
   */
  widget_properties[PROP_MENU] =
    g_param_spec_object ("menu",
                         "Menu",
                         "The MxMenu associated with the widget",
                         MX_TYPE_MENU,
                         MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_MENU,
                                   widget_properties[PROP_MENU]);

  widget_properties[PROP_DISABLED] = g_param_spec_boolean ("disabled",
                                                           "Disabled",
                                                           "Whether disabled"
                                                           " styling should be"
                                                           " applied and the"
                                                           " widget made"
                                                           " unreactive.",
                                                           FALSE,
                                                           MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_DISABLED,
                                   widget_properties[PROP_DISABLED]);

  widget_properties[PROP_TOOLTIP_DELAY] =
    g_param_spec_int ("tooltip-delay",
                      "Tooltip delay",
                      "Delay time before showing the tooltip",
                      0, G_MAXINT, MX_WIDGET_TOOLTIP_TIMEOUT,
                      MX_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_TOOLTIP_DELAY,
                                   widget_properties[PROP_TOOLTIP_DELAY]);

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
mx_widget_set_style (MxStylable *stylable,
                     MxStyle    *style)
{
  MxWidgetPrivate *priv = MX_WIDGET (stylable)->priv;

  if (priv->style)
    g_object_unref (priv->style);

  priv->style = g_object_ref_sink (style);

  mx_widget_apply_style (MX_WIDGET (stylable), style);
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

      g_object_notify_by_pspec (G_OBJECT (actor),
                                widget_properties[PROP_STYLE_CLASS]);
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

      g_object_notify_by_pspec (G_OBJECT (actor),
                                widget_properties[PROP_STYLE_PSEUDO_CLASS]);
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
                                 MX_FONT_WEIGHT_NORMAL,
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

      iface->get_style = mx_widget_get_style;
      iface->set_style = mx_widget_set_style;

      iface->get_style_pseudo_class = _mx_stylable_get_style_pseudo_class;
      iface->set_style_pseudo_class = _mx_stylable_set_style_pseudo_class;
      iface->get_style_class = _mx_widget_get_style_class;
      iface->set_style_class = _mx_widget_set_style_class;
    }
}

static void
mx_widget_init (MxWidget *actor)
{
  actor->priv = MX_WIDGET_GET_PRIVATE (actor);

  /* set the default style */
  mx_stylable_set_style (MX_STYLABLE (actor), mx_style_get_default ());

  /* connect the notifiers for the stylable */
  mx_stylable_connect_change_notifiers (MX_STYLABLE (actor));
}


/**
 * mx_widget_get_border_image:
 * @actor: A #MxWidget
 *
 * Get the texture used as the border image. This is set using the
 * "border-image" CSS property. This function should normally only be used
 * by subclasses.
 *
 * Returns: (transfer none): #ClutterActor
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
 * Returns: (transfer none): a #ClutterActor
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

static void
mx_widget_set_has_tooltip (MxWidget *widget,
                           gboolean  has_tooltip)
{
  MxWidgetPrivate *priv;
  ClutterActor *actor;

  g_return_if_fail (MX_IS_WIDGET (widget));

  actor = CLUTTER_ACTOR (widget);

  priv = widget->priv;

  if (has_tooltip)
    {
      clutter_actor_set_reactive (actor, TRUE);

      if (!priv->tooltip)
        {
          priv->tooltip = g_object_new (MX_TYPE_TOOLTIP, NULL);
          clutter_actor_set_parent (CLUTTER_ACTOR (priv->tooltip), actor);
          if (mx_stylable_style_pseudo_class_contains (MX_STYLABLE (widget), "hover"))
            mx_widget_show_tooltip (widget);
        }
    }
  else
    {
      if (priv->tooltip)
        {
          clutter_actor_unparent (CLUTTER_ACTOR (priv->tooltip));
          priv->tooltip = NULL;
        }

      mx_widget_remove_tooltip_timeout (widget);
    }
}

/**
 * mx_widget_set_tooltip_text:
 * @widget: A #MxWidget
 * @text: text to set as the tooltip
 *
 * Set the tooltip text of the widget. Note that setting tooltip text will cause
 * the widget to be set reactive. If you no longer need tooltips and you do not
 * need the widget to be reactive, you must set ClutterActor::reactive to
 * %FALSE.
 *
 */
void
mx_widget_set_tooltip_text (MxWidget    *widget,
                            const gchar *text)
{
  MxWidgetPrivate *priv;
  const gchar *old_text;

  g_return_if_fail (MX_IS_WIDGET (widget));

  priv = widget->priv;

  if (priv->tooltip)
    old_text = mx_tooltip_get_text (priv->tooltip);
  else
    old_text = NULL;

  /* Don't do anything if the text hasn't changed */
  if ((text == old_text) ||
      (text && old_text && g_str_equal (text, old_text)))
    return;

  if (text == NULL)
    mx_widget_set_has_tooltip (widget, FALSE);
  else
    mx_widget_set_has_tooltip (widget, TRUE);

  if (priv->tooltip)
    mx_tooltip_set_text (priv->tooltip, text);

  g_object_notify_by_pspec (G_OBJECT (widget),
                            widget_properties[PROP_TOOLTIP_TEXT]);
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

  if (!priv->tooltip)
    return NULL;

  return mx_tooltip_get_text (widget->priv->tooltip);
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
  gint i;
  gfloat x, y, x2, y2;
  ClutterGeometry area;
  ClutterVertex verts[4];

  g_return_if_fail (MX_IS_WIDGET (widget));

  /* Remove any timeout so we don't show the tooltip again */
  mx_widget_remove_tooltip_timeout (widget);

  /* XXX not necceary, but first allocate transform is wrong */

  /* Work out the bounding box */
  clutter_actor_get_abs_allocation_vertices ((ClutterActor*) widget,
                                             verts);

  x = y = G_MAXFLOAT;
  x2 = y2 = -G_MAXFLOAT;
  for (i = 0; i < G_N_ELEMENTS (verts); i++)
    {
      if (verts[i].x < x)
        x = verts[i].x;
      if (verts[i].x > x2)
        x2 = verts[i].x;
      if (verts[i].y < y)
        y = verts[i].y;
      if (verts[i].y > y2)
        y2 = verts[i].y;
    }

  area.x = x;
  area.y = y;
  area.width = x2 - x;
  area.height = y2 - y;


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

  mx_widget_remove_tooltip_timeout (widget);

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

/**
 * mx_widget_get_available_area:
 * @widget: A #MxWidget
 * @allocation: A #ClutterActorBox
 * @area: A #ClutterActorBox
 *
 * Copies @allocation into @area and accounts for the padding values. This
 * gives the area that is available in which to allocate children with respect
 * to padding.
 *
 */
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

/**
 * mx_widget_set_menu:
 * @widget: A #MxWidget
 * @menu: A #MxMenu
 *
 * Set the value of the #MxWidget:menu property.
 *
 */
void
mx_widget_set_menu (MxWidget *widget,
                    MxMenu   *menu)
{
  MxWidgetPrivate *priv = widget->priv;

  if (priv->menu)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->menu));
      priv->menu = NULL;
    }

  if (menu)
    {
      priv->menu = menu;
      clutter_actor_set_parent (CLUTTER_ACTOR (menu), CLUTTER_ACTOR (widget));
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (widget));
}

/**
 * mx_widget_get_menu:
 * @widget: A #MxWidget
 *
 * Get the object in the #MxWidget:menu property.
 *
 * Returns: (transfer none): The current object in the "menu" property.
 */
MxMenu *
mx_widget_get_menu (MxWidget *widget)
{
  return widget->priv->menu;
}

/**
 * mx_widget_set_disabled:
 * @widget: an #MxWidget
 * @disabled: value to set
 *
 * Set the disabled property. Disabled widgets have a "disabled" pseudo-class
 * until disabled is set to #FALSE.
 */
void
mx_widget_set_disabled (MxWidget *widget,
                        gboolean  disabled)
{
  MxWidgetPrivate *priv;

  g_return_if_fail (MX_IS_WIDGET (widget));

  priv = widget->priv;

  if (priv->is_disabled != disabled)
    {
      priv->is_disabled = disabled;

      if (disabled)
        mx_stylable_style_pseudo_class_add (MX_STYLABLE (widget), "disabled");
      else
        mx_stylable_style_pseudo_class_remove (MX_STYLABLE (widget), "disabled");

      /* Propagate the disabled state to our children, if necessary */
      if (!priv->parent_disabled && CLUTTER_IS_CONTAINER (widget))
        _mx_widget_propagate_disabled ((ClutterContainer *) widget, disabled);

      /* when a widget is disabled, get_style_pseudo_class will always return
       * "disabled" */

      clutter_actor_queue_relayout (CLUTTER_ACTOR (widget));

      mx_stylable_style_changed (MX_STYLABLE (widget), 0);

      g_object_notify_by_pspec (G_OBJECT (widget),
                                widget_properties[PROP_DISABLED]);
    }
}

/**
 * mx_widget_get_disabled:
 * @widget: an #MxWidget
 *
 * Get the value of the "disabled" property.
 */
gboolean
mx_widget_get_disabled (MxWidget *widget)
{
  g_return_val_if_fail (MX_IS_WIDGET (widget), FALSE);
  return widget->priv->is_disabled || widget->priv->parent_disabled;
}

/**
 * mx_widget_set_tooltip_delay:
 * @widget: an #MxWidget
 *
 * Set the value, in milliseconds, of the "tooltip-delay" property.
 * This is initially set to MX_WIDGET_TOOLTIP_TIMEOUT.
 */
void
mx_widget_set_tooltip_delay (MxWidget *widget,
                             guint delay)
{
  g_return_if_fail (MX_IS_WIDGET (widget));

  if (widget->priv->tooltip_delay != delay)
    {
      widget->priv->tooltip_delay = delay;
      g_object_notify_by_pspec (G_OBJECT (widget),
                                widget_properties[PROP_TOOLTIP_DELAY]);
    }
}

/**
 * mx_widget_get_tooltip_delay:
 * @widget: an #MxWidget
 *
 * Get the value of the "tooltip-delay" property.
 *
 * Returns: the current delay value in milliseconds
 */
guint
mx_widget_get_tooltip_delay (MxWidget *widget)
{
  g_return_val_if_fail (MX_IS_WIDGET (widget), 0);

  return widget->priv->tooltip_delay;
}

/* Support translateable strings from JSON */
static void
widget_scriptable_set_custom_property (ClutterScriptable *scriptable,
                                       ClutterScript     *script,
                                       const gchar       *name,
                                       const GValue      *value)
{
  GParamSpec *pspec;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (scriptable), name);

  if (pspec && pspec->flags & MX_PARAM_TRANSLATEABLE &&
      pspec->value_type == G_TYPE_STRING)
    {
      g_object_set (scriptable, name,
                    gettext (g_value_get_string (value)), NULL);
    }
  else
    {
      /* chain up */
      if (parent_scriptable_iface->set_custom_property)
        parent_scriptable_iface->set_custom_property (scriptable, script,
                                                      name,
                                                      value);
    }
}

static void
scriptable_iface_init (ClutterScriptableIface *iface)
{
  parent_scriptable_iface = g_type_interface_peek_parent (iface);

  if (!parent_scriptable_iface)
    parent_scriptable_iface = g_type_default_interface_peek
                                          (CLUTTER_TYPE_SCRIPTABLE);

  iface->set_custom_property = widget_scriptable_set_custom_property;
}
