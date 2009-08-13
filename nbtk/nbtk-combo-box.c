/* nbtk-combo-box.c */

#include "nbtk-combo-box.h"
#include "nbtk-popup.h"

G_DEFINE_TYPE (NbtkComboBox, nbtk_combo_box, NBTK_TYPE_WIDGET)

#define COMBO_BOX_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_COMBO_BOX, NbtkComboBoxPrivate))

struct _NbtkComboBoxPrivate
{
  ClutterActor *label;
  ClutterActor *popup;
  GSList       *text_list;
  gfloat        clip_x;
  gfloat        clip_y;
};

enum
{
  PROP_0,

  PROP_TITLE
};

static void
nbtk_combo_box_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (object)->priv;

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value,
                          clutter_text_get_text ((ClutterText*) priv->label));
      break;

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
  NbtkComboBox *combo = (NbtkComboBox*) object;

  switch (property_id)
    {
    case PROP_TITLE:
      nbtk_combo_box_set_title (combo, g_value_get_string (value));
      break;

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

  if (priv->popup)
    {
      ClutterActor *stage;

      stage = clutter_actor_get_stage (CLUTTER_ACTOR (object));
      clutter_container_remove_actor ((ClutterContainer*) stage, priv->popup);
    }
}

static void
nbtk_combo_box_finalize (GObject *object)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (object)->priv;

  G_OBJECT_CLASS (nbtk_combo_box_parent_class)->finalize (object);

  if (priv->text_list)
    {
      g_slist_foreach (priv->text_list, (GFunc) g_free, NULL);
      g_slist_free (priv->text_list);
    }
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

  if (priv->popup)
    {
      /* reset the width so we do not get a cached size */
      clutter_actor_set_size (priv->popup, -1, -1);

      nbtk_widget_ensure_style ((NbtkWidget*) priv->popup);
      clutter_actor_get_preferred_width (priv->popup,
                                         -1,
                                         &min_label_w,
                                         &nat_label_w);
    }
  else
    {
      clutter_actor_get_preferred_width (priv->label,
                                         height,
                                         &min_label_w,
                                         &nat_label_w);
    }

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

static void
nbtk_combo_box_action_activated_cb (ClutterActor *popup,
                                    NbtkAction   *action,
                                    NbtkComboBox *box)
{
  nbtk_combo_box_set_title (box, nbtk_action_get_name (action));
}

static void
paint_cb (ClutterActor        *actor,
          NbtkComboBoxPrivate *priv)
{
  ClutterActorBox box;

  /* set a clip so the menu appears to slide out of the button */
  clutter_actor_get_allocation_box (actor, &box);
  cogl_clip_push (priv->clip_x - box.x1, priv->clip_y - box.y1,
                  box.x2 - box.x1, box.y2 - box.y1);
}

static void
paint_after_cb (ClutterActor *actor,
                gpointer     *data)
{
  /* reset the clip from above*/
  cogl_clip_pop ();
}

static void
nbtk_combo_box_update_popup (NbtkComboBox *box)
{
  NbtkComboBoxPrivate *priv = box->priv;
  GSList *l;

  if (!priv->popup)
    {
      ClutterActor *stage;

      stage = clutter_actor_get_stage ((ClutterActor*) box);

      priv->popup = (ClutterActor*) nbtk_popup_new ();
      clutter_container_add_actor ((ClutterContainer *) stage, priv->popup);
    }
  else
    {
      nbtk_popup_clear (NBTK_POPUP (priv->popup));
    }

  for (l = priv->text_list; l; l = g_slist_next (l))
    {
      NbtkAction *action;
      action = nbtk_action_new ();
      nbtk_action_set_name (action, (gchar*) l->data);

      nbtk_popup_add_action (NBTK_POPUP (priv->popup), action);
    }

  /* queue a relayout so the combobox size can match the new popup */
  clutter_actor_queue_relayout ((ClutterActor*) box);
}

static gboolean
nbtk_combo_box_button_release_event (ClutterActor       *actor,
                                     ClutterButtonEvent *event)
{
  NbtkComboBoxPrivate *priv = NBTK_COMBO_BOX (actor)->priv;
  ClutterContainer *stage;
  gfloat x, y, width, height, popup_height;

  stage = (ClutterContainer *) clutter_actor_get_stage (actor);


  clutter_actor_get_transformed_position (actor, &x, &y);
  clutter_actor_get_size (actor, &width, &height);


  clutter_actor_set_scale (priv->popup, 1.0, 1.0);
  clutter_actor_set_width (priv->popup, width);


  g_signal_connect (priv->popup, "action-activated",
                    G_CALLBACK (nbtk_combo_box_action_activated_cb), actor);

  clutter_actor_show (priv->popup);

  priv->clip_x = x;
  priv->clip_y = y + height;
  g_signal_connect (priv->popup, "paint", G_CALLBACK (paint_cb), priv);
  g_signal_connect_after (priv->popup, "paint", G_CALLBACK (paint_after_cb),
                          NULL);

  popup_height = clutter_actor_get_height (priv->popup);
  clutter_actor_set_position (priv->popup, x,
                              y + height - popup_height);

  clutter_actor_animate (priv->popup, CLUTTER_EASE_OUT_CUBIC, 250,
                         "y", y + height, NULL);

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

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
}

NbtkComboBox *
nbtk_combo_box_new (void)
{
  return g_object_new (NBTK_TYPE_COMBO_BOX, NULL);
}

void
nbtk_combo_box_insert_text (NbtkComboBox *box,
                            gint          position,
                            const gchar  *text)
{
  g_return_if_fail (NBTK_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_insert (box->priv->text_list, g_strdup (text),
                                         position);
  nbtk_combo_box_update_popup (box);
}

void
nbtk_combo_box_append_text (NbtkComboBox *box,
                            const gchar  *text)
{
  g_return_if_fail (NBTK_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_append (box->priv->text_list, g_strdup (text));
  nbtk_combo_box_update_popup (box);
}

void
nbtk_combo_box_prepend_text (NbtkComboBox *box,
                             const gchar  *text)
{
  g_return_if_fail (NBTK_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_prepend (box->priv->text_list,
                                          g_strdup (text));
  nbtk_combo_box_update_popup (box);
}

void
nbtk_combo_box_remove_text (NbtkComboBox *box,
                            gint          position)
{
  GSList *item;

  g_return_if_fail (NBTK_IS_COMBO_BOX (box));

  /* find the item, free the string and remove it from the list */
  item = g_slist_nth (box->priv->text_list, position);
  g_free (item->data);
  box->priv->text_list = g_slist_delete_link (box->priv->text_list, item);
  nbtk_combo_box_update_popup (box);
}

void
nbtk_combo_box_set_title (NbtkComboBox *box,
                          const gchar  *title)
{
  g_return_if_fail (NBTK_IS_COMBO_BOX (box));

  clutter_text_set_text ((ClutterText*) box->priv->label, title);
}

const gchar*
nbtk_combo_box_get_title (NbtkComboBox *box)
{
  g_return_val_if_fail (NBTK_IS_COMBO_BOX (box), NULL);

  return clutter_text_get_text ((ClutterText*) box->priv->label);
}

