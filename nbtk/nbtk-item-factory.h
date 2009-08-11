/*
 * nbtk-item-factory.h: An item factory interface
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


#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly."
#endif

#ifndef __NBTK_ITEM_FACTORY_H__
#define __NBTK_ITEM_FACTORY_H__

#include <glib-object.h>

#include <clutter/clutter.h>

#define NBTK_TYPE_ITEM_FACTORY (nbtk_item_factory_get_type ())
#define NBTK_ITEM_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_ITEM_FACTORY, NbtkItemFactory))
#define NBTK_IS_ITEM_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_ITEM_FACTORY))
#define NBTK_ITEM_FACTORY_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), NBTK_TYPE_ITEM_FACTORY, NbtkItemFactoryIface))

typedef struct _NbtkItemFactory NbtkItemFactory; /* dummy typedef */
typedef struct _NbtkItemFactoryIface NbtkItemFactoryIface;

/**
 * NbtkItemFactoryIface:
 * @create: virtual function called when creating a new item
 *
 * Interface for creating custom items
 */
struct _NbtkItemFactoryIface
{
    GTypeInterface g_iface;

    /*< public >*/
    /* vfuncs, not signals */
    ClutterActor *(* create) (NbtkItemFactory *factory);
};

GType nbtk_item_factory_get_type (void) G_GNUC_CONST;

ClutterActor *nbtk_item_factory_create (NbtkItemFactory *factory);

#endif
