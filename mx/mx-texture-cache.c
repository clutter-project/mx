/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-widget.h: Base class for Mx actors
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

/**
 * SECTION:mx-texture-cache
 * @short_description: A per-process store to cache textures
 *
 * #MxTextureCache allows an application to re-use an previously loaded
 * textures.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

#include "mx-texture-cache.h"
#include "mx-marshal.h"
#include "mx-private.h"

G_DEFINE_TYPE (MxTextureCache, mx_texture_cache, G_TYPE_OBJECT)

#define TEXTURE_CACHE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_TEXTURE_CACHE, MxTextureCachePrivate))

typedef struct _MxTextureCachePrivate MxTextureCachePrivate;

struct _MxTextureCachePrivate
{
  GHashTable *cache;
  GRegex     *is_uri;
};

typedef struct FinalizedClosure
{
  gchar          *uri;
  MxTextureCache *cache;
} FinalizedClosure;

enum
{
  PROP_0,
};

static MxTextureCache* __cache_singleton = NULL;

/*
 * Convention: posX with a value of -1 indicates whole texture
 */
typedef struct MxTextureCacheItem {
  char          filename[256];
  int           width, height;
  int           posX, posY;
  CoglHandle    ptr;
  GHashTable   *meta;
} MxTextureCacheItem;

typedef struct
{
  gpointer        ident;
  CoglHandle     *texture;
  GDestroyNotify  destroy_func;
} MxTextureCacheMetaEntry;

static MxTextureCacheItem *
mx_texture_cache_item_new (void)
{
  return g_slice_new0 (MxTextureCacheItem);
}

static void
mx_texture_cache_item_free (MxTextureCacheItem *item)
{
  if (item->ptr)
    cogl_handle_unref (item->ptr);

  if (item->meta)
    g_hash_table_unref (item->meta);

  g_slice_free (MxTextureCacheItem, item);
}

static void
mx_texture_cache_set_property (GObject      *object,
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
mx_texture_cache_get_property (GObject    *object,
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
mx_texture_cache_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (mx_texture_cache_parent_class)->dispose)
    G_OBJECT_CLASS (mx_texture_cache_parent_class)->dispose (object);
}

static void
mx_texture_cache_finalize (GObject *object)
{
  MxTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(object);

  if (priv->cache)
    g_hash_table_unref (priv->cache);

  if (priv->is_uri)
    g_regex_unref (priv->is_uri);

  G_OBJECT_CLASS (mx_texture_cache_parent_class)->finalize (object);
}

static void
mx_texture_cache_class_init (MxTextureCacheClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxTextureCachePrivate));

  object_class->get_property = mx_texture_cache_get_property;
  object_class->set_property = mx_texture_cache_set_property;
  object_class->dispose = mx_texture_cache_dispose;
  object_class->finalize = mx_texture_cache_finalize;

}

static void
mx_texture_cache_init (MxTextureCache *self)
{
  GError *error = NULL;
  MxTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  priv->cache =
    g_hash_table_new_full (g_str_hash, g_str_equal,
                           g_free, (GDestroyNotify)mx_texture_cache_item_free);

  priv->is_uri = g_regex_new ("^([a-zA-Z0-9+.-]+)://.*",
                              G_REGEX_OPTIMIZE, 0, &error);
  if (!priv->is_uri)
    g_error (G_STRLOC ": Unable to compile regex: %s", error->message);
}

/**
 * mx_texture_cache_get_default:
 *
 * Returns the default texture cache. This is owned by Mx and should not be
 * unreferenced or freed.
 *
 * Returns: (transfer none): a MxTextureCache
 */
MxTextureCache*
mx_texture_cache_get_default (void)
{
  if (G_UNLIKELY (__cache_singleton == NULL))
    __cache_singleton = g_object_new (MX_TYPE_TEXTURE_CACHE, NULL);

  return __cache_singleton;
}

#if 0
static void
on_texure_finalized (gpointer data,
                     GObject *where_the_object_was)
{
  FinalizedClosure *closure = (FinalizedClosure *) data;
  MxTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(closure->cache);

  g_hash_table_remove (priv->cache, closure->uri);

  g_free(closure->uri);
  g_free(closure);
}
#endif

/**
 * mx_texture_cache_get_size:
 * @self: A #MxTextureCache
 *
 * Returns the number of items in the texture cache
 *
 * Returns: the current size of the cache
 */
gint
mx_texture_cache_get_size (MxTextureCache *self)
{
  MxTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  return g_hash_table_size (priv->cache);
}

static void
add_texture_to_cache (MxTextureCache     *self,
                      const gchar        *uri,
                      MxTextureCacheItem *item)
{
  /*  FinalizedClosure        *closure; */
  MxTextureCachePrivate *priv = TEXTURE_CACHE_PRIVATE(self);

  g_hash_table_insert (priv->cache, g_strdup (uri), item);

#if 0
  /* Make sure we can remove from hash */
  closure = g_new0 (FinalizedClosure, 1);
  closure->uri = g_strdup (uri);
  closure->cache = self;

  g_object_weak_ref (G_OBJECT (res), on_texure_finalized, closure);
#endif
}

/* NOTE: you should unref the returned texture when not needed */

static gchar *
mx_texture_cache_resolve_relative_path (const gchar *path)
{
  gchar *cwd, *new_path;

  if (g_path_is_absolute (path))
    return NULL;

  cwd = g_get_current_dir ();
  new_path = g_build_filename (cwd, path, NULL);
  g_free (cwd);

  return new_path;
}

static gchar *
mx_texture_cache_filename_to_uri (const gchar *file)
{
  gchar *uri;
  gchar *new_file;
  GError *error = NULL;

  new_file = mx_texture_cache_resolve_relative_path (file);
  if (new_file)
    {
      uri = g_filename_to_uri (new_file, NULL, &error);
      g_free (new_file);
    }
  else
    uri = g_filename_to_uri (file, NULL, &error);

  if (!uri)
    {
      g_warning ("Unable to transform filename to URI: %s",
                 error->message);
      g_error_free (error);
      return NULL;
    }

  return uri;
}

static gchar *
mx_texture_cache_uri_to_filename (const gchar *uri)
{
  GError *error = NULL;
  gchar *file = g_filename_from_uri (uri, NULL, &error);

  if (!file)
    g_warning (G_STRLOC ": Unable to transform URI to filename: %s",
               error->message);

  return file;
}

static MxTextureCacheItem *
mx_texture_cache_get_item (MxTextureCache *self,
                           const gchar    *uri,
                           gboolean        create_if_not_exists)
{
  MxTextureCachePrivate *priv;
  MxTextureCacheItem *item;
  gchar *new_file, *new_uri;
  const gchar *file = NULL;

  priv = TEXTURE_CACHE_PRIVATE (self);

  /* Make sure we have the URI (and the path if we're loading) */
  new_file = new_uri = NULL;

  if (g_regex_match (priv->is_uri, uri, 0, NULL))
    {
      if (create_if_not_exists)
        {
          file = new_file = mx_texture_cache_uri_to_filename (uri);
          if (!new_file)
            return NULL;
        }
    }
  else
    {
      file = uri;
      uri = new_uri = mx_texture_cache_filename_to_uri (file);
      if (!new_uri)
        return NULL;
    }

  item = g_hash_table_lookup (priv->cache, uri);

  if ((!item || !item->ptr) && create_if_not_exists)
    {
      gboolean created;
      GError *err = NULL;

      if (!item)
        {
          item = mx_texture_cache_item_new ();
          created = TRUE;
        }
      else
        created = FALSE;

      item->ptr = cogl_texture_new_from_file (file, COGL_TEXTURE_NONE,
                                              COGL_PIXEL_FORMAT_ANY,
                                              &err);

      if (!item->ptr)
        {
          if (err)
            {
              g_warning ("Error loading image: %s", err->message);
              g_error_free (err);
            }

          if (created)
            mx_texture_cache_item_free (item);

          g_free (new_file);
          g_free (new_uri);

          return NULL;
        }

      if (created)
        add_texture_to_cache (self, uri, item);
    }

  g_free (new_file);
  g_free (new_uri);

  return item;
}

/**
 * mx_texture_cache_get_cogl_texture:
 * @self: A #MxTextureCache
 * @uri: A URI or path to an image file
 *
 * Create a #CoglHandle representing a texture of the specified image. Adds
 * the image to the cache if the image had not been previously loaded.
 * Subsequent calls with the same image URI/path will return the #CoglHandle of
 * the previously loaded image with an increased reference count.
 *
 * Returns: (transfer none): a #CoglHandle to the cached texture
 */
CoglHandle
mx_texture_cache_get_cogl_texture (MxTextureCache *self,
                                   const gchar    *uri)
{
  MxTextureCacheItem *item;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  item = mx_texture_cache_get_item (self, uri, TRUE);

  if (item)
    return cogl_handle_ref (item->ptr);
  else
    return NULL;
}

/**
 * mx_texture_cache_get_texture:
 * @self: A #MxTextureCache
 * @uri: A URI or path to a image file
 *
 * Create a new ClutterTexture with the specified image. Adds the image to the
 * cache if the image had not been previously loaded. Subsequent calls with
 * the same image URI/path will return a new ClutterTexture with the previously
 * loaded image.
 *
 * Returns: (transfer none): a newly created ClutterTexture
 */
ClutterTexture*
mx_texture_cache_get_texture (MxTextureCache *self,
                              const gchar    *uri)
{
  MxTextureCacheItem *item;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  item = mx_texture_cache_get_item (self, uri, TRUE);

  if (item)
    {
      ClutterActor *texture = clutter_texture_new ();
      clutter_texture_set_cogl_texture ((ClutterTexture*) texture, item->ptr);

      return (ClutterTexture *)texture;
    }
  else
    return NULL;
}


/**
 * mx_texture_cache_get_actor:
 * @self: A #MxTextureCache
 * @uri: A URI or path to a image file
 *
 * This is a wrapper around mx_texture_cache_get_texture() which returns
 * a ClutterActor.
 *
 * Returns: (transfer none): a newly created ClutterTexture
 */
ClutterActor*
mx_texture_cache_get_actor (MxTextureCache *self,
                            const gchar    *uri)
{
  ClutterTexture *tex;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  if ((tex = mx_texture_cache_get_texture (self, uri)))
    return CLUTTER_ACTOR (tex);
  else
    return NULL;
}

/**
 * mx_texture_cache_get_meta_texture:
 * @self: A #MxTextureCache
 * @uri: A URI or path to an image file
 * @ident: A unique identifier
 *
 * Create a new ClutterTexture using the previously added image associated
 * with the given unique identifier.
 *
 * See mx_texture_cache_insert_meta()
 *
 * Returns: (transfer full): A newly allocated #ClutterTexture, or
 *   %NULL if no image was found
 *
 * Since: 1.2
 */
ClutterTexture *
mx_texture_cache_get_meta_texture (MxTextureCache *self,
                                   const gchar    *uri,
                                   gpointer        ident)
{
  MxTextureCacheItem *item;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  item = mx_texture_cache_get_item (self, uri, TRUE);

  if (item && item->meta)
    {
      MxTextureCacheMetaEntry *entry = g_hash_table_lookup (item->meta, ident);

      if (entry->texture)
        {
          ClutterActor *texture = clutter_texture_new ();
          clutter_texture_set_cogl_texture ((ClutterTexture*) texture,
                                            entry->texture);
          return (ClutterTexture *)texture;
        }
    }

  return NULL;
}

/**
 * mx_texture_cache_get_meta_cogl_texture:
 * @self: A #MxTextureCache
 * @uri: A URI or path to an image file
 * @ident: A unique identifier
 *
 * Retrieves the #CoglHandle of the previously added image associated
 * with the given unique identifier.
 *
 * See mx_texture_cache_insert_meta()
 *
 * Returns: (transfer full): A #CoglHandle to a texture, with an added
 *   reference. %NULL if no image was found.
 *
 * Since: 1.2
 */
CoglHandle
mx_texture_cache_get_meta_cogl_texture (MxTextureCache *self,
                                        const gchar    *uri,
                                        gpointer        ident)
{
  MxTextureCacheItem *item;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  item = mx_texture_cache_get_item (self, uri, TRUE);

  if (item && item->meta)
    {
      MxTextureCacheMetaEntry *entry = g_hash_table_lookup (item->meta, ident);

      if (entry->texture)
        return cogl_handle_ref (entry->texture);
    }

  return NULL;
}

/**
 * mx_texture_cache_contains:
 * @self: A #MxTextureCache
 * @uri: A URI or path to an image file
 *
 * Checks whether the given URI/path is contained within the texture
 * cache.
 *
 * Returns: %TRUE if the image exists, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_texture_cache_contains (MxTextureCache *self,
                           const gchar    *uri)
{
  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  return mx_texture_cache_get_item (self, uri, FALSE) ? TRUE : FALSE;
}

/**
 * mx_texture_cache_contains_meta:
 * @self: A #MxTextureCache
 * @uri: A URI or path to an image file
 * @ident: A unique identifier
 *
 * Checks whether there are any textures associated with the given URI by
 * the given identifier.
 *
 * Returns: %TRUE if the data exists, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_texture_cache_contains_meta (MxTextureCache *self,
                                const gchar    *uri,
                                gpointer        ident)
{
  MxTextureCacheItem *item;

  g_return_val_if_fail (MX_IS_TEXTURE_CACHE (self), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  item = mx_texture_cache_get_item (self, uri, FALSE);

  if (item && item->meta &&
      g_hash_table_lookup (item->meta, ident))
    return TRUE;
  else
    return FALSE;
}

/**
 * mx_texture_cache_insert:
 * @self: A #MxTextureCache
 * @uri: A URI or local file path
 * @texture: A #CoglHandle to a texture
 *
 * Inserts a texture into the texture cache. This can be useful if you
 * want to cache a texture from a custom or unhandled URI type, or you
 * want to override a particular texture.
 *
 * If the image is already in the cache, this texture will replace it. A
 * reference will be taken on the given texture.
 *
 * Since: 1.2
 */
void
mx_texture_cache_insert (MxTextureCache *self,
                         const gchar    *uri,
                         CoglHandle     *texture)
{
  gchar *new_uri = NULL;
  MxTextureCacheItem *item;
  MxTextureCachePrivate *priv;

  g_return_if_fail (MX_IS_TEXTURE_CACHE (self));
  g_return_if_fail (uri != NULL);
  g_return_if_fail (cogl_is_texture (texture));

  priv = TEXTURE_CACHE_PRIVATE (self);

  /* Transform path to URI, if necessary */
  if (!g_regex_match (priv->is_uri, uri, 0, NULL))
    {
      uri = new_uri = mx_texture_cache_filename_to_uri (uri);
      if (!new_uri)
        return;
    }

  item = mx_texture_cache_item_new ();
  item->ptr = cogl_handle_ref (texture);
  add_texture_to_cache (self, uri, item);

  g_free (new_uri);
}

static void
mx_texture_cache_destroy_meta_entry (gpointer data)
{
  MxTextureCacheMetaEntry *entry = data;

  if (entry->destroy_func)
    entry->destroy_func (entry->ident);

  if (entry->texture)
    cogl_handle_unref (entry->texture);

  g_slice_free (MxTextureCacheMetaEntry, entry);
}

/**
 * mx_texture_cache_insert_meta:
 * @self: A #MxTextureCache
 * @uri: A URI or local file path
 * @ident: A unique identifier
 * @texture: A #CoglHandle to a texture
 * @destroy_func: An optional destruction function for @ident
 *
 * Inserts a texture that's associated with a URI into the cache.
 * If the metadata already exists for this URI, it will be replaced.
 *
 * This is useful if you have a widely used modification of an image,
 * for example, an image with a border composited around it.
 *
 * Since: 1.2
 */
void
mx_texture_cache_insert_meta (MxTextureCache *self,
                              const gchar    *uri,
                              gpointer        ident,
                              CoglHandle     *texture,
                              GDestroyNotify  destroy_func)
{
  gchar *new_uri = NULL;
  MxTextureCacheItem *item;
  MxTextureCachePrivate *priv;
  MxTextureCacheMetaEntry *entry;

  g_return_if_fail (MX_IS_TEXTURE_CACHE (self));
  g_return_if_fail (uri != NULL);
  g_return_if_fail (cogl_is_texture (texture));

  priv = TEXTURE_CACHE_PRIVATE (self);

  /* Transform path to URI, if necessary */
  if (!g_regex_match (priv->is_uri, uri, 0, NULL))
    {
      uri = new_uri = mx_texture_cache_filename_to_uri (uri);
      if (!new_uri)
        return;
    }

  item = mx_texture_cache_get_item (self, uri, FALSE);
  if (!item)
    {
      item = mx_texture_cache_item_new ();
      add_texture_to_cache (self, uri, item);
    }

  g_free (new_uri);

  if (!item->meta)
    item->meta = g_hash_table_new_full (NULL, NULL, NULL,
                                        mx_texture_cache_destroy_meta_entry);

  entry = g_slice_new0 (MxTextureCacheMetaEntry);
  entry->ident = ident;
  entry->texture = cogl_handle_ref (texture);
  entry->destroy_func = destroy_func;

  g_hash_table_insert (item->meta, ident, entry);
}

void
mx_texture_cache_load_cache (MxTextureCache *self,
                             const gchar    *filename)
{
  FILE *file;
  MxTextureCacheItem *element, head;
  int ret;
  CoglHandle full_texture;
  MxTextureCachePrivate *priv;

  g_return_if_fail (MX_IS_TEXTURE_CACHE (self));
  g_return_if_fail (filename != NULL);

  priv = TEXTURE_CACHE_PRIVATE (self);

  file = fopen(filename, "rm");
  if (!file)
    return;

  ret = fread (&head, sizeof(MxTextureCacheItem), 1, file);
  if (ret < 0)
    {
      fclose (file);
      return;
    }

  /* check if we already if this texture in the cache */
  if (g_hash_table_lookup (priv->cache, head.filename))
    {
      /* skip it, we're done */
      fclose (file);
      return;
    }

  full_texture = mx_texture_cache_get_cogl_texture (self, head.filename);

  if (full_texture == COGL_INVALID_HANDLE)
    {
      g_critical (G_STRLOC ": Error opening cache image file");
      fclose (file);
      return;
    }

  while (!feof (file))
    {
      gchar *uri;

      element = mx_texture_cache_item_new ();
      ret = fread (element, sizeof (MxTextureCacheItem), 1, file);

      if (ret < 1)
        {
          /* end of file */
          mx_texture_cache_item_free (element);
          break;
        }

      uri = mx_texture_cache_filename_to_uri (element->filename);
      if (!uri)
        {
          /* Couldn't resolve path */
          mx_texture_cache_item_free (element);
          continue;
        }

      if (g_hash_table_lookup (priv->cache, uri))
        {
          /* URI is already in the cache.... */
          mx_texture_cache_item_free (element);
          g_free (uri);
        }
      else
        {
          element->ptr = cogl_texture_new_from_sub_texture (full_texture,
                                                            element->posX,
                                                            element->posY,
                                                            element->width,
                                                            element->height);
          g_hash_table_insert (priv->cache, uri, element);
        }
    }

  fclose (file);
}
