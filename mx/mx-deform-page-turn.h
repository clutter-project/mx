/*
 * mx-deform-page-turn.h: A page-turning deformation actor
 *
 * Copyright 2010 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#ifndef _MX_DEFORM_PAGE_TURN_H
#define _MX_DEFORM_PAGE_TURN_H

#include <glib-object.h>
#include <mx/mx-deform-texture.h>

G_BEGIN_DECLS

#define MX_TYPE_DEFORM_PAGE_TURN mx_deform_page_turn_get_type()

#define MX_DEFORM_PAGE_TURN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DEFORM_PAGE_TURN, MxDeformPageTurn))

#define MX_DEFORM_PAGE_TURN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DEFORM_PAGE_TURN, MxDeformPageTurnClass))

#define MX_IS_DEFORM_PAGE_TURN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DEFORM_PAGE_TURN))

#define MX_IS_DEFORM_PAGE_TURN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DEFORM_PAGE_TURN))

#define MX_DEFORM_PAGE_TURN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DEFORM_PAGE_TURN, MxDeformPageTurnClass))

typedef struct _MxDeformPageTurn MxDeformPageTurn;
typedef struct _MxDeformPageTurnClass MxDeformPageTurnClass;
typedef struct _MxDeformPageTurnPrivate MxDeformPageTurnPrivate;

struct _MxDeformPageTurn
{
  MxDeformTexture parent;

  MxDeformPageTurnPrivate *priv;
};

struct _MxDeformPageTurnClass
{
  MxDeformTextureClass parent_class;
};

GType mx_deform_page_turn_get_type (void) G_GNUC_CONST;

ClutterActor *mx_deform_page_turn_new (void);

G_END_DECLS

#endif /* _MX_DEFORM_PAGE_TURN_H */
