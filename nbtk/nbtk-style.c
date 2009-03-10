#ifndef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include <clutter/clutter.h>

#include <ccss/ccss.h>

#include "nbtk-stylable.h"
#include "nbtk-style.h"
#include "nbtk-types.h"
#include "nbtk-marshal.h"
#include "nbtk-widget.h"

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
  GList *image_paths;
};

typedef struct {
  ccss_node_t parent;
  NbtkStylable *stylable;
  NbtkStylableIface *iface;
} nbtk_style_node_t;

static ccss_node_class_t * peek_node_class (void);

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
  gchar *path;
  GList *l;

  g_return_val_if_fail (NBTK_IS_STYLE (style), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  priv = NBTK_STYLE (style)->priv;

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

  for (l = priv->image_paths; l; l = l->next)
    {
      if (g_str_equal ((gchar *)l->data, path))
        {
          /* we have this path already */
          g_free (path);
          return TRUE;
        }
    }

  /* Add the new path */
  priv->image_paths = g_list_append (priv->image_paths, path);

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
  NbtkStylePrivate *priv = ((NbtkStyle *)gobject)->priv;
  GList *l;

  for (l = priv->image_paths; l; l = g_list_delete_link (l, l))
  {
    g_free (l->data);
  }

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
  GList *l;

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
    for (l = default_style->priv->image_paths; l; l = l->next)
      {
        test_path = g_build_filename ((gchar *)l->data, filename, NULL);

        if (g_file_test (test_path, G_FILE_TEST_IS_REGULAR))
          return test_path;

        g_free (test_path);
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

static nbtk_style_node_t *
get_container (nbtk_style_node_t *node)
{
  nbtk_style_node_t *container;
  ClutterActor      *parent;

  g_return_val_if_fail (node, NULL);
  g_return_val_if_fail (node->iface, NULL);
  g_return_val_if_fail (node->stylable, NULL);

  parent = clutter_actor_get_parent (CLUTTER_ACTOR (node->stylable));
  while (parent && !NBTK_IS_WIDGET (parent))
    parent = clutter_actor_get_parent (CLUTTER_ACTOR (parent));

  if (!parent)
    return NULL;

  container = g_new0 (nbtk_style_node_t, 1);
  ccss_node_init ((ccss_node_t*) container, peek_node_class ());
  container->iface = node->iface;
  container->stylable = NBTK_STYLABLE (parent);

  return container;
}

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

static const gchar*
get_attribute (nbtk_style_node_t *node, const char *name)
{
  return nbtk_stylable_get_attribute (node->stylable, name);
}

static void
release (nbtk_style_node_t *node)
{
  g_return_if_fail (node);

  g_free (node);
}

static ccss_node_class_t *
peek_node_class (void)
{
  static ccss_node_class_t _node_class = {
    .is_a             = NULL,
    .get_container    = (ccss_node_get_container_f) get_container,
    .get_id           = (ccss_node_get_id_f) get_style_id,
    .get_type         = (ccss_node_get_type_f) get_style_type,
    .get_class        = (ccss_node_get_class_f) get_style_class,
    .get_pseudo_class = (ccss_node_get_pseudo_class_f) get_pseudo_class,
    .get_viewport     = NULL,// (ccss_node_get_viewport_f) get_viewport,
    .get_attribute    = (ccss_node_get_attribute_f) get_attribute,
    .release          = (ccss_node_release_f) release
  };

  return &_node_class;
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

      ccss_node = g_new0 (nbtk_style_node_t, 1);
      ccss_node_init ((ccss_node_t*) ccss_node, peek_node_class ());
      ccss_node->iface = iface;
      ccss_node->stylable = stylable;

      ccss_style = ccss_style_new ();

      if (ccss_stylesheet_query (priv->stylesheet, (ccss_node_t*) ccss_node, ccss_style))
        {
          if (G_PARAM_SPEC_VALUE_TYPE (pspec))
            {
              double number;

              if (G_IS_PARAM_SPEC_INT (pspec))
                {
                  if (ccss_style_get_double (ccss_style, pspec->name, &number))
                    {
                      g_value_set_int (&real_value, (int) number);
                      value_set = TRUE;
                    }
                }
              else if (NBTK_TYPE_BORDER_IMAGE == G_PARAM_SPEC_VALUE_TYPE (pspec) &&
                       0 == g_strcmp0 ("border-image", pspec->name))
                {
                  gpointer css_value;

                  if (ccss_style_get_property (ccss_style,
                                               "border-image",
                                               &css_value))
                    {
                      ccss_property_t *border_image = css_value;
                      if (border_image &&
                          border_image->type != CCSS_PROPERTY_STATE_UNSET &&
                          border_image->type != CCSS_PROPERTY_STATE_NONE)
                        {
                          g_value_set_boxed (&real_value, border_image);
                          value_set = TRUE;
                        }
                    }
                }
              else if (NBTK_TYPE_PADDING == G_PARAM_SPEC_VALUE_TYPE (pspec) &&
                       0 == g_strcmp0 ("padding", pspec->name))
                {
                  NbtkPadding padding = { 0, 0, 0, 0 };
                  gboolean padding_set = 0;

                  if (ccss_style_get_double (ccss_style, "padding-top", &number))
                    {
                      padding.top = CLUTTER_UNITS_FROM_INT ((int) number);
                      padding_set = TRUE;
                    }

                  if (ccss_style_get_double (ccss_style, "padding-right", &number))
                    {
                      padding.right = CLUTTER_UNITS_FROM_INT ((int) number);
                      padding_set = TRUE;
                    }

                  if (ccss_style_get_double (ccss_style, "padding-bottom", &number))
                    {
                      padding.bottom = CLUTTER_UNITS_FROM_INT ((int) number);
                      padding_set = TRUE;
                    }

                  if (ccss_style_get_double (ccss_style, "padding-left", &number))
                    {
                      padding.left = CLUTTER_UNITS_FROM_INT ((int) number);
                      padding_set = TRUE;
                    }

                  if (padding_set)
                    {
                      g_value_set_boxed (&real_value, &padding);
                      value_set = TRUE;
                    }
                }
              else
                {
                  gchar *string = NULL;

                  ccss_style_get_string (ccss_style, pspec->name, &string);

                  if (string)
                    {
                      if (CLUTTER_IS_PARAM_SPEC_COLOR (pspec))
                        {
                          ClutterColor color = { 0, };

                          clutter_color_from_string (&color, string);
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
