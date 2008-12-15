/* nbtk-tooltip.h: Plain tooltip actor
 *
 * Copyright (C) 2008 Intel Corporation
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas@linux.intel.com>
 */

#ifndef __NBTK_TOOLTIP_H__
#define __NBTK_TOOLTIP_H__

G_BEGIN_DECLS

#include <nbtk-widget.h>

#define NBTK_TYPE_TOOLTIP                (nbtk_tooltip_get_type ())
#define NBTK_TOOLTIP(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_TOOLTIP, NbtkTooltip))
#define NBTK_IS_TOOLTIP(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_TOOLTIP))
#define NBTK_TOOLTIP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_TOOLTIP, NbtkTooltipClass))
#define NBTK_IS_TOOLTIP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_TOOLTIP))
#define NBTK_TOOLTIP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_TOOLTIP, NbtkTooltipClass))

typedef struct _NbtkTooltip              NbtkTooltip;
typedef struct _NbtkTooltipPrivate       NbtkTooltipPrivate;
typedef struct _NbtkTooltipClass         NbtkTooltipClass;

struct _NbtkTooltip
{
  /*< private >*/
  NbtkWidget parent_instance;

  NbtkTooltipPrivate *priv;
};

struct _NbtkTooltipClass
{
  NbtkWidgetClass parent_class;
};

GType nbtk_tooltip_get_type (void) G_GNUC_CONST;

NbtkWidget *          nbtk_tooltip_new       (NbtkWidget  *widget, const gchar *text);
G_CONST_RETURN gchar *nbtk_tooltip_get_label (NbtkTooltip *tooltip);
void                  nbtk_tooltip_set_label (NbtkTooltip *tooltip,
                                              const gchar *text);
void                  nbtk_tooltip_show      (NbtkTooltip *tooltip);
void                  nbtk_tooltip_hide      (NbtkTooltip *tooltip);

G_END_DECLS

#endif /* __NBTK_TOOLTIP_H__ */
