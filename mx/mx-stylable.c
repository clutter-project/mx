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
                               "Pseudo class, such as current state",
                               "",
                               MX_PARAM_READWRITE);
  g_object_interface_install_property (g_iface, pspec);

  /**
   * MxStylable::style-changed:
   * @stylable: the #MxStylable that received the signal
   * @old_style: the previously set #MxStyle for @stylable
   *
   * The ::style-changed signal is emitted each time one of the style
   * properties have changed.
   */
  stylable_signals[STYLE_CHANGED] =
    g_signal_new (I_("style-changed"),
                  iface_type,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MxStylableIface, style_changed),
                  NULL, NULL,
                  _mx_marshal_VOID__FLAGS,
                  G_TYPE_NONE, 1, MX_TYPE_STYLE_CHANGED_FLAGS);

#if 0
  stylable_signals[STYLE_NOTIFY] =
    g_signal_new (I_("style-notify"),
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
                                         I_("MxStylable"),
                                         &stylable_info, 0);
    }

  return our_type;
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
  if (iface->get_style)
    return iface->get_style (stylable);

  return g_object_get_data (G_OBJECT (stylable), "mx-stylable-style");
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
  MxStyle *old_style;

  g_return_if_fail (MX_IS_STYLABLE (stylable));
  g_return_if_fail (MX_IS_STYLE (style));

  iface = MX_STYLABLE_GET_IFACE (stylable);

  old_style = mx_stylable_get_style (stylable);
  g_object_ref (old_style);

  if (iface->set_style)
    iface->set_style (stylable, style);
  else
    {
      g_object_set_qdata_full (G_OBJECT (stylable),
                               quark_style,
                               g_object_ref_sink (style),
                               g_object_unref);
    }

  g_signal_emit (stylable, stylable_signals[STYLE_CHANGED], 0, old_style);
  g_object_unref (old_style);

  g_object_notify (G_OBJECT (stylable), "style");
}

/**
 * mx_stylable_get_style_pseudo_class:
 * @stylable: a #MxStylable
 *
 * Get the current style pseudo class
 *
 * Returns: the pseudo class string. The string is owned by the #MxWidget and
 * should not be modified or freed.
 */
G_CONST_RETURN gchar*
mx_stylable_get_style_pseudo_class (MxStylable *stylable)
{
  MxStylableIface *iface;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (iface->get_style_pseudo_class)
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
 * Set the style pseudo class
 */
void
mx_stylable_set_style_pseudo_class (MxStylable  *stylable,
                                    const gchar *pseudo_class)
{
  MxStylableIface *iface;

  g_return_if_fail (MX_IS_STYLABLE (stylable));

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (iface->set_style_pseudo_class)
    iface->set_style_pseudo_class (stylable, pseudo_class);
  else
    g_warning ("MxStylable of type '%s' does not implement"
               " set_style_pseudo_class()",
               g_type_name (G_OBJECT_TYPE (stylable)));
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
G_CONST_RETURN gchar*
mx_stylable_get_style_class (MxStylable *stylable)
{
  MxStylableIface *iface;

  g_return_val_if_fail (MX_IS_STYLABLE (stylable), NULL);

  iface = MX_STYLABLE_GET_IFACE (stylable);

  if (iface->get_style_class)
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

  if (iface->set_style_class)
    iface->set_style_class (stylable, style_class);
  else
    g_warning ("MxStylable of type '%s' does not implement"
               " set_style_class()",
               g_type_name (G_OBJECT_TYPE (stylable)));
}

static void
mx_stylable_property_changed_notify (MxStylable *stylable)
{
  mx_stylable_style_changed (stylable, MX_STYLE_CHANGED_NONE);
}

static void
mx_stylable_parent_set_notify (ClutterActor *actor,
                               ClutterActor *old_parent)
{
  ClutterActor *new_parent = clutter_actor_get_parent (actor);

  /* check the actor has a new parent */
  if (new_parent)
    {
      mx_stylable_style_changed (MX_STYLABLE (actor), MX_STYLE_CHANGED_NONE);
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
  if (!CLUTTER_ACTOR_IS_MAPPED (CLUTTER_ACTOR (stylable)) &&
      !(flags & MX_STYLE_CHANGED_FORCE))
    return;

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

  g_signal_connect_swapped (mx_stylable_get_style (stylable), "changed",
                            G_CALLBACK (mx_stylable_property_changed_notify),
                            stylable);
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
