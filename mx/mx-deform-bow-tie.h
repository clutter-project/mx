/*
 * mx-deform-bow-tie.h: A bow-tie deformation actor
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

#ifndef _MX_DEFORM_BOW_TIE_H
#define _MX_DEFORM_BOW_TIE_H

#include <glib-object.h>
#include <mx/mx-deform-texture.h>

G_BEGIN_DECLS

#define MX_TYPE_DEFORM_BOW_TIE mx_deform_bow_tie_get_type()

#define MX_DEFORM_BOW_TIE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DEFORM_BOW_TIE, MxDeformBowTie))

#define MX_DEFORM_BOW_TIE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DEFORM_BOW_TIE, MxDeformBowTieClass))

#define MX_IS_DEFORM_BOW_TIE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DEFORM_BOW_TIE))

#define MX_IS_DEFORM_BOW_TIE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DEFORM_BOW_TIE))

#define MX_DEFORM_BOW_TIE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DEFORM_BOW_TIE, MxDeformBowTieClass))

typedef struct _MxDeformBowTie MxDeformBowTie;
typedef struct _MxDeformBowTieClass MxDeformBowTieClass;
typedef struct _MxDeformBowTiePrivate MxDeformBowTiePrivate;

/**
 * MxDeformBowTie:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxDeformBowTie
{
  MxDeformTexture parent;

  MxDeformBowTiePrivate *priv;
};

struct _MxDeformBowTieClass
{
  MxDeformTextureClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_deform_bow_tie_get_type (void) G_GNUC_CONST;

ClutterActor *mx_deform_bow_tie_new (void);

gdouble mx_deform_bow_tie_get_period (MxDeformBowTie *bow_tie);
void    mx_deform_bow_tie_set_period (MxDeformBowTie *bow_tie, gdouble period);

gboolean mx_deform_bow_tie_get_flip_back (MxDeformBowTie *bow_tie);
void     mx_deform_bow_tie_set_flip_back (MxDeformBowTie *bow_tie,
                                          gboolean        flip_back);

G_END_DECLS

#endif /* _MX_DEFORM_BOW_TIE_H */
