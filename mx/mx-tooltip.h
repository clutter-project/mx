/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-tooltip.h: Plain tooltip actor
 *
 * Copyright 2008, 2009 Intel Corporation.
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_TOOLTIP_H__
#define __MX_TOOLTIP_H__

G_BEGIN_DECLS

#include "mx-floating-widget.h"

#define MX_TYPE_TOOLTIP                (mx_tooltip_get_type ())
#define MX_TOOLTIP(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_TOOLTIP, MxTooltip))
#define MX_IS_TOOLTIP(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_TOOLTIP))
#define MX_TOOLTIP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_TOOLTIP, MxTooltipClass))
#define MX_IS_TOOLTIP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_TOOLTIP))
#define MX_TOOLTIP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_TOOLTIP, MxTooltipClass))

typedef struct _MxTooltip              MxTooltip;
typedef struct _MxTooltipPrivate       MxTooltipPrivate;
typedef struct _MxTooltipClass         MxTooltipClass;

/**
 * MxTooltip:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxTooltip
{
  /*< private >*/
  MxFloatingWidget parent_instance;

  MxTooltipPrivate *priv;
};

struct _MxTooltipClass
{
  MxFloatingWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_tooltip_get_type (void) G_GNUC_CONST;


const gchar          *mx_tooltip_get_text (MxTooltip   *tooltip);
void                  mx_tooltip_set_text (MxTooltip   *tooltip,
                                           const gchar *text);

void                  mx_tooltip_show      (MxTooltip   *tooltip);
void                  mx_tooltip_hide      (MxTooltip   *tooltip);

void                   mx_tooltip_set_tip_area (MxTooltip             *tooltip,
                                                const ClutterGeometry *area);
const ClutterGeometry* mx_tooltip_get_tip_area (MxTooltip             *tooltip);

gboolean              mx_tooltip_is_in_browse_mode (void);

void                mx_tooltip_set_tip_area_from_actor (MxTooltip    *tooltip,
                                                        ClutterActor *actor);

G_END_DECLS

#endif /* __MX_TOOLTIP_H__ */
