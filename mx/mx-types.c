/*
 * mx-types.c
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */


#include <mx-types.h>

#include <stdlib.h>
#include <string.h>
#include "mx-private.h"

void
mx_font_weight_set_from_string (GValue      *dest,
                                const gchar *src)
{
  if (!src)
    {
      g_value_set_enum (dest, MX_FONT_WEIGHT_NORMAL);
      return;
    }

  if (!strcmp (src, "bold"))
    g_value_set_enum (dest, MX_FONT_WEIGHT_BOLD);
  else if (!strcmp (src, "normal"))
    g_value_set_enum (dest, MX_FONT_WEIGHT_NORMAL);
  else if (!strcmp (src, "lighter"))
    g_value_set_enum (dest, MX_FONT_WEIGHT_LIGHTER);
  else if (!strcmp (src, "bolder"))
    g_value_set_enum (dest, MX_FONT_WEIGHT_BOLDER);
  else
    g_value_set_enum (dest, MX_FONT_WEIGHT_NORMAL);
}

static gpointer
mx_padding_copy (gpointer data)
{
  return g_slice_dup (MxPadding, data);
}

static void
mx_padding_free (gpointer data)
{
  if (G_LIKELY (data))
    g_slice_free (MxPadding, data);
}

static void
mx_padding_from_string (const GValue *src,
                        GValue       *dest)
{
  const gchar *str;
  gchar **strv;
  MxPadding padding = { 0, };

  str = g_value_get_string (src);
  if (!str)
    {
      g_value_set_boxed (dest, &padding);
      return;
    }

  strv = g_strsplit (str, " ", 0);
  if (!strv)
    {
      g_value_set_boxed (dest, &padding);
      return;
    }

  switch (g_strv_length (strv))
    {
    case 1:
      padding.top = padding.right
        = padding.bottom = padding.left = atoi (strv[0]);
      break;

    case 2:
      padding.top = padding.bottom = atoi (strv[0]);
      padding.left = padding.right = atoi (strv[1]);
      break;

    case 3:
      padding.top = atoi (strv[0]);
      padding.right = padding.left = atoi (strv[1]);
      padding.bottom = atoi (strv[2]);
      break;

    case 4:
      padding.top = atoi (strv[0]);
      padding.right = atoi (strv[1]);
      padding.bottom = atoi (strv[2]);
      padding.left = atoi (strv[3]);
      break;
    }
  g_strfreev (strv);
  g_value_set_boxed (dest, &padding);
}

GType
mx_padding_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    our_type = g_boxed_type_register_static (g_intern_static_string ("MxPadding"),
                                             mx_padding_copy,
                                             mx_padding_free);

  g_value_register_transform_func (G_TYPE_STRING, our_type,
                                   mx_padding_from_string);

  return our_type;
}


static MxBorderImage *
mx_border_image_copy (const MxBorderImage *border_image)
{
  MxBorderImage *copy;

  g_return_val_if_fail (border_image != NULL, NULL);

  copy = g_slice_new0 (MxBorderImage);
  *copy = *border_image;
  copy->uri = g_strdup (border_image->uri);

  return copy;
}

static void
mx_border_image_free (MxBorderImage *border_image)
{
  if (G_LIKELY (border_image))
    {
      g_free (border_image->uri);
      border_image->uri = NULL;
      g_slice_free (MxBorderImage, border_image);
    }
}

void
mx_border_image_set_from_string (GValue *dest,
                                 const gchar *str,
                                 const gchar *filename)
{
  MxBorderImage border_image = { 0, };
  gchar **strv;
  gint n_tokens;
  gchar *base;

  if (!g_strcmp0 (str, "none"))
    {
      g_value_set_boxed (dest, &border_image);
      return;
    }

  strv = g_strsplit_set (str, " (\"\')", 0);

  n_tokens = g_strv_length (strv);

  if (n_tokens < 2)
    {
      g_strfreev (strv);
      g_value_set_boxed (dest, &border_image);
      g_warning ("Could not parse border image from \"%s\"", str);
      return;
    }

  if (g_strcmp0 (strv[0], "url"))
    {
      g_strfreev (strv);
      g_value_set_boxed (dest, &border_image);
      g_warning ("Could not parse border image from \"%s\"", str);
      return;
    }

  /* check for relative path */
  if (strv[2][0] == '/')
    border_image.uri = g_strdup (strv[2]);
  else
    {
      base = g_path_get_dirname (filename);

      border_image.uri = g_build_filename (base, strv[2], NULL);

      g_free (base);
    }

  if (n_tokens == 6) /* one */
    {
      border_image.top = border_image.right
        = border_image.bottom = border_image.left = atoi (strv[5]);
    }
  else if (n_tokens == 7) /* two */
    {
      border_image.top = border_image.bottom = atoi (strv[5]);
      border_image.right = border_image.left = atoi (strv[6]);
    }
  else if (n_tokens == 8) /* three */
    {
      border_image.top = atoi (strv[5]);
      border_image.right = border_image.left = atoi (strv[6]);
      border_image.bottom = atoi (strv[7]);
    }
  else if (n_tokens == 9) /* four */
    {
      border_image.top = atoi (strv[5]);
      border_image.right = atoi (strv[6]);
      border_image.bottom = atoi (strv[7]);
      border_image.left = atoi (strv[8]);
    }

  g_strfreev (strv);
  g_value_set_boxed (dest, &border_image);
  g_free (border_image.uri);
}

GType
mx_border_image_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    our_type =
      g_boxed_type_register_static (g_intern_static_string ("MxBorderImage"),
                                    (GBoxedCopyFunc) mx_border_image_copy,
                                    (GBoxedFreeFunc) mx_border_image_free);

  return our_type;
}
