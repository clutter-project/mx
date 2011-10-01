/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-stylable.c: Interface for stylable objects
 *
 * Copyright 2008 Intel Corporation
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
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 *             Thomas Wood <thomas@linux.intel.com>
 *
 */

/**
 * SECTION:mx-stylable
 * @short_description: Interface for stylable objects
 *
 * Stylable objects are classes that can have "style properties", that is
 * properties that can be changed by attaching a #MxStyle to them.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <gobject/gobjectnotifyqueue.c>

#include "mx-marshal.h"
#include "mx-private.h"
#include "mx-stylable.h"
#include "mx-settings.h"

enum
{
  STYLE_CHANGED,
#if 0
  STYLE_NOTIFY,
#endif
  CHANGED,

  LAST_SIGNAL
};

static GObjectNotifyContext property_notify_context = { 0, };

static GParamSpecPool *style_property_spec_pool = NULL;

static GQuark quark_real_owner         = 0;
static GQuark quark_style              = 0;

static guint stylable_signals[LAST_SIGNAL] = { 0, };

static void mx_stylable_property_changed_notify (MxStylable *stylable);

static void
mx_stylable_notify_dispatcher (GObject     *gobject,
                               guint        n_pspecs,
                               GParamSpec **pspecs)
{
#if 0
  guint i;

  for (i = 0; i < n_pspecs; i++)
    g_signal_emit (gobject, stylable_signals[STYLE_NOTIFY],
                   g_quark_from_string (pspecs[i]->name),
                   pspecs[i]);
#endif

}

static void
mx_stylable_base_finalize (gpointer g_iface)
{
  GList *list, *node;

  list = g_param_spec_pool_list_owned (style_property_spec_pool,
                                       G_TYPE_FROM_INTERFACE (g_iface));

  for (node = list; node; node = node->next)
    {
      GParamSpec *pspec = node->data;

      g_param_spec_pool_remove (style_property_spec_pool, pspec);
      g_param_spec_unref (pspec);
    }

  g_list_free (list);
}

static void
mx_stylable_base_init (gpointer g_iface)
{
  static gboolean initialised = FALSE;
  GParamSpec *pspec;
  GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);

  if (G_LIKELY (initialised))
    return;

  initialised = TRUE;

  quark_real_owner =
    g_quark_from_static_string ("mx-stylable-real-owner-quark");
  quark_style = g_quark_from_static_string ("mx-stylable-style-quark");

  style_property_spec_pool = g_param_spec_pool_new (FALSE);

  property_notify_context.quark_notify_queue =
    g_quark_from_static_string ("MxStylable-style-property-notify-queue");
  property_notify_context.dispatcher = mx_stylable_notify_dispatcher;

  pspec = g_param_spec_object ("style",
                               "Style",
                               "A style object",
                               MX_TYPE_STYLE,
                               MX_PARAM_READWRITE);
  g_object_interface_install_property (g_iface, pspec);

  pspec = g_param_spec_string ("style-class",
                               "Style Class",
                               "String representation of the item's class",
                               "",
                               MX_PARAM_READWRITE);
  g_object_interface_install_property (g_iface, pspec);

  pspec = g_param_spec_string ("style-pseudo-class",
                               "Style Pseudo Class",
                               "List of pseudo class, such as current state,"
                               "separated by ':'.",
                               "",
                               MX_PARAM_READWRITE);
  g_object_interface_install_property (g_iface, pspec);

  /**
   * MxStylable::style-changed:
   * @stylable: the #MxStylable that received the signal
   * @flags: the #MxStyleChangedFlags associated with the signal
   *
   * The ::style-changed signal is emitted each time one of the style
   * properties have changed.
   */
  stylable_signals[STYLE_CHANGED] =
    g_signal_new (g_intern_static_string ("style-changed"),
                  iface_type,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MxStylableIface, style_changed),
                  NULL, NULL,
                  _mx_marshal_VOID__FLAGS,
                  G_TYPE_NONE, 1, MX_TYPE_STYLE_CHANGED_FLAGS);

#if 0
  stylable_signals[STYLE_NOTIFY] =
    g_signal_new (g_intern_static_string ("style-notify"),
                  iface_type,
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_DETAILED
                  | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (MxStylableIface, style_notify),
                  NULL, NULL,
                  _mx_marshal_VOID__PARAM,
                  G_TYPE_NONE, 1,
                  G_TYPE_PARAM);
#endif
}

GType
mx_stylable_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    {
      GTypeInfo stylable_info = {
        sizeof (MxStylableIface),
        mx_stylable_base_init,
        mx_stylable_base_finalize
      };

      our_type = g_type_register_static (G_TYPE_INTERFACE,
                                         g_intern_static_string ("MxStylable"),
                                         &stylable_info, 0);
    }

  return our_type;
}

static void
_mx_stylable_prepend_style_string (GString    *string,
                                   MxStylable *stylable)
{
  GType type_id;
  gchar *style_string;
  const gchar *type, *id, *class, *pseudo_class;

  type_id = G_OBJECT_CLASS_TYPE (G_OBJECT_GET_CLASS (stylable));
  type = g_type_name (type_id);

  id = clutter_actor_get_name ((ClutterActor*) stylable);

  class = mx_stylable_get_style_class (stylable);

  pseudo_class = mx_stylable_get_style_pseudo_class (stylable);

  style_string = g_strconcat (type,
                              "#", id ? id : "",
                              ".", class ? class : "",
                              ":", pseudo_class ? pseudo_class : "",
                              ">",
                              NULL);
  g_string_prepend (string, style_string);
  g_free (style_string);
}

gchar *
_mx_stylable_get_style_string (MxStylable *stylable)
{
  GType type_id;
  gchar *cstring;
  GString *string;
  const gchar *type, *id, *class, *pseudo_class;
  ClutterActor *actor;
  ClutterActor *parent;

  /* Create a string that contains all the properties of a
   * Stylable that can be matched against in the CSS.
   */
  type_id = G_OBJECT_CLASS_TYPE (G_OBJECT_GET_CLASS (stylable));
  type = g_type_name (type_id);

  actor = CLUTTER_ACTOR (stylable);

  id = clutter_actor_get_name (actor);

  class = mx_stylable_get_style_class (stylable);

  pseudo_class = mx_stylable_get_style_pseudo_class (stylable);

  string = g_string_sized_new (512);

  /* Add the actor hierarchy */
  parent = clutter_actor_get_parent (actor);

  while (parent)
    {
      if (MX_IS_STYLABLE (parent))
        _mx_stylable_prepend_style_string (string, (MxStylable*) parent);

      parent = clutter_actor_get_parent (parent);
    }

  g_string_append_printf (string, "%s#%s.%s:%s",
                          type,
                          id ? id : "",
                          class ? class : "",
                          pseudo_class ? pseudo_class : "");

  cstring = string->str;
  g_string_free (string, FALSE);

  return cstring;
}

#if 0
void
mx_stylable_freeze_notify (MxStylable *stylable)
{
  g_return_if_fail (MX_IS_STYLABLE (stylable));

  g_object_ref (stylable);
  g_object_notify_queue_freeze (G_OBJECT (stylable), &property_notify_context);
  g_object_unref (stylable);
}

void
mx_stylable_thaw_notify (MxStylable *stylable)
{
  GObjectNotifyQueue *nqueue;

  g_return_if_fail (MX_IS_STYLABLE (stylable));

  g_object_ref (stylable);

  nqueue = g_object_notify_queue_from_object (G_OBJECT (stylable),
                                              &property_notify_context);

  if (!nqueue || !nqueue->freeze_count)
    g_warning ("%s: property-changed notification for %s(%p) is not frozen",
               G_STRFUNC, G_OBJECT_TYPE_NAME (stylable), stylable);
  else
    g_object_notify_queue_thaw (G_OBJECT (stylable), nqueue);

  g_object_unref (stylable);
}

void
mx_stylable_notify (MxStylable  *stylable,
                    const gchar *property_name)
{
  GParamSpec *pspec;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (property_name != NULL);

  g_object_ref (stylable);

  pspec = g_param_spec_pool_lookup (style_property_spec_pool,
                                    property_name,
                                    G_OBJECT_TYPE (stylable),
                                    TRUE);

  if (!pspec)
    g_warning ("%s: object class `%s' has no style property named `%s'",
               G_STRFUNC,
               G_OBJECT_TYPE_NAME (stylable),
               property_name);
  else
    {
      GObjectNotifyQueue *nqueue;

      nqueue = g_object_notify_queue_freeze (G_OBJECT (stylable),
                                             &property_notify_context);
      g_object_notify_queue_add (G_OBJECT (stylable), nqueue, pspec);
      g_object_notify_queue_thaw (G_OBJECT (stylable), nqueue);
    }

  g_object_unref (stylable);
}
#endif

/**
 * mx_stylable_iface_install_property:
 * @iface: a #MxStylableIface
 * @owner_type: #GType of the style property owner
 * @pspec: a #GParamSpec
 *
 * Installs a property for @owner_type using @pspec as the property
 * description.
 *
 * This function should be used inside the #MxStylableIface initialization
 * function of a class, for instance:
 *
 * <informalexample><programlisting>
 * G_DEFINE_TYPE_WITH_CODE (FooActor, foo_actor, CLUTTER_TYPE_ACTOR,
 *                          G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
 *                                                 mx_stylable_init));
 * ...
 * static void
 * mx_stylable_init (MxStylableIface *iface)
 * {
 *   static gboolean is_initialized = FALSE;
 *
 *   if (!is_initialized)
 *     {
 *       ...
 *       mx_stylable_iface_install_property (stylable,
 *                                             FOO_TYPE_ACTOR,
 *                                             g_param_spec_int ("x-spacing",
 *                                                               "X Spacing",
 *                                                               "Horizontal spacing",
 *                                                               -1, G_MAXINT,
 *                                                               2,
 *                                                               G_PARAM_READWRITE));
 *       ...
 *     }
 * }
 * </programlisting></informalexample>
 */
void
mx_stylable_iface_install_property (MxStylableIface *iface,
                                    GType            owner_type,
                                    GParamSpec      *pspec)
{
  g_return_if_fail (MX_IS_STYLABLE_IFACE (iface));
  g_return_if_fail (owner_type != G_TYPE_INVALID);
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));
  g_return_if_fail (pspec->flags & G_PARAM_READABLE);
  g_return_if_fail (!(pspec->flags & (G_PARAM_CONSTRUCT_ONLY | G_PARAM_CONSTRUCT
                                      )));

  if (g_param_spec_pool_lookup (style_property_spec_pool, pspec->name,
                                owner_type,
                                FALSE))
    {
      g_warning ("%s: class `%s' already contains a style property named `%s'",
                 G_STRLOC,
                 g_type_name (owner_type),
                 pspec->name);
      return;
    }

  g_param_spec_ref_sink (pspec);
  g_param_spec_set_qdata_full (pspec, quark_real_owner,
                               g_strdup (g_type_name (owner_type)),
                               g_free);

  g_param_spec_pool_insert (style_property_spec_pool,
                            pspec,
                            owner_type);
}

/**
 * mx_stylable_list_properties:
 * @stylable: a #MxStylable
 * @n_props: (out): return location for the number of properties, or %NULL
 *
 * Retrieves all the #GParamSpec<!-- -->s installed by @stylable.
 *
 * Return value: (transfer container) (array length=n_props): an array
 *  of #GParamSpec<!-- -->s. Free it with  g_free() when done.
 */
GParamSpec **
mx_stylable_list_properties (MxStylable *stylable,
                             guint      *n_props)
{
  GParamSpec **pspecs = NULL;
  guint n;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  pspecs = g_param_spec_pool_list (style_property_spec_pool,
                                   G_OBJECT_TYPE (stylable),
                                   &n);
  if (n_props)
    *n_props = n;

  return pspecs;
}

/**
 * mx_stylable_find_property:
 * @stylable: a #MxStylable
 * @property_name: the name of the property to find
 *
 * Finds the #GParamSpec installed by @stylable for the property
 * with @property_name.
 *
 * Return value: (transfer none): a #GParamSpec for the given property,
 *   or %NULL if no property with that name was found
 */
GParamSpec *
mx_stylable_find_property (MxStylable  *stylable,
                           const gchar *property_name)
{
  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);

  return g_param_spec_pool_lookup (style_property_spec_pool,
                                   property_name,
                                   G_OBJECT_TYPE (stylable),
                                   TRUE);
}

static inline void
mx_stylable_get_property_internal (MxStylable *stylable,
                                   GParamSpec *pspec,
                                   GValue     *value)
{
  MxStyle *style;
  GValue real_value = { 0, };

  style = mx_stylable_get_style (stylable);

  if (!style)
    {
      g_value_reset (value);
      return;
    }

  mx_style_get_property (style, stylable, pspec, &real_value);

  g_value_copy (&real_value, value);
  g_value_unset (&real_value);

}

/**
 * mx_stylable_get_property:
 * @stylable: a #MxStylable
 * @property_name: the name of the property
 * @value: (out): return location for an empty #GValue
 *
 * Retrieves the value of @property_name for @stylable, and puts it
 * into @value.
 */
void
mx_stylable_get_property (MxStylable  *stylable,
                          const gchar *property_name,
                          GValue      *value)
{
  GParamSpec *pspec;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (property_name != NULL);
  g_return_if_fail (value != NULL);

  pspec = mx_stylable_find_property (stylable, property_name);
  if (!pspec)
    {
      g_warning ("Stylable class `%s' doesn't have a property named `%s'",
                 g_type_name (G_OBJECT_TYPE (stylable)),
                 property_name);
      return;
    }

  if (!(pspec->flags & G_PARAM_READABLE))
    {
      g_warning ("Style property `%s' of class `%s' is not readable",
                 pspec->name,
                 g_type_name (G_OBJECT_TYPE (stylable)));
      return;
    }

  if (G_VALUE_TYPE (value) != G_PARAM_SPEC_VALUE_TYPE (pspec))
    {
      g_warning ("Passed value is not of the requested type `%s' for "
                 "the style property `%s' of class `%s'",
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)),
                 pspec->name,
                 g_type_name (G_OBJECT_TYPE (stylable)));
      return;
    }

  mx_stylable_get_property_internal (stylable, pspec, value);
}

/**
 * mx_stylable_get:
 * @stylable: a #MxStylable
 * @first_property_name: name of the first property to get
 * @Varargs: return location for the first property, followed optionally
 *   by more name/return location pairs, followed by %NULL
 *
 * Gets the style properties for @stylable.
 *
 * In general, a copy is made of the property contents and the called
 * is responsible for freeing the memory in the appropriate manner for
 * the property type.
 *
 * <example>
 * <title>Using mx_stylable_get(<!-- -->)</title>
 * <para>An example of using mx_stylable_get() to get the contents of
 * two style properties - one of type #G_TYPE_INT and one of type
 * #CLUTTER_TYPE_COLOR:</para>
 * <programlisting>
 *   gint x_spacing;
 *   ClutterColor *bg_color;
 *
 *   mx_stylable_get (stylable,
 *                      "x-spacing", &amp;x_spacing,
 *                      "bg-color", &amp;bg_color,
 *                      NULL);
 *
 *   /<!-- -->* do something with x_spacing and bg_color *<!-- -->/
 *
 *   clutter_color_free (bg_color);
 * </programlisting>
 * </example>
 */
void
mx_stylable_get (MxStylable  *stylable,
                 const gchar *first_property_name,
                 ...)
{
  MxStyle *style;
  va_list args;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (first_property_name != NULL);

  style = mx_stylable_get_style (stylable);

  va_start (args, first_property_name);
  mx_style_get_valist (style, stylable, first_property_name, args);
  va_end (args);
}

static gboolean
_set_from_xsettings (GParamSpec *pspec,
                     GValue     *value)
{
  gchar *font_string;
  gint font_size;
  PangoFontDescription *descr;
  MxSettings *settings = mx_settings_get_default ();
  gboolean result = FALSE;

  if (!strcmp (pspec->name, "font-family")
      || !strcmp (pspec->name, "font-size"))
    {
      const gchar *fn;
      PangoFontMask set_fields;

      g_object_get (settings, "font-name", &font_string, NULL);

      if (!font_string)
        return FALSE;

      descr = pango_font_description_from_string (font_string);
      g_free (font_string);

      set_fields = pango_font_description_get_set_fields (descr);

      /* font name */
      if ((set_fields & PANGO_FONT_MASK_FAMILY)
          && !strcmp (pspec->name, "font-family")
          && G_VALUE_HOLDS_STRING (value))
        {
          fn = pango_font_description_get_family (descr);
          g_value_set_string (value, fn);
          result = TRUE;
        }
      /* font size */
      else if ((set_fields & PANGO_FONT_MASK_SIZE)
               && !strcmp (pspec->name, "font-size")
               && G_VALUE_HOLDS_INT (value))
        {
          font_size = pango_font_description_get_size (descr) / PANGO_SCALE;
          if (!pango_font_description_get_size_is_absolute (descr))
            {
              ClutterBackend *backend = clutter_get_default_backend ();
              gdouble res = clutter_backend_get_resolution (backend);
              font_size = font_size * res / 72.0;
            }
          g_value_set_int (value, font_size);
          result = TRUE;
        }

      pango_font_description_free (descr);
      descr = NULL;
    }

  return result;
}

/**
 * mx_stylable_get_default_value:
 * @stylable: a #MxStylable
 * @property_name: name of the property to query
 * @value_out: (out): return location for the default value
 *
 * Query @stylable for the default value of property @property_name and
 * fill @value_out with the result.
 *
 * Returns: %TRUE if property @property_name exists and the default value has
 * been returned.
 */
gboolean
mx_stylable_get_default_value (MxStylable  *stylable,
                               const gchar *property_name,
                               GValue      *value_out)
{
  GParamSpec *pspec;

  pspec = mx_stylable_find_property (stylable, property_name);
  if (!pspec)
    {
      g_warning ("%s: no style property named `%s' found for class `%s'",
                 G_STRLOC,
                 property_name,
                 g_type_name (G_OBJECT_TYPE (stylable)));
      return FALSE;
    }

  if (!(pspec->flags & G_PARAM_READABLE))
    {
      g_warning ("Style property `%s' of class `%s' is not readable",
                 pspec->name,
                 g_type_name (G_OBJECT_TYPE (stylable)));
      return FALSE;
    }

  g_value_init (value_out, G_PARAM_SPEC_VALUE_TYPE (pspec));

  /* default font values come from xsettings if possible */
  if (!_set_from_xsettings (pspec, value_out))
    g_param_value_set_default (pspec, value_out);

  return TRUE;
}

/**
 * mx_stylable_get_style:
 * @stylable: a #MxStylable
 *
 * Retrieves the #MxStyle used by @stylable. This function does not
 * alter the reference count of the returned object.
 *
 * Return value: (transfer none): a #MxStyle
 */
MxStyle *
mx_stylable_get_style (MxStylable *stylable)
{
  MxStylableIface *iface;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  iface = MX_STYLABLE_GET_IFACE (stylable);
  if (G_LIKELY (iface->get_style))
    return iface->get_style (stylable);

  return g_object_get_data (G_OBJECT (stylable), "mx-stylable-style");
}

typedef struct
{
  GObject *instance;
  gulong   handler_id;
} SignalData;

static void
disconnect_style_changed_signal (SignalData *data)
{
  g_signal_handler_disconnect (data->instance, data->handler_id);
  g_object_unref (data->instance);
  g_slice_free (SignalData, data);
}

/**
 * mx_stylable_set_style:
 * @stylable: a #MxStylable
 * @style: a #MxStyle
 *
 * Sets @style as the new #MxStyle to be used by @stylable.
 *
 * The #MxStylable will take ownership of the passed #MxStyle.
 *
 * After the #MxStyle has been set, the MxStylable::style-set signal
 * will be emitted.
 */
void
mx_stylable_set_style (MxStylable *stylable,
                       MxStyle    *style)
{
  MxStylableIface *iface;
  SignalData *data;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (MX_IS_STYLE (style));

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (G_LIKELY (iface->set_style))
    iface->set_style (stylable, style);

  /* connect to the "changed" signal of the MxStyle and take a reference to
   * the style */
  data = g_slice_new (SignalData);
  data->instance = g_object_ref_sink (style);
  data->handler_id =
    g_signal_connect_swapped (style, "changed",
                              G_CALLBACK (mx_stylable_property_changed_notify),
                              stylable);

  g_object_set_qdata_full (G_OBJECT (stylable),
                           quark_style,
                           data,
                           (GDestroyNotify) disconnect_style_changed_signal);

  mx_stylable_style_changed (stylable, MX_STYLE_CHANGED_INVALIDATE_CACHE);

  g_object_notify (G_OBJECT (stylable), "style");
}

/**
 * mx_stylable_get_style_pseudo_class:
 * @stylable: a #MxStylable
 *
 * Get the current style pseudo class. This can contain multiple pseudo class
 * names, separated by ':'.
 *
 * Returns: the pseudo class string. The string is owned by the #MxWidget and
 * should not be modified or freed.
 */
const gchar*
mx_stylable_get_style_pseudo_class (MxStylable *stylable)
{
  MxStylableIface *iface;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (G_LIKELY (iface->get_style_pseudo_class))
    return iface->get_style_pseudo_class (stylable);

  g_warning ("MxStylable of type '%s' does not implement"
             " get_style_pseudo_class()",
             g_type_name (G_OBJECT_TYPE (stylable)));
  return NULL;
}

/**
 * mx_stylable_set_style_pseudo_class:
 * @stylable: a #MxStylable
 * @pseudo_class: a new pseudo class string
 *
 * Set the style pseudo class. The string can contain multiple pseudo class
 * names, separated by ':'.
 */
void
mx_stylable_set_style_pseudo_class (MxStylable  *stylable,
                                    const gchar *pseudo_class)
{
  MxStylableIface *iface;

  g_return_if_fail (MX_IS_STYLABLE (stylable));

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (G_LIKELY (iface->set_style_pseudo_class))
    iface->set_style_pseudo_class (stylable, pseudo_class);
  else
    g_warning ("MxStylable of type '%s' does not implement"
               " set_style_pseudo_class()",
               g_type_name (G_OBJECT_TYPE (stylable)));
}

/**
 * mx_stylable_style_pseudo_class_contains:
 * @stylable: A #MxStylable
 * @pseudo_class: A pseudo-class name
 *
 * Check if the given pseudo-class name is contained in the list of
 * set pseudo classes on this #MxStylable object.
 *
 * Returns: %TRUE if the given pseudo-class is set, %FALSE otherwise
 *
 * Since: 1.2
 */
gboolean
mx_stylable_style_pseudo_class_contains (MxStylable  *stylable,
                                         const gchar *pseudo_class)
{
  const gchar *old_class, *match;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), FALSE);
  g_return_val_if_fail (pseudo_class != NULL, FALSE);

  old_class = mx_stylable_get_style_pseudo_class (stylable);

  if (old_class && pseudo_class && (match = strstr (old_class, pseudo_class)))
    {
      if ((match == old_class) ||
           (match[-1] == ':'))
        {
          size_t length = strlen (match);
          if ((match[length] == ':') ||
              (match[length] == '\0'))
            return TRUE;
        }
    }

  return FALSE;
}

/**
 * mx_stylable_style_pseudo_class_add:
 * @stylable: A #MxStylable
 * @new_class: A pseudo-class name to add
 *
 * Add a pseudo-class name to the list of pseudo classes, contained in the
 * #MxStylable:style-pseudo-class property.
 *
 * Since: 1.2
 */
void
mx_stylable_style_pseudo_class_add (MxStylable  *stylable,
                                    const gchar *new_class)
{
  const gchar *old_class;
  gchar *tmp;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (new_class != NULL);

  /* check if the pseudo class already contains new_class */
  if (mx_stylable_style_pseudo_class_contains (stylable, new_class))
    return;

  old_class = mx_stylable_get_style_pseudo_class (stylable);

  /* add the new pseudo class */
  if (old_class)
    tmp = g_strconcat (old_class, ":", new_class, NULL);
  else
    tmp = g_strdup (new_class);

  mx_stylable_set_style_pseudo_class (stylable, tmp);

  g_free (tmp);
}

/**
 * mx_stylable_style_pseudo_class_remove:
 * @stylable: An #MxStylable
 * @remove_class: A pseudo class name to remove
 *
 * Remove the specified pseudo class name from the list of pseudo classes
 * contained in the #MxStylable:style-pseudo-class property.
 *
 * Since: 1.2
 */
void
mx_stylable_style_pseudo_class_remove (MxStylable  *stylable,
                                       const gchar *remove_class)
{
  const gchar *old_class;
  gchar *tmp;
  gchar **list;
  gint i, len;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (remove_class != NULL);

  /* check if the pseudo class does not container remove_class */
  if (!mx_stylable_style_pseudo_class_contains (stylable, remove_class))
    return;

  old_class = mx_stylable_get_style_pseudo_class (stylable);

  /* remove the old pseudo class */
  list = g_strsplit (old_class, ":", -1);

  len = g_strv_length (list);
  tmp = NULL;
  for (i = 0; i < len; i++)
    {
      /* skip over any instances of remove_class */
      if (!strcmp (list[i], remove_class))
        continue;

      if (tmp)
        tmp = g_strconcat (list[i], ":", tmp, NULL);
      else
        tmp = g_strdup (list[i]);
    }

  mx_stylable_set_style_pseudo_class (stylable, tmp);

  g_strfreev (list);
  g_free (tmp);
}

/**
 * mx_stylable_get_style_class:
 * @stylable: a #MxStylable
 *
 * Get the current style class name
 *
 * Returns: the class name string. The string is owned by the #MxWidget and
 * should not be modified or freed.
 */
const gchar*
mx_stylable_get_style_class (MxStylable *stylable)
{
  MxStylableIface *iface;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (G_LIKELY (iface->get_style_class))
    return iface->get_style_class (stylable);

  g_warning ("MxStylable of type '%s' does not implement get_style_class()",
             g_type_name (G_OBJECT_TYPE (stylable)));
  return NULL;
}

/**
 * mx_stylable_set_style_class:
 * @stylable: a #MxStylable
 * @style_class: a new style class string
 *
 * Set the style class name
 */
void
mx_stylable_set_style_class (MxStylable  *stylable,
                             const gchar *style_class)
{
  MxStylableIface *iface;

  g_return_if_fail (MX_IS_STYLABLE (stylable));

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (G_LIKELY (iface->set_style_class))
    iface->set_style_class (stylable, style_class);
  else
    g_warning ("MxStylable of type '%s' does not implement"
               " set_style_class()",
               g_type_name (G_OBJECT_TYPE (stylable)));
}

static void
mx_stylable_property_changed_notify (MxStylable *stylable)
{
  mx_stylable_style_changed (stylable, MX_STYLE_CHANGED_INVALIDATE_CACHE);
}

static void
mx_stylable_parent_set_notify (ClutterActor *actor,
                               ClutterActor *old_parent)
{
  ClutterActor *new_parent = clutter_actor_get_parent (actor);

  /* check the actor has a new parent */
  if (new_parent)
    {
      mx_stylable_style_changed (MX_STYLABLE (actor),
                                 MX_STYLE_CHANGED_INVALIDATE_CACHE);
    }
}

static void mx_stylable_style_changed_internal (MxStylable          *stylable,
                                                MxStyleChangedFlags  flags);

static void
mx_stylable_child_notify (ClutterActor *actor,
                          gpointer      flags)
{
  if (MX_IS_STYLABLE (actor))
    mx_stylable_style_changed_internal (MX_STYLABLE (actor),
                                        GPOINTER_TO_INT (flags));
}

static void
mx_stylable_style_changed_internal (MxStylable          *stylable,
                                    MxStyleChangedFlags  flags)
{

  /* don't update stylables until they are mapped (unless ensure is set) */
  if (G_LIKELY (CLUTTER_IS_ACTOR (stylable)) &&
      !CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (stylable)) &&
      !(flags & MX_STYLE_CHANGED_FORCE))
    return;

  if (flags & MX_STYLE_CHANGED_INVALIDATE_CACHE)
    _mx_style_invalidate_cache (stylable);

  /* If the parent style has changed, child cache needs to be
   * invalidated. This needs to happen for internal children as
   * well, which is why it's here and not in the container block
   * lower down.
   */
  flags |= MX_STYLE_CHANGED_INVALIDATE_CACHE;

  g_signal_emit (stylable, stylable_signals[STYLE_CHANGED], 0, flags);

  /* propagate the style-changed signal to children, since their style may
   * depend on one or more properties of the parent */

  if (CLUTTER_IS_CONTAINER (stylable))
    {
      /* notify our children that their parent stylable has changed */
      clutter_container_foreach ((ClutterContainer *) stylable,
                                 mx_stylable_child_notify,
                                 GINT_TO_POINTER (flags));
    }
}

/**
 * mx_stylable_style_changed:
 * @stylable: an MxStylable
 * @flags: flags that control the style changing
 *
 * Emit the "style-changed" signal on @stylable to notify it that one or more
 * of the style properties has changed.
 *
 * If @stylable is a #ClutterContainer then the "style-changed" notification is
 * propagated to it's children, since their style may depend on one or more
 * properties of the parent.
 *
 */
void
mx_stylable_style_changed (MxStylable *stylable, MxStyleChangedFlags flags)
{
  mx_stylable_style_changed_internal (stylable, flags);
}

void
mx_stylable_connect_change_notifiers (MxStylable *stylable)
{
  g_return_if_fail (CLUTTER_IS_ACTOR (stylable));

  g_return_if_fail (MX_IS_STYLABLE (stylable));

  /* ClutterActor signals */
  g_signal_connect (stylable, "notify::name",
                    G_CALLBACK (mx_stylable_property_changed_notify), NULL);
  g_signal_connect (stylable, "parent-set",
                    G_CALLBACK (mx_stylable_parent_set_notify), NULL);

  /* style-changed is blocked until the actor is mapped, so style-changed
   * needs to be sent as soon as the actor is mapped */
  g_signal_connect (stylable, "notify::mapped",
                    G_CALLBACK (mx_stylable_property_changed_notify), NULL);

  /* MxStylable notifiers */
  g_signal_connect (stylable, "notify::style-class",
                    G_CALLBACK (mx_stylable_property_changed_notify), NULL);
  g_signal_connect (stylable, "notify::style-pseudo-class",
                    G_CALLBACK (mx_stylable_property_changed_notify), NULL);

}

void
mx_stylable_apply_clutter_text_attributes (MxStylable  *stylable,
                                           ClutterText *text)
{
  ClutterColor *real_color = NULL;
  gchar *font_name = NULL;
  gint font_size = 0;
  MxFontWeight font_weight;
  PangoWeight weight;
  PangoFontDescription *descr;
  gchar *descr_string;

  mx_stylable_get (stylable,
                   "color", &real_color,
                   "font-family", &font_name,
                   "font-size", &font_size,
                   "font-weight", &font_weight,
                   NULL);


  /* Create a description, we will convert to a string and set on the
   * ClutterText. When Clutter gets API to set the description directly this
   * won't be necessary. */
  descr = pango_font_description_new ();

  /* font name */
  pango_font_description_set_family (descr, font_name);
  g_free (font_name);

  /* font size */
  pango_font_description_set_absolute_size (descr, font_size * PANGO_SCALE);

  /* font weight */
  switch (font_weight)
    {
  case MX_FONT_WEIGHT_BOLD:
    weight = PANGO_WEIGHT_BOLD;
    break;
  case MX_FONT_WEIGHT_LIGHTER:
    weight = PANGO_WEIGHT_LIGHT;
    break;
  case MX_FONT_WEIGHT_BOLDER:
    weight = PANGO_WEIGHT_HEAVY;
    break;
  default:
    weight = PANGO_WEIGHT_NORMAL;
    break;
    }
  pango_font_description_set_weight (descr, weight);

  descr_string = pango_font_description_to_string (descr);
  clutter_text_set_font_name (text, descr_string);
  g_free (descr_string);
  pango_font_description_free (descr);

  /* font color */
  if (real_color)
    {
      clutter_text_set_color (text, real_color);
      clutter_color_free (real_color);
    }
}
