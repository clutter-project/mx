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

enum
{
  CHANGED,

  LAST_SIGNAL
};

#define MX_STYLE_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_STYLE, MxStylePrivate))

#define MX_STYLE_ERROR g_style_error_quark ()

typedef struct {
  GType value_type;
  gchar *value_name;
  GValue value;
} StyleProperty;

struct _MxStylePrivate
{
  MxStyleSheet *stylesheet;
  GList *image_paths;

  GHashTable *style_hash;
  GHashTable *node_hash;
};

static guint style_signals[LAST_SIGNAL] = { 0, };

static MxStyle *default_style = NULL;

G_DEFINE_TYPE (MxStyle, mx_style, G_TYPE_OBJECT);

static GQuark
g_style_error_quark (void)
{
  return g_quark_from_static_string ("mx-style-error-quark");
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

static void
mx_style_finalize (GObject *gobject)
{
  MxStylePrivate *priv = ((MxStyle *)gobject)->priv;
  GList *l;

  for (l = priv->image_paths; l; l = g_list_delete_link (l, l))
  {
    g_free (l->data);
  }

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
 * Returns: a #MxStyle object. This must not be freed or unref'd by
 * applications
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
        g_value_set_int (value, atoi (css_value->string));
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

  if (strstr (name, "x-mx") == name)
    return &name[1];
  else
    return name;
}

/**
 * mx_style_get_property:
 * @style: the style data store object
 * @stylable: a stylable to retreive the data for
 * @pspec: a #GParamSpec describing the property required
 * @value: a #GValue to place the return value in
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

      properties = mx_style_sheet_get_properties (priv->stylesheet, stylable);

      css_value = g_hash_table_lookup (properties,
                                       mx_style_normalize_property_name (pspec->name));

      if (!css_value)
        {
          /* get the default value */
          g_value_init (value, G_PARAM_SPEC_VALUE_TYPE (pspec));
          g_param_value_set_default (pspec, value);
        }
      else
        mx_style_transform_css_value (css_value, stylable, pspec, value);

      g_hash_table_destroy (properties);
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

      properties = mx_style_sheet_get_properties (priv->stylesheet, stylable);

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
              /* get the default value */
              g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
              g_param_value_set_default (pspec, &value);
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

