/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-label.c: Plain label actor
 *
 * Copyright 2008,2009 Intel Corporation
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

/**
 * SECTION:mx-label
 * @short_description: Widget for displaying text
 *
 * #MxLabel is a simple widget for displaying one or more lines of text.
 * It derives from #MxWidget to add extra style and placement functionality over
 * #ClutterText. The internal #ClutterText is publicly accessibly to allow
 * applications to set further properties.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include "mx-label.h"

#include "mx-widget.h"
#include "mx-stylable.h"
#include "mx-private.h"
#include "mx-fade-effect.h"

enum
{
  PROP_0,

  PROP_CLUTTER_TEXT,
  PROP_TEXT,
  PROP_USE_MARKUP,
  PROP_X_ALIGN,
  PROP_Y_ALIGN,
  PROP_LINE_WRAP,
  PROP_FADE_OUT,
  PROP_SHOW_TOOLTIP
};

#define MX_LABEL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_LABEL, MxLabelPrivate))

struct _MxLabelPrivate
{
  ClutterActor  *label;
  ClutterEffect *fade_effect;

  MxAlign x_align;
  MxAlign y_align;

  ClutterTimeline *fade_timeline;
  ClutterAlpha    *fade_alpha;

  gint em_width;

  guint fade_out           : 1;
  guint label_should_fade  : 1;
  guint show_tooltip       : 1;
};

G_DEFINE_TYPE (MxLabel, mx_label, MX_TYPE_WIDGET);

static void
mx_label_set_property (GObject      *gobject,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  MxLabel *label = MX_LABEL (gobject);

  switch (prop_id)
    {
    case PROP_TEXT:
      mx_label_set_text (label, g_value_get_string (value));
      break;

    case PROP_USE_MARKUP:
      mx_label_set_use_markup (label, g_value_get_boolean (value));
      break;

    case PROP_Y_ALIGN:
      mx_label_set_y_align (label, g_value_get_enum (value));
      break;

    case PROP_X_ALIGN:
      mx_label_set_x_align (label, g_value_get_enum (value));
      break;

    case PROP_LINE_WRAP:
      mx_label_set_line_wrap (label, g_value_get_boolean (value));
      break;

    case PROP_FADE_OUT:
      mx_label_set_fade_out (label, g_value_get_boolean (value));
      break;

    case PROP_SHOW_TOOLTIP:
      mx_label_set_show_tooltip (label, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_label_get_property (GObject    *gobject,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  MxLabel *self = MX_LABEL (gobject);
  MxLabelPrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_CLUTTER_TEXT:
      g_value_set_object (value, priv->label);
      break;

    case PROP_TEXT:
      g_value_set_string (value,
                          clutter_text_get_text (CLUTTER_TEXT (priv->label)));
      break;

    case PROP_USE_MARKUP:
      g_value_set_boolean (value, mx_label_get_use_markup (self));
      break;

    case PROP_X_ALIGN:
      g_value_set_enum (value, priv->x_align);
      break;

    case PROP_Y_ALIGN:
      g_value_set_enum (value, priv->y_align);
      break;

    case PROP_LINE_WRAP:
      g_value_set_boolean (value, mx_label_get_line_wrap (self));
      break;

    case PROP_FADE_OUT:
      g_value_set_boolean (value, priv->fade_out);
      break;

    case PROP_SHOW_TOOLTIP:
      g_value_set_boolean (value, priv->show_tooltip);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_label_style_changed (MxWidget            *self,
                        MxStyleChangedFlags  flags)
{
  MxLabelPrivate *priv = MX_LABEL (self)->priv;

  mx_stylable_apply_clutter_text_attributes (MX_STYLABLE (self),
                                             CLUTTER_TEXT (priv->label));
}

static void
mx_label_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_height -= padding.top + padding.bottom;

  clutter_actor_get_preferred_width (priv->label, for_height,
                                     min_width_p,
                                     natural_width_p);

  /* If we're fading out, make sure our minimum width is zero */
  if (priv->fade_out && min_width_p)
    *min_width_p = 0;

  if (min_width_p)
    *min_width_p += padding.left + padding.right;

  if (natural_width_p)
    *natural_width_p += padding.left + padding.right;
}

static void
mx_label_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  MxPadding padding = { 0, };

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  for_width -= padding.left + padding.right;

  clutter_actor_get_preferred_height (priv->label, for_width,
                                      min_height_p,
                                      natural_height_p);

  if (min_height_p)
    *min_height_p += padding.top + padding.bottom;

  if (natural_height_p)
    *natural_height_p += padding.top + padding.bottom;
}

static void
mx_label_allocate (ClutterActor          *actor,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  gboolean label_did_fade = priv->label_should_fade;

  ClutterActorClass *parent_class;
  ClutterActorBox child_box;
  gboolean x_fill, y_fill;
  gfloat avail_width;

  parent_class = CLUTTER_ACTOR_CLASS (mx_label_parent_class);
  parent_class->allocate (actor, box, flags);

  mx_widget_get_available_area (MX_WIDGET (actor), box, &child_box);
  avail_width = child_box.x2 - child_box.x1;

  /* The default behaviour of ClutterText is to align to the
   * top-left when it gets more space than is needed. Because
   * of this behaviour, if we're aligning to the left, we can
   * assign all our horizontal space to the label without
   * measuring it (i.e. x-fill), and the same applies for
   * aligning to the top and vertical space.
   */
  x_fill = (priv->x_align == MX_ALIGN_START) ? TRUE : FALSE;
  y_fill = (priv->y_align == MX_ALIGN_START) ? TRUE : FALSE;

  mx_allocate_align_fill (priv->label, &child_box, priv->x_align,
                          priv->y_align, x_fill, y_fill);

  priv->label_should_fade = FALSE;

  if (priv->fade_out)
    {
      /* If we're fading out, make sure the label has its full width
       * allocated. This ensures that the offscreen effect has the full
       * label inside its texture.
       */
      gfloat label_width;

      clutter_actor_get_preferred_width (priv->label, -1, NULL, &label_width);

      if (label_width > avail_width)
        {
          priv->label_should_fade = TRUE;
          child_box.x2 = child_box.x1 + label_width;
        }

      mx_fade_effect_set_bounds (MX_FADE_EFFECT (priv->fade_effect),
                                 0, 0, MIN (label_width, avail_width), 0);
    }

  /* Allocate the label */
  clutter_actor_allocate (priv->label, &child_box, flags);

  if (priv->show_tooltip)
    {
      PangoLayout *layout;
      const gchar *text;

      layout = clutter_text_get_layout (CLUTTER_TEXT (priv->label));

      if (pango_layout_is_ellipsized (layout))
        text = clutter_text_get_text (CLUTTER_TEXT (priv->label));
      else
        text = NULL;

      mx_widget_set_tooltip_text (MX_WIDGET (actor), text);
    }

  /* Animate in/out the faded end of the label */
  if (label_did_fade != priv->label_should_fade)
    {
      /* Begin/reverse the fading timeline when necessary */
      if (priv->label_should_fade)
        clutter_timeline_set_direction (priv->fade_timeline,
                                        CLUTTER_TIMELINE_FORWARD);
      else
        clutter_timeline_set_direction (priv->fade_timeline,
                                        CLUTTER_TIMELINE_BACKWARD);

      if (!clutter_timeline_is_playing (priv->fade_timeline))
        clutter_timeline_rewind (priv->fade_timeline);

      clutter_timeline_start (priv->fade_timeline);
    }
}

static void
mx_label_paint (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  ClutterActorClass *parent_class;

  parent_class = CLUTTER_ACTOR_CLASS (mx_label_parent_class);
  parent_class->paint (actor);

  clutter_actor_paint (priv->label);
  _mx_fade_effect_set_freeze_update (MX_FADE_EFFECT (priv->fade_effect), TRUE);
}

static void
mx_label_pick (ClutterActor *actor,
               const ClutterColor *pick_color)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;
  ClutterActorClass *parent_class;

  parent_class = CLUTTER_ACTOR_CLASS (mx_label_parent_class);
  parent_class->pick (actor, pick_color);

  clutter_actor_paint (priv->label);
}

static void
mx_label_map (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_label_parent_class)->map (actor);

  clutter_actor_map (priv->label);
}

static void
mx_label_unmap (ClutterActor *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;

  if (priv->label)
    clutter_actor_unmap (priv->label);

  CLUTTER_ACTOR_CLASS (mx_label_parent_class)->unmap (actor);
}

static void
mx_label_dispose (GObject *actor)
{
  MxLabelPrivate *priv = MX_LABEL (actor)->priv;

  if (priv->fade_timeline)
    {
      clutter_timeline_stop (priv->fade_timeline);
      g_object_unref (priv->fade_timeline);
      priv->fade_timeline = NULL;
    }

  if (priv->fade_alpha)
    {
      g_object_unref (priv->fade_alpha);
      priv->fade_alpha = NULL;
    }

  if (priv->label)
    {
      clutter_actor_destroy (priv->label);
      priv->label = NULL;
    }

  G_OBJECT_CLASS (mx_label_parent_class)->dispose (actor);
}

static void
mx_label_class_init (MxLabelClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxLabelPrivate));

  gobject_class->set_property = mx_label_set_property;
  gobject_class->get_property = mx_label_get_property;
  gobject_class->dispose = mx_label_dispose;

  actor_class->paint = mx_label_paint;
  actor_class->pick = mx_label_pick;
  actor_class->allocate = mx_label_allocate;
  actor_class->get_preferred_width = mx_label_get_preferred_width;
  actor_class->get_preferred_height = mx_label_get_preferred_height;
  actor_class->map = mx_label_map;
  actor_class->unmap = mx_label_unmap;

  pspec = g_param_spec_object ("clutter-text",
                               "Clutter Text",
                               "Internal ClutterText actor",
                               CLUTTER_TYPE_TEXT,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class, PROP_CLUTTER_TEXT, pspec);

  pspec = g_param_spec_string ("text",
                               "Text",
                               "Text of the label",
                               NULL,
                               MX_PARAM_READWRITE | MX_PARAM_TRANSLATEABLE);
  g_object_class_install_property (gobject_class, PROP_TEXT, pspec);

  pspec = g_param_spec_boolean ("use-markup",
                                "Use markup",
                                "Whether the text of the label should be "
                                "treated as Pango markup",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_USE_MARKUP, pspec);

  pspec = g_param_spec_enum ("x-align",
                             "X Align",
                             "Horizontal position of the text layout",
                             MX_TYPE_ALIGN, MX_ALIGN_START,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_X_ALIGN, pspec);

  pspec = g_param_spec_enum ("y-align",
                             "Y Align",
                             "Vertical position of the text layout",
                             MX_TYPE_ALIGN, MX_ALIGN_START,
                             MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_Y_ALIGN, pspec);

  /**
   * MxLabel:line-wrap:
   *
   * Whether to wrap the lines of #MxLabel:text if the contents
   * exceed the available allocation.
   *
   * Since: 1.2
   */
  pspec = g_param_spec_boolean ("line-wrap",
                                "Line wrap",
                                "If set, wrap the lines if the text becomes too wide",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LINE_WRAP, pspec);

  pspec = g_param_spec_boolean ("fade-out",
                                "Fade out",
                                "Fade out the end of the label, instead "
                                "of ellipsizing",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_FADE_OUT, pspec);

  /**
   * MxLabel:show-tooltip:
   *
   * Show a tooltip when there is not enough space to display the text. If set
   * to %TRUE, this will also cause the #ClutterActor:reactive property to be
   * enabled.
   *
   * Since: 1.4
   */
  pspec = g_param_spec_boolean ("show-tooltip",
                                "Show Tooltip",
                                "Show a tooltip when there is not enough space"
                                " to display the text.",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_SHOW_TOOLTIP, pspec);
}

static void
mx_label_single_line_mode_cb (ClutterText *text,
                              GParamSpec  *pspec,
                              MxLabel     *self)
{
  MxLabelPrivate *priv = self->priv;
  if (!clutter_text_get_single_line_mode (text) && priv->fade_out)
    mx_label_set_fade_out (self, FALSE);
}

static void
mx_label_label_changed_cb (MxLabel *label)
{
  MxLabelPrivate *priv = label->priv;

  /* Enable updating of the off-screen texture */
  _mx_fade_effect_set_freeze_update (MX_FADE_EFFECT (priv->fade_effect), FALSE);
}

static void
mx_label_font_description_cb (ClutterText *text,
                              GParamSpec  *pspec,
                              MxLabel     *self)
{
  PangoFontDescription *font;

  MxLabelPrivate *priv = self->priv;

  /* Find out the em-width - code pretty much copied from Clutter,
   * clutter-backend.c, get_units_per_em ()
   */
  font = clutter_text_get_font_description (text);
  if (font)
    {
      gint i_dpi;
      gdouble dpi;

      gdouble font_size = 0;
      gint pango_size = pango_font_description_get_size (font);
      ClutterSettings *settings = clutter_settings_get_default ();

      g_object_get (G_OBJECT (settings), "font-dpi", &i_dpi, NULL);
      dpi = i_dpi / 1024.0;

      if (pango_font_description_get_size_is_absolute (font))
        font_size = pango_size / PANGO_SCALE;
      else
        font_size = pango_size / PANGO_SCALE * dpi / 96.f;

      priv->em_width = (1.2f * font_size) * dpi / 96.f;

      mx_fade_effect_set_border (MX_FADE_EFFECT (priv->fade_effect),
                                 0, priv->em_width * 5, 0, 0);
    }
}

static void
mx_label_fade_new_frame_cb (ClutterTimeline *timeline,
                            gint             msecs,
                            MxLabel         *self)
{
  guint8 a;
  ClutterColor color;

  MxLabelPrivate *priv = self->priv;

  a = (1.0 - clutter_alpha_get_alpha (priv->fade_alpha)) * 255;

  color.red = a;
  color.green = a;
  color.blue = a;
  color.alpha = a;

  mx_fade_effect_set_color (MX_FADE_EFFECT (priv->fade_effect), &color);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));
}

static void
mx_label_fade_started_cb (ClutterTimeline *timeline,
                          MxLabel         *label)
{
  clutter_actor_meta_set_enabled (CLUTTER_ACTOR_META (label->priv->fade_effect),
                                  TRUE);
}

static void
mx_label_fade_completed_cb (ClutterTimeline *timeline,
                            MxLabel         *label)
{
  MxLabelPrivate *priv = label->priv;

  if (!priv->label_should_fade)
    clutter_actor_meta_set_enabled (CLUTTER_ACTOR_META (priv->fade_effect),
                                    FALSE);
}

static void
mx_label_init (MxLabel *label)
{
  MxLabelPrivate *priv;
  const ClutterColor opaque = { 0xff, 0xff, 0xff, 0xff };

  label->priv = priv = MX_LABEL_GET_PRIVATE (label);

  priv->label = g_object_new (CLUTTER_TYPE_TEXT,
                              "ellipsize", PANGO_ELLIPSIZE_END,
                              NULL);

  clutter_actor_set_parent (priv->label, CLUTTER_ACTOR (label));

  priv->fade_effect = mx_fade_effect_new ();
  mx_fade_effect_set_color (MX_FADE_EFFECT (priv->fade_effect), &opaque);
  clutter_actor_add_effect (priv->label, priv->fade_effect);
  clutter_actor_meta_set_enabled (CLUTTER_ACTOR_META (priv->fade_effect),
                                  FALSE);

  g_signal_connect (label, "style-changed",
                    G_CALLBACK (mx_label_style_changed), NULL);
  g_signal_connect (priv->label, "notify::single-line-mode",
                    G_CALLBACK (mx_label_single_line_mode_cb), label);
  g_signal_connect_swapped (priv->label, "queue-redraw",
                            G_CALLBACK (mx_label_label_changed_cb), label);

  priv->fade_timeline = clutter_timeline_new (250);
  priv->fade_alpha = clutter_alpha_new_full (priv->fade_timeline,
                                             CLUTTER_EASE_OUT_QUAD);
  g_signal_connect (priv->fade_timeline, "new-frame",
                    G_CALLBACK (mx_label_fade_new_frame_cb), label);
  g_signal_connect (priv->fade_timeline, "started",
                    G_CALLBACK (mx_label_fade_started_cb), label);
  g_signal_connect (priv->fade_timeline, "completed",
                    G_CALLBACK (mx_label_fade_completed_cb), label);
}

/**
 * mx_label_new:
 *
 * Create a new #MxLabel
 *
 * Returns: a new #MxLabel
 */
ClutterActor *
mx_label_new (void)
{
  return g_object_new (MX_TYPE_LABEL, NULL);
}

/**
 * mx_label_new_with_text:
 * @text: text to set the label to
 *
 * Create a new #MxLabel with the specified label
 *
 * Returns: a new #MxLabel
 */
ClutterActor *
mx_label_new_with_text (const gchar *text)
{
  if (text == NULL || *text == '\0')
    return g_object_new (MX_TYPE_LABEL, NULL);
  else
    return g_object_new (MX_TYPE_LABEL,
                         "text", text,
                         NULL);
}

/**
 * mx_label_get_text:
 * @label: a #MxLabel
 *
 * Get the text displayed on the label
 *
 * Returns: the text for the label. This must not be freed by the application
 */
const gchar *
mx_label_get_text (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), NULL);

  return clutter_text_get_text (CLUTTER_TEXT (label->priv->label));
}

/**
 * mx_label_set_text:
 * @label: a #MxLabel
 * @text: text to set the label to
 *
 * Sets the text displayed on the label
 */
void
mx_label_set_text (MxLabel     *label,
                   const gchar *text)
{
  MxLabelPrivate *priv;

  g_return_if_fail (MX_IS_LABEL (label));
  g_return_if_fail (text != NULL);

  priv = label->priv;

  if (clutter_text_get_use_markup (CLUTTER_TEXT (priv->label)))
    clutter_text_set_markup (CLUTTER_TEXT (priv->label), text);
  else
    clutter_text_set_text (CLUTTER_TEXT (priv->label), text);

  g_object_notify (G_OBJECT (label), "text");
}

/**
 * mx_label_get_use_markup:
 * @label: a #MxLabel
 *
 * Determines whether the text of the label is being treated as Pango markup.
 *
 * Returns: %TRUE if the text of the label is treated as Pango markup,
 *   %FALSE otherwise.
 *
 * Since: 1.2
 */
gboolean
mx_label_get_use_markup (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), FALSE);
  return clutter_text_get_use_markup (CLUTTER_TEXT (label->priv->label));
}

/**
 * mx_label_set_use_markup:
 * @label: a #MxLabel
 * @use_markup: %TRUE to use Pango markup, %FALSE otherwise
 *
 * Sets whether the text of the label should be treated as Pango markup.
 */
void
mx_label_set_use_markup (MxLabel  *label,
                         gboolean  use_markup)
{
  MxLabelPrivate *priv;

  g_return_if_fail (MX_IS_LABEL (label));

  priv = label->priv;

  clutter_text_set_use_markup (CLUTTER_TEXT (priv->label), use_markup);

  g_object_notify (G_OBJECT (label), "use-markup");
}

/**
 * mx_label_get_clutter_text:
 * @label: a #MxLabel
 *
 * Retrieve the internal #ClutterText so that extra parameters can be set
 *
 * Returns: (transfer none): the #ClutterText used by #MxLabel. The label
 * is owned by the #MxLabel and should not be unref'ed by the application.
 */
ClutterActor*
mx_label_get_clutter_text (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), NULL);

  return label->priv->label;
}

void
mx_label_set_x_align (MxLabel *label,
                      MxAlign  align)
{
  g_return_if_fail (MX_IS_LABEL (label));


  if (align != label->priv->x_align)
    {
      label->priv->x_align = align;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (label));

      g_object_notify (G_OBJECT (label), "x-align");
    }
}

MxAlign
mx_label_get_x_align (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), 0);

  return label->priv->x_align;
}
void
mx_label_set_y_align (MxLabel *label,
                      MxAlign  align)
{
  g_return_if_fail (MX_IS_LABEL (label));


  if (align != label->priv->y_align)
    {
      label->priv->y_align = align;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (label));

      g_object_notify (G_OBJECT (label), "y-align");
    }
}

MxAlign
mx_label_get_y_align (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), 0);

  return label->priv->y_align;
}

/**
 * mx_label_set_line_wrap:
 * @label: An #MxLabel
 * @line_wrap: new value of the line-wrap property.
 *
 * Set the value of the #MxLabel:line-wrap property.
 *
 * Since: 1.2
 */
void
mx_label_set_line_wrap (MxLabel  *label,
                        gboolean  line_wrap)
{
  g_return_if_fail (MX_IS_LABEL (label));

  clutter_text_set_line_wrap (CLUTTER_TEXT (label->priv->label), line_wrap);
  g_object_notify (G_OBJECT (label), "line-wrap");
}

/**
 * mx_label_get_line_wrap:
 * @label: An #MxLabel
 *
 * Get the value of the #MxLabel:line-wrap property.
 *
 * Returns: %TRUE if the "line-wrap" property is set.
 *
 * Since: 1.2
 */
gboolean
mx_label_get_line_wrap (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), FALSE);

  return clutter_text_get_line_wrap (CLUTTER_TEXT (label->priv->label));
}

/**
 * mx_label_set_fade_out:
 * @label: A #MxLabel
 * @fade: %TRUE to fade out, %FALSE otherwise
 *
 * Set whether to fade out the end of the label, instead of ellipsizing.
 * Enabling this mode will also set the #ClutterText:single-line-mode and
 * #ClutterText:ellipsize properties.
 *
 * Since: 1.2
 */
void
mx_label_set_fade_out (MxLabel  *label,
                       gboolean  fade)
{
  MxLabelPrivate *priv;

  g_return_if_fail (MX_IS_LABEL (label));

  priv = label->priv;
  if (priv->fade_out != fade)
    {
      priv->fade_out = fade;
      g_object_notify (G_OBJECT (label), "fade-out");

      /* Enable the fade-effect */
      if (fade)
        {
          priv->label_should_fade = FALSE;
          clutter_text_set_single_line_mode (CLUTTER_TEXT (priv->label), TRUE);
          clutter_text_set_ellipsize (CLUTTER_TEXT (priv->label),
                                      PANGO_ELLIPSIZE_NONE);
        }

      /* If we need to fade, listen for the font-description changing so
       * we can keep track of the em-width of the label.
       */
      if (fade)
        {
          g_signal_connect (priv->label, "notify::font-description",
                            G_CALLBACK (mx_label_font_description_cb), label);
          mx_label_font_description_cb (CLUTTER_TEXT (priv->label),
                                        NULL, label);
        }
      else
        {
          g_signal_handlers_disconnect_by_func (priv->label,
                                                mx_label_font_description_cb,
                                                label);
        }
    }
}

/**
 * mx_label_get_fade_out:
 * @label: A #MxLabel
 *
 * Determines whether the label has been set to fade out when there isn't
 * enough space allocated to display the entire label.
 *
 * Returns: %TRUE if the label is set to fade out, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_label_get_fade_out (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), FALSE);
  return label->priv->fade_out;
}

/**
 * mx_label_set_show_tooltip:
 * @label: A #MxLabel
 * @show_tooltip: %TRUE if the tooltip should be shown
 *
 * Set the value of the #MxLabel:show-tooltip property
 *
 * Since: 1.4
 */
void
mx_label_set_show_tooltip (MxLabel *label,
                           gboolean show_tooltip)
{
  MxLabelPrivate *priv;

  g_return_if_fail (MX_IS_LABEL (label));

  priv = label->priv;

  if (priv->show_tooltip != show_tooltip)
    {
      priv->show_tooltip = show_tooltip;

      clutter_actor_queue_relayout (CLUTTER_ACTOR (label));

      g_object_notify (G_OBJECT (label), "show-tooltip");
    }
}

/**
 * mx_label_get_show_tooltip:
 * @label: A #MxLabel
 *
 * Returns the current value of the #MxLabel:show-tooltip property.
 *
 * Returns: %TRUE if the #MxLabel:show-tooltip property is enabled
 *
 * Since: 1.4
 */
gboolean
mx_label_get_show_tooltip (MxLabel *label)
{
  g_return_val_if_fail (MX_IS_LABEL (label), FALSE);

  return label->priv->show_tooltip;
}
