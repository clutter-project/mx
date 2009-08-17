/*
 * nbtk-item-factory.c: an item factory interface
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
 * Written by: Iain Holmes  <iain@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nbtk-item-factory.h"

static void
nbtk_item_factory_base_init (gpointer base)
{
  static gboolean initialized = FALSE;

  if (initialized)
    {
      return;
    }

  initialized = TRUE;
}

GType
nbtk_item_factory_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      const GTypeInfo info =
        {
          sizeof (NbtkItemFactoryIface),
          nbtk_item_factory_base_init,
          NULL,
        };

      type = g_type_register_static (G_TYPE_INTERFACE,
                                     "NbtkItemFactory", &info, 0);
    }

  return type;
}

ClutterActor *
nbtk_item_factory_create (NbtkItemFactory *factory)
{
  return NBTK_ITEM_FACTORY_GET_IFACE (factory)->create (factory);
}
