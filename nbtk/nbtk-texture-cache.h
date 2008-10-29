/* nbtk-texture-cache.h: Cached textures object
 *
 * Copyright (C) 2007 OpenedHand
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
 */


#ifndef _NBTK_TEXTURE_CACHE
#define _NBTK_TEXTURE_CACHE

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define NBTK_TYPE_TEXTURE_CACHE nbtk_texture_cache_get_type()

#define NBTK_TEXTURE_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  NBTK_TYPE_TEXTURE_CACHE, NbtkTextureCache))

#define NBTK_TEXTURE_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  NBTK_TYPE_TEXTURE_CACHE, NbtkTextureCacheClass))

#define NBTK_IS_TEXTURE_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NBTK_TYPE_TEXTURE_CACHE))

#define NBTK_IS_TEXTURE_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  NBTK_TYPE_TEXTURE_CACHE))

#define NBTK_TEXTURE_CACHE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  NBTK_TYPE_TEXTURE_CACHE, NbtkTextureCacheClass))

typedef struct {
  GObject parent;
} NbtkTextureCache;

typedef struct {
  GObjectClass parent_class;

  void (* loaded)        (NbtkTextureCache *self,
                          const gchar      *path,
                          ClutterTexture   *texture);

  void (* error_loading) (NbtkTextureCache *self,
                          GError           *error);
} NbtkTextureCacheClass;

GType nbtk_texture_cache_get_type (void);

NbtkTextureCache* nbtk_texture_cache_new (void);

NbtkTextureCache*
nbtk_texture_cache_get_default (void);

ClutterActor*
nbtk_texture_cache_get_texture (NbtkTextureCache *self,
				const gchar      *path, 
				gboolean          want_clone);

void
nbtk_texture_cache_get_texture_async (NbtkTextureCache *self,
                                      const gchar      *path);

gint
nbtk_texture_cache_get_size (NbtkTextureCache *self);

G_END_DECLS

#endif /* _NBTK_TEXTURE_CACHE */
