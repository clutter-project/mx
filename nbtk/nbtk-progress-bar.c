/*
 * nbtk-progress-bar.c: Progress bar widget
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:nbtk-progress-bar
 * @short_description: visual representation of progress
 *
 * #NbtkProgressBar visually represents the progress of an action or a value in
 * a range.
 *
 */

#include "nbtk-progress-bar.h"
#include "nbtk-texture-frame.h"

#define NBTK_TYPE_PROGRESS_BAR_FILL nbtk_progress_bar_fill_get_type()

#define NBTK_PROGRESS_BAR_FILL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_PROGRESS_BAR_FILL, NbtkProgressBarFill))

#define NBTK_PROGRESS_BAR_FILL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_PROGRESS_BAR_FILL, NbtkProgressBarFillClass))

#define NBTK_IS_PROGRESS_BAR_FILL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_PROGRESS_BAR_FILL))

#define NBTK_IS_PROGRESS_BAR_FILL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_PROGRESS_BAR_FILL))

#define NBTK_PROGRESS_BAR_FILL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_PROGRESS_BAR_FILL, NbtkProgressBarFillClass))

typedef struct
{
  NbtkWidget parent;
} NbtkProgressBarFill;

typedef struct
{
  NbtkWidgetClass parent_class;
} NbtkProgressBarFillClass;

GType nbtk_progress_bar_fill_get_type (void);

G_DEFINE_TYPE (NbtkProgressBar, nbtk_progress_bar, NBTK_TYPE_WIDGET)
G_DEFINE_TYPE (NbtkProgressBarFill, nbtk_progress_bar_fill, NBTK_TYPE_WIDGET)

#define PROGRESS_BAR_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_PROGRESS_BAR, NbtkProgressBarPrivate))

struct _NbtkProgressBarPrivate
{
  ClutterActor *fill;
  gdouble       progress;
};

enum
{
  PROP_0,

  PROP_PROGRESS
};

static void
nbtk_progress_bar_get_property (GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec)
{
  NbtkProgressBar *self = NBTK_PROGRESS_BAR (object);

  switch (property_id)
    {
    case PROP_PROGRESS:
      g_value_set_double (value, nbtk_progress_bar_get_progress (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_progress_bar_set_property (GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec)
{
  NbtkProgressBar *self = NBTK_PROGRESS_BAR (object);

  switch (property_id)
    {
    case PROP_PROGRESS:
      nbtk_progress_bar_set_progress (self, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
nbtk_progress_bar_dispose (GObject *object)
{
  NbtkProgressBarPrivate *priv = NBTK_PROGRESS_BAR (object)->priv;

  if (priv->fill)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->fill));
      priv->fill = NULL;
    }

  G_OBJECT_CLASS (nbtk_progress_bar_parent_class)->dispose (object);
}

static void
nbtk_progress_bar_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_progress_bar_parent_class)->finalize (object);
}

static void
nbtk_progress_bar_paint (ClutterActor *actor)
{
  NbtkProgressBarPrivate *priv = NBTK_PROGRESS_BAR (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_progress_bar_parent_class)->paint (actor);

  if (priv->progress)
    clutter_actor_paint (priv->fill);
}

static void
nbtk_progress_bar_allocate_fill (NbtkProgressBar        *self,
                                 const ClutterActorBox  *box,
                                 ClutterAllocationFlags  flags)
{
  ClutterActorBox box_data;
  NbtkProgressBarPrivate *priv = self->priv;

  if (!box)
    {
      clutter_actor_get_allocation_box (CLUTTER_ACTOR (self), &box_data);
      box = &box_data;
    }

  if (priv->progress)
    {
      ClutterActorBox child_box;
      NbtkPadding padding;

      nbtk_widget_get_padding (NBTK_WIDGET (self), &padding);

      child_box.x1 = padding.left;
      child_box.y1 = padding.top;
      child_box.y2 = (box->y2 - box->y1) - padding.bottom;
      child_box.x2 = ((box->x2 - box->x1 - padding.left - padding.right) *
                      priv->progress) + padding.left;

      clutter_actor_allocate (priv->fill, &child_box, flags);
    }
}

static void
nbtk_progress_bar_allocate (ClutterActor           *actor,
                            const ClutterActorBox  *box,
                            ClutterAllocationFlags  flags)
{
  NbtkProgressBar *self = NBTK_PROGRESS_BAR (actor);

  CLUTTER_ACTOR_CLASS (nbtk_progress_bar_parent_class)->
    allocate (actor, box, flags);

  nbtk_progress_bar_allocate_fill (self, box, flags);
}

static void
nbtk_progress_bar_map (ClutterActor *actor)
{
  NbtkProgressBarPrivate *priv = NBTK_PROGRESS_BAR (actor)->priv;
  CLUTTER_ACTOR_CLASS (nbtk_progress_bar_parent_class)->map (actor);
  clutter_actor_map (priv->fill);
}

static void
nbtk_progress_bar_unmap (ClutterActor *actor)
{
  NbtkProgressBarPrivate *priv = NBTK_PROGRESS_BAR (actor)->priv;
  CLUTTER_ACTOR_CLASS (nbtk_progress_bar_parent_class)->unmap (actor);
  clutter_actor_unmap (priv->fill);
}

static void
nbtk_progress_bar_class_init (NbtkProgressBarClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkProgressBarPrivate));

  object_class->get_property = nbtk_progress_bar_get_property;
  object_class->set_property = nbtk_progress_bar_set_property;
  object_class->dispose = nbtk_progress_bar_dispose;
  object_class->finalize = nbtk_progress_bar_finalize;

  actor_class->paint = nbtk_progress_bar_paint;
  actor_class->allocate = nbtk_progress_bar_allocate;
  actor_class->map = nbtk_progress_bar_map;
  actor_class->unmap = nbtk_progress_bar_unmap;

  pspec = g_param_spec_double ("progress",
                               "Progress",
                               "Progress",
                               0.0, 1.0, 0.0, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PROGRESS, pspec);
}

static void
nbtk_progress_bar_init (NbtkProgressBar *self)
{
  NbtkProgressBarPrivate *priv = self->priv = PROGRESS_BAR_PRIVATE (self);

  priv->fill = g_object_new (NBTK_TYPE_PROGRESS_BAR_FILL,
                             "clip-to-allocation", TRUE,
                             NULL);
  clutter_actor_set_parent (priv->fill, CLUTTER_ACTOR (self));
}

/**
 * nbtk_progress_bar_new:
 *
 * Create a new progress bar
 *
 * Returns: a new #NbtkProgressBar
 */
NbtkWidget *
nbtk_progress_bar_new (void)
{
  return g_object_new (NBTK_TYPE_PROGRESS_BAR, NULL);
}

/**
 * nbtk_progress_bar_set_progress:
 * @bar: A #NbtkProgressBar
 * @progress: A value between 0.0 and 1.0
 *
 * Set the progress of the progress bar
 *
 */
void
nbtk_progress_bar_set_progress (NbtkProgressBar *bar,
                                gdouble          progress)
{
  NbtkProgressBarPrivate *priv = bar->priv;

  if (priv->progress != progress)
    {
      priv->progress = progress;
      nbtk_progress_bar_allocate_fill (bar, NULL, 0);
      clutter_actor_queue_redraw (CLUTTER_ACTOR (bar));
      g_object_notify (G_OBJECT (bar), "progress");
    }
}

/**
 * nbtk_progress_bar_get_progress:
 * @bar: A #NbtkProgressBar
 *
 * Get the progress of the progress bar
 *
 * Returns: A value between 0.0 and 1.0
 */
gdouble
nbtk_progress_bar_get_progress (NbtkProgressBar *bar)
{
  return bar->priv->progress;
}

static void
nbtk_progress_bar_fill_class_init (NbtkProgressBarFillClass *klass)
{
}

static void
nbtk_progress_bar_fill_init (NbtkProgressBarFill *self)
{
}

