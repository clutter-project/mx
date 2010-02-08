/*
 * mx-deform-cloth.h: A cloth deformation actor
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

#ifndef _MX_DEFORM_CLOTH_H
#define _MX_DEFORM_CLOTH_H

#include <glib-object.h>
#include <mx/mx-deform-texture.h>

G_BEGIN_DECLS

#define MX_TYPE_DEFORM_CLOTH mx_deform_cloth_get_type()

#define MX_DEFORM_CLOTH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DEFORM_CLOTH, MxDeformCloth))

#define MX_DEFORM_CLOTH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DEFORM_CLOTH, MxDeformClothClass))

#define MX_IS_DEFORM_CLOTH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DEFORM_CLOTH))

#define MX_IS_DEFORM_CLOTH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DEFORM_CLOTH))

#define MX_DEFORM_CLOTH_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DEFORM_CLOTH, MxDeformClothClass))

typedef struct _MxDeformCloth MxDeformCloth;
typedef struct _MxDeformClothClass MxDeformClothClass;
typedef struct _MxDeformClothPrivate MxDeformClothPrivate;

struct _MxDeformCloth
{
  MxDeformTexture parent;

  MxDeformClothPrivate *priv;
};

struct _MxDeformClothClass
{
  MxDeformTextureClass parent_class;
};

GType mx_deform_cloth_get_type (void) G_GNUC_CONST;

ClutterActor *mx_deform_cloth_new (void);

G_END_DECLS

#endif /* _MX_DEFORM_CLOTH_H */
