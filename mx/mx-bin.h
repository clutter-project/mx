/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-bin.h: Basic container actor
 *
 * Copyright 2009, 2008 Intel Corporation.
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
 * Written by: Emmanuele Bassi <ebassi@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_BIN_H__
#define __MX_BIN_H__

#include <mx/mx-types.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_BIN                   (mx_bin_get_type ())
#define MX_BIN(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_BIN, MxBin))
#define MX_IS_BIN(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_BIN))
#define MX_BIN_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_BIN, MxBinClass))
#define MX_IS_BIN_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_BIN))
#define MX_BIN_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_BIN, MxBinClass))

typedef struct _MxBin                 MxBin;
typedef struct _MxBinPrivate          MxBinPrivate;
typedef struct _MxBinClass            MxBinClass;

/**
 * MxBin:
 *
 * The #MxBin struct contains only private data
 */
struct _MxBin
{
  /*< private >*/
  MxWidget parent_instance;

  MxBinPrivate *priv;
};

/**
 * MxBinClass:
 *
 * The #MxBinClass struct contains only private data
 */
struct _MxBinClass
{
  /*< private >*/
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_bin_get_type (void) G_GNUC_CONST;

void          mx_bin_allocate_child (MxBin                  *bin,
                                     const ClutterActorBox  *box,
                                     ClutterAllocationFlags  flags);
void          mx_bin_set_child      (MxBin                  *bin,
                                     ClutterActor           *child);
ClutterActor *mx_bin_get_child      (MxBin                  *bin);
void          mx_bin_set_alignment  (MxBin                  *bin,
                                     MxAlign                 x_align,
                                     MxAlign                 y_align);
void          mx_bin_get_alignment  (MxBin                  *bin,
                                     MxAlign                *x_align,
                                     MxAlign                *y_align);
void          mx_bin_set_fill       (MxBin                  *bin,
                                     gboolean                x_fill,
                                     gboolean                y_fill);
void          mx_bin_get_fill       (MxBin                  *bin,
                                     gboolean               *x_fill,
                                     gboolean               *y_fill);

G_END_DECLS

#endif /* __MX_BIN_H__ */
