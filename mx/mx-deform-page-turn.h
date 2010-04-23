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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

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

/**
 * MxDeformPageTurn:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
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

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_deform_page_turn_get_type (void) G_GNUC_CONST;

ClutterActor *mx_deform_page_turn_new (void);

gdouble mx_deform_page_turn_get_period (MxDeformPageTurn *page_turn);
void    mx_deform_page_turn_set_period (MxDeformPageTurn *page_turn,
                                        gdouble period);

gdouble mx_deform_page_turn_get_angle (MxDeformPageTurn *page_turn);
void    mx_deform_page_turn_set_angle (MxDeformPageTurn *page_turn,
                                       gdouble angle);

gdouble mx_deform_page_turn_get_radius (MxDeformPageTurn *page_turn);
void    mx_deform_page_turn_set_radius (MxDeformPageTurn *page_turn,
                                        gdouble radius);

G_END_DECLS

#endif /* _MX_DEFORM_PAGE_TURN_H */
