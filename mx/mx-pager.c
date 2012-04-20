/*
 * mx-pager.c: A container that allows you to display several pages of widgets
 *
 * Copyright 2012 Intel Corporation.
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
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Danielle Madeley <danielle.madeley@collabora.co.uk>
 */

#include "mx-pager.h"

#define PAGER_WIDTH 30. /* width of the pager boxes on the sides */
#define HOVER_TIMEOUT 300 /* ms until we preview the next page */
#define ANIMATION_DURATION 200 /* ms to animate page turns */

static void clutter_container_iface_init (gpointer, gpointer);

G_DEFINE_TYPE_WITH_CODE (MxPager, mx_pager, MX_TYPE_STACK,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init)
                        )

struct _MxPagerPrivate
{
  GList *pages;
  GList *current_page;

  ClutterActor *button_box;
  MxButtonGroup *button_group;
  GHashTable *pages_to_buttons; /* ClutterActor* -> MxButton* */

  guint hover_timeout;
};

/**
 * mx_pager_add_internal_actor:
 *
 * Chains up to the internal add() call of the #MxStack.
 */
static void
mx_pager_add_internal_actor (MxPager      *self,
                             ClutterActor *child,
                             const char   *first_prop,
                             ...)
{
  ClutterContainerIface *parent_iface =
    g_type_interface_peek_parent (CLUTTER_CONTAINER_GET_IFACE (self));
  ClutterChildMeta *meta;
  va_list var_args;

  parent_iface->add ((ClutterContainer *) self, child);

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (self), child);

  va_start (var_args, first_prop);
  g_object_set_valist ((GObject *) meta, first_prop, var_args);
  va_end (var_args);
}

static void
pager_page_button_clicked (MxButton *button,
                           MxPager  *self)
{
  ClutterActor *page;

  page = g_object_get_data (G_OBJECT (button), "page-actor");

  g_return_if_fail (CLUTTER_IS_ACTOR (page));

  mx_pager_set_current_page_by_actor (self, page, TRUE);
}

static void
mx_pager_add_page_button (MxPager      *self,
                          ClutterActor *page)
{
  ClutterActor *button;

  button = mx_button_new ();
  mx_button_set_is_toggle (MX_BUTTON (button), TRUE);
  // FIXME: add style class

  mx_button_group_add (self->priv->button_group, MX_BUTTON (button));
  clutter_container_add_actor (CLUTTER_CONTAINER (self->priv->button_box),
                               button);

  g_hash_table_insert (self->priv->pages_to_buttons, page, button);
  g_object_set_data (G_OBJECT (button), "page-actor", page);

  g_signal_connect (button, "clicked",
      G_CALLBACK (pager_page_button_clicked), self);
}

static ClutterActor *
mx_pager_get_button_for_page (MxPager      *self,
                              ClutterActor *page)
{
  ClutterActor *button;

  button = g_hash_table_lookup (self->priv->pages_to_buttons, page);

  g_return_val_if_fail (MX_IS_BUTTON (button), NULL);

  return button;
}

static void
mx_pager_relayout_pages (MxPager *self,
                         gboolean animate)
{
  float width = clutter_actor_get_width (CLUTTER_ACTOR (self));
  GList *page;
  int current, i;

  current = mx_pager_get_current_page (self);

  for (page = self->priv->pages, i = 0;
       page != NULL;
       page = page->next, i++)
    {
      gfloat x = width * (current - i);

      if (animate)
        clutter_actor_animate (page->data,
                               CLUTTER_EASE_IN_OUT_SINE,
                               ANIMATION_DURATION,
                               "anchor-x", x,
                               NULL);
      else
        clutter_actor_set_anchor_point (page->data, x, 0.);
    }
}

/**
 * mx_pager_change_page:
 * @self:
 * @new_page: pointer to the new page
 * @animate: whether to animate the transition
 *
 * Changes the currently visible page.
 */
static void
mx_pager_change_page (MxPager *self,
                      GList   *new_page,
                      gboolean animate)
{
  if (new_page == self->priv->current_page)
    return;

  g_debug ("Change page!");

  if (new_page != NULL)
    {
      ClutterActor *page = new_page->data;

      mx_button_group_set_active_button (self->priv->button_group,
          (MxButton *) mx_pager_get_button_for_page (self, page));
    }

  self->priv->current_page = new_page;
  mx_pager_relayout_pages (self, animate);
}

static void
mx_pager_dispose (GObject *self)
{
  MxPagerPrivate *priv = MX_PAGER (self)->priv;

  g_clear_object (&priv->button_group);

  if (priv->pages_to_buttons != NULL)
    {
      g_hash_table_unref (priv->pages_to_buttons);
      priv->pages_to_buttons = NULL;
    }

  G_OBJECT_CLASS (mx_pager_parent_class)->dispose (self);
}

static void
mx_pager_class_init (MxPagerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (MxPagerPrivate));

  gobject_class->dispose = mx_pager_dispose;
}

/**
 * mx_pager_bump:
 * @self:
 * @direction: -1 to bump left, +1 to bump right
 *
 * Shows a preview of the prev/next page
 */
static void
mx_pager_bump (MxPager *self,
               int      direction)
{
  ClutterActor *page;
  GList *other;
  float x;

  switch (direction)
    {
      case -1:
        other = self->priv->current_page->prev;
        break;

      case 1:
        other = self->priv->current_page->next;
        break;

      default:
        g_assert_not_reached ();
    }

  if (other == NULL)
    return;

  page = self->priv->current_page->data;
  clutter_actor_get_anchor_point (page, &x, NULL);
  clutter_actor_animate (page, CLUTTER_EASE_OUT_CIRC, ANIMATION_DURATION,
      "anchor-x", x + direction * PAGER_WIDTH,
      NULL);

  page = other->data;
  clutter_actor_get_anchor_point (page, &x, NULL);
  clutter_actor_animate (page, CLUTTER_EASE_OUT_CIRC, ANIMATION_DURATION,
      "anchor-x", x + direction * PAGER_WIDTH,
      NULL);
}

static gboolean
pager_box_hover_timeout (gpointer user_data)
{
  ClutterActor *box = user_data;
  MxPager *self = MX_PAGER (clutter_actor_get_parent (box));;
  int bump = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (box), "bump"));

  mx_pager_bump (self, bump);

  self->priv->hover_timeout = 0;

  return FALSE;
}

static void
pager_box_hover (ClutterActor *box,
                 ClutterEvent *event,
                 MxPager      *self)
{
  switch (event->type)
    {
      case CLUTTER_ENTER:
        self->priv->hover_timeout = g_timeout_add (HOVER_TIMEOUT,
            pager_box_hover_timeout,
            box);
        break;

      case CLUTTER_LEAVE:
        if (self->priv->hover_timeout != 0)
          {
            g_source_remove (self->priv->hover_timeout);
            self->priv->hover_timeout = 0;
          }

        mx_pager_relayout_pages (self, TRUE);

        break;

      default:
        g_assert_not_reached ();
    }
}

static void
mx_pager_init (MxPager *self)
{
  ClutterActor *prevbox, *nextbox;

  const ClutterColor transparent = { 0xf0, 0x00, 0x00, 0x20 };

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, MX_TYPE_PAGER,
                                            MxPagerPrivate);

  clutter_actor_set_clip_to_allocation (CLUTTER_ACTOR (self), TRUE);

  /* refs are held by Clutter */
  self->priv->pages_to_buttons = g_hash_table_new (NULL, NULL);
  self->priv->button_group = mx_button_group_new ();
  self->priv->button_box = mx_box_layout_new ();
  mx_pager_add_internal_actor (self, self->priv->button_box,
      "x-fill", FALSE,
      "x-align", MX_ALIGN_MIDDLE,
      "y-fill", FALSE,
      "y-align", MX_ALIGN_END,
      NULL);

  prevbox = clutter_rectangle_new_with_color (&transparent);

  clutter_actor_set_width (prevbox, PAGER_WIDTH);
  clutter_actor_set_reactive (prevbox, TRUE);
  g_object_set_data (G_OBJECT (prevbox), "bump", GINT_TO_POINTER (-1));
  mx_pager_add_internal_actor (self, prevbox,
      "x-fill", FALSE,
      "x-align", MX_ALIGN_START,
      NULL);

  g_signal_connect_swapped (prevbox, "button-press-event",
      G_CALLBACK (mx_pager_previous), self);
  g_signal_connect (prevbox, "enter-event",
      G_CALLBACK (pager_box_hover), self);
  g_signal_connect (prevbox, "leave-event",
      G_CALLBACK (pager_box_hover), self);

  nextbox = clutter_rectangle_new_with_color (&transparent);

  clutter_actor_set_width (nextbox, PAGER_WIDTH);
  clutter_actor_set_reactive (nextbox, TRUE);
  g_object_set_data (G_OBJECT (nextbox), "bump", GINT_TO_POINTER (1));
  mx_pager_add_internal_actor (self, nextbox,
      "x-fill", FALSE,
      "x-align", MX_ALIGN_END,
      NULL);

  g_signal_connect_swapped (nextbox, "button-press-event",
      G_CALLBACK (mx_pager_next), self);
  g_signal_connect (nextbox, "enter-event",
      G_CALLBACK (pager_box_hover), self);
  g_signal_connect (nextbox, "leave-event",
      G_CALLBACK (pager_box_hover), self);
}

static void
mx_pager_add (ClutterContainer *self,
              ClutterActor     *child)
{
  MxPagerPrivate *priv = MX_PAGER (self)->priv;

  priv->pages = g_list_append (priv->pages, child);

  mx_pager_add_internal_actor ((MxPager *) self, child,
      "fit", TRUE,
      NULL);
  clutter_actor_lower_bottom (child);

  mx_pager_add_page_button ((MxPager *) self, child);

  if (priv->current_page == NULL)
    priv->current_page = priv->pages;

  mx_pager_relayout_pages ((MxPager *) self, FALSE);
}

static void
mx_pager_remove (ClutterContainer *self,
                 ClutterActor     *child)
{
  MxPagerPrivate *priv = MX_PAGER (self)->priv;
  ClutterContainerIface *parent_iface =
    g_type_interface_peek_parent (CLUTTER_CONTAINER_GET_IFACE (self));
  GList *l;

  l = g_list_find (priv->pages, child);

  if (l == NULL)
    {
      g_warning (G_STRLOC ": Actor of type '%s' is not a child of container "
                 "of type '%s'",
                 G_OBJECT_TYPE_NAME (child),
                 G_OBJECT_TYPE_NAME (self));
      return;
    }

  if (priv->current_page == l)
    {
      /* change the current page */
      if (l->next != NULL)
        mx_pager_change_page (MX_PAGER (self), l->next, FALSE);
      else
        mx_pager_change_page (MX_PAGER (self), priv->pages, FALSE);
    }

  priv->pages = g_list_delete_link (priv->pages, l);

  clutter_actor_destroy (mx_pager_get_button_for_page ((MxPager *) self,
                                                       child));
  parent_iface->remove (self, child);
}

static void
mx_pager_foreach (ClutterContainer *self,
                  ClutterCallback   callback,
                  gpointer          user_data)
{
  MxPagerPrivate *priv = MX_PAGER (self)->priv;

  g_list_foreach (priv->pages, (GFunc) callback, user_data);
}

static void
clutter_container_iface_init (gpointer g_iface,
                              gpointer iface_data)
{
  ClutterContainerIface *iface = g_iface;

#define IMPLEMENT(x) \
  iface->x = mx_pager_##x;

  IMPLEMENT (add);
  IMPLEMENT (remove);
  IMPLEMENT (foreach);

#undef IMPLEMENT
}

/**
 * mx_pager_new:
 *
 * Returns: (transfer full) (type Mx.Pager): a new #MxPager
 */
ClutterActor *
mx_pager_new (void)
{
  return g_object_new (MX_TYPE_PAGER, NULL);
}

/**
 * mx_pager_next:
 * @self: a #MxPager
 *
 * Move to the next page.
 */
void
mx_pager_next (MxPager *self)
{
  g_return_if_fail (MX_IS_PAGER (self));
  g_return_if_fail (self->priv->current_page != NULL);

  if (self->priv->current_page->next == NULL)
    return;

  g_debug ("next page");

  mx_pager_change_page (self, self->priv->current_page->next, TRUE);
}

/**
 * mx_pager_previous:
 * @self: a #MxPager
 *
 * Move to the previous page.
 */
void
mx_pager_previous (MxPager *self)
{
  g_return_if_fail (MX_IS_PAGER (self));
  g_return_if_fail (self->priv->current_page != NULL);

  if (self->priv->current_page->prev == NULL)
    return;

  g_debug ("prev page");

  mx_pager_change_page (self, self->priv->current_page->prev, TRUE);
}

/**
 * mx_pager_set_current_page:
 * @self: a #MxPager
 * @page: the page to move to
 * @animate: whether to animate the move between pages
 *
 * Move to @page.
 */
void
mx_pager_set_current_page (MxPager *self,
                           guint    page,
                           gboolean animate)
{
  GList *page_l;

  g_return_if_fail (MX_IS_PAGER (self));

  page_l = g_list_nth (self->priv->pages, page);

  g_return_if_fail (page_l != NULL);

  mx_pager_change_page (self, page_l, animate);
}

/**
 * mx_pager_get_current_page:
 * @self: a #MxPager
 *
 * Returns: the current page number
 */
guint
mx_pager_get_current_page (MxPager *self)
{
  int pos;

  g_return_val_if_fail (MX_IS_PAGER (self), 0);

  pos = g_list_position (self->priv->pages, self->priv->current_page);

  g_return_val_if_fail (pos >= 0, 0);

  return pos;
}

/**
 * mx_pager_set_current_page_by_actor:
 * @self: a #MxPager
 * @actor: the actor of the page to move to
 * @animate: whether to animate the move between pages
 *
 * Move to the page containing @actor.
 */
void
mx_pager_set_current_page_by_actor (MxPager      *self,
                                    ClutterActor *actor,
                                    gboolean      animate)
{
  GList *page_l;

  g_return_if_fail (MX_IS_PAGER (self));

  page_l = g_list_find (self->priv->pages, actor);

  g_return_if_fail (page_l != NULL);

  mx_pager_change_page (self, page_l, animate);
}

/**
 * mx_pager_get_current_page_actor:
 * @self: a #MxPager
 *
 * Returns: (transfer none): the #ClutterActor on the current page
 */
ClutterActor *
mx_pager_get_current_page_actor (MxPager *self)
{
  g_return_val_if_fail (MX_IS_PAGER (self), NULL);

  return CLUTTER_ACTOR (self->priv->current_page->data);
}

/**
 * mx_pager_get_actor_for_page:
 * @self: a #MxPager
 * @page: a page number
 *
 * Returns: (transfer none): the #ClutterActor for @page
 */
ClutterActor *
mx_pager_get_actor_for_page (MxPager *self,
                             guint    page)
{
  g_return_val_if_fail (MX_IS_PAGER (self), NULL);

  return CLUTTER_ACTOR (g_list_nth_data (self->priv->pages, page));
}
