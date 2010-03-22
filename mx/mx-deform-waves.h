/*
 * mx-deform-waves.h: A waves deformation actor
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

#ifndef _MX_DEFORM_WAVES_H
#define _MX_DEFORM_WAVES_H

#include <glib-object.h>
#include <mx/mx-deform-texture.h>

G_BEGIN_DECLS

#define MX_TYPE_DEFORM_WAVES mx_deform_waves_get_type()

#define MX_DEFORM_WAVES(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_DEFORM_WAVES, MxDeformWaves))

#define MX_DEFORM_WAVES_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_DEFORM_WAVES, MxDeformWavesClass))

#define MX_IS_DEFORM_WAVES(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_DEFORM_WAVES))

#define MX_IS_DEFORM_WAVES_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_DEFORM_WAVES))

#define MX_DEFORM_WAVES_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_DEFORM_WAVES, MxDeformWavesClass))

typedef struct _MxDeformWaves MxDeformWaves;
typedef struct _MxDeformWavesClass MxDeformWavesClass;
typedef struct _MxDeformWavesPrivate MxDeformWavesPrivate;

struct _MxDeformWaves
{
  MxDeformTexture parent;

  MxDeformWavesPrivate *priv;
};

struct _MxDeformWavesClass
{
  MxDeformTextureClass parent_class;
};

GType mx_deform_waves_get_type (void) G_GNUC_CONST;

ClutterActor *mx_deform_waves_new (void);

G_END_DECLS

#endif /* _MX_DEFORM_WAVES_H */
