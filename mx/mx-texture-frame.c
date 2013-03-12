/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-texture-frame.h: Expandible texture actor
 *
 * Copyright 2007 OpenedHand
 * Copyright 2009, 2012 Intel Corporation.
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
 */

/**
 * SECTION:mx-texture-frame
 * @short_description: Stretch a texture to fit the entire allocation
 *
 * #MxTextureFrame
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cogl/cogl.h>

#include "mx-texture-frame.h"
#include "mx-private.h"

static CoglMaterial *template_material = NULL;

static void
mx_texture_frame_paint_texture_internal (CoglHandle  material,
                                         CoglHandle  texture,
                                         guint8      opacity,
                                         gfloat      top,
                                         gfloat      right,
                                         gfloat      bottom,
                                         gfloat      left,
                                         gfloat      width,
                                         gfloat      height)
{
  gfloat tex_width, tex_height;
  gfloat ex, ey;
  gfloat tx1, ty1, tx2, ty2;

  /* apply opacity */
  cogl_material_set_color4ub (material, opacity, opacity, opacity, opacity);

  /* add the texture */
  cogl_material_set_layer (material, 0, texture);

  /* set the source */
  cogl_set_source (material);

  tex_width  = cogl_texture_get_width (texture);
  tex_height = cogl_texture_get_height (texture);

  /* simple stretch */
  if (left == 0 && right == 0 && top == 0
      && bottom == 0)
    {
      cogl_rectangle (0, 0, width, height);
      return;
    }

  tx1 = left / tex_width;
  tx2 = (tex_width - right) / tex_width;
  ty1 = top / tex_height;
  ty2 = (tex_height - bottom) / tex_height;

  ex = width - right;
  if (ex < left)
    ex = left;

  ey = height - bottom;
  if (ey < top)
    ey = top;


  {
    float rectangles[] =
    {
      /* top left corner */
      0, 0,
      left, top,
      0.0, 0.0,
      tx1, ty1,

      /* top middle */
      left, 0,
      MAX (left, ex), top,
      tx1, 0.0,
      tx2, ty1,

      /* top right */
      ex, 0,
      MAX (ex + right, width), top,
      tx2, 0.0,
      1.0, ty1,

      /* mid left */
      0, top,
      left,  ey,
      0.0, ty1,
      tx1, ty2,

      /* center */
      left, top,
      ex, ey,
      tx1, ty1,
      tx2, ty2,

      /* mid right */
      ex, top,
      MAX (ex + right, width), ey,
      tx2, ty1,
      1.0, ty2,

      /* bottom left */
      0, ey,
      left, MAX (ey + bottom, height),
      0.0, ty2,
      tx1, 1.0,

      /* bottom center */
      left, ey,
      ex, MAX (ey + bottom, height),
      tx1, ty2,
      tx2, 1.0,

      /* bottom right */
      ex, ey,
      MAX (ex + right, width), MAX (ey + bottom, height),
      tx2, ty2,
      1.0, 1.0
    };

    cogl_rectangles_with_texture_coords (rectangles, 9);
  }
}

void
mx_texture_frame_paint_texture (CoglHandle  texture,
                                guint8      opacity,
                                gfloat      top,
                                gfloat      right,
                                gfloat      bottom,
                                gfloat      left,
                                gfloat      width,
                                gfloat      height)
{
  CoglHandle material;

  /* setup the template material */
  if (!template_material)
    template_material = cogl_material_new ();

  /* create the material and apply opacity */
  material = cogl_material_copy (template_material);

  mx_texture_frame_paint_texture_internal (material, texture,
                                           opacity,
                                           top, right,
                                           bottom, left,
                                           width, height);

  cogl_handle_unref (material);
}
