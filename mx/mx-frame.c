/* mx-frame.c */

#include "mx-frame.h"

G_DEFINE_TYPE (MxFrame, mx_frame, MX_TYPE_BIN)

#define FRAME_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_FRAME, MxFramePrivate))

struct _MxFramePrivate
{
  gpointer dummy;
};


static void
mx_frame_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_frame_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_frame_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_frame_parent_class)->dispose (object);
}

static void
mx_frame_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_frame_parent_class)->finalize (object);
}

static void
mx_frame_allocate (ClutterActor          *self,
                   const ClutterActorBox *box,
                   ClutterAllocationFlags flags)
{
  CLUTTER_ACTOR_CLASS (mx_frame_parent_class)->allocate (self, box, flags);

  mx_bin_allocate_child (MX_BIN (self), box, flags);
}

static void
mx_frame_class_init (MxFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxFramePrivate));

  object_class->get_property = mx_frame_get_property;
  object_class->set_property = mx_frame_set_property;
  object_class->dispose = mx_frame_dispose;
  object_class->finalize = mx_frame_finalize;

  actor_class->allocate = mx_frame_allocate;
}

static void
mx_frame_init (MxFrame *self)
{
  self->priv = FRAME_PRIVATE (self);
}

ClutterActor *
mx_frame_new (void)
{
  return g_object_new (MX_TYPE_FRAME, NULL);
}
