/* nbtk-viewport.h: Viewport actor
 *
 * Copyright (C) 2008 OpenedHand
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
 * Written by: Chris Lord <chris@openedhand.com>
 * Port to Nbtk by: Robert Staudinger <robsta@openedhand.com>
 */

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_VIEWPORT_H__
#define __NBTK_VIEWPORT_H__

#include <clutter/clutter.h>
#include <nbtk/nbtk-bin.h>

G_BEGIN_DECLS

#define NBTK_TYPE_VIEWPORT            (nbtk_viewport_get_type())
#define NBTK_VIEWPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_VIEWPORT, NbtkViewport))
#define NBTK_IS_VIEWPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_VIEWPORT))
#define NBTK_VIEWPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_VIEWPORT, NbtkViewportClass))
#define NBTK_IS_VIEWPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_VIEWPORT))
#define NBTK_VIEWPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_VIEWPORT, NbtkViewportClass))

typedef struct _NbtkViewport          NbtkViewport;
typedef struct _NbtkViewportPrivate   NbtkViewportPrivate;
typedef struct _NbtkViewportClass     NbtkViewportClass;

struct _NbtkViewport
{
  NbtkBin parent;

  NbtkViewportPrivate *priv;
};

struct _NbtkViewportClass
{
  NbtkBinClass parent_class;
};

GType nbtk_viewport_get_type (void) G_GNUC_CONST;

NbtkWidget *   nbtk_viewport_new         (void);

void           nbtk_viewport_set_originu (NbtkViewport *viewport,
                                          ClutterUnit   x,
                                          ClutterUnit   y,
                                          ClutterUnit   z);

void           nbtk_viewport_set_origin  (NbtkViewport *viewport,
                                          gint          x,
                                          gint          y,
                                          gint          z);

void           nbtk_viewport_get_originu (NbtkViewport *viewport,
                                          ClutterUnit  *x,
                                          ClutterUnit  *y,
                                          ClutterUnit  *z);

void           nbtk_viewport_get_origin  (NbtkViewport *viewport,
                                          gint         *x,
                                          gint         *y,
                                          gint         *z);

G_END_DECLS

#endif /* __NBTK_VIEWPORT_H__ */
