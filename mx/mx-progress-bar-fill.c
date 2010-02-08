/*
 * mx-progress-bar-fill.c: Fill used in progress bar/slider widgets
 *
 * Copyright 2009 Intel Corporation.
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

/*
 * This class is private to MX
 */

#include "mx-progress-bar-fill.h"
#include "mx-stylable.h"

static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxProgressBarFill, _mx_progress_bar_fill, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("height",
                                 "Height",
                                 "Height of the bar, in px",
                                 0, G_MAXUINT, 16,
                                 G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface,
                                          MX_TYPE_PROGRESS_BAR_FILL, pspec);
    }
}

static void
mx_progress_bar_fill_get_preferred_height (ClutterActor *actor,
                                           gfloat        for_width,
                                           gfloat       *min_height_p,
                                           gfloat       *nat_height_p)
{
  MxProgressBarFill *self = MX_PROGRESS_BAR_FILL (actor);

  if (min_height_p)
    *min_height_p = (gfloat)self->height;
  if (nat_height_p)
    *nat_height_p = (gfloat)self->height;
}

static void
_mx_progress_bar_fill_class_init (MxProgressBarFillClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->get_preferred_height = mx_progress_bar_fill_get_preferred_height;
}

static void
mx_progress_bar_fill_style_changed_cb (MxProgressBarFill *self)
{
  guint height;

  mx_stylable_get (MX_STYLABLE (self),
                   "height", &height,
                   NULL);

  if (self->height != height)
    {
      self->height = height;
      clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
    }
}

static void
_mx_progress_bar_fill_init (MxProgressBarFill *self)
{
  self->height = 16;
  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_progress_bar_fill_style_changed_cb), NULL);
}

ClutterActor *
_mx_progress_bar_fill_new (void)
{
  return g_object_new (MX_TYPE_PROGRESS_BAR_FILL, NULL);
}
