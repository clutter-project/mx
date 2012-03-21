/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-entry.c: Plain entry actor
 *
 * Copyright 2008, 2009 Intel Corporation
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

/**
 * SECTION:mx-entry
 * @short_description: Widget for displaying text
 *
 * #MxEntry is a widget for displaying and editing a single line of text.
 * It derives from #MxWidget to add extra style and placement functionality over
 * #ClutterText. The internal #ClutterText is publicly accessibly to allow
 * applications to set further properties.
 *
 * #MxEntry supports the following pseudo style states:
 * <itemizedlist>
 *  <listitem>
 *   <para>focus: the widget has focus</para>
 *  </listitem>
 *  <listitem>
 *   <para>indeterminate: the widget is showing the hint text</para>
 *  </listitem>
 * </itemizedlist>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#ifdef HAVE_CLUTTER_IMCONTEXT
#include <clutter-imcontext/clutter-imtext.h>
#endif

#include "mx-entry.h"

#include "mx-widget.h"
#include "mx-stylable.h"
#include "mx-texture-cache.h"
#include "mx-marshal.h"
#include "mx-clipboard.h"
#include "mx-focusable.h"
#include "mx-private.h"
#include "mx-tooltip.h"

#ifdef HAVE_X11
/* for pointer cursor support */
#include <clutter/x11/clutter-x11.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#endif

#define HAS_FOCUS(actor) (clutter_actor_get_stage (actor) && clutter_stage_get_key_focus ((ClutterStage *) clutter_actor_get_stage (actor)) == actor)

#define MX_ENTRY_TOOLTIP_DELAY 500

/* properties */
enum
{
  PROP_0,

  PROP_CLUTTER_TEXT,
  PROP_HINT_TEXT,
  PROP_TEXT,
  PROP_PASSWORD_CHAR,
  PROP_ICON_HIGHLIGHT_SUFFIX,
  PROP_PRIMARY_ICON_TOOLTIP_TEXT,
  PROP_SECONDARY_ICON_TOOLTIP_TEXT
};

/* signals */
enum
{
  PRIMARY_ICON_CLICKED,
  SECONDARY_ICON_CLICKED,

  LAST_SIGNAL
};

#define MX_ENTRY_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_ENTRY, MxEntryPrivate))
#define MX_ENTRY_PRIV(x) ((MxEntry *) x)->priv


struct _MxEntryPrivate
{
  ClutterActor *entry;
  gchar        *hint;

  ClutterActor *primary_icon;
  ClutterActor *primary_icon_highlight;
  MxTooltip    *primary_icon_tooltip;

  ClutterActor *secondary_icon;
  ClutterActor *secondary_icon_highlight;
  MxTooltip    *secondary_icon_tooltip;

  gchar  *primary_icon_filename;
  gchar  *secondary_icon_filename;

  gchar  *icon_highlight_suffix;

  gfloat        spacing;

  gunichar password_char;

  GQueue   *undo_history;
  gulong    undo_timeout_source;

  guint hint_visible : 1;
  guint pause_undo : 1;
  guint scrolling : 1;
  guint unicode_input_mode : 1;
  guint pointer_in_entry : 1;

  GString *preedit_string;

  guint tooltip_timeout;
};

static guint entry_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_iface_init (MxStylableIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxEntry, mx_entry, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init));

static void
mx_entry_set_property (GObject      *gobject,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  MxEntry *entry = MX_ENTRY (gobject);

  switch (prop_id)
    {
    case PROP_HINT_TEXT:
      mx_entry_set_hint_text (entry, g_value_get_string (value));
      break;

    case PROP_TEXT:
      mx_entry_set_text (entry, g_value_get_string (value));
      break;

    case PROP_PASSWORD_CHAR:
      mx_entry_set_password_char (entry, g_value_get_uint (value));
      break;

    case PROP_ICON_HIGHLIGHT_SUFFIX:
      mx_entry_set_icon_highlight_suffix (entry,
                                          g_value_get_string (value));
      break;

    case PROP_PRIMARY_ICON_TOOLTIP_TEXT:
      mx_entry_set_primary_icon_tooltip_text (entry,
                                              g_value_get_string (value));
      break;

    case PROP_SECONDARY_ICON_TOOLTIP_TEXT:
      mx_entry_set_secondary_icon_tooltip_text (entry,
                                                g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_entry_get_property (GObject    *gobject,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (gobject);

  switch (prop_id)
    {
    case PROP_CLUTTER_TEXT:
      g_value_set_object (value, priv->entry);
      break;

    case PROP_HINT_TEXT:
      g_value_set_string (value, priv->hint);
      break;

    case PROP_TEXT:
      g_value_set_string (value,
                          clutter_text_get_text (CLUTTER_TEXT (priv->entry)));
      break;

    case PROP_PASSWORD_CHAR:
      g_value_set_uint (value, priv->password_char);
      break;

    case PROP_ICON_HIGHLIGHT_SUFFIX:
      g_value_set_string (value, priv->icon_highlight_suffix);
      break;

    case PROP_PRIMARY_ICON_TOOLTIP_TEXT:
      if (priv->primary_icon_tooltip)
        g_value_set_string (value,
                            mx_tooltip_get_text (priv->primary_icon_tooltip));
      break;

    case PROP_SECONDARY_ICON_TOOLTIP_TEXT:
      if (priv->secondary_icon_tooltip)
        g_value_set_string (value,
                            mx_tooltip_get_text (priv->secondary_icon_tooltip));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_entry_dispose (GObject *object)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (object);

  if (priv->entry)
    {
      clutter_actor_destroy (priv->entry);
      priv->entry = NULL;
    }

  if (priv->primary_icon)
    {
      clutter_actor_destroy (priv->primary_icon);
      priv->primary_icon = NULL;
    }

  if (priv->secondary_icon)
    {
      clutter_actor_destroy (priv->secondary_icon);
      priv->secondary_icon = NULL;
    }

  if (priv->primary_icon_highlight)
    {
      clutter_actor_destroy (priv->primary_icon_highlight);
      priv->primary_icon_highlight = NULL;
    }

  if (priv->secondary_icon_highlight)
    {
      clutter_actor_destroy (priv->secondary_icon_highlight);
      priv->secondary_icon_highlight = NULL;
    }

  if (priv->primary_icon_tooltip)
    {
      clutter_actor_destroy (CLUTTER_ACTOR (priv->primary_icon_tooltip));
      priv->primary_icon_tooltip = NULL;
    }

  if (priv->secondary_icon_tooltip)
    {
      clutter_actor_destroy (CLUTTER_ACTOR (priv->secondary_icon_tooltip));
      priv->secondary_icon_tooltip = NULL;
    }

  G_OBJECT_CLASS (mx_entry_parent_class)->dispose (object);
}

static void
mx_entry_finalize (GObject *object)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (object);

  g_free (priv->hint);
  priv->hint = NULL;

  if (priv->undo_history)
    {
      g_queue_foreach (priv->undo_history, (GFunc) g_free, NULL);
      g_queue_free (priv->undo_history);
      priv->undo_history = NULL;
    }

  if (priv->undo_timeout_source)
    {
      g_source_remove (priv->undo_timeout_source);
      priv->undo_timeout_source = 0;
    }

  if (priv->tooltip_timeout)
    {
      g_source_remove (priv->tooltip_timeout);
      priv->tooltip_timeout = 0;
    }

  if (priv->preedit_string)
    {
      g_string_free (priv->preedit_string, TRUE);
      priv->preedit_string = NULL;
    }

  if (priv->primary_icon_filename)
    {
      g_free (priv->primary_icon_filename);
      priv->primary_icon_filename = NULL;
    }

  if (priv->secondary_icon_filename)
    {
      g_free (priv->secondary_icon_filename);
      priv->secondary_icon_filename = NULL;
    }

  if (priv->icon_highlight_suffix)
    {
      g_free (priv->icon_highlight_suffix);
      priv->icon_highlight_suffix = NULL;
    }

  G_OBJECT_CLASS (mx_entry_parent_class)->finalize (object);
}

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;
      static const ClutterColor default_color
        = { 0x0, 0x9c, 0xcf, 0xff };
      static const ClutterColor white
        = { 0xff, 0xff, 0xff, 0xff };

      is_initialized = TRUE;

      pspec = clutter_param_spec_color ("caret-color",
                                        "Caret Color",
                                        "Color of the entry's caret",
                                        &default_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ENTRY, pspec);

      pspec = clutter_param_spec_color ("selection-background-color",
                                        "Selection Background Color",
                                        "Color of the entry's selection",
                                        &default_color,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ENTRY, pspec);

      pspec = clutter_param_spec_color ("selected-text-color",
                                        "Selected Text Color",
                                        "Color of the selected text",
                                        &white,
                                        G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ENTRY, pspec);
    }
}

static MxFocusable*
mx_entry_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  clutter_actor_grab_key_focus (MX_ENTRY (focusable)->priv->entry);

  return focusable;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_entry_accept_focus;
}

static void
mx_entry_style_changed (MxWidget *self)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (self);
  ClutterColor *caret_color = NULL;
  ClutterColor *selection_background_color = NULL;
  ClutterColor *selected_text_color = NULL;

  mx_stylable_get (MX_STYLABLE (self),
                   "caret-color", &caret_color,
                   "selection-background-color", &selection_background_color,
                   "selected-text-color", &selected_text_color,
                   NULL);

  if (caret_color)
    {
      clutter_text_set_cursor_color (CLUTTER_TEXT (priv->entry), caret_color);
      clutter_color_free (caret_color);
    }

  if (selection_background_color)
    {
      clutter_text_set_selection_color (CLUTTER_TEXT (priv->entry),
                                        selection_background_color);
      clutter_color_free (selection_background_color);
    }

  if (selected_text_color)
    {
      clutter_text_set_selected_text_color (CLUTTER_TEXT (priv->entry),
                                            selected_text_color);
      clutter_color_free (selected_text_color);
    }

  mx_stylable_apply_clutter_text_attributes (MX_STYLABLE (self),
                                             CLUTTER_TEXT (priv->entry));
}

static void
mx_entry_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);
  MxPadding padding;
  gfloat icon_w;
  gfloat entry_height;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_height -= padding.top + padding.bottom;

  clutter_actor_get_preferred_width (priv->entry, for_height,
                                     min_width_p,
                                     natural_width_p);
  clutter_actor_get_preferred_height (priv->entry, -1,
                                      NULL, &entry_height);

  /* ensure the preferred width is at least large enough to be useful */
  if (natural_width_p)
    *natural_width_p = MAX ((entry_height * 6), *natural_width_p);

  if (priv->primary_icon)
    {
      clutter_actor_get_preferred_width (priv->primary_icon, -1, NULL, &icon_w);

      if (min_width_p)
        *min_width_p += icon_w + priv->spacing;

      if (natural_width_p)
        *natural_width_p += icon_w + priv->spacing;
    }

  if (priv->secondary_icon)
    {
      clutter_actor_get_preferred_width (priv->secondary_icon,
                                         -1, NULL, &icon_w);

      if (min_width_p)
        *min_width_p += icon_w + priv->spacing;

      if (natural_width_p)
        *natural_width_p += icon_w + priv->spacing;
    }

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
mx_entry_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);
  MxPadding padding;
  gfloat icon_h;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_width -= padding.left + padding.right;

  clutter_actor_get_preferred_height (priv->entry, for_width,
                                      min_height_p,
                                      natural_height_p);

  if (priv->primary_icon)
    {
      clutter_actor_get_preferred_height (priv->primary_icon,
                                          -1, NULL, &icon_h);

      if (min_height_p && icon_h > *min_height_p)
        *min_height_p = icon_h;

      if (natural_height_p && icon_h > *natural_height_p)
        *natural_height_p = icon_h;
    }

  if (priv->secondary_icon)
    {
      clutter_actor_get_preferred_height (priv->secondary_icon,
                                          -1, NULL, &icon_h);

      if (min_height_p && icon_h > *min_height_p)
        *min_height_p = icon_h;

      if (natural_height_p && icon_h > *natural_height_p)
        *natural_height_p = icon_h;
    }

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;
}

static void
mx_entry_allocate (ClutterActor          *actor,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);
  ClutterActorClass *parent_class;
  ClutterActorBox child_box, icon_box;
  MxPadding padding;
  gfloat icon_w, icon_h;
  gfloat entry_h, min_h, pref_h, avail_h, pref_w;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  parent_class = CLUTTER_ACTOR_CLASS (mx_entry_parent_class);
  parent_class->allocate (actor, box, flags);

  avail_h = (box->y2 - box->y1) - padding.top - padding.bottom;

  child_box.x1 = padding.left;
  child_box.x2 = box->x2 - box->x1 - padding.right;

  if (priv->primary_icon)
    {
      clutter_actor_get_preferred_width (priv->primary_icon,
                                         -1, NULL, &icon_w);
      clutter_actor_get_preferred_height (priv->primary_icon,
                                          -1, NULL, &icon_h);

      icon_box.x1 = padding.left;
      icon_box.x2 = icon_box.x1 + icon_w;

      icon_box.y1 = (int)(padding.top + avail_h / 2 - icon_h / 2);
      icon_box.y2 = icon_box.y1 + icon_h;

      clutter_actor_allocate (priv->primary_icon,
                              &icon_box,
                              flags);

      /* reduce the size for the entry */
      child_box.x1 += icon_w + priv->spacing;

      if (priv->primary_icon_tooltip)
        mx_tooltip_set_tip_area_from_actor (priv->primary_icon_tooltip,
                                            priv->primary_icon);

      if (priv->primary_icon_highlight)
        {
          clutter_actor_allocate (priv->primary_icon_highlight,
                                  &icon_box,
                                  flags);
        }
    }

  if (priv->secondary_icon)
    {
      clutter_actor_get_preferred_width (priv->secondary_icon,
                                         -1, NULL, &icon_w);
      clutter_actor_get_preferred_height (priv->secondary_icon,
                                          -1, NULL, &icon_h);

      icon_box.x2 = (box->x2 - box->x1) - padding.right;
      icon_box.x1 = icon_box.x2 - icon_w;

      icon_box.y1 = (int)(padding.top + avail_h / 2 - icon_h / 2);
      icon_box.y2 = icon_box.y1 + icon_h;

      clutter_actor_allocate (priv->secondary_icon,
                              &icon_box,
                              flags);

      /* reduce the size for the entry */
      child_box.x2 -= icon_w - priv->spacing;

      if (priv->secondary_icon_tooltip)
        mx_tooltip_set_tip_area_from_actor (priv->secondary_icon_tooltip,
                                            priv->secondary_icon);

      if (priv->secondary_icon_highlight)
        {
          clutter_actor_allocate (priv->secondary_icon_highlight,
                                  &icon_box,
                                  flags);
        }
    }

  clutter_actor_get_preferred_height (priv->entry, child_box.x2 - child_box.x1,
                                      &min_h, &pref_h);

  entry_h = CLAMP (pref_h, min_h, avail_h);

  clutter_actor_get_preferred_width (priv->entry, entry_h, NULL, &pref_w);

  priv->scrolling = (pref_w > (child_box.x2 - child_box.x1));

  child_box.y1 = (int)(padding.top + avail_h / 2 - entry_h / 2);
  child_box.y2 = child_box.y1 + entry_h;

  clutter_actor_allocate (priv->entry, &child_box, flags);

 if (priv->secondary_icon_tooltip)
   clutter_actor_allocate_preferred_size (CLUTTER_ACTOR (priv->secondary_icon_tooltip), flags);

 if (priv->primary_icon_tooltip)
   clutter_actor_allocate_preferred_size (CLUTTER_ACTOR (priv->primary_icon_tooltip), flags);

}

static void clutter_text_changed_cb (ClutterText *text, MxEntry     *entry);

static void
clutter_text_focus_in_cb (ClutterText  *text,
                          ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);

  /* remove the hint if visible */
  if (priv->hint && priv->hint_visible)
    {
      priv->hint_visible = FALSE;

      /* We don't want to emit the notify text change when we're swapping
       * between the hint text and blank for entry.
       */
      g_signal_handlers_block_by_func (priv->entry,
                                       clutter_text_changed_cb,
                                       MX_ENTRY (actor));

      clutter_text_set_text (text, "");

      g_signal_handlers_unblock_by_func (priv->entry,
                                         clutter_text_changed_cb,
                                         MX_ENTRY (actor));


      if (priv->password_char != 0)
        {
          clutter_text_set_password_char (text, priv->password_char);
        }
    }
  mx_stylable_set_style_pseudo_class (MX_STYLABLE (actor), "focus");
  clutter_text_set_cursor_visible (text, TRUE);
}

static void
clutter_text_focus_out_cb (ClutterText  *text,
                           ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);

  /* move the cursor back to the beginning of the entry */
  clutter_text_set_cursor_position (CLUTTER_TEXT (priv->entry), 0);

  /* add a hint if the entry is empty */
  if (priv->hint && !strcmp (clutter_text_get_text (text), ""))
    {
      priv->hint_visible = TRUE;

      /* We don't want to emit the notify text change when we're swapping
       * between the hint text and blank for entry.
       */
      g_signal_handlers_block_by_func (priv->entry,
                                       clutter_text_changed_cb,
                                       MX_ENTRY (actor));

      clutter_text_set_text (text, priv->hint);

      g_signal_handlers_unblock_by_func (priv->entry,
                                         clutter_text_changed_cb,
                                         MX_ENTRY (actor));

      mx_stylable_set_style_pseudo_class (MX_STYLABLE (actor), "indeterminate");

      if (clutter_text_get_password_char (text) != 0)
        {
          clutter_text_set_password_char (text, 0);
        }
    }
  else
    {
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (actor), NULL);
    }
  clutter_text_set_cursor_visible (text, FALSE);
}

static void
mx_entry_paint (ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);
  ClutterActorClass *parent_class;
  ClutterStage *stage;

  parent_class = CLUTTER_ACTOR_CLASS (mx_entry_parent_class);
  parent_class->paint (actor);

  clutter_actor_paint (priv->entry);

  if (priv->primary_icon)
    clutter_actor_paint (priv->primary_icon);

  if (priv->primary_icon_highlight)
    clutter_actor_paint (priv->primary_icon_highlight);

  if (priv->primary_icon_tooltip)
    clutter_actor_paint (CLUTTER_ACTOR (priv->primary_icon_tooltip));

  if (priv->secondary_icon)
    clutter_actor_paint (priv->secondary_icon);

  if (priv->secondary_icon_highlight)
    clutter_actor_paint (priv->secondary_icon_highlight);

  if (priv->secondary_icon_tooltip)
    clutter_actor_paint (CLUTTER_ACTOR (priv->secondary_icon_tooltip));

  /* draw a shadow if the entry is not focused and the text is scrolling */
  stage = (ClutterStage *) clutter_actor_get_stage (priv->entry);

  if (clutter_stage_get_key_focus (stage) != priv->entry && priv->scrolling)
    {
      ClutterGeometry geo;
      CoglTextureVertex top[4] = { { 0,}, };
      ClutterColor *color;
      guint8 r, g, b;


      mx_stylable_get (MX_STYLABLE (actor), "background-color", &color, NULL);

      r = color->red;
      g = color->green;
      b = color->blue;
      clutter_color_free (color);

      cogl_set_source_color4ub (0, 0, 0, 0);

      clutter_actor_get_allocation_geometry (priv->entry, &geo);

      top[0].x = geo.x + geo.width;
      top[0].y = geo.y + geo.height;
      top[1].x = geo.x + geo.width;
      top[1].y = geo.y;
      top[2].x = geo.x + geo.width - 30;
      top[2].y = geo.y;
      top[3].x = geo.x + geo.width - 30;
      top[3].y = geo.y + geo.height;

      cogl_color_set_from_4ub (&top[0].color, r, g, b, 0xff);
      cogl_color_set_from_4ub (&top[1].color, r, g, b, 0xff);
      cogl_color_set_from_4ub (&top[2].color, 0, 0, 0, 0);
      cogl_color_set_from_4ub (&top[3].color, 0, 0, 0, 0);
      cogl_polygon (top, 4, TRUE);
    }
}

static void
mx_entry_pick (ClutterActor       *actor,
               const ClutterColor *c)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);

  CLUTTER_ACTOR_CLASS (mx_entry_parent_class)->pick (actor, c);

  if (!clutter_actor_should_pick_paint (actor))
    return;

  clutter_actor_paint (priv->entry);

  if (priv->primary_icon)
    clutter_actor_paint (priv->primary_icon);

  if (priv->secondary_icon)
    clutter_actor_paint (priv->secondary_icon);
}

static void
mx_entry_map (ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_entry_parent_class)->map (actor);

  clutter_actor_map (priv->entry);

  if (priv->primary_icon)
    clutter_actor_map (priv->primary_icon);

  if (priv->primary_icon_highlight)
    clutter_actor_map (priv->primary_icon_highlight);

  if (priv->secondary_icon)
    clutter_actor_map (priv->secondary_icon);

  if (priv->secondary_icon_highlight)
    clutter_actor_map (priv->secondary_icon_highlight);

  if (priv->secondary_icon_tooltip)
    clutter_actor_map (CLUTTER_ACTOR (priv->secondary_icon_tooltip));

  if (priv->primary_icon_tooltip)
    clutter_actor_map (CLUTTER_ACTOR (priv->primary_icon_tooltip));
}

static void
mx_entry_unmap (ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_entry_parent_class)->unmap (actor);

  if (priv->entry)
    clutter_actor_unmap (priv->entry);

  if (priv->primary_icon)
    clutter_actor_unmap (priv->primary_icon);

  if (priv->primary_icon_highlight)
    clutter_actor_unmap (priv->primary_icon_highlight);

  if (priv->primary_icon_tooltip)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->primary_icon_tooltip));

  if (priv->secondary_icon)
    clutter_actor_unmap (priv->secondary_icon);

  if (priv->secondary_icon_highlight)
    clutter_actor_unmap (priv->secondary_icon_highlight);

  if (priv->secondary_icon_tooltip)
    clutter_actor_unmap (CLUTTER_ACTOR (priv->secondary_icon_tooltip));
}

static void
mx_entry_clipboard_callback (MxClipboard *clipboard,
                             const gchar *text,
                             gpointer     data)
{
  ClutterText *ctext = (ClutterText*)((MxEntry *) data)->priv->entry;
  gint cursor_pos;

  if (!text)
    return;

  /* delete the current selection before pasting */
  clutter_text_delete_selection (ctext);

  /* "paste" the clipboard text into the entry */
  cursor_pos = clutter_text_get_cursor_position (ctext);
  clutter_text_insert_text (ctext, text, cursor_pos);
}

static void
mx_entry_update_preedit (MxEntry *entry)
{
  PangoAttrList *list;
  PangoAttribute *attr;

  attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
  list = pango_attr_list_new ();
  pango_attr_list_insert (list, attr);
  clutter_text_set_preedit_string (CLUTTER_TEXT (entry->priv->entry),
                                   entry->priv->preedit_string->str,
                                   list, 0);
  pango_attr_list_unref (list);
}

static gboolean
mx_entry_captured_event (ClutterActor *actor,
                         ClutterEvent *event)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);
  ClutterKeyEvent *kevent;

  /* handle key events during unicode input mode */
  if (event->type == CLUTTER_KEY_PRESS && priv->unicode_input_mode)
    {
      kevent = (ClutterKeyEvent*) event;

      if (kevent->keyval == CLUTTER_KEY_space
          || kevent->keyval == CLUTTER_KEY_Return
          || kevent->keyval == CLUTTER_KEY_Escape)
        {
          gunichar unichar;

          if (sscanf (priv->preedit_string->str + 1, "%x", &unichar) == 1)
            {
              clutter_text_set_preedit_string (CLUTTER_TEXT (priv->entry), NULL,
                                               NULL, 0);
              if (kevent->keyval != CLUTTER_KEY_Escape)
                clutter_text_insert_unichar (CLUTTER_TEXT (priv->entry),
                                             unichar);
              g_string_free (priv->preedit_string, TRUE);
              priv->preedit_string = NULL;
            }

          priv->unicode_input_mode = FALSE;

          return TRUE;
        }
      else if (kevent->keyval == CLUTTER_KEY_BackSpace)
        {
          g_string_truncate (priv->preedit_string,
                             MAX (1, priv->preedit_string->len - 1));
          mx_entry_update_preedit (MX_ENTRY (actor));
        }
      else
        {
          if (g_unichar_isxdigit (kevent->unicode_value))
            {
              g_string_append_c (priv->preedit_string, kevent->unicode_value);
              mx_entry_update_preedit (MX_ENTRY (actor));
            }
        }

      return TRUE;
    }

  /* button press should cancel unicode input mode */
  if (event->type == CLUTTER_BUTTON_PRESS && priv->unicode_input_mode)
    {
      clutter_text_set_preedit_string (CLUTTER_TEXT (priv->entry), NULL,
                                       NULL, 0);
      g_string_free (priv->preedit_string, TRUE);
      priv->preedit_string = NULL;
      priv->unicode_input_mode = FALSE;
    }


  return FALSE;
}


static gboolean
mx_entry_key_press_event (ClutterActor    *actor,
                          ClutterKeyEvent *event)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);

  /* This is expected to handle events that were emitted for the inner
     ClutterText. They only reach this function if the ClutterText
     didn't handle them */

  /* paste */
  if ((event->modifier_state & CLUTTER_CONTROL_MASK)
      && event->keyval == CLUTTER_KEY_v)
    {
      MxClipboard *clipboard;

      clipboard = mx_clipboard_get_default ();

      mx_clipboard_get_text (clipboard, mx_entry_clipboard_callback, actor);

      return TRUE;
    }

  /* copy */
  if ((event->modifier_state & CLUTTER_CONTROL_MASK)
      && event->keyval == CLUTTER_KEY_c)
    {
      MxClipboard *clipboard;
      gchar *text;

      clipboard = mx_clipboard_get_default ();

      text = clutter_text_get_selection ((ClutterText*) priv->entry);

      if (text && strlen (text))
        mx_clipboard_set_text (clipboard, text);

      return TRUE;
    }


  /* cut */
  if ((event->modifier_state & CLUTTER_CONTROL_MASK)
      && event->keyval == CLUTTER_KEY_x)
    {
      MxClipboard *clipboard;
      gchar *text;

      clipboard = mx_clipboard_get_default ();

      text = clutter_text_get_selection ((ClutterText*) priv->entry);

      if (text && strlen (text))
        {
          mx_clipboard_set_text (clipboard, text);

          /* now delete the text */
          clutter_text_delete_selection ((ClutterText *) priv->entry);
        }

      return TRUE;
    }

  /* undo */
  if ((event->modifier_state & CLUTTER_CONTROL_MASK)
      && event->keyval == CLUTTER_KEY_z)
    {
      gchar *str;

      /* pop the current value off the undo history stack and set the text to
       * the new value */
      if (priv->undo_timeout_source == 0)
        {
          str = g_queue_pop_head (priv->undo_history);
          g_free (str);
        }

      /* prevent storing the value just restored */
      priv->pause_undo = TRUE;

      if (priv->undo_history)
        {
          clutter_text_set_text (CLUTTER_TEXT (priv->entry),
                                 (gchar*) g_queue_peek_head (priv->undo_history));
        }
      return TRUE;
    }

  /* unicode input mode */
  if (event->modifier_state & CLUTTER_CONTROL_MASK
      && event->modifier_state & CLUTTER_SHIFT_MASK
      && event->keyval == CLUTTER_KEY_U)
  {
    priv->unicode_input_mode = TRUE;

    priv->preedit_string = g_string_new ("u");
    mx_entry_update_preedit (MX_ENTRY (actor));
  }


  return FALSE;
}

static void
mx_entry_key_focus_in (ClutterActor *actor)
{
  MxEntryPrivate *priv = MX_ENTRY_PRIV (actor);

  /* We never want key focus. The ClutterText should be given first
     pass for all key events */
  clutter_actor_grab_key_focus (priv->entry);
}

#ifdef HAVE_X11
static void
mx_entry_set_cursor (MxEntry  *entry,
                     gboolean  use_ibeam)
{
  Display *dpy;
  ClutterActor *stage, *actor = CLUTTER_ACTOR (entry);
  Window wid;
  static Cursor ibeam = None;

  dpy = clutter_x11_get_default_display ();
  stage = clutter_actor_get_stage (actor);
  wid = clutter_x11_get_stage_window (CLUTTER_STAGE (stage));

  if (ibeam == None)
    ibeam = XCreateFontCursor (dpy, XC_xterm);

  if (use_ibeam)
    XDefineCursor (dpy, wid, ibeam);
  else
    XUndefineCursor (dpy, wid);
}
#endif

static gboolean
mx_entry_swallow_crossing_event (ClutterActor         *actor,
                                 ClutterCrossingEvent *event)
{

#ifdef HAVE_X11
  if (event->source == MX_ENTRY (actor)->priv->entry && event->related != NULL)
    mx_entry_set_cursor (MX_ENTRY (actor), (event->type == CLUTTER_ENTER));
#endif

  /* swallow enter and leave events, since the pseudo-class must not be set to
   * 'hover' because it would loose the 'focus' state.
   */
  return TRUE;
}

static gboolean
mx_entry_swallow_button_event (ClutterActor       *actor,
                               ClutterButtonEvent *event)
{
  /* swallow button events, since the pseudo-class must not be set to
   * 'active' because it would loose the 'focus' state.
   */

  return TRUE;
}

static void
mx_entry_class_init (MxEntryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxEntryPrivate));

  gobject_class->set_property = mx_entry_set_property;
  gobject_class->get_property = mx_entry_get_property;
  gobject_class->finalize = mx_entry_finalize;
  gobject_class->dispose = mx_entry_dispose;

  actor_class->get_preferred_width = mx_entry_get_preferred_width;
  actor_class->get_preferred_height = mx_entry_get_preferred_height;
  actor_class->allocate = mx_entry_allocate;
  actor_class->paint = mx_entry_paint;
  actor_class->pick = mx_entry_pick;
  actor_class->map = mx_entry_map;
  actor_class->unmap = mx_entry_unmap;
  actor_class->enter_event = mx_entry_swallow_crossing_event;
  actor_class->leave_event = mx_entry_swallow_crossing_event;
  actor_class->button_press_event = mx_entry_swallow_button_event;
  actor_class->button_release_event = mx_entry_swallow_button_event;

  actor_class->captured_event = mx_entry_captured_event;
  actor_class->key_press_event = mx_entry_key_press_event;
  actor_class->key_focus_in = mx_entry_key_focus_in;

  pspec = g_param_spec_object ("clutter-text",
			       "Clutter Text",
			       "Internal ClutterText actor",
			       CLUTTER_TYPE_TEXT,
			       G_PARAM_READABLE);
  g_object_class_install_property (gobject_class, PROP_CLUTTER_TEXT, pspec);

  pspec = g_param_spec_string ("hint-text",
                               "Hint Text",
                               "Text to display when the entry is not focused "
                               "and the text property is empty",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_HINT_TEXT, pspec);

  pspec = g_param_spec_string ("text",
                               "Text",
                               "Text of the entry",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_TEXT, pspec);

  pspec = g_param_spec_unichar ("password-char",
                                "Password Character",
                                "Character to display instead of entered text",
                                0, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_PASSWORD_CHAR, pspec);

  pspec = g_param_spec_string ("icon-highlight-suffix",
                               "Icon highlight suffix",
                               "The filename suffix for the highligh icon",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ICON_HIGHLIGHT_SUFFIX,
                                   pspec);

  pspec = g_param_spec_string ("primary-icon-tooltip-text",
                               "Tooltip for the primary icon",
                               "The tooltip text for the primary icon",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_PRIMARY_ICON_TOOLTIP_TEXT,
                                   pspec);

  pspec = g_param_spec_string ("secondary-icon-tooltip-text",
                               "Tooltip for the secondary icon",
                               "The tooltip text for the secondary icon",
                               NULL, G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_SECONDARY_ICON_TOOLTIP_TEXT,
                                   pspec);

  /* signals */
  /**
   * MxEntry::primary-icon-clicked:
   *
   * Emitted when the primary icon is clicked
   */
  entry_signals[PRIMARY_ICON_CLICKED] =
    g_signal_new ("primary-icon-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxEntryClass, primary_icon_clicked),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  /**
   * MxEntry::secondary-icon-clicked:
   *
   * Emitted when the secondary icon is clicked
   */
  entry_signals[SECONDARY_ICON_CLICKED] =
    g_signal_new ("secondary-icon-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxEntryClass, secondary_icon_clicked),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}


static gboolean
mx_entry_store_undo_timeout (MxEntry *entry)
{
  const gchar *str;
  MxEntryPrivate *priv = entry->priv;

  priv->undo_timeout_source = 0;

  str = mx_entry_get_text (entry);

  if (!priv->undo_history)
    priv->undo_history = g_queue_new ();

  /* prevent duplicated */
  if (!g_strcmp0 (str, (gchar*) g_queue_peek_head (priv->undo_history)))
    {
      return FALSE;
    }

  g_queue_push_head (priv->undo_history, g_strdup (str));

  /* keep the undo history to only 20 items */
  if (g_queue_get_length (priv->undo_history) > 20)
    {
      gpointer *tmp;

      tmp = g_queue_pop_tail (priv->undo_history);

      g_free (tmp);
    }

  return FALSE;
}

static void
mx_entry_store_undo_history (ClutterText *text,
                             MxEntry     *entry)
{
  if (entry->priv->undo_timeout_source)
    {
      g_source_remove (entry->priv->undo_timeout_source);
      entry->priv->undo_timeout_source = 0;
    }

  if (entry->priv->pause_undo)
    {
      entry->priv->pause_undo = FALSE;
      return;
    }

  entry->priv->undo_timeout_source =
    g_timeout_add (750, (GSourceFunc) mx_entry_store_undo_timeout, entry);
}

static gboolean
mx_entry_store_undo_on_keypress (ClutterText     *text,
                                 ClutterKeyEvent *event,
                                 MxEntry         *entry)
{
  if (event->keyval == CLUTTER_KEY_space)
    {
      /* store the text immediately */
      if (entry->priv->undo_timeout_source)
        {
          g_source_remove (entry->priv->undo_timeout_source);
          entry->priv->undo_timeout_source = 0;
        }

      mx_entry_store_undo_timeout (entry);

      entry->priv->pause_undo = TRUE;
    }

  return FALSE;
}

static void
clutter_text_changed_cb (ClutterText *text,
                         MxEntry     *entry)
{
  g_return_if_fail (MX_IS_ENTRY (entry));

  g_object_notify (G_OBJECT (entry), "text");

  mx_entry_store_undo_history (text, entry);
}


static gboolean
entry_event (ClutterActor *actor,
             ClutterEvent *event,
             MxEntry      *entry)
{
#ifdef HAVE_X11
  /* ensure the cursor is in the correct state when the mouse button is
   * released */
  if (event->type == CLUTTER_BUTTON_RELEASE)
    mx_entry_set_cursor (entry, entry->priv->pointer_in_entry);
#endif

  /* don't track enter and leave events for actors other than the entry */
  if (event->any.source != actor)
    return FALSE;

  switch (event->type)
    {
    case CLUTTER_LEAVE:
      entry->priv->pointer_in_entry = FALSE;
      break;

    case CLUTTER_ENTER:
      entry->priv->pointer_in_entry = TRUE;
      break;

    default:
      break;
    }

  return FALSE;
}

static void
mx_entry_init (MxEntry *entry)
{
  MxEntryPrivate *priv;

  priv = entry->priv = MX_ENTRY_GET_PRIVATE (entry);

#ifdef HAVE_CLUTTER_IMCONTEXT
  priv->entry = g_object_new (CLUTTER_TYPE_IMTEXT,
#else
  priv->entry = g_object_new (CLUTTER_TYPE_TEXT,
#endif
                              "line-alignment", PANGO_ALIGN_LEFT,
                              "editable", TRUE,
                              "reactive", TRUE,
                              "single-line-mode", TRUE,
                              NULL);

  g_signal_connect (priv->entry, "key-focus-in",
                    G_CALLBACK (clutter_text_focus_in_cb), entry);

  g_signal_connect (priv->entry, "key-focus-out",
                    G_CALLBACK (clutter_text_focus_out_cb), entry);

  g_signal_connect (priv->entry, "text-changed",
                    G_CALLBACK (clutter_text_changed_cb), entry);

  g_signal_connect (priv->entry, "key-press-event",
                    G_CALLBACK (mx_entry_store_undo_on_keypress), entry);

  g_signal_connect (priv->entry, "event", G_CALLBACK (entry_event), entry);

  priv->spacing = 6.0f;

  clutter_actor_push_internal (CLUTTER_ACTOR (entry));
  clutter_actor_set_parent (priv->entry, CLUTTER_ACTOR (entry));
  clutter_actor_pop_internal (CLUTTER_ACTOR (entry));

  clutter_actor_set_reactive ((ClutterActor *) entry, TRUE);

  /* set cursor hidden until we receive focus */
  clutter_text_set_cursor_visible ((ClutterText *) priv->entry, FALSE);

  g_signal_connect (entry, "style-changed",
                    G_CALLBACK (mx_entry_style_changed), NULL);
}

/**
 * mx_entry_new:
 *
 * Create a new #MxEntry
 *
 * Returns: a new #MxEntry
 */
ClutterActor *
mx_entry_new (void)
{
  return g_object_new (MX_TYPE_ENTRY, NULL);
}


/**
 * mx_entry_new_with_text:
 * @text: text to set the entry to
 *
 * Create a new #MxEntry with the specified entry
 *
 * Returns: a new #MxEntry
 */
ClutterActor *
mx_entry_new_with_text (const gchar *text)
{
  MxWidget *entry;

  entry = g_object_new (MX_TYPE_ENTRY,
                        "text", text,
                        NULL);

  return (ClutterActor*) entry;
}

/**
 * mx_entry_get_text:
 * @entry: a #MxEntry
 *
 * Get the text displayed on the entry
 *
 * Returns: the text for the entry. This must not be freed by the application
 */
const gchar *
mx_entry_get_text (MxEntry *entry)
{
  g_return_val_if_fail (MX_IS_ENTRY (entry), NULL);

  if (entry->priv->hint_visible)
    return "";
  else
    return clutter_text_get_text (CLUTTER_TEXT (entry->priv->entry));
}

/**
 * mx_entry_set_text:
 * @entry: a #MxEntry
 * @text: text to set the entry to
 *
 * Sets the text displayed on the entry
 */
void
mx_entry_set_text (MxEntry     *entry,
                   const gchar *entry_text)
{
  MxEntryPrivate *priv;
  gunichar password_char;
  const gchar *text;

  g_return_if_fail (MX_IS_ENTRY (entry));

  if (entry_text)
    text = entry_text;
  else
    text = "";

  priv = entry->priv;

  /* set a hint if we are blanking the entry */
  if (priv->hint
      && text && !strcmp ("", text)
      && !HAS_FOCUS (priv->entry))
    {
      text = priv->hint;
      priv->hint_visible = TRUE;
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (entry), "indeterminate");
      password_char = 0;
    }
  else
    {
      if (HAS_FOCUS (priv->entry))
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (entry), "focus");
      else
        mx_stylable_set_style_pseudo_class (MX_STYLABLE (entry), NULL);

      priv->hint_visible = FALSE;
      password_char = priv->password_char;
    }

  clutter_text_set_text (CLUTTER_TEXT (priv->entry), text);
  if (clutter_text_get_password_char (CLUTTER_TEXT (priv->entry)) != password_char)
    {
      clutter_text_set_password_char (CLUTTER_TEXT (priv->entry), password_char);
    }
}

/**
 * mx_entry_get_clutter_text:
 * @entry: a #MxEntry
 *
 * Retrieve the internal #ClutterText so that extra parameters can be set
 *
 * Returns: (transfer none): the #ClutterText used by #MxEntry. The entry is
 * owned by the #MxEntry and should not be unref'ed by the application.
 */
ClutterActor*
mx_entry_get_clutter_text (MxEntry *entry)
{
  g_return_val_if_fail (MX_IS_ENTRY (entry), NULL);

  return entry->priv->entry;
}

/**
 * mx_entry_set_hint_text:
 * @entry: a #MxEntry
 * @text: text to set as the entry hint
 *
 * Sets the text to display when the entry is empty and unfocused. When the
 * entry is displaying the hint, it has a pseudo class of "indeterminate".
 * A value of NULL unsets the hint.
 */
void
mx_entry_set_hint_text (MxEntry     *entry,
                        const gchar *text)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  g_free (priv->hint);

  priv->hint = g_strdup (text);

  if (!strcmp (clutter_text_get_text (CLUTTER_TEXT (priv->entry)), "")
      && !HAS_FOCUS (priv->entry))
    {
      priv->hint_visible = TRUE;

      clutter_text_set_text (CLUTTER_TEXT (priv->entry), priv->hint);
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (entry), "indeterminate");
      if (clutter_text_get_password_char (CLUTTER_TEXT (priv->entry)) != 0)
        {
          clutter_text_set_password_char (CLUTTER_TEXT (priv->entry), 0);
        }
    }
}

/**
 * mx_entry_get_hint_text:
 * @entry: a #MxEntry
 *
 * Gets the text that is displayed when the entry is empty and unfocused
 *
 * Returns: the current value of the hint property. This string is owned by the
 * #MxEntry and should not be freed or modified.
 */
const
gchar *
mx_entry_get_hint_text (MxEntry *entry)
{
  g_return_val_if_fail (MX_IS_ENTRY (entry), NULL);

  return entry->priv->hint;
}

/**
 * mx_entry_set_password_char:
 * @entry: a #MxEntry
 * @password_char: text to set as the entry hint
 *
 * Sets the character to display instead of the text. Use 0 to display
 * the actual text.
 */
void
mx_entry_set_password_char (MxEntry  *entry,
                            gunichar  password_char)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  priv->password_char = password_char;

  if (!priv->hint_visible)
    {
      clutter_text_set_password_char (CLUTTER_TEXT (priv->entry),
                                      password_char);
    }
}

/**
 * mx_entry_get_password_char:
 * @entry: a #MxEntry
 *
 * Gets the character to display instead of the text.
 *
 * Return value: a character, or 0 if input should not be hidden.
 */
gunichar
mx_entry_get_password_char (MxEntry *entry)
{
  g_return_val_if_fail (MX_IS_ENTRY (entry), 0);

  return entry->priv->password_char;
}

static gboolean
_mx_entry_icon_press_cb (ClutterActor       *actor,
                         ClutterButtonEvent *event,
                         MxEntry            *entry)
{
  MxEntryPrivate *priv = entry->priv;

  if (actor == priv->primary_icon)
    {
      g_signal_emit (entry, entry_signals[PRIMARY_ICON_CLICKED], 0);
      if (priv->primary_icon_tooltip)
        mx_tooltip_hide (MX_TOOLTIP (priv->primary_icon_tooltip));
    }
  else
    {
      g_signal_emit (entry, entry_signals[SECONDARY_ICON_CLICKED], 0);
      if (priv->secondary_icon_tooltip)
        mx_tooltip_hide (MX_TOOLTIP (priv->secondary_icon_tooltip));
    }

  return FALSE;
}


static gboolean _mx_entry_tooltip_timeout_cb (gpointer data)
{
  if (!CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (data)))
    mx_tooltip_show (MX_TOOLTIP (data));
  else
    mx_tooltip_hide (MX_TOOLTIP (data));

  return FALSE;
}


static guint
_mx_entry_which_icon (ClutterActor *actor, MxEntry *entry)
{
  MxEntryPrivate *priv = entry->priv;

  if (actor == priv->primary_icon)
    return 1;
  else if (actor == priv->secondary_icon)
    return 2;

  return 0;
}

static gchar *
_mx_entry_make_highlight_filename (MxEntry     *entry,
                                   const gchar *original_filename)
{
  MxEntryPrivate *priv = entry->priv;

  gchar *extension, *highlight_icon_path, *filepath;

  extension = g_strrstr (original_filename, ".");
  filepath = g_strndup (original_filename, (strlen (original_filename)
                                            - strlen (extension)));

  highlight_icon_path = g_strconcat (filepath,
                                     priv->icon_highlight_suffix,
                                     extension,
                                     NULL);

  g_free (filepath);

  if (!g_file_test (highlight_icon_path, G_FILE_TEST_EXISTS))
    {
      g_free (highlight_icon_path);
      highlight_icon_path = NULL;
    }

  return highlight_icon_path;
}

static void
_mx_entry_switch_to_highlight_icon (ClutterActor *icon,
                                    ClutterActor *icon_highlight,
                                    gboolean highlighted)
{
  /* Switches round the highlight icon to visible or the normal icon
   * depending on highlighted
   */
  guint icon_opacity;
  guint highlight_icon_opacity;

  if (!icon_highlight)
    return;

  icon_opacity = !highlighted*255;
  highlight_icon_opacity = highlighted*255;

  if (clutter_actor_get_opacity (icon) != icon_opacity)
    clutter_actor_set_opacity (icon, icon_opacity);
  if(clutter_actor_get_opacity (icon_highlight) != highlight_icon_opacity)
    clutter_actor_set_opacity (icon_highlight, highlight_icon_opacity);
}


static gboolean
_mx_entry_icon_motion_event_cb (ClutterActor *actor,
                                ClutterEvent *event,
                                MxEntry      *entry)
{
  MxEntryPrivate *priv = entry->priv;
  MxTooltip *target_tooltip = NULL;
  guint icon_position;

  icon_position = _mx_entry_which_icon (actor, entry);

  if (icon_position  == 1)
    {
      target_tooltip = priv->primary_icon_tooltip;

      _mx_entry_switch_to_highlight_icon (priv->primary_icon,
                                          priv->primary_icon_highlight,
                                          TRUE);
    }
  else if (icon_position == 2)
    {
      target_tooltip = priv->secondary_icon_tooltip;


      _mx_entry_switch_to_highlight_icon (priv->secondary_icon,
                                          priv->secondary_icon_highlight,
                                          TRUE);
    }

  /* Show the tooltip if it's not already visible */

  if (target_tooltip && !CLUTTER_ACTOR_IS_VISIBLE (target_tooltip))
    {
      if (mx_tooltip_is_in_browse_mode ())
        {
          mx_tooltip_show (target_tooltip);

          if (priv->tooltip_timeout)
            {
              g_source_remove (priv->tooltip_timeout);
              priv->tooltip_timeout = 0;
            }
        }
      else
        {
          /* Not in browse mode so add a small delay before showing tooltip */
          if (!priv->tooltip_timeout)
            priv->tooltip_timeout =
              clutter_threads_add_timeout (MX_ENTRY_TOOLTIP_DELAY,
                                           _mx_entry_tooltip_timeout_cb,
                                           target_tooltip);
        }
    }
  return FALSE;
}

static gboolean
_mx_entry_icon_leave_cb (ClutterActor *actor,
                         ClutterEvent *event,
                         MxEntry      *entry)
{
  MxEntryPrivate *priv = entry->priv;
  MxTooltip *target_tooltip = NULL;

  guint icon_position;

  icon_position = _mx_entry_which_icon (actor, entry);

  if (icon_position == 1)
    {
      target_tooltip = priv->primary_icon_tooltip;

      _mx_entry_switch_to_highlight_icon (priv->primary_icon,
                                          priv->primary_icon_highlight,
                                          FALSE);
    }
  else if (icon_position == 2)
    {
      target_tooltip = priv->secondary_icon_tooltip;

      _mx_entry_switch_to_highlight_icon (priv->secondary_icon,
                                          priv->secondary_icon_highlight,
                                          FALSE);
    }

  if (priv->tooltip_timeout)
    {
      g_source_remove (priv->tooltip_timeout);
      priv->tooltip_timeout = 0;
    }

  mx_tooltip_hide (target_tooltip);

  return FALSE;
}


static void
_mx_entry_set_icon_from_file (MxEntry       *entry,
                              ClutterActor **icon,
                              const gchar   *filename)
{
  if (*icon)
    {
      g_signal_handlers_disconnect_by_func (*icon,
                                            _mx_entry_icon_press_cb,
                                            entry);
      g_signal_handlers_disconnect_by_func (*icon,
                                            _mx_entry_icon_motion_event_cb,
                                            entry);
      g_signal_handlers_disconnect_by_func (*icon,
                                            _mx_entry_icon_leave_cb,
                                            entry);
      clutter_actor_unparent (*icon);
      *icon = NULL;
    }

  if (filename)
    {
      MxTextureCache *cache;

      cache = mx_texture_cache_get_default ();



      *icon = (ClutterActor*) mx_texture_cache_get_texture (cache, filename);

      if (!*icon)
          return;

      clutter_actor_set_reactive (*icon, TRUE);

      clutter_actor_push_internal (CLUTTER_ACTOR (entry));
      clutter_actor_set_parent (*icon, CLUTTER_ACTOR (entry));
      clutter_actor_pop_internal (CLUTTER_ACTOR (entry));

      g_signal_connect (*icon, "button-release-event",
                        G_CALLBACK (_mx_entry_icon_press_cb), entry);

      g_signal_connect (*icon, "motion-event",
                        G_CALLBACK (_mx_entry_icon_motion_event_cb),
                        entry);

      g_signal_connect (*icon, "leave-event",
                        G_CALLBACK (_mx_entry_icon_leave_cb),
                        entry);
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (entry));
}


static void
_mx_entry_create_highlight_icon (MxEntry *entry, guint icon_position)
{
  MxEntryPrivate *priv = entry->priv;

  ClutterActor **target = NULL;
  const gchar *filename = NULL;
  gchar *highlight_filename;

  if (!priv->icon_highlight_suffix)
    return;

  if (icon_position == 1)
    {
      target = &priv->primary_icon_highlight;
      filename = priv->primary_icon_filename;
    }
  else if (icon_position == 2)
    {
      target = &priv->secondary_icon_highlight;
      filename = priv->secondary_icon_filename;
    }

  if (!filename)
    return;

  highlight_filename = _mx_entry_make_highlight_filename (entry, filename);

  if (highlight_filename)
    {
      _mx_entry_set_icon_from_file (entry, target, highlight_filename);

      clutter_actor_set_opacity (*target, 0x0);
      g_free (highlight_filename);
    }
}

/**
 * mx_entry_set_primary_icon_from_file:
 * @entry: a #MxEntry
 * @filename: filename of an icon
 *
 * Set the primary icon of the entry to the given filename
 */
void
mx_entry_set_primary_icon_from_file (MxEntry     *entry,
                                     const gchar *filename)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  if (priv->primary_icon_filename)
    g_free (priv->primary_icon_filename);

  priv->primary_icon_filename = g_strdup (filename);

  _mx_entry_set_icon_from_file (entry, &priv->primary_icon, filename);
  _mx_entry_create_highlight_icon (entry, 1);
}

/**
 * mx_entry_set_secondary_icon_from_file:
 * @entry: a #MxEntry
 * @filename: filename of an icon
 *
 * Set the primary icon of the entry to the given filename
 */
void
mx_entry_set_secondary_icon_from_file (MxEntry     *entry,
                                       const gchar *filename)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  if (priv->secondary_icon_filename)
    g_free (priv->secondary_icon_filename);

  priv->secondary_icon_filename = g_strdup (filename);

  _mx_entry_set_icon_from_file (entry, &priv->secondary_icon, filename);
  _mx_entry_create_highlight_icon (entry, 2);
}

/**
 * mx_entry_set_secondary_icon_tooltip:
 * @entry: a #MxEntry
 * @text: the secondary icon tooltip
 *
 * Set the secondary icon tooltip text
*/
void
mx_entry_set_secondary_icon_tooltip_text (MxEntry     *entry,
                                          const gchar *text)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  if (!priv->secondary_icon_tooltip)
    {
      priv->secondary_icon_tooltip = g_object_new (MX_TYPE_TOOLTIP,
                                                  "text", text,
                                                  NULL);

      mx_tooltip_set_text (priv->secondary_icon_tooltip, text);
      clutter_actor_set_parent (CLUTTER_ACTOR (priv->secondary_icon_tooltip),
                                CLUTTER_ACTOR (entry));
    }
  else
    {
      mx_tooltip_set_text (priv->secondary_icon_tooltip, text);
    }
}

/**
 * mx_entry_set_primary_icon_tooltip:
 * @entry: a #MxEntry
 * @text: the primary icon tooltip
 *
 * Set the primary icon tooltip text
*/
void
mx_entry_set_primary_icon_tooltip_text (MxEntry *entry,
                                          const gchar *text)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  if (!priv->primary_icon_tooltip)
    {
      priv->primary_icon_tooltip = g_object_new (MX_TYPE_TOOLTIP,
                                                  "text", text,
                                                  NULL);

      mx_tooltip_set_text (priv->primary_icon_tooltip, text);
      clutter_actor_set_parent (CLUTTER_ACTOR (priv->primary_icon_tooltip),
                                CLUTTER_ACTOR (entry));


    }
  else
    {
      mx_tooltip_set_text (priv->primary_icon_tooltip, text);
    }
}

/**
 * mx_entry_get_icon_highlight_suffix:
 * @entry: a #MxEntry
 *
 * Get the suffix appended to the filename to use for the highlighted version
 * of the icon.
 *
 * Returns: the highlight filename suffix. This string is owned by the
 * #MxEntry and should not be freed or modified.
*/
const gchar *mx_entry_get_icon_highlight_suffix (MxEntry *entry)
{
  MxEntryPrivate *priv;

  g_return_val_if_fail (MX_IS_ENTRY (entry), NULL);

  priv = entry->priv;

  return priv->icon_highlight_suffix;
}

/**
 * mx_entry_set_icon_highlight_suffix:
 * @entry: a #MxEntry
 * @suffix: the suffix to append to the filename for the highlight version
 *
 * Sets the suffix appended to the filename to use for the highlighted version
 * of the icon. e.g. if you have set your primay icon to "primary-icon.png" 
 * and the suffix to "-highlight" #MxEntry will look for
 * "primary-icon-highlight.png"
*/
void
mx_entry_set_icon_highlight_suffix (MxEntry *entry, const gchar* suffix)
{
  MxEntryPrivate *priv;

  g_return_if_fail (MX_IS_ENTRY (entry));

  priv = entry->priv;

  if (g_strcmp0 (priv->icon_highlight_suffix, suffix) == 0)
      return;

  if (priv->icon_highlight_suffix)
    g_free (priv->icon_highlight_suffix);

  priv->icon_highlight_suffix = g_strdup (suffix);

  _mx_entry_create_highlight_icon (entry, 1);
  _mx_entry_create_highlight_icon (entry, 2);
}
