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
  GList       *loading;
  GThreadPool *thread_pool;
};

typedef struct FinalizedClosure
{
  gchar             *path;
  NbtkTextureCache  *cache;
} FinalizedClosure;

enum
{
  PROP_0,
  
  PROP_MAX_THREADS
};

enum
{
  LOADED,
  ERROR_LOADING,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static NbtkTextureCache* __cache_singleton = NULL;

static void texture_loading_thread_func (gpointer data, gpointer user_data);

static void
nbtk_texture_cache_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_MAX_THREADS:
      g_thread_pool_set_max_threads (priv->thread_pool,
                                     g_value_get_int (value),
                                     NULL);
      break;
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
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_MAX_THREADS:
      g_value_set_int (value,
                       g_thread_pool_get_max_threads (priv->thread_pool));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
nbtk_texture_cache_dispose (GObject *object)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(object);
  
  if (priv->thread_pool)
    {
      g_thread_pool_free (priv->thread_pool, FALSE, TRUE);
      priv->thread_pool = NULL;
    }
  
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

  g_object_class_install_property 
             (object_class,
	      PROP_MAX_THREADS,
	      g_param_spec_int ("max-threads",
				"Max. threads",
				"Maximum amount of threads to use when "
                                "asynchronously loading textures.",
				-1, G_MAXINT, 1,
				G_PARAM_READWRITE));

  /**
   * NbtkTextureCache::loaded:
   * @self: the object that received the signal
   * @path: path to the image that has been loaded
   * @texture: the texture that has been loaded
   *
   * Emitted when a texture has completed loading asynchronously
   */
  signals[LOADED] =
    g_signal_new ("loaded",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkTextureCacheClass, loaded),
                  NULL, NULL,
                  _nbtk_marshal_VOID__STRING_OBJECT,
                  G_TYPE_NONE, 2, G_TYPE_STRING, CLUTTER_TYPE_TEXTURE);

  /**
   * NbtkTextureCache::error-loading:
   * @self: the object that received the signal
   * @error: a #GError describing the error encountered when loading the image
   *
   * Emitted when there is an error in loading any texture asynchronously
   */
  signals[ERROR_LOADING] =
    g_signal_new ("error-loading",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkTextureCacheClass, error_loading),
                  NULL, NULL,
                  _nbtk_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
nbtk_texture_cache_init (NbtkTextureCache *self)
{
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  priv->cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  if (g_thread_supported())
    {
      GError *error = NULL;

      priv->thread_pool = g_thread_pool_new (texture_loading_thread_func,
                                             self,
                                             1,
                                             FALSE,
                                             &error);
      
      if (!priv->thread_pool)
        {
          g_warning ("Error creating thread pool, asynchronous texture loading "
                     "will actually be synchronous. Error message: %s",
                     error->message);
          g_error_free (error);
        }
    }
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

typedef struct {
  NbtkTextureCache *cache;
  gchar            *path;
  GdkPixbufLoader  *pixbuf_loader;
  GError           *error;
} NbtkTextureCacheThreadData;

static gboolean
texture_loading_idle_cb (gpointer user_data)
{
  gint                        bpp;
  gboolean                    success;
  gboolean                    has_alpha;
  ClutterActor               *texture;
  GdkPixbuf                  *pixbuf;
  NbtkTextureCacheThreadData *data   = user_data;
  GError                     *error  = NULL;
  NbtkTextureCachePrivate    *priv   = TEXTURE_CACHE_PRIVATE (data->cache);
  
  /* Check that we haven't synchronously loaded the texture in the meantime */
  /* It would be good if we could easily guard against this, but you
   * really oughtn't be mixing synchronous and asynchronous texture loading.
   */
  texture = nbtk_texture_cache_get_texture_if_exists (data->cache,
                                                      data->path,
                                                      FALSE);
  if (texture)
    {
      g_signal_emit (data->cache, signals[LOADED], 0, data->path, texture);
      goto free_thread_data;
    }
  
  if (data->error)
    {
      g_signal_emit (data->cache, signals[ERROR_LOADING], 0, data->error);
      g_error_free (data->error);
      goto free_thread_data;
    }
  
  /* Get the pixbuf from the pixbuf loader (NULL if it failed to load) */
  pixbuf = gdk_pixbuf_loader_get_pixbuf (data->pixbuf_loader);
  if (!pixbuf)
    goto free_thread_data;
  
  /* Create a texture and set the data from the pixbuf */
  texture = clutter_texture_new ();
  has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
  bpp = (gdk_pixbuf_get_bits_per_sample (pixbuf) != 8) ?
    0 : (has_alpha ? 4 : 3);
  success =
    clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (texture),
                                       gdk_pixbuf_get_pixels (pixbuf),
                                       has_alpha,
                                       gdk_pixbuf_get_width (pixbuf),
                                       gdk_pixbuf_get_height (pixbuf),
                                       gdk_pixbuf_get_rowstride (pixbuf),
                                       bpp,
                                       0,
                                       &error);
  
  if (!success)
    {
      g_warning ("Error setting texture data: %s", error->message);
      g_error_free (error);
    }
  else
    g_signal_emit (data->cache, signals[LOADED], 0, data->path, texture);

free_thread_data:
  
  g_object_unref (texture);
  priv->loading = g_list_remove (priv->loading, data->path);
  g_object_unref (data->pixbuf_loader);
  g_free (data->path);
  g_slice_free (NbtkTextureCacheThreadData, data);
  
  return FALSE;
}

static void
texture_loading_thread_func (gpointer data, gpointer user_data)
{
  gsize                       length;
  gchar                      *contents;
  gboolean                    success;
  NbtkTextureCacheThreadData *thread_data = data;
  GError                     *error       = NULL;
  
  if (!(g_file_get_contents (thread_data->path, &contents, &length, &error)))
    {
      thread_data->error = error;
    }
  else
    {
      success = gdk_pixbuf_loader_write (thread_data->pixbuf_loader,
                                         (const guchar *)contents,
                                         length,
                                         &error);

      if (!success)
        {
          thread_data->error = error;
        }
      
      g_free (contents);
    }
  
  error = NULL;
  if (!gdk_pixbuf_loader_close (thread_data->pixbuf_loader, &error))
    {
      g_warning ("Error closing pixbuf loader: %s", error->message);
      g_error_free (error);
    }

  clutter_threads_add_idle (texture_loading_idle_cb, thread_data);
}

void
nbtk_texture_cache_get_texture_async (NbtkTextureCache *self,
                                      const gchar      *path)
{
  ClutterActor *texture;
  NbtkTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE (self);

  g_return_if_fail (path != NULL);

  texture = nbtk_texture_cache_get_texture_if_exists (self, path, FALSE);

  if (texture || !priv->thread_pool)
    {
      if (!texture)
        texture = nbtk_texture_cache_get_texture (self, path, FALSE);
      
      g_signal_emit (self, signals[LOADED], 0, path, texture);
      g_object_unref (texture);
    }
  else
    {
      NbtkTextureCacheThreadData *thread_data;
      
      /* Check if we're already loading the image */
      if (g_list_find_custom (priv->loading, path, (GCompareFunc)strcmp))
        return;
      
      thread_data = g_slice_new0 (NbtkTextureCacheThreadData);
      
      thread_data->cache = self;
      thread_data->path = g_strdup (path);
      thread_data->pixbuf_loader = gdk_pixbuf_loader_new ();
      
      g_thread_pool_push (priv->thread_pool,
                          thread_data,
                          NULL);
      
      /* Add it to the loading list */
      priv->loading = g_list_prepend (priv->loading, thread_data->path);
    }
}
