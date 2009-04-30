/*
 * nbtk-widget.h: Base class for Nbtk actors
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

#include "nbtk-texture-cache.h"
#include "nbtk-marshal.h"
#include "nbtk-private.h"

G_DEFINE_TYPE (NbtkTextureCache, nbtk_texture_cache, G_TYPE_OBJECT)

#define TEXTURE_CACHE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_TYPE_TEXTURE_CACHE, NbtkTextureCachePrivate))

typedef struct _NbtkTextureCachePrivate NbtkTextureCachePrivate;

struct _NbtkTextureCachePrivate
{
  GHashTable  *cache;
};

typedef struct FinalizedClosure
{
  gchar             *path;
  NbtkTextureCache  *cache;
} FinalizedClosure;

enum
{
  PROP_0,
};

static NbtkTextureCache* __cache_singleton = NULL;

static void
nbtk_texture_cache_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
nbtk_texture_cache_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
nbtk_texture_cache_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (nbtk_texture_cache_parent_class)->dispose)
    G_OBJECT_CLASS (nbtk_texture_cache_parent_class)->dispose (object);
}

static void
nbtk_texture_cache_finalize (GObject *object)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(object);
  
  if (priv->cache)
    {
      g_hash_table_unref (priv->cache);
      priv->cache = NULL;
    }
  
  G_OBJECT_CLASS (nbtk_texture_cache_parent_class)->finalize (object);
}

static void
nbtk_texture_cache_class_init (NbtkTextureCacheClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkTextureCachePrivate));

  object_class->get_property = nbtk_texture_cache_get_property;
  object_class->set_property = nbtk_texture_cache_set_property;
  object_class->dispose = nbtk_texture_cache_dispose;
  object_class->finalize = nbtk_texture_cache_finalize;

}

static void
nbtk_texture_cache_init (NbtkTextureCache *self)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  priv->cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

}

NbtkTextureCache*
nbtk_texture_cache_get_default (void)
{
  if (G_UNLIKELY (__cache_singleton == NULL))
    __cache_singleton = g_object_new (NBTK_TYPE_TEXTURE_CACHE, NULL);
  
  return  __cache_singleton;
}

static void
on_texure_finalized (gpointer data,
		     GObject *where_the_object_was)
{
  FinalizedClosure *closure = (FinalizedClosure *)data;
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(closure->cache);

  g_hash_table_remove (priv->cache, closure->path);

  g_free(closure->path);
  g_free(closure);
}

gint
nbtk_texture_cache_get_size (NbtkTextureCache *self)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  return g_hash_table_size (priv->cache);
}

static void
add_texture_to_cache (NbtkTextureCache *self,
                      const gchar      *path,
                      ClutterActor     *res)
{
  FinalizedClosure        *closure;
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  g_hash_table_insert (priv->cache, g_strdup (path), res);

  /* Make sure we can remove from hash */
  closure = g_new0 (FinalizedClosure, 1);
  closure->path = g_strdup (path);
  closure->cache = self;

  g_object_weak_ref (G_OBJECT (res), on_texure_finalized, closure);
}

static ClutterActor *
nbtk_texture_cache_get_texture_if_exists (NbtkTextureCache *self,
				          const gchar      *path, 
				          gboolean          want_clone)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);
  ClutterActor *res = NULL;

  if (!path)
    return res;

  res = g_hash_table_lookup (priv->cache, path);

  if (res)
    {
      if (want_clone)
        res = clutter_clone_new (res);
      else
        g_object_ref (res);
    }
  
  return res;
}

/* NOTE: you should unref the returned texture when not needed */
ClutterActor*
nbtk_texture_cache_get_texture (NbtkTextureCache *self,
				const gchar      *path, 
				gboolean          want_clone)
{
  ClutterActor *res;

  g_return_val_if_fail (path != NULL, NULL);

  res = nbtk_texture_cache_get_texture_if_exists (self, path, want_clone);

  if (!res)
    {
      res = clutter_texture_new_from_file (path, NULL);
      if (!res)
	return NULL;

      add_texture_to_cache (self, path, res);
      
      if (want_clone)
        {
          ClutterActor *clone;

          clone = clutter_clone_new (res);
          g_object_unref (res);

          res = clone;
        }
    }

  return res;
}


