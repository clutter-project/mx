/*
 * mx-spinner.c: a loading indicator widget
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:mx-spinner
 * @short_description: a processing indicator widget
 *
 * The #MxSpinner is a widget to use to indicate that something is being
 * processed, usually a task of indeterminate length.
 */

#include "mx-spinner.h"
#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-stylable.h"

static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxSpinner, mx_spinner, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

#define SPINNER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_SPINNER, MxSpinnerPrivate))

enum
{
  LOOPED,

  LAST_SIGNAL
};

enum
{
  PROP_0,

  PROP_ANIMATING
};

struct _MxSpinnerPrivate
{
  CoglHandle  texture;
  CoglHandle  material;
  guint       frames;
  guint       anim_duration;

  guint       current_frame;
  guint       update_id;

  guint       animating : 1;
};

static guint signals[LAST_SIGNAL] = { 0, };


static void
mx_spinner_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  MxSpinner *spinner = MX_SPINNER (object);

  switch (property_id)
    {
    case PROP_ANIMATING:
      g_value_set_boolean (value, mx_spinner_get_animating (spinner));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_spinner_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  MxSpinner *spinner = MX_SPINNER (object);

  switch (property_id)
    {
    case PROP_ANIMATING:
      mx_spinner_set_animating (spinner, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_spinner_dispose (GObject *object)
{
  MxSpinnerPrivate *priv = MX_SPINNER (object)->priv;

  if (priv->update_id)
    {
      g_source_remove (priv->update_id);
      priv->update_id = 0;
    }

  if (priv->material)
    {
      cogl_handle_unref (priv->material);
      priv->material = COGL_INVALID_HANDLE;
    }

  G_OBJECT_CLASS (mx_spinner_parent_class)->dispose (object);
}

static void
mx_spinner_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_spinner_parent_class)->finalize (object);
}

static void
mx_spinner_get_preferred_width (ClutterActor *actor,
                                gfloat        for_height,
                                gfloat       *min_width_p,
                                gfloat       *nat_width_p)
{
  guint min_width, width, height;
  MxPadding padding;

  MxSpinnerPrivate *priv = MX_SPINNER (actor)->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (priv->material != COGL_INVALID_HANDLE)
    {
      width = cogl_texture_get_width (priv->texture) / priv->frames;
      height = cogl_texture_get_height (priv->texture);
      min_width = width;

      if (for_height >= 0 && for_height < height)
        {
          for_height = MAX (0, for_height - padding.top - padding.bottom);

          width = (guint)((gfloat)width * (for_height / (gfloat)height));
        }
    }
  else
    min_width = width = 0;

  width += padding.left + padding.right;

  if (min_width_p)
    *min_width_p = MIN (min_width, width);
  if (nat_width_p)
    *nat_width_p = width;
}

static void
mx_spinner_get_preferred_height (ClutterActor *actor,
                                 gfloat        for_width,
                                 gfloat       *min_height_p,
                                 gfloat       *nat_height_p)
{
  guint min_height, height, width;
  MxPadding padding;

  MxSpinnerPrivate *priv = MX_SPINNER (actor)->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  if (priv->material != COGL_INVALID_HANDLE)
    {
      height = cogl_texture_get_height (priv->texture);
      width = cogl_texture_get_width (priv->texture);
      min_height = height;

      if (for_width >= 0 && for_width < width)
        {
          width = cogl_texture_get_width (priv->texture) / priv->frames;
          for_width = MAX (0, for_width - padding.left - padding.right);

          height = (guint)((gfloat)height * (for_width / (gfloat)width));
        }
    }
  else
    min_height = height = 0;

  height += padding.top + padding.bottom;

  if (min_height_p)
    *min_height_p = MIN (min_height, height);
  if (nat_height_p)
    *nat_height_p = height;
}

static void
mx_spinner_paint (ClutterActor *actor)
{
  guint8 opacity;
  MxPadding padding;
  gfloat width, height;
  MxSpinnerPrivate *priv = MX_SPINNER (actor)->priv;

  /* Chain up for background */
  CLUTTER_ACTOR_CLASS (mx_spinner_parent_class)->paint (actor);

  if (priv->material == COGL_INVALID_HANDLE)
    return;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  clutter_actor_get_size (actor, &width, &height);
  opacity = clutter_actor_get_paint_opacity (actor);

  cogl_material_set_color4ub (priv->material,
                              opacity, opacity, opacity, opacity);
  cogl_set_source (priv->material);
  cogl_rectangle_with_texture_coords (padding.left,
                                      padding.top,
                                      width - padding.right,
                                      height - padding.bottom,
                                      priv->current_frame /
                                      (gfloat)priv->frames,
                                      0,
                                      (priv->current_frame + 1) /
                                      (gfloat)priv->frames,
                                      1);
}

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_boxed ("x-mx-spinner-image",
                                  "Spinner image",
                                  "Image containing the frames to use for "
                                  "the spinner animation",
                                  MX_TYPE_BORDER_IMAGE,
                                  MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_SPINNER, pspec);

      pspec = g_param_spec_uint ("x-mx-spinner-frames",
                                 "Spinner frames",
                                 "Number of frames contained in the spinner "
                                 "image, horizontally.",
                                  1, G_MAXUINT, 1,
                                  MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_SPINNER, pspec);

      pspec = g_param_spec_uint ("x-mx-spinner-animation-duration",
                                 "Spinner animation duration",
                                 "Duration of the entire spinner animation, "
                                 "in milliseconds.",
                                  1, G_MAXUINT, 500,
                                  MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_SPINNER, pspec);
    }
}

static void
mx_spinner_class_init (MxSpinnerClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxSpinnerPrivate));

  object_class->get_property = mx_spinner_get_property;
  object_class->set_property = mx_spinner_set_property;
  object_class->dispose = mx_spinner_dispose;
  object_class->finalize = mx_spinner_finalize;

  actor_class->get_preferred_width = mx_spinner_get_preferred_width;
  actor_class->get_preferred_height = mx_spinner_get_preferred_height;
  actor_class->paint = mx_spinner_paint;

  pspec = g_param_spec_boolean ("animating",
                                "Animating",
                                "Whether the spinner is animating.",
                                TRUE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ANIMATING, pspec);

  /**
   * MxSpinner::looped:
   * @spinner: the #MxSpinner that received the signal
   *
   * Emitted after the animation has displayed the final frame.
   *
   * Since: 1.2
   */
  signals[LOOPED] =
    g_signal_new ("looped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxSpinnerClass, looped),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static gboolean
mx_spinner_timeout_cb (MxSpinner *spinner)
{
  MxSpinnerPrivate *priv = spinner->priv;

  /* We may be destroyed during the signal emission, so
   * queue the redraw here instead of below.
   */
  clutter_actor_queue_redraw (CLUTTER_ACTOR (spinner));

  if (++priv->current_frame == priv->frames)
    {
      priv->current_frame = 0;
      g_signal_emit (spinner, signals[LOOPED], 0);
    }

  return TRUE;
}

static void
mx_spinner_update_timeout (MxSpinner *spinner)
{
  MxSpinnerPrivate *priv = spinner->priv;

  if (priv->update_id)
    {
      g_source_remove (priv->update_id);
      priv->update_id = 0;
    }

  if (priv->animating && priv->frames && priv->material)
    priv->update_id = clutter_threads_add_timeout_full (CLUTTER_PRIORITY_REDRAW,
                                                        MAX (1, priv->anim_duration /
                                                             priv->frames),
                                                        (GSourceFunc)
                                                        mx_spinner_timeout_cb,
                                                        spinner,
                                                        NULL);
  else
    priv->current_frame = 0;
}

static void
mx_spinner_style_changed_cb (MxStylable          *stylable,
                             MxStyleChangedFlags  flags)
{
  MxBorderImage *image;
  guint frames, anim_duration;

  MxSpinner *spinner = MX_SPINNER (stylable);
  MxSpinnerPrivate *priv = spinner->priv;

  mx_stylable_get (stylable,
                   "x-mx-spinner-image", &image,
                   "x-mx-spinner-frames", &frames,
                   "x-mx-spinner-animation-duration", &anim_duration,
                   NULL);

  if (priv->material)
    {
      cogl_handle_unref (priv->material);
      priv->material = NULL;
    }

  priv->anim_duration = anim_duration;
  priv->frames = frames;
  priv->current_frame = 0;

  if (image)
    {
      MxTextureCache *cache = mx_texture_cache_get_default ();
      priv->texture = mx_texture_cache_get_cogl_texture (cache, image->uri);
      g_boxed_free (MX_TYPE_BORDER_IMAGE, image);

      priv->material = cogl_material_new ();
      cogl_material_set_layer (priv->material, 0, priv->texture);
      cogl_handle_unref (priv->texture);
    }

  mx_spinner_update_timeout (spinner);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (stylable));
}

static void
mx_spinner_init (MxSpinner *self)
{
  MxSpinnerPrivate *priv = self->priv = SPINNER_PRIVATE (self);

  priv->anim_duration = 500;
  priv->frames = 1;
  priv->animating = TRUE;

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_spinner_style_changed_cb), NULL);
}

/**
 * mx_spinner_new:
 *
 * Create a new #MxSpinner widget.
 *
 * Returns: a newly allocated #MxSpinner
 *
 * Since: 1.2
 */
ClutterActor *
mx_spinner_new (void)
{
  return g_object_new (MX_TYPE_SPINNER, NULL);
}

/**
 * mx_spinner_get_animating:
 * @spinner: A #MxSpinner widget
 *
 * Determines whether the spinner is animating.
 *
 * Returns: %TRUE if the spinner is animating, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_spinner_get_animating (MxSpinner *spinner)
{
  g_return_val_if_fail (MX_IS_SPINNER (spinner), FALSE);
  return spinner->priv->animating;
}

/**
 * mx_spinner_set_animating:
 * @spinner: A #MxSpinner widget
 * @animating: %TRUE to enable animation, %FALSE to disable
 *
 * Sets whether the spinner is animating. A spinner can be stopped if
 * the task it represents has finished, or to save energy.
 *
 * Since: 1.2
 */
void
mx_spinner_set_animating (MxSpinner *spinner, gboolean animating)
{
  MxSpinnerPrivate *priv;

  g_return_if_fail (MX_IS_SPINNER (spinner));

  priv = spinner->priv;
  if (priv->animating != animating)
    {
      priv->animating = animating;
      mx_spinner_update_timeout (spinner);
      g_object_notify (G_OBJECT (spinner), "animating");
    }
}
