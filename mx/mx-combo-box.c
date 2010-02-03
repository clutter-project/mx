/*
 * mx-combo-box.c: combo box
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

/**
 * SECTION:mx-combo-box
 * @short_description: combo box actor
 *
 * #MxComboBox combines a button with a popup menu to allow the user to select
 * an option from a list.
 */

#include "mx-combo-box.h"
#include "mx-popup.h"

#include "mx-private.h"
#include "mx-stylable.h"

static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxComboBox, mx_combo_box, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init));

#define COMBO_BOX_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_COMBO_BOX, MxComboBoxPrivate))

struct _MxComboBoxPrivate
{
  ClutterActor *label;
  GSList       *text_list;
  gfloat        clip_x;
  gfloat        clip_y;
  gint          index;
};

enum
{
  PROP_0,

  PROP_TITLE,
  PROP_INDEX
};

static void
mx_combo_box_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (object)->priv;

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value,
                          clutter_text_get_text ((ClutterText*) priv->label));
      break;

    case PROP_INDEX:
      g_value_set_int (value, priv->index);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_combo_box_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  MxComboBox *combo = (MxComboBox*) object;

  switch (property_id)
    {
    case PROP_TITLE:
      mx_combo_box_set_title (combo, g_value_get_string (value));
      break;

    case PROP_INDEX:
      mx_combo_box_set_index (combo, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_combo_box_dispose (GObject *object)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (object)->priv;

  if (priv->label)
    {
      clutter_actor_destroy (priv->label);
      priv->label = NULL;
    }

  G_OBJECT_CLASS (mx_combo_box_parent_class)->dispose (object);
}

static void
mx_combo_box_finalize (GObject *object)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (object)->priv;

  if (priv->text_list)
    {
      g_slist_foreach (priv->text_list, (GFunc) g_free, NULL);
      g_slist_free (priv->text_list);
    }

  G_OBJECT_CLASS (mx_combo_box_parent_class)->finalize (object);
}

static void
mx_combo_box_map (ClutterActor *actor)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_combo_box_parent_class)->map (actor);

  clutter_actor_map (priv->label);
}

static void
mx_combo_box_unmap (ClutterActor *actor)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_combo_box_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->label);
}

static void
mx_combo_box_paint (ClutterActor *actor)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_combo_box_parent_class)->paint (actor);

  clutter_actor_paint (priv->label);
}

static void
mx_combo_box_get_preferred_width (ClutterActor *actor,
                                  gfloat        for_height,
                                  gfloat       *min_width_p,
                                  gfloat       *natural_height_p)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;
  gfloat height;
  gfloat min_label_w, nat_label_w;
  MxPadding padding;
  ClutterActor *popup;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  height = for_height - padding.top - padding.bottom;

  popup = (ClutterActor *) mx_widget_get_popup (MX_WIDGET (actor));
  if (popup)
    {
      mx_widget_ensure_style ((MxWidget*) popup);

      clutter_actor_get_preferred_width (popup, -1, &min_label_w, &nat_label_w);
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
mx_combo_box_get_preferred_height (ClutterActor *actor,
                                   gfloat        for_width,
                                   gfloat       *min_height_p,
                                   gfloat       *natural_height_p)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;
  gfloat width;
  gfloat min_label_h, nat_label_h;
  MxPadding padding;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

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
mx_combo_box_allocate (ClutterActor          *actor,
                       const ClutterActorBox *box,
                       ClutterAllocationFlags flags)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;
  MxPadding padding;
  gfloat x, y, width, height;
  gfloat min_label_h, nat_label_h, label_h, min_popup_h, nat_popup_h;
  ClutterActorBox childbox;
  ClutterActor *popup;

  CLUTTER_ACTOR_CLASS (mx_combo_box_parent_class)->allocate (actor, box,
                                                             flags);

  mx_widget_get_padding (MX_WIDGET (actor), &padding);

  x = padding.left;
  y = padding.top;
  width = box->x2 - box->x1 - padding.left - padding.right;
  height = box->y2 - box->y1 - padding.top - padding.bottom;


  clutter_actor_get_preferred_height (priv->label, width, &min_label_h,
                                      &nat_label_h);
  label_h = CLAMP (nat_label_h, min_label_h, height);

  childbox.x1 = (int)(x);
  childbox.y1 = (int)(y + (height / 2 - label_h / 2));
  childbox.x2 = (int)(x + width);
  childbox.y2 = (int)(childbox.y1 + label_h);
  clutter_actor_allocate (priv->label, &childbox, flags);

  popup = (ClutterActor*) mx_widget_get_popup (MX_WIDGET (actor));
  clutter_actor_get_preferred_height (popup, (box->x2 - box->x1), &min_popup_h,
                                      &nat_popup_h);
  childbox.x1 = 0;
  childbox.y1 = (box->y2 - box->y1);
  childbox.x2 = (box->x2 - box->x1);
  childbox.y2 = childbox.y1 + nat_popup_h;
  clutter_actor_allocate (popup, &childbox, flags);
}

static void
mx_combo_box_action_activated_cb (ClutterActor *popup,
                                  MxAction     *action,
                                  MxComboBox   *box)
{
  gint index;

  /* set the title */
  clutter_text_set_text ((ClutterText*) box->priv->label,
                         mx_action_get_display_name (action));
  g_object_notify ((GObject*) box, "title");

  index = GPOINTER_TO_INT (g_object_get_data ((GObject*) action, "index"));
  box->priv->index = index;
  g_object_notify ((GObject*) box, "index");

}

static void
mx_combo_box_update_popup (MxComboBox *box)
{
  MxComboBoxPrivate *priv = box->priv;
  GSList *l;
  gint index;
  MxPopup *popup;

  popup = mx_widget_get_popup (MX_WIDGET (box));
  if (!popup)
    return;

  mx_popup_clear (popup);

  for (index = 0, l = priv->text_list; l; l = g_slist_next (l), index++)
    {
      MxAction *action;

      action = mx_action_new ();
      g_object_set_data ((GObject*) action, "index", GINT_TO_POINTER (index));
      mx_action_set_display_name (action, (gchar*) l->data);

      mx_popup_add_action (popup, action);
    }

  /* queue a relayout so the combobox size can match the new popup */
  clutter_actor_queue_relayout ((ClutterActor*) box);
}

static gboolean
mx_combo_box_button_press_event (ClutterActor       *actor,
                                 ClutterButtonEvent *event)
{
  ClutterActor *popup;

  popup = (ClutterActor *) mx_widget_get_popup (MX_WIDGET (actor));
  if (!popup)
    return FALSE;

  clutter_actor_show (popup);

  return TRUE;
}

static gboolean
mx_combo_box_key_press_event (ClutterActor    *actor,
                              ClutterKeyEvent *event)
{
  MxComboBoxPrivate *priv = MX_COMBO_BOX (actor)->priv;
  gint n_items;

  n_items = g_slist_length (priv->text_list);

  switch (event->keyval)
    {
    case CLUTTER_Down:
      mx_combo_box_set_index (MX_COMBO_BOX (actor),
                              CLAMP (priv->index + 1, 0, n_items -1));
      return TRUE;

    case CLUTTER_Up:
      mx_combo_box_set_index (MX_COMBO_BOX (actor),
                              CLAMP (priv->index + 1, 0, n_items -1));
      return TRUE;

    default:
      return FALSE;
    }
}


static void
mx_combo_box_style_changed (MxComboBox *combo)
{
  mx_stylable_apply_clutter_text_attributes (MX_STYLABLE (combo),
                                             CLUTTER_TEXT (combo->priv->label));
}

static void
mx_combo_box_class_init (MxComboBoxClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxComboBoxPrivate));

  object_class->get_property = mx_combo_box_get_property;
  object_class->set_property = mx_combo_box_set_property;
  object_class->dispose = mx_combo_box_dispose;
  object_class->finalize = mx_combo_box_finalize;

  actor_class->map = mx_combo_box_map;
  actor_class->unmap = mx_combo_box_unmap;
  actor_class->paint = mx_combo_box_paint;

  actor_class->get_preferred_width = mx_combo_box_get_preferred_width;
  actor_class->get_preferred_height = mx_combo_box_get_preferred_height;
  actor_class->allocate = mx_combo_box_allocate;

  actor_class->button_press_event = mx_combo_box_button_press_event;
  actor_class->key_press_event = mx_combo_box_key_press_event;

  pspec = g_param_spec_string ("title",
                               "Title",
                               "Text currently displayed in the combo box"
                               " button",
                               "",
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TITLE, pspec);

  pspec = g_param_spec_int ("index",
                            "Index",
                            "Index of the selected item, or -1 if no item is"
                            " selected.",
                            -1, G_MAXINT, -1,
                            MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_INDEX, pspec);
}

static void
mx_combo_box_init (MxComboBox *self)
{
  MxComboBoxPrivate *priv;
  ClutterActor *popup;

  priv = self->priv = COMBO_BOX_PRIVATE (self);


  priv->label = clutter_text_new ();
  clutter_actor_set_parent (priv->label, (ClutterActor*) self);

  popup = mx_popup_new ();
  mx_widget_set_popup (MX_WIDGET (self), MX_POPUP (popup));

  g_signal_connect (popup, "action-activated",
                    G_CALLBACK (mx_combo_box_action_activated_cb),
                    self);

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_combo_box_style_changed), NULL);

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
}

static MxFocusable *
mx_combo_box_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  mx_stylable_set_style_pseudo_class (MX_STYLABLE (focusable), "focus");

  clutter_actor_grab_key_focus (CLUTTER_ACTOR (focusable));

  return focusable;
}

static MxFocusable *
mx_combo_box_move_focus (MxFocusable *focusable,
                         MxDirection  direction,
                         MxFocusable *from)
{
  if (focusable == from)
    {
      mx_stylable_set_style_pseudo_class (MX_STYLABLE (focusable), "");
    }

  return NULL;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus   = mx_combo_box_accept_focus;
  iface->move_focus     = mx_combo_box_move_focus;
}

/**
 * mx_combo_box_new:
 *
 * Create a new MxComboBox
 *
 * Returns: a newly allocated MxComboBox
 */
ClutterActor *
mx_combo_box_new (void)
{
  return g_object_new (MX_TYPE_COMBO_BOX, NULL);
}

/**
 * mx_combo_box_insert_text:
 * @box: A #MxComboBox
 * @position: zero indexed position to insert the item at
 * @text: name of the item
 *
 * Insert an item into the combo box list.
 *
 */
void
mx_combo_box_insert_text (MxComboBox  *box,
                          gint         position,
                          const gchar *text)
{
  g_return_if_fail (MX_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_insert (box->priv->text_list, g_strdup (text),
                                         position);
  mx_combo_box_update_popup (box);
}

/**
 * mx_combo_box_append_text:
 * @box: A #MxComboBox
 * @text: name of the item
 *
 * Append an item to the combo box list
 *
 */
void
mx_combo_box_append_text (MxComboBox  *box,
                          const gchar *text)
{
  g_return_if_fail (MX_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_append (box->priv->text_list, g_strdup (text));
  mx_combo_box_update_popup (box);
}

/**
 * mx_combo_box_prepend_text:
 * @box: A #MxComboBox
 * @text: name of the item
 *
 * Prepend an item to the combo box list
 *
 */
void
mx_combo_box_prepend_text (MxComboBox  *box,
                           const gchar *text)
{
  g_return_if_fail (MX_IS_COMBO_BOX (box));

  box->priv->text_list = g_slist_prepend (box->priv->text_list,
                                          g_strdup (text));
  mx_combo_box_update_popup (box);
}

/**
 * mx_combo_box_remove_text:
 * @box: A #MxComboBox
 * @position: position of the item to remove
 *
 * Remove the item at @position
 *
 */
void
mx_combo_box_remove_text (MxComboBox *box,
                          gint        position)
{
  GSList *item;

  g_return_if_fail (MX_IS_COMBO_BOX (box));

  /* find the item, free the string and remove it from the list */
  item = g_slist_nth (box->priv->text_list, position);
  g_free (item->data);
  box->priv->text_list = g_slist_delete_link (box->priv->text_list, item);
  mx_combo_box_update_popup (box);
}

/**
 * mx_combo_box_set_title:
 * @box: A #MxComboBox
 * @title: text to display
 *
 * Set the text displayed in the combo box
 *
 */
void
mx_combo_box_set_title (MxComboBox  *box,
                        const gchar *title)
{
  g_return_if_fail (MX_IS_COMBO_BOX (box));

  box->priv->index = -1;
  clutter_text_set_text ((ClutterText*) box->priv->label, title);

  g_object_notify (G_OBJECT (box), "index");
  g_object_notify (G_OBJECT (box), "title");
}

/**
 * mx_combo_box_get_title:
 * @box: A #MxComboBox
 *
 * Get the text displayed in the combo box
 *
 * Returns: the text string, owned by the combo box
 */
const gchar*
mx_combo_box_get_title (MxComboBox *box)
{
  g_return_val_if_fail (MX_IS_COMBO_BOX (box), NULL);

  return clutter_text_get_text ((ClutterText*) box->priv->label);
}

/**
 * mx_combo_box_set_index:
 * @box: A #MxComboBox
 * @index: the index of the list item to set
 *
 * Set the currenet combo box text from the item at @index in the list.
 *
 */
void
mx_combo_box_set_index (MxComboBox *box,
                        gint        index)
{
  GSList *item;

  g_return_if_fail (MX_IS_COMBO_BOX (box));

  item = g_slist_nth (box->priv->text_list, index);

  if (!item)
    {
      box->priv->index = -1;
      clutter_text_set_text ((ClutterText*) box->priv->label, NULL);
      return;
    }

  box->priv->index = index;
  clutter_text_set_text ((ClutterText*) box->priv->label, item->data);

  g_object_notify (G_OBJECT (box), "index");
  g_object_notify (G_OBJECT (box), "title");
}

/**
 * mx_combo_box_get_index:
 * @box: A #MxComboBox
 *
 * Get the index of the last item selected
 *
 * Returns: const gint
 */
const gint
mx_combo_box_get_index (MxComboBox *box)
{
  g_return_val_if_fail (MX_IS_COMBO_BOX (box), 0);

  return box->priv->index;
}

