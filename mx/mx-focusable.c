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
mx_focusable_move_focus (MxFocusable *focusable,
                         MxDirection  direction,
                         MxFocusable *from)
{
  MxFocusableIface *iface;

  g_return_val_if_fail (MX_IS_FOCUSABLE (focusable), FALSE);

  iface = MX_FOCUSABLE_GET_INTERFACE (focusable);

  ClutterActor *actor, *parent;
  MxFocusable  *moved = NULL;

  if (iface->move_focus)
    return iface->move_focus (focusable, direction, from);
  else
    {
      /* try and pass the focus up to something that can manage it */
      actor = CLUTTER_ACTOR (focusable);
      parent = clutter_actor_get_parent (actor);

      while (parent)
        {
          if (MX_IS_FOCUSABLE (parent))
            {
              moved = mx_focusable_move_focus (MX_FOCUSABLE (parent), direction,
                                               from);
              if (moved)
                break;
            }

          actor = parent;
          parent = clutter_actor_get_parent (actor);
        }

      return moved;
    }
}

MxFocusable *
mx_focusable_accept_focus (MxFocusable *focusable)
{
  MxFocusableIface *iface;

  g_return_val_if_fail (MX_IS_FOCUSABLE (focusable), FALSE);

  iface = MX_FOCUSABLE_GET_INTERFACE (focusable);

  if (iface->accept_focus)
    return iface->accept_focus (focusable);
  else
    return NULL;
}

