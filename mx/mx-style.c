/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
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
 *
 */

/**
 * SECTION:mx-style
 * @short_description: a data store for style properties
 *
 * #MxStyle is a property data store that can read properties from a style
 * sheet. It is queried with objects that implement the MxStylable
 * interface.
 */


#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include "mx-stylable.h"
#include "mx-css.h"

#include "mx-marshal.h"
#include "mx-style.h"
#include "mx-enum-types.h"
#include "mx-types.h"
#include "mx-private.h"

enum
{
  CHANGED,

  LAST_SIGNAL
};

#define MX_STYLE_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_STYLE, MxStylePrivate))

#define MX_STYLE_ERROR g_style_error_quark ()

#define MX_STYLE_CACHE g_style_cache_quark ()

/* This is the amount of entries that will be allowed per
 * stylable object.
 *
 * In the usual case, objects share rules and
 * so we don't really need to allocate more than one space in
 * the cache per object, but in case of the pathological case
 * of every stylable object having a unique style rule that
 * applies only to itself, we allow 6 per object.
 *
 * e.g. normal, hover, active, checked, focus + 1 extra
 */
#define MX_STYLE_CACHE_SIZE 6

/* A style cache entry is the unique string representing all the properties
 * that can be matched against in CSS, and the matched properties themselves.
 */
typedef struct
{
  gchar      *style_string;
  gint        age;
  GHashTable *properties;
} MxStyleCacheEntry;

/* This is the per-stylable cache store. We need a reference back to the
 * parent style so that we can maintain the count of alive stylables.
 */
typedef struct
{
  GList   *styles;
  gchar   *string;
} MxStylableCache;

typedef struct {
  GType value_type;
  gchar *value_name;
  GValue value;
} StyleProperty;

struct _MxStylePrivate
{
  MxStyleSheet *stylesheet;

  GHashTable *style_hash;
  GHashTable *node_hash;

  gint        alive_stylables;
  GQueue     *cached_matches;
  GHashTable *cache_hash;
  gint        age;
};

static guint style_signals[LAST_SIGNAL] = { 0, };

static MxStyle *default_style = NULL;

G_DEFINE_TYPE (MxStyle, mx_style, G_TYPE_OBJECT);

static GQuark
g_style_error_quark (void)
{
  return g_quark_from_static_string ("mx-style-error-quark");
}

static GQuark
g_style_cache_quark (void)
{
  return g_quark_from_static_string ("mx-style-cache-quark");
}

static gboolean
mx_style_real_load_from_file (MxStyle    *style,
                                const gchar  *filename,
                                GError      **error,
                                gint          priority)
{
  MxStylePrivate *priv;
  GError *internal_error;

  g_return_val_if_fail (MX_IS_STYLE (style), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  priv = MX_STYLE (style)->priv;

  if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
      internal_error = g_error_new (MX_STYLE_ERROR,
                                    MX_STYLE_ERROR_INVALID_FILE,
                                    "Invalid theme file '%s'", filename);
      g_propagate_error (error, internal_error);
      return FALSE;
    }

  if (!priv->stylesheet)
    priv->stylesheet = mx_style_sheet_new ();

  mx_style_sheet_add_from_file (priv->stylesheet, filename, NULL);

  /* Increment the age so we know if a style cache entry is valid */
  priv->age ++;

  g_signal_emit (style, style_signals[CHANGED], 0, NULL);

  return TRUE;
}

/**
 * mx_style_load_from_file:
 * @style: a #MxStyle
 * @filename: filename of the style sheet to load
 * @error: a #GError or #NULL
 *
 * Load style information from the specified file.
 *
 * returns: TRUE if the style information was loaded successfully. Returns
 * FALSE on error.
 */
gboolean
mx_style_load_from_file (MxStyle    *style,
                           const gchar  *filename,
                           GError      **error)
{
  return mx_style_real_load_from_file (style, filename, error, 0);
}

static void
mx_style_load (MxStyle *style)
{
  const gchar *env_var;
  gchar *rc_file = NULL;
  GError *error;

  env_var = g_getenv ("MX_RC_FILE");
  if (env_var && *env_var)
    rc_file = g_strdup (env_var);

  if (!rc_file)
    rc_file = g_build_filename (PACKAGE_DATA_DIR,
                                "mx",
                                "style",
                                "default.css",
                                NULL);

  error = NULL;

  if (g_file_test (rc_file, G_FILE_TEST_EXISTS))
    {
      /* load the default theme with lowest priority */
      if (!mx_style_real_load_from_file (style, rc_file, &error, 0))
        {
          g_critical ("Unable to load resource file '%s': %s",
                      rc_file,
                      error->message);
          g_error_free (error);
        }
    }

  g_free (rc_file);
}

static MxStyleCacheEntry *
mx_style_cache_entry_new (const gchar *style_string,
                          GHashTable  *properties,
                          gint         age)
{
  MxStyleCacheEntry *entry = g_slice_new (MxStyleCacheEntry);

  entry->style_string = g_strdup (style_string);
  entry->properties = properties;
  entry->age = age;

  return entry;
}

static void
mx_style_cache_entry_free (MxStyleCacheEntry *entry,
                           gboolean           free_struct)
{
  g_free (entry->style_string);
  g_hash_table_unref (entry->properties);
  if (free_struct)
    g_slice_free (MxStyleCacheEntry, entry);
}

static void
mx_style_finalize (GObject *gobject)
{
  MxStylePrivate *priv = MX_STYLE (gobject)->priv;

  g_hash_table_unref (priv->cache_hash);

  while (g_queue_get_length (priv->cached_matches))
    mx_style_cache_entry_free (g_queue_pop_head (priv->cached_matches), TRUE);
  g_queue_free (priv->cached_matches);

  G_OBJECT_CLASS (mx_style_parent_class)->finalize (gobject);
}

static void
mx_style_class_init (MxStyleClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxStylePrivate));

  gobject_class->finalize = mx_style_finalize;

  /**
   * MxStyle::changed:
   *
   * Indicates that the style data has changed in some way. For example, a new
   * stylesheet may have been loaded.
   */

  style_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MxStyleClass, changed),
                  NULL, NULL,
                  _mx_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}


static void
mx_style_init (MxStyle *style)
{
  MxStylePrivate *priv;

  style->priv = priv = MX_STYLE_GET_PRIVATE (style);

  priv->cached_matches = g_queue_new ();
  priv->cache_hash = g_hash_table_new (g_str_hash, g_str_equal);

  mx_style_load (style);
}

/**
 * mx_style_new:
 *
 * Creates a new #MxStyle object. This must be freed using #g_object_unref
 * when no longer required.
 *
 * Returns: a newly allocated #MxStyle
 */
MxStyle *
mx_style_new (void)
{
  return g_object_new (MX_TYPE_STYLE, NULL);
}

/**
 * mx_style_get_default:
 *
 * Return the default MxStyle object. This includes the current theme (if
 * any).
 *
 * Returns: (transfer none): a #MxStyle object. This must not be freed or
 * unref'd by applications
 */
MxStyle *
mx_style_get_default (void)
{
  if (G_LIKELY (default_style))
    return default_style;

  default_style = g_object_new (MX_TYPE_STYLE, NULL);

  return default_style;
}


static void
mx_style_transform_css_value (MxStyleSheetValue *css_value,
                              MxStylable        *stylable,
                              GParamSpec        *pspec,
                              GValue            *value)
{
  if (pspec->value_type == G_TYPE_INT)
    {
      g_value_init (value, pspec->value_type);

      if (css_value->string)
        {
          gint number = atoi (css_value->string);

          if (g_str_equal (g_param_spec_get_name (pspec), "font-size") &&
              g_str_has_suffix (css_value->string, "pt"))
            {
              ClutterBackend *backend = clutter_get_default_backend ();
              gdouble res = clutter_backend_get_resolution (backend);
              number = number * res / 72.0;
            }

          g_value_set_int (value, number);
        }
      else
        g_value_set_int (value, 0);
    }
  else if (pspec->value_type == G_TYPE_UINT)
    {
      g_value_init (value, pspec->value_type);

      if (css_value->string)
        g_value_set_uint (value, atoi (css_value->string));
      else
        g_value_set_uint (value, 0);
    }
  else if (pspec->value_type == MX_TYPE_BORDER_IMAGE)
    {
      g_value_init (value, pspec->value_type);

      mx_border_image_set_from_string (value,
                                       css_value->string,
                                       css_value->source);
    }
  else if (pspec->value_type == MX_TYPE_FONT_WEIGHT)
    {
      g_value_init (value, pspec->value_type);

      mx_font_weight_set_from_string (value, css_value->string);
    }
  else if (pspec->value_type == G_TYPE_STRING)
    {
      gchar *stripped, *original;
      gint len;

      g_value_init (value, pspec->value_type);

      if (!g_strcmp0 (css_value->string, "none"))
        {
          g_value_set_string (value, NULL);
          return;
        }


      original = g_strdup (css_value->string);
      len = strlen (original);

      if ((original[0] == '\'' && original[len -1] == '\'')
          || (original[0] == '\"' && original[len -1] == '\"'))
        {
          stripped = original + 1;
          original[len -1] = '\0';
        }
      else
        stripped = original;

      g_value_set_string (value, stripped);

      g_free (original);
    }
  else if (g_type_is_a (pspec->value_type, G_TYPE_ENUM))
    {
      GEnumValue *enum_value;
      GEnumClass *class;

      g_value_init (value, pspec->value_type);


      class = g_type_class_ref (pspec->value_type);

      enum_value = g_enum_get_value_by_nick (class, css_value->string);

      if (!enum_value)
        {
          g_warning ("Error setting property \"%s\" on \"%s\", could"
                     " not transform \"%s\" from string to type %s",
                     pspec->name,
                     G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS (stylable)),
                     css_value->string,
                     g_type_name (pspec->value_type));
        }
      else
        {
          g_value_set_enum (value, enum_value->value);
        }

      g_type_class_unref (class);
    }
  else
    {
      GValue strval = { 0, };

      g_value_init (value, pspec->value_type);

      g_value_init (&strval, G_TYPE_STRING);
      g_value_set_string (&strval, css_value->string);

      if (!g_value_transform (&strval, value))
        {
          g_warning ("Error setting property \"%s\" on \"%s\", could"
                     " not transform \"%s\" from string to type %s",
                     pspec->name,
                     G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS (stylable)),
                     css_value->string,
                     g_type_name (pspec->value_type));
        }
      g_value_unset (&strval);
    }
}


static const gchar*
mx_style_normalize_property_name (const gchar *name)
{
  /* gobject properties cannot start with a '-', but custom CSS properties
   * must be prefixed with '-' + vendor identifier. Therefore, the custom
   * style properties in mx are installed with "x-"
   */

  if (!name)
    return NULL;

  if (strncmp (name, "x-mx", 4) == 0)
    return &name[1];
  else
    return name;
}

static void
mx_style_cache_weak_ref_cb (gpointer  data,
                            GObject  *old_object)
{
  MxStylableCache *cache = data;
  GList *style_link = g_list_find (cache->styles, old_object);

  if (style_link)
    cache->styles = g_list_delete_link (cache->styles, style_link);
  else
    g_warning (G_STRLOC ": Weak unref on a stylable with no style reference");
}

static void
mx_style_stylable_cache_free (MxStylableCache *cache)
{
  /* If there are still styles referencing this stylable, decrement their
   * count of alive stylables and remove the weak reference.
   */
  while (cache->styles)
    {
      MxStyle *style = cache->styles->data;
      MxStylePrivate *priv = style->priv;

      g_object_weak_unref (G_OBJECT (style),
                           mx_style_cache_weak_ref_cb,
                           cache);
      priv->alive_stylables --;

      MX_NOTE (STYLE_CACHE, "(%p) Alive stylables: %d",
               style, priv->alive_stylables);

      cache->styles = g_list_delete_link (cache->styles, cache->styles);
    }

  g_free (cache->string);
  g_slice_free (MxStylableCache, cache);
}

void
_mx_style_invalidate_cache (MxStylable *stylable)
{
  GObject *object = G_OBJECT (stylable);
  MxStylableCache *cache = g_object_get_qdata (object, MX_STYLE_CACHE);

  /* Reset the cache string */
  if (cache)
    {
      g_free (cache->string);
      cache->string = NULL;
    }
}

static GHashTable *
mx_style_get_style_sheet_properties (MxStyle    *style,
                                     MxStylable *stylable)
{
  GList *entry_link;
  MxStylableCache *cache;

  MxStyleCacheEntry *entry = NULL;
  MxStylePrivate *priv = style->priv;

  /* see if we have a cached style and return that if possible */
  cache = g_object_get_qdata (G_OBJECT (stylable), MX_STYLE_CACHE);

  if (cache)
    {
      /* Make sure that the style string is up-to-date. We set this
       * to NULL when invalidating the stylable's cache.
       */
      if (!cache->string)
        cache->string = _mx_stylable_get_style_string (stylable);

      /* Check that the stylable has a reference to us. If the stylable
       * cache struct was created by another style, we need to add ourselves
       * to the list.
       */
      if (!g_list_find (cache->styles, style))
        {
          cache->styles = g_list_prepend (cache->styles, style);
          g_object_weak_ref (G_OBJECT (style), mx_style_cache_weak_ref_cb,
                             cache);
          priv->alive_stylables ++;
        }
    }
  else
    {
      /* This is the first time this stylable has tried to get style
       * properties, initialise a cache.
       */
      cache = g_slice_new0 (MxStylableCache);
      cache->string = _mx_stylable_get_style_string (stylable);
      cache->styles = g_list_prepend (NULL, style);

      /* Increase the alive-stylables count and add a weak reference so we
       * can remove it.
       */
      priv->alive_stylables ++;
      g_object_weak_ref (G_OBJECT (style), mx_style_cache_weak_ref_cb, cache);

      MX_NOTE (STYLE_CACHE, "(%p) Alive stylables: %d",
               style, priv->alive_stylables);

      /* Use qdata to associate the cache entry with the stylable object */
      g_object_set_qdata_full (G_OBJECT (stylable), MX_STYLE_CACHE, cache,
                               (GDestroyNotify)mx_style_stylable_cache_free);
    }

  if ((entry_link = g_hash_table_lookup (priv->cache_hash, cache->string)))
    {
      entry = entry_link->data;

      /* If the entry is old, remove it from the cache */
      if (entry->age != priv->age)
        {
          g_hash_table_remove (priv->cache_hash, entry->style_string);
          g_queue_delete_link (priv->cached_matches, entry_link);
          mx_style_cache_entry_free (entry, TRUE);
          entry = NULL;
        }

      /* As the cache is rarely emptied, FIFO is good enough for the
       * eviction strategy (preferring speedy retrieval to efficient
       * eviction)
       *
       * If we did want LRU though, for example, here would be the place
       * to do it. You could remove the link from the queue using
       * g_queue_unlink() and reinsert it with g_queue_push_head_link()
       */
    }

  /* No cached style properties were found, or the entry found is out of date,
   * so look them up from the style-sheet and (re-)add them to the cache.
   */
  if (!entry || (entry->age != priv->age))
    {
      /* Look up style properties */
      GHashTable *properties = mx_style_sheet_get_properties (priv->stylesheet,
                                                              stylable);

      /* Append this to the style cache */
      entry = mx_style_cache_entry_new (cache->string, properties, priv->age);
      g_queue_push_head (priv->cached_matches, entry);
      g_hash_table_insert (priv->cache_hash, entry->style_string,
                           priv->cached_matches->head);

      /* Shrink the cache if its grown too large */
      while (g_queue_get_length (priv->cached_matches) >
             (priv->alive_stylables * MX_STYLE_CACHE_SIZE))
        {
          MxStyleCacheEntry *old_entry =
            g_queue_pop_tail (priv->cached_matches);

          g_hash_table_remove (priv->cache_hash, old_entry->style_string);
          mx_style_cache_entry_free (old_entry, TRUE);
        }

      MX_NOTE (STYLE_CACHE, "(%p) Cache size: %d, (Max-size: %d)",
               style, g_queue_get_length (priv->cached_matches),
               priv->alive_stylables * MX_STYLE_CACHE_SIZE);
    }

  return entry->properties ? g_hash_table_ref (entry->properties) : NULL;
}

/**
 * mx_style_get_property:
 * @style: the style data store object
 * @stylable: a stylable to retreive the data for
 * @pspec: a #GParamSpec describing the property required
 * @value: (out): a #GValue to place the return value in
 *
 * Requests the property described in @pspec for the specified stylable
 */

void
mx_style_get_property (MxStyle    *style,
                       MxStylable *stylable,
                       GParamSpec *pspec,
                       GValue     *value)
{
  MxStylePrivate *priv;

  g_return_if_fail (MX_IS_STYLE (style));
  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (pspec != NULL);
  g_return_if_fail (value != NULL);

  priv = style->priv;

  /* look up the property in the css */
  if (priv->stylesheet)
    {
      MxStyleSheetValue *css_value;
      GHashTable *properties;

      properties = mx_style_get_style_sheet_properties (style, stylable);

      css_value = g_hash_table_lookup (properties,
                                       mx_style_normalize_property_name (pspec->name));

      if (!css_value)
        {
          mx_stylable_get_default_value (stylable, pspec->name, value);
        }
      else
        mx_style_transform_css_value (css_value, stylable, pspec, value);

      g_hash_table_unref (properties);
    }
}

/**
 * mx_style_get_valist:
 * @style: a #MxStyle
 * @stylable: a #MxStylable
 * @first_property_name: name of the first property to get
 * @va_args: return location for the first property, followed optionally
 *   by more name/return location pairs, followed by %NULL
 *
 * Gets the style properties for @stylable from @style.
 *
 * Please refer to mx_style_get() for further information.
 */
void
mx_style_get_valist (MxStyle     *style,
                     MxStylable  *stylable,
                     const gchar *first_property_name,
                     va_list      va_args)
{
  MxStylePrivate *priv;
  const gchar *name = first_property_name;
  gboolean values_set = FALSE;

  g_return_if_fail (MX_IS_STYLE (style));
  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (style->priv != NULL);

  priv = style->priv;

  /* look up the property in the css */
  if (priv->stylesheet)
    {
      GHashTable *properties;

      properties = mx_style_get_style_sheet_properties (style, stylable);

      while (name)
        {
          GValue value = { 0, };
          GParamSpec *pspec = mx_stylable_find_property (stylable, name);
          gchar *error;
          MxStyleSheetValue *css_value;

          if (!pspec)
            {
              g_critical ("No style property \"%s\" installed on object of"
                          " type \"%s\".", name,
                         G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (stylable)));

              break;
            }

          css_value = g_hash_table_lookup (properties,
                                           mx_style_normalize_property_name (name));

          if (!css_value)
            {
              mx_stylable_get_default_value (stylable, pspec->name, &value);
            }
          else
            mx_style_transform_css_value (css_value, stylable, pspec, &value);

          G_VALUE_LCOPY (&value, va_args, 0, &error);

          if (error)
            {
              g_warning ("%s: %s", G_STRLOC, error);
              g_free (error);
              g_value_unset (&value);
              break;
            }

          g_value_unset (&value);

          name = va_arg (va_args, gchar*);
        }
      values_set = TRUE;

      g_hash_table_unref (properties);
    }

  if (!values_set)
    {
      /* Set the remaining properties to their default values
       * even if broken out of the above loop. */
      while (name)
        {
          GValue value = { 0, };
          gchar *error = NULL;
          mx_stylable_get_default_value (stylable, name, &value);
          G_VALUE_LCOPY (&value, va_args, 0, &error);
          if (error)
            {
              g_warning ("%s: %s", G_STRLOC, error);
              g_free (error);
              g_value_unset (&value);
              break;
            }
          g_value_unset (&value);
          name = va_arg (va_args, gchar*);
        }
    }
}

/**
 * mx_style_get:
 * @style: a #MxStyle
 * @stylable: a #MxStylable
 * @first_property_name: name of the first property to get
 * @Varargs: return location for the first property, followed optionally
 *   by more name/return location pairs, followed by %NULL
 *
 * Gets the style properties for @stylable from @style.
 *
 * In general, a copy is made of the property contents and the caller
 * is responsible for freeing the memory in the appropriate manner for
 * the property type.
 */
void
mx_style_get (MxStyle     *style,
                MxStylable  *stylable,
                const gchar   *first_property_name,
                ...)
{
  va_list va_args;

  g_return_if_fail (MX_IS_STYLE (style));
  g_return_if_fail (first_property_name != NULL);

  va_start (va_args, first_property_name);
  mx_style_get_valist (style, stylable, first_property_name, va_args);
  va_end (va_args);
}

