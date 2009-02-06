/* nbtk-depth-group.h: Simple ClutterGroup extension that enables depth testing
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
 */

#ifndef __NBTK_DEPTH_GROUP_H__
#define __NBTK_DEPTH_GROUP_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_DEPTH_GROUP            (nbtk_depth_group_get_type())
#define NBTK_DEPTH_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_DEPTH_GROUP, NbtkDepthGroup))
#define NBTK_IS_DEPTH_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_DEPTH_GROUP))
#define NBTK_DEPTH_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_DEPTH_GROUP, NbtkDepthGroupClass))
#define NBTK_IS_DEPTH_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_DEPTH_GROUP))
#define NBTK_DEPTH_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_DEPTH_GROUP, NbtkDepthGroupClass))

typedef struct _NbtkDepthGroup          NbtkDepthGroup;
typedef struct _NbtkDepthGroupClass     NbtkDepthGroupClass;

struct _NbtkDepthGroup
{
  /*< private >*/
  ClutterGroup parent_instance;
};

struct _NbtkDepthGroupClass
{
  ClutterGroupClass parent_class;
};

GType nbtk_depth_group_get_type (void) G_GNUC_CONST;

ClutterActor *nbtk_depth_group_new ();

G_END_DECLS

#endif /* __NBTK_DEPTH_GROUP_H__ */
