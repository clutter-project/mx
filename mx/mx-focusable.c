/*
 * mx-focusable: An interface for actors than can accept keyboard focus
 *
 * Copyright 2010 Intel Corporation
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

#include "mx-focusable.h"
#include "mx-enum-types.h"
#include "mx-private.h"
#include "mx-widget.h"
#include <clutter/clutter.h>

static void
mx_focusable_base_init (gpointer g_iface)
{

}

GType
mx_focusable_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info =
      {
        sizeof (MxFocusableIface),
        mx_focusable_base_init,          /* base_init */
        NULL,
      };
      type = g_type_register_static (G_TYPE_INTERFACE, "MxFocusable", &info,
                                     0);
    }
  return type;
}

MxFocusable*
mx_focusable_move_focus (MxFocusable      *focusable,
                         MxFocusDirection  direction,
                         MxFocusable      *from)
{
  MxFocusableIface *iface;
  ClutterActor *actor, *parent;
  MxFocusable  *moved = NULL;

  g_return_val_if_fail (MX_IS_FOCUSABLE (focusable), FALSE);

  iface = MX_FOCUSABLE_GET_INTERFACE (focusable);

  if (iface->move_focus)
    moved = iface->move_focus (focusable, direction, from);

  if (moved)
    goto found;

  /* try and pass the focus up to something that can manage it */
  actor = CLUTTER_ACTOR (focusable);
  parent = clutter_actor_get_parent (actor);

  while (parent && !CLUTTER_IS_STAGE (parent))
    {
      /* the parent will only have knowledge of its direct children
       * that are focusable.
       */
      if (MX_IS_FOCUSABLE (actor))
        from = MX_FOCUSABLE (actor);

      if (MX_IS_FOCUSABLE (parent))
        {
          moved = mx_focusable_move_focus (MX_FOCUSABLE (parent), direction,
                                           from);
          if (moved)
            goto found;

          from = MX_FOCUSABLE (parent);
        }

      actor = parent;
      parent = clutter_actor_get_parent (actor);
    }

  /* special case the stage */
  if (CLUTTER_IS_STAGE (parent) &&
      ((direction == MX_FOCUS_DIRECTION_NEXT) ||
       (direction == MX_FOCUS_DIRECTION_PREVIOUS)))
    {
      GList *children, *l;
      GList *child_link;

      children = clutter_container_get_children (CLUTTER_CONTAINER (parent));

      /* find the current focused widget */
      child_link = g_list_find (children, actor);

      if (child_link)
        {
          if (direction == MX_FOCUS_DIRECTION_NEXT)
            {
              /* find the next widget to focus */
              for (l = child_link->next; l; l = g_list_next (l))
                {
                  if (MX_IS_FOCUSABLE (l->data))
                    {
                      moved = mx_focusable_accept_focus (l->data,
                                                         MX_FOCUS_HINT_FIRST);
                      if (moved)
                        break;
                    }
                }
            }
          else if (direction == MX_FOCUS_DIRECTION_PREVIOUS)
            {
              /* find the previous widget to focus */
              for (l = child_link->prev; l; l = g_list_previous (l))
                {
                  if (MX_IS_FOCUSABLE (l->data))
                    {
                      moved = mx_focusable_accept_focus (l->data,
                                                         MX_FOCUS_HINT_LAST);
                      if (moved)
                        break;
                    }
                }
            }
        }

      g_list_free (children);
    }

found:
  if (moved)
    {
      MX_NOTE (FOCUS, "Moving focus from %s (%p) to %s (%p) with direction %s",
               G_OBJECT_TYPE_NAME (from), from,
               G_OBJECT_TYPE_NAME (moved), moved,
               _mx_enum_to_string (MX_TYPE_FOCUS_DIRECTION, direction));
    }

  return moved;
}

MxFocusable *
mx_focusable_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxFocusableIface *iface;

  g_return_val_if_fail (MX_IS_FOCUSABLE (focusable), NULL);

  /* hidden actors should not accept focus */
  if (!CLUTTER_ACTOR_IS_VISIBLE (focusable))
    return NULL;

  /* disabled widgets should also not accept focus */
  if (MX_IS_WIDGET (focusable) &&
      mx_widget_get_disabled (MX_WIDGET (focusable)))
    return NULL;

  iface = MX_FOCUSABLE_GET_INTERFACE (focusable);

  if (iface->accept_focus)
    {
      MX_NOTE (FOCUS, "Accept focus on %s (%p) with hint %s",
               G_OBJECT_TYPE_NAME (focusable), focusable,
               _mx_enum_to_string (MX_TYPE_FOCUS_HINT, hint));
      return iface->accept_focus (focusable, hint);
    }
  else
    {
      return NULL;
    }
}

