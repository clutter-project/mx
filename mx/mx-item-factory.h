/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-item-factory.h: An item factory interface
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


#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly."
#endif

#ifndef __MX_ITEM_FACTORY_H__
#define __MX_ITEM_FACTORY_H__

#include <glib-object.h>

#include <clutter/clutter.h>

#define MX_TYPE_ITEM_FACTORY (mx_item_factory_get_type ())
#define MX_ITEM_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_ITEM_FACTORY, MxItemFactory))
#define MX_IS_ITEM_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_ITEM_FACTORY))
#define MX_ITEM_FACTORY_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), MX_TYPE_ITEM_FACTORY, MxItemFactoryIface))

/**
 * MxItemFactory:
 *
 * This is an opaque structure whose members cannot be directly accessed.
 */
typedef struct _MxItemFactory MxItemFactory; /* dummy typedef */
typedef struct _MxItemFactoryIface MxItemFactoryIface;

/**
 * MxItemFactoryIface:
 * @create: virtual function called when creating a new item
 *
 * Interface for creating custom items
 */
struct _MxItemFactoryIface
{
  /*< private >*/
  GTypeInterface g_iface;

  /*< public >*/
  /* vfuncs, not signals */
  ClutterActor *(* create) (MxItemFactory *factory);

  /*< private >*/
  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_item_factory_get_type (void) G_GNUC_CONST;

ClutterActor *mx_item_factory_create (MxItemFactory *factory);

#endif
