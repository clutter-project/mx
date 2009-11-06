/* mx-toggle.c */

#include "mx-toggle.h"
#include "mx-private.h"
#include "mx-stylable.h"

static void mx_toggle_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxToggle, mx_toggle, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_toggle_stylable_iface_init))

#define TOGGLE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_TOGGLE, MxTogglePrivate))

struct _MxTogglePrivate
{
  gboolean active;

  ClutterActor *handle;
  gchar        *handle_filename;
};

enum
{
  PROP_ACTIVE = 1
};

static void
mx_toggle_stylable_iface_init (MxStylableIface *iface)
{
  GParamSpec *pspec;

  pspec = g_param_spec_string ("handle-image",
                               "Handle Image",
                               "Image used for the handle of the toggle",
                               "",
                               MX_PARAM_READWRITE);
  mx_stylable_iface_install_property (iface, MX_TYPE_TOGGLE, pspec);
}

static void
mx_toggle_style_changed (MxStylable *stylable)
{
  gboolean changed;
  gchar *handle_image;
  MxTogglePrivate *priv = MX_TOGGLE (stylable)->priv;

  changed = FALSE;
  mx_stylable_get (stylable, "handle-image", &handle_image, NULL);

  if (g_strcmp0 (handle_image, priv->handle_filename))
    {
      g_free (priv->handle_filename);
      priv->handle_filename = handle_image;
      clutter_texture_set_from_file (CLUTTER_TEXTURE (priv->handle),
                                     handle_image, NULL);
      changed = TRUE;
    }
  else
      g_free (handle_image);
}

static void
mx_toggle_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  MxTogglePrivate *priv = MX_TOGGLE (object)->priv;

  switch (property_id)
    {
    case PROP_ACTIVE:
      g_value_set_boolean (value, priv->active);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_toggle_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  MxToggle *toggle = MX_TOGGLE (object);

  switch (property_id)
    {
    case PROP_ACTIVE:
      mx_toggle_set_active (toggle, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_toggle_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_toggle_parent_class)->dispose (object);
}

static void
mx_toggle_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_toggle_parent_class)->finalize (object);
}

static void
mx_toggle_pick (ClutterActor       *actor,
                const ClutterColor *color)
{
  CLUTTER_ACTOR_CLASS (mx_toggle_parent_class)->pick (actor, color);

  clutter_actor_paint (MX_TOGGLE (actor)->priv->handle);
}

static void
mx_toggle_paint (ClutterActor *actor)
{
  CLUTTER_ACTOR_CLASS (mx_toggle_parent_class)->paint (actor);

  clutter_actor_paint (MX_TOGGLE (actor)->priv->handle);
}

static void
mx_toggle_map (ClutterActor *actor)
{
  MxTogglePrivate *priv = MX_TOGGLE (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_toggle_parent_class)->map (actor);

  clutter_actor_map (priv->handle);
}

static void
mx_toggle_unmap (ClutterActor *actor)
{
  MxTogglePrivate *priv = MX_TOGGLE (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_toggle_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->handle);
}

static void
mx_toggle_get_preferred_width (ClutterActor *actor,
                               gfloat        for_height,
                               gfloat       *min_width_p,
                               gfloat       *pref_width_p)
{
  if (min_width_p)
    *min_width_p = 105;

  if (pref_width_p)
    *pref_width_p = 105;
}

static void
mx_toggle_get_preferred_height (ClutterActor *actor,
                                gfloat        for_width,
                                gfloat       *min_height_p,
                                gfloat       *pref_height_p)
{
  if (min_height_p)
    *min_height_p = 39;

  if (pref_height_p)
    *pref_height_p = 39;
}



static void
mx_toggle_class_init (MxToggleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MxTogglePrivate));

  object_class->get_property = mx_toggle_get_property;
  object_class->set_property = mx_toggle_set_property;
  object_class->dispose = mx_toggle_dispose;
  object_class->finalize = mx_toggle_finalize;

  actor_class->pick = mx_toggle_pick;
  actor_class->paint = mx_toggle_paint;
  actor_class->map = mx_toggle_map;
  actor_class->unmap = mx_toggle_unmap;
  actor_class->get_preferred_width = mx_toggle_get_preferred_width;
  actor_class->get_preferred_height = mx_toggle_get_preferred_height;

  pspec = g_param_spec_boolean ("active",
                                "Active",
                                "Whether the toggle switch is activated",
                                FALSE,
                                MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ACTIVE, pspec);
}

static void
mx_toggle_init (MxToggle *self)
{
  self->priv = TOGGLE_PRIVATE (self);

  self->priv->handle = clutter_texture_new ();
  clutter_actor_set_parent (self->priv->handle, CLUTTER_ACTOR (self));

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);

  g_signal_connect (self, "style-changed",
                   G_CALLBACK (mx_toggle_style_changed), NULL);
}

ClutterActor *
mx_toggle_new (void)
{
  return g_object_new (MX_TYPE_TOGGLE, NULL);
}

void
mx_toggle_set_active (MxToggle *toggle, gboolean active)
{
  g_return_if_fail (MX_IS_TOGGLE (toggle));

  if (toggle->priv->active != active)
    {
      toggle->priv->active = active;

      g_object_notify (G_OBJECT (toggle), "active");
    }
}

gboolean
mx_toggle_get_active (MxToggle *toggle)
{
  g_return_val_if_fail (MX_IS_TOGGLE (toggle), FALSE);

  return toggle->priv->active;
}
