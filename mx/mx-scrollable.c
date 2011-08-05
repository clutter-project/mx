/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-scrollable.c: Scrollable interface
 *
 * Copyright 2008 OpenedHand
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
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Mx by: Robert Staudinger <robsta@openedhand.com>
 *
 */

#include "mx-scrollable.h"
#include "mx-private.h"

static void
mx_scrollable_base_init (gpointer g_iface)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      GParamSpec *pspec;
      pspec = g_param_spec_object ("horizontal-adjustment",
                                   "Horizontal adjustment",
                                   "The MxAdjustment for horizontal scrolling.",
                                   MX_TYPE_ADJUSTMENT,
                                   MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      pspec = g_param_spec_object ("vertical-adjustment",
                                   "Vertical adjustment",
                                   "The MxAdjustment for vertical scrolling.",
                                   MX_TYPE_ADJUSTMENT,
                                   MX_PARAM_READWRITE);
      g_object_interface_install_property (g_iface, pspec);

      initialized = TRUE;
    }
}

GType
mx_scrollable_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info =
      {
        sizeof (MxScrollableIface),
        mx_scrollable_base_init,          /* base_init */
        NULL,
      };
      type = g_type_register_static (G_TYPE_INTERFACE,
                                     "MxScrollable", &info, 0);
    }
  return type;
}

void
mx_scrollable_set_adjustments (MxScrollable *scrollable,
                               MxAdjustment *hadjustment,
                               MxAdjustment *vadjustment)
{
  MX_SCROLLABLE_GET_IFACE (scrollable)->set_adjustments (scrollable,
                                                         hadjustment,
                                                         vadjustment);
}

/**
 * mx_scrollable_get_adjustments:
 * @hadjustment: (transfer none) (out) (allow-none): location to store the horizontal adjustment, or %NULL
 * @vadjustment: (transfer none) (out) (allow-none): location to store the vertical adjustment, or %NULL
 *
 * Gets the adjustment objects that store the offsets of the scrollable widget
 * into its possible scrolling area.
 */
void
mx_scrollable_get_adjustments (MxScrollable  *scrollable,
                               MxAdjustment **hadjustment,
                               MxAdjustment **vadjustment)
{
  MX_SCROLLABLE_GET_IFACE (scrollable)->get_adjustments (scrollable,
                                                         hadjustment,
                                                         vadjustment);
}
