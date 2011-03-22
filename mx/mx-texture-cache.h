/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-texture-cache.h: Cached textures object
 *
 * Copyright 2007 OpenedHand
 * Copyright 2009 Intel Corporation.
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

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_TEXTURE_CACHE
#define _MX_TEXTURE_CACHE

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_TEXTURE_CACHE mx_texture_cache_get_type()

#define MX_TEXTURE_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_TEXTURE_CACHE, MxTextureCache))

#define MX_TEXTURE_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_TEXTURE_CACHE, MxTextureCacheClass))

#define MX_IS_TEXTURE_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_TEXTURE_CACHE))

#define MX_IS_TEXTURE_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_TEXTURE_CACHE))

#define MX_TEXTURE_CACHE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_TEXTURE_CACHE, MxTextureCacheClass))

/**
 * MxTextureCache:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
    /*< private >*/
    GObject parent;
} MxTextureCache;

typedef struct {
  GObjectClass parent_class;

  void (* loaded)        (MxTextureCache *self,
                          const gchar      *uri,
                          ClutterTexture   *texture);

  void (* error_loading) (MxTextureCache *self,
                          GError           *error);


  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
} MxTextureCacheClass;

GType mx_texture_cache_get_type (void);

MxTextureCache* mx_texture_cache_get_default (void);

ClutterTexture* mx_texture_cache_get_texture (MxTextureCache *self,
                                              const gchar    *uri);
ClutterActor*   mx_texture_cache_get_actor   (MxTextureCache *self,
                                              const gchar    *uri);
CoglHandle      mx_texture_cache_get_cogl_texture (MxTextureCache *self,
                                                   const gchar    *uri);

ClutterTexture *mx_texture_cache_get_meta_texture (MxTextureCache *self,
                                                   const gchar    *uri,
                                                   gpointer        ident);
CoglHandle      mx_texture_cache_get_meta_cogl_texture (MxTextureCache *self,
                                                        const gchar    *uri,
                                                        gpointer        ident);

gint            mx_texture_cache_get_size    (MxTextureCache *self);

gboolean        mx_texture_cache_contains    (MxTextureCache *self,
                                              const gchar    *uri);

gboolean        mx_texture_cache_contains_meta (MxTextureCache *self,
                                                const gchar    *uri,
                                                gpointer        ident);

void            mx_texture_cache_insert      (MxTextureCache *self,
                                              const gchar    *uri,
                                              CoglHandle     *texture);

void            mx_texture_cache_insert_meta (MxTextureCache *self,
                                              const gchar    *uri,
                                              gpointer        ident,
                                              CoglHandle     *texture,
                                              GDestroyNotify  destroy_func);

void mx_texture_cache_load_cache (MxTextureCache *self,
                                  const char     *filename);

G_END_DECLS

#endif /* _MX_TEXTURE_CACHE */
