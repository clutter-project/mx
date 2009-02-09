/* nbtk-scrollable.h: Scrollable interface
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

#ifndef __NBTK_SCROLLABLE_H__
#define __NBTK_SCROLLABLE_H__

#include <glib-object.h>
#include <nbtk/nbtk-adjustment.h>

G_BEGIN_DECLS

#define NBTK_TYPE_SCROLLABLE                (nbtk_scrollable_get_type ())
#define NBTK_SCROLLABLE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_SCROLLABLE, NbtkScrollable))
#define NBTK_IS_SCROLLABLE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_SCROLLABLE))
#define NBTK_SCROLLABLE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), NBTK_TYPE_SCROLLABLE, NbtkScrollableInterface))

typedef struct _NbtkScrollable NbtkScrollable; /* Dummy object */
typedef struct _NbtkScrollableInterface NbtkScrollableInterface;

struct _NbtkScrollableInterface
{
  GTypeInterface parent;

  void (* set_adjustments) (NbtkScrollable  *scrollable,
                            NbtkAdjustment  *hadjustment,
                            NbtkAdjustment  *vadjustment);
  void (* get_adjustments) (NbtkScrollable  *scrollable,
                            NbtkAdjustment **hadjustment,
                            NbtkAdjustment **vadjustment);
};

GType nbtk_scrollable_get_type (void) G_GNUC_CONST;

void nbtk_scrollable_set_adjustments (NbtkScrollable  *scrollable,
                                      NbtkAdjustment  *hadjustment,
                                      NbtkAdjustment  *vadjustment);
void nbtk_scrollable_get_adjustments (NbtkScrollable  *scrollable,
                                      NbtkAdjustment **hadjustment,
                                      NbtkAdjustment **vadjustment);

G_END_DECLS

#endif /* __NBTK_SCROLLABLE_H__ */
