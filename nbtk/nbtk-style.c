#ifndef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include <clutter/clutter-behaviour.h>
#include <clutter/clutter-color.h>

#include <ccss/ccss.h>

#include "nbtk-style.h"
#include "nbtk-marshal.h"

enum
{
  CHANGED,

  LAST_SIGNAL
};

#define NBTK_STYLE_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NBTK_TYPE_STYLE, NbtkStylePrivate))

#define NBTK_STYLE_ERROR g_style_error_quark ()

typedef struct {
  GType value_type;
  gchar *value_name;
  GValue value;
} StyleProperty;

struct _NbtkStylePrivate
{
  ccss_stylesheet_t *stylesheet;
  gchar **image_paths;
};

typedef struct {
  ccss_node_t parent;
  NbtkStylable *stylable;
  NbtkStylableIface *iface;
} nbtk_style_node_t;

static guint style_signals[LAST_SIGNAL] = { 0, };

static NbtkStyle *default_style = NULL;

G_DEFINE_TYPE (NbtkStyle, nbtk_style, G_TYPE_OBJECT);


GQuark
g_style_error_quark (void)
{
  return g_quark_from_static_string ("nbtk-style-error-quark");
}

/**
 * nbtk_style_load_from_file:
 * @style: a #NbtkStyle
 * @filename: filename of the style sheet to load
 * @error: a #GError or #NULL
 *
 * Load style information from the specified file.
 *
 * returns: TRUE if the style information was loaded successfully. Returns
 * FALSE on error.
 */

gboolean
nbtk_style_load_from_file (NbtkStyle    *style,
                           const gchar  *filename,
                           GError      **error)
{
  NbtkStylePrivate *priv;
  GError *internal_error;  
  gint length;
  gchar *path;


  g_return_val_if_fail (NBTK_IS_STYLE (style), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  priv = NBTK_STYLE_GET_PRIVATE (style);

  if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
      internal_error = g_error_new (NBTK_STYLE_ERROR,
                                    NBTK_STYLE_ERROR_INVALID_FILE,
                                    "Invalid theme file '%s'", filename);
      g_propagate_error (error, internal_error);
      return FALSE;
    }


  /* add the path of the stylesheet to the search path */
  path = g_path_get_dirname (filename);

  /* make sure path is valid */
  if (!path)
    return TRUE;

  if (!priv->image_paths)
    length = 0;
  else
    {
      /* check we don't have this path already */
      gchar **s;

      length = g_strv_length (priv->image_paths);

      for (s = priv->image_paths; *s; s++)
        {
          if (g_str_equal (*s, path))
              break;
        }

      if (*s)
        {
          /* we have this path already */
          g_free (path);
          return TRUE;
        }
    }

  length++;

  priv->image_paths = g_realloc (priv->image_paths, length + 1);

  priv->image_paths[length - 1] = path;
  priv->image_paths[length] = NULL;

  /* now load the stylesheet */
  priv->stylesheet = ccss_stylesheet_new_from_file (filename);

  return TRUE;
}

static void
nbtk_style_load (NbtkStyle *style)
{
  const gchar *env_var;
  gchar *rc_file = NULL;
  GError *error;
  
  env_var = g_getenv ("NBTK_RC_FILE");
  if (env_var && *env_var)
    rc_file = g_strdup (env_var);
  
  if (!rc_file)
    rc_file = g_build_filename (g_get_user_config_dir (),
                                "nbtk",
                                "default.css",
                                NULL);

  error = NULL;

  if (g_file_test (rc_file, G_FILE_TEST_EXISTS))
    {
      if (!nbtk_style_load_from_file (style, rc_file, &error))
        {
          g_critical ("Unable to load resource file `%s': %s",
                      rc_file,
                      error->message);
          g_error_free (error);
        }
    }
  
  g_free (rc_file);
}

static void
nbtk_style_finalize (GObject *gobject)
{
  G_OBJECT_CLASS (nbtk_style_parent_class)->finalize (gobject);
}

static void
nbtk_style_class_init (NbtkStyleClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NbtkStylePrivate));

  gobject_class->finalize = nbtk_style_finalize;

  style_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (NbtkStyleClass, changed),
                  NULL, NULL,
                  _nbtk_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

/* url loader for libccss */
static char *
ccss_url (ccss_block_t  *block,
          char const    *property_name,
          char const    *function_name,
          GSList const  *args)
{
  const gchar *given_path, *filename;
  gchar *test_path;
  gchar **s;

  g_return_val_if_fail (args, NULL);

  given_path = (char const *) args->data;

  /* we can only deal with local paths */
  if (!g_str_has_prefix (given_path, "file://"))
    return NULL;
  filename = &given_path[7];

  /* first try looking in the theme dir */
  test_path = g_build_filename (g_get_user_config_dir (),
                                "nbtk",
                                filename,
                                NULL);
  if (g_file_test (test_path, G_FILE_TEST_IS_REGULAR))
    return test_path;
  g_free (test_path);

  /* we can only check the default style right now due no user-data in this
   * callback
   */
  if (default_style)
  {
    if (default_style->priv->image_paths)
      {
        for (s = default_style->priv->image_paths; *s; s++)
          {
            test_path = g_build_filename (*s, filename, NULL);

            if (g_file_test (test_path, G_FILE_TEST_IS_REGULAR))
              return test_path;

            g_free (test_path);
          }
      }
  }


  /* couldn't find the image anywhere, so just return the filename */
  return strdup (given_path);
}

static ccss_function_t const ccss_functions[] = 
{
  { "url", ccss_url },
  { NULL }
};


static void
nbtk_style_init (NbtkStyle *style)
{
  NbtkStylePrivate *priv;
  static gboolean inited = FALSE;

  style->priv = priv = NBTK_STYLE_GET_PRIVATE (style);

  if (!inited)
  {
    inited = TRUE;
    ccss_init (NULL, ccss_functions);
  }

  nbtk_style_load (style);
}

/* need to unref */
NbtkStyle *
nbtk_style_new (void)
{
  return g_object_new (NBTK_TYPE_STYLE, NULL);
}

/* never ref/unref */
NbtkStyle *
nbtk_style_get_default (void)
{
  if (G_LIKELY (default_style))
    return default_style;

  default_style = g_object_new (NBTK_TYPE_STYLE, NULL);
  
  return default_style;
}

/* functions for ccss */

static const gchar*
get_style_id (nbtk_style_node_t *node)
{
  return nbtk_stylable_get_style_id (node->stylable);
}

static const gchar*
get_style_type (nbtk_style_node_t *node)
{
  return nbtk_stylable_get_style_type (node->stylable);
}

static const gchar*
get_style_class (nbtk_style_node_t *node)
{
  return nbtk_stylable_get_style_class (node->stylable);
}

static const gchar*
get_pseudo_class (nbtk_style_node_t *node)
{
  return nbtk_stylable_get_pseudo_class (node->stylable);
}

const gchar*
get_attribute (nbtk_style_node_t *node, const char *name)
{
  return nbtk_stylable_get_attribute (node->stylable, name);
}

void
nbtk_style_get_property (NbtkStyle    *style,
                         NbtkStylable *stylable,
                         GParamSpec   *pspec,
                         GValue       *value)
{
  NbtkStylePrivate *priv = style->priv;
  GValue real_value = { 0, };
  gboolean value_set = FALSE;

  g_return_if_fail (NBTK_IS_STYLE (style));
  g_return_if_fail (NBTK_IS_STYLABLE (stylable));
  g_return_if_fail (pspec != NULL);
  g_return_if_fail (value != NULL);

  g_value_init (&real_value, G_PARAM_SPEC_VALUE_TYPE (pspec));

  /* look up the property in the css */
  if (priv->stylesheet)
    {
      NbtkStylableIface *iface = NBTK_STYLABLE_GET_IFACE (stylable);
      ccss_style_t *ccss_style;
      nbtk_style_node_t *ccss_node;
      ccss_node_class_t ccss_node_class = {
        .is_a             = NULL,
        .get_container    = NULL,//(ccss_node_get_container_f) get_container,
        .get_id           = (ccss_node_get_id_f) get_style_id,
        .get_type         = (ccss_node_get_type_f) get_style_type,
        .get_class        = (ccss_node_get_class_f) get_style_class,
        .get_pseudo_class = (ccss_node_get_pseudo_class_f) get_pseudo_class,
        .get_viewport     = NULL,// (ccss_node_get_viewport_f) get_viewport,
        .get_attribute    = (ccss_node_get_attribute_f) get_attribute,
        .release          = NULL,
      };

      ccss_node = g_new0 (nbtk_style_node_t, 1);
      ccss_node->iface = iface;
      ccss_node->stylable = stylable;
      ccss_node_init ((ccss_node_t*) ccss_node, &ccss_node_class);

      ccss_style = ccss_style_new ();

      if (ccss_stylesheet_query (priv->stylesheet, (ccss_node_t*) ccss_node, ccss_style))
        {
          if (G_PARAM_SPEC_VALUE_TYPE (pspec))
            {
              gchar *string = NULL;

              ccss_style_get_string (ccss_style, pspec->name, &string);

              if (string)
                {
                  if (CLUTTER_IS_PARAM_SPEC_COLOR (pspec))
                    {
                      ClutterColor color;
                      clutter_color_parse (string, &color);
                      clutter_value_set_color (&real_value, &color);
                      value_set = TRUE;
                    }
                  else
                    if (G_IS_PARAM_SPEC_STRING (pspec))
                      {
                        g_value_set_string (&real_value, string);
                        value_set = TRUE;
                      }
                  g_free (string);
                }
            }
        }
      ccss_style_free (ccss_style);
      g_free (ccss_node);
    }

  /* no value was found in css, so copy in the default value */
  if (!value_set)
    g_param_value_set_default (pspec, &real_value);

  g_value_init (value, G_VALUE_TYPE (&real_value));
  g_value_copy (&real_value, value);

  g_value_unset (&real_value);
}

void
nbtk_style_set_property (NbtkStyle    *style,
                         const gchar  *property_name,
                         const GValue *value)
{
  /*
  StyleProperty *property;

  g_return_if_fail (NBTK_IS_STYLE (style));
  g_return_if_fail (property_name != NULL);
  g_return_if_fail (value != NULL);

  property = nbtk_style_find_property (style, property_name);
  if (!property)
    {
      g_warning ("No style property named `%s' found.", property_name);
      return;
    }

  g_value_copy (value, &property->value);

  g_signal_emit (style, style_signals[CHANGED], 0);
   */
  g_warning ("nbtk_style_set_property() not yet implemented");
}
