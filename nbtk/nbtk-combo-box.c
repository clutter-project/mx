/* nbtk-combo-box.c */

#include "nbtk-combo-box.h"

G_DEFINE_TYPE (NbtkComboBox, nbtk_combo_box, NBTK_TYPE_WIDGET)

#define COMBO_BOX_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_COMBO_BOX, NbtkComboBoxPrivate))

struct _NbtkComboBoxPrivate
{
  ClutterActor *label;
};

static void
nbtk_combo_box_get_property (GObject    *object,
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
nbtk_combo_box_set_property (GObject      *object,
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
nbtk_combo_box_dispose (GObject *object)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (object)->priv;

  G_OBJECT_CLASS (nbtk_combo_box_parent_class)->dispose (object);

  if (priv->label)
    {
      g_object_unref (priv->label);
      priv->label = NULL;
    }
}

static void
nbtk_combo_box_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_combo_box_parent_class)->finalize (object);
}

static void
nbtk_combo_box_map (ClutterActor *actor)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_combo_box_parent_class)->map (actor);

  clutter_actor_map (priv->label);
}

static void
nbtk_combo_box_unmap (ClutterActor *actor)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_combo_box_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->label);
}

static void
nbtk_combo_box_paint (ClutterActor *actor)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (nbtk_combo_box_parent_class)->paint (actor);

  clutter_actor_paint (priv->label);
}

static void
nbtk_combo_box_get_preferred_width (ClutterActor *actor,
                                    gfloat        for_height,
                                    gfloat       *min_width_p,
                                    gfloat       *natural_height_p)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;
  gfloat height;
  gfloat min_label_w, nat_label_w;
  NbtkPadding padding;

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  height = for_height - padding.top - padding.bottom;

  clutter_actor_get_preferred_width (priv->label,
                                     height,
                                     &min_label_w,
                                     &nat_label_w);

  if (min_width_p)
    *min_width_p = padding.left + padding.right + min_label_w;

  if (natural_height_p)
    *natural_height_p = padding.left + padding.right + nat_label_w;

}

static void
nbtk_combo_box_get_preferred_height (ClutterActor *actor,
                                     gfloat        for_width,
                                     gfloat       *min_height_p,
                                     gfloat       *natural_height_p)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;
  gfloat width;
  gfloat min_label_h, nat_label_h;
  NbtkPadding padding;

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  width = for_width - padding.left - padding.right;

  clutter_actor_get_preferred_height (priv->label,
                                      width,
                                      &min_label_h,
                                      &nat_label_h);

  if (min_height_p)
    *min_height_p = padding.top + padding.bottom + min_label_h;

  if (natural_height_p)
    *natural_height_p = padding.top + padding.bottom + nat_label_h;
}

static void
nbtk_combo_box_allocate (ClutterActor            *actor,
                         const ClutterActorBox   *box,
                         ClutterAllocationFlags   flags)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;
  NbtkPadding padding;
  gfloat x, y, width, height;
  gfloat min_label_h, nat_label_h, label_h;
  ClutterActorBox childbox;

  CLUTTER_ACTOR_CLASS (nbtk_combo_box_parent_class)->allocate (actor, box,
                                                               flags);

  nbtk_widget_get_padding (NBTK_WIDGET (actor), &padding);

  x = padding.left;
  y = padding.top;
  width = box->x2 - box->x1 - padding.left - padding.right;
  height = box->y2 - box->y1 - padding.top - padding.bottom;


  clutter_actor_get_preferred_height (priv->label, width, &min_label_h,
                                      &nat_label_h);
  label_h = CLAMP (nat_label_h, min_label_h, height);

  childbox.x1 = (int) (x);
  childbox.y1 = (int) (y + (height / 2 - label_h / 2));
  childbox.x2 = (int) (x + width);
  childbox.y2 = (int) (childbox.y1 + label_h);
  clutter_actor_allocate (priv->label, &childbox, flags);
}

static gboolean
nbtk_combo_box_button_release_event (ClutterActor       *actor,
                                     ClutterButtonEvent *event)
{
  g_debug ("clicked");
  return FALSE;
}

static void
nbtk_combo_box_class_init (NbtkComboBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkComboBoxPrivate));

  object_class->get_property = nbtk_combo_box_get_property;
  object_class->set_property = nbtk_combo_box_set_property;
  object_class->dispose = nbtk_combo_box_dispose;
  object_class->finalize = nbtk_combo_box_finalize;

  actor_class->map = nbtk_combo_box_map;
  actor_class->unmap = nbtk_combo_box_unmap;
  actor_class->paint = nbtk_combo_box_paint;

  actor_class->get_preferred_width = nbtk_combo_box_get_preferred_width;
  actor_class->get_preferred_height = nbtk_combo_box_get_preferred_height;
  actor_class->allocate = nbtk_combo_box_allocate;

  actor_class->button_release_event = nbtk_combo_box_button_release_event;
}

static void
nbtk_combo_box_init (NbtkComboBox *self)
{
  self->priv = COMBO_BOX_PRIVATE (self);

  self->priv->label = clutter_text_new ();
  clutter_actor_set_parent (self->priv->label, (ClutterActor*) self);
  clutter_text_set_text ((ClutterText*) self->priv->label, "testing");

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
}

NbtkComboBox *
nbtk_combo_box_new (void)
{
  return g_object_new (NBTK_TYPE_COMBO_BOX, NULL);
}
