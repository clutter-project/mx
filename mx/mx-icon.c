/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-icon.c: icon widget
 *
 * Copyright 2009, 2010 Intel Corporation.
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
 * Written by: Thomas Wood <thomas.wood@intel.com>,
 *             Chris Lord <chris@linux.intel.com>
 *
 */

/**
 * SECTION:mx-icon
 * @short_description: a simple styled icon actor
 *
 * #MxIcon is a simple styled texture actor that displays an image from
 * a stylesheet.
 */

#include "mx-icon.h"
#include "mx-icon-theme.h"
#include "mx-stylable.h"

#include "mx-private.h"

enum
{
  PROP_0,

  PROP_ICON_NAME,
  PROP_ICON_SIZE
};

static void mx_stylable_iface_init (MxStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxIcon, mx_icon, MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init))

#define MX_ICON_GET_PRIVATE(obj)    \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MX_TYPE_ICON, MxIconPrivate))

struct _MxIconPrivate
{
  guint         icon_set         : 1;
  guint         size_set         : 1;
  guint         is_content_image : 1;

  ClutterActor *icon_texture;

  gchar        *icon_name;
  gchar        *icon_suffix;
  gint          icon_size;
};

static void mx_icon_update (MxIcon *icon);

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_boxed ("x-mx-content-image",
                                   "Content Image",
                                   "Image used as the button",
                                   MX_TYPE_BORDER_IMAGE,
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ICON, pspec);

      pspec = g_param_spec_string ("x-mx-icon-name",
                                   "Icon name",
                                   "Icon name to load from the theme",
                                   NULL,
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ICON, pspec);

      pspec = g_param_spec_string ("x-mx-icon-suffix",
                                   "Icon suffix",
                                   "Suffix to append to the icon name",
                                   NULL,
                                   G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ICON, pspec);

      pspec = g_param_spec_int ("x-mx-icon-size",
                                "Icon size",
                                "Size to use for icon",
                                -1, G_MAXINT, -1,
                                G_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_ICON, pspec);
    }
}

static void
mx_icon_set_property (GObject      *gobject,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  MxIcon *icon = MX_ICON (gobject);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      mx_icon_set_icon_name (icon, g_value_get_string (value));
      break;

    case PROP_ICON_SIZE:
      mx_icon_set_icon_size (icon, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_icon_get_property (GObject    *gobject,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  MxIcon *icon = MX_ICON (gobject);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      g_value_set_string (value, mx_icon_get_icon_name (icon));
      break;

    case PROP_ICON_SIZE:
      g_value_set_int (value, mx_icon_get_icon_size (icon));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_icon_notify_theme_name_cb (MxIconTheme *theme,
                              GParamSpec  *pspec,
                              MxIcon      *self)
{
  mx_icon_update (self);
}

static void
mx_icon_dispose (GObject *gobject)
{
  MxIconPrivate *priv = MX_ICON (gobject)->priv;

  if (priv->icon_texture)
    {
      clutter_actor_destroy (priv->icon_texture);
      priv->icon_texture = NULL;
    }

  if (mx_icon_theme_get_default ())
    {
      g_signal_handlers_disconnect_by_func (mx_icon_theme_get_default (),
                                            mx_icon_notify_theme_name_cb,
                                            gobject);
    }

  G_OBJECT_CLASS (mx_icon_parent_class)->dispose (gobject);
}

static void
mx_icon_finalize (GObject *gobject)
{
  MxIconPrivate *priv = MX_ICON (gobject)->priv;

  g_free (priv->icon_name);
  g_free (priv->icon_suffix);

  G_OBJECT_CLASS (mx_icon_parent_class)->finalize (gobject);
}

static void
mx_icon_get_preferred_height (ClutterActor *actor,
                              gfloat        for_width,
                              gfloat       *min_height_p,
                              gfloat       *nat_height_p)
{
  MxPadding padding;
  gfloat pref_height;

  MxIconPrivate *priv = MX_ICON (actor)->priv;

  if (priv->icon_texture)
    {
      gint width, height;
      clutter_texture_get_base_size (CLUTTER_TEXTURE (priv->icon_texture),
                                     &width,
                                     &height);

      if (!priv->is_content_image)
        {
          if (width <= height)
            pref_height = priv->icon_size;
          else
            pref_height = height / (gfloat)width * priv->icon_size;
        }
      else
        pref_height = height;
    }
  else
    pref_height = 0;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  pref_height += padding.top + padding.bottom;

  if (min_height_p)
    *min_height_p = pref_height;

  if (nat_height_p)
    *nat_height_p = pref_height;
}

static void
mx_icon_get_preferred_width (ClutterActor *actor,
                             gfloat        for_height,
                             gfloat       *min_width_p,
                             gfloat       *nat_width_p)
{
  MxPadding padding;
  gfloat pref_width;

  MxIconPrivate *priv = MX_ICON (actor)->priv;

  if (priv->icon_texture)
    {
      gint width, height;
      clutter_texture_get_base_size (CLUTTER_TEXTURE (priv->icon_texture),
                                     &width,
                                     &height);

      if (!priv->is_content_image)
        {
          if (height <= width)
            pref_width = priv->icon_size;
          else
            pref_width = width / (gfloat)height * priv->icon_size;
        }
      else
        pref_width = width;
    }
  else
    pref_width = 0;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  pref_width += padding.left + padding.right;

  if (min_width_p)
    *min_width_p = pref_width;

  if (nat_width_p)
    *nat_width_p = pref_width;
}

static void
mx_icon_allocate (ClutterActor           *actor,
                  const ClutterActorBox  *box,
                  ClutterAllocationFlags  flags)
{
  MxIconPrivate *priv = MX_ICON (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_icon_parent_class)->allocate (actor, box, flags);

  if (priv->icon_texture)
    {
      MxPadding padding;
      ClutterActorBox child_box;

      mx_widget_get_padding (MX_WIDGET (actor), &padding);
      child_box.x1 = padding.left;
      child_box.y1 = padding.top;
      child_box.x2 = box->x2 - box->x1 - padding.right;
      child_box.y2 = box->y2 - box->y1 - padding.bottom;

      clutter_actor_allocate (priv->icon_texture, &child_box, flags);
    }
}

static void
mx_icon_paint (ClutterActor *actor)
{
  MxIconPrivate *priv = MX_ICON (actor)->priv;

  /* Chain up to paint background */
  if (!priv->is_content_image)
    CLUTTER_ACTOR_CLASS (mx_icon_parent_class)->paint (actor);

  if (priv->icon_texture)
    clutter_actor_paint (priv->icon_texture);
}

static void
mx_icon_map (ClutterActor *actor)
{
  MxIconPrivate *priv = MX_ICON (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_icon_parent_class)->map (actor);

  if (priv->icon_texture)
    clutter_actor_map (priv->icon_texture);
}

static void
mx_icon_unmap (ClutterActor *actor)
{
  MxIconPrivate *priv = MX_ICON (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_icon_parent_class)->unmap (actor);

  if (priv->icon_texture)
    clutter_actor_unmap (priv->icon_texture);
}

static void
mx_icon_class_init (MxIconClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxIconPrivate));

  object_class->get_property = mx_icon_get_property;
  object_class->set_property = mx_icon_set_property;
  object_class->dispose = mx_icon_dispose;
  object_class->finalize = mx_icon_finalize;

  actor_class->get_preferred_height = mx_icon_get_preferred_height;
  actor_class->get_preferred_width = mx_icon_get_preferred_width;
  actor_class->allocate = mx_icon_allocate;
  actor_class->paint = mx_icon_paint;
  actor_class->map = mx_icon_map;
  actor_class->unmap = mx_icon_unmap;

  pspec = g_param_spec_string ("icon-name",
                               "Icon name",
                               "An icon name",
                               NULL, MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ICON_NAME, pspec);

  pspec = g_param_spec_int ("icon-size",
                            "Icon size",
                            "Size of the icon",
                            1, G_MAXINT, 48,
                            MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ICON_SIZE, pspec);
}

static void
mx_icon_update (MxIcon *icon)
{
  MxIconPrivate *priv = icon->priv;

  if (priv->is_content_image)
    {
      priv->is_content_image = FALSE;
      g_signal_connect (mx_icon_theme_get_default (), "notify::theme-name",
                        G_CALLBACK (mx_icon_notify_theme_name_cb), icon);
    }

  /* Get rid of the old one */
  if (priv->icon_texture)
    {
      clutter_actor_destroy (priv->icon_texture);
      priv->icon_texture = NULL;
    }

  /* Try to lookup the new one */
  if (priv->icon_name)
    {
      gchar *icon_name;
      MxIconTheme *theme = mx_icon_theme_get_default ();

      icon_name = g_strconcat (priv->icon_name, priv->icon_suffix, NULL);
      priv->icon_texture = (ClutterActor *)
        mx_icon_theme_lookup_texture (theme, icon_name, priv->icon_size);
      g_free (icon_name);

      /* If the icon is missing, use the image-missing icon */
      if (!priv->icon_texture)
        priv->icon_texture = (ClutterActor *)
          mx_icon_theme_lookup_texture (theme,
                                        "image-missing",
                                        priv->icon_size);

      if (priv->icon_texture)
        clutter_actor_set_parent (priv->icon_texture, CLUTTER_ACTOR (icon));
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (icon));
}

static void
mx_icon_style_changed_cb (MxWidget *widget)
{
  MxIcon *self = MX_ICON (widget);
  MxIconPrivate *priv = self->priv;

  MxBorderImage *content_image = NULL;
  gboolean changed = FALSE;
  gchar *icon_name = NULL;
  gchar *icon_suffix = NULL;
  gint icon_size = -1;

  mx_stylable_get (MX_STYLABLE (widget),
                   "x-mx-content-image", &content_image,
                   "x-mx-icon-name", &icon_name,
                   "x-mx-icon-size", &icon_size,
                   "x-mx-icon-suffix", &icon_suffix,
                   NULL);

  /* Content-image overrides drawing of the icon, so
   * don't bother reading those properties if it's set.
   */
  if (content_image)
    {
      GError *error = NULL;

      priv->is_content_image = TRUE;
      g_signal_handlers_disconnect_by_func (mx_icon_theme_get_default (),
                                            mx_icon_notify_theme_name_cb,
                                            self);

      if (priv->icon_texture)
        {
          clutter_actor_destroy (priv->icon_texture);
          priv->icon_texture = NULL;
        }

      if (content_image->uri)
        priv->icon_texture = clutter_texture_new_from_file (content_image->uri,
                                                            &error);
      if (priv->icon_texture)
        clutter_actor_set_parent (priv->icon_texture, CLUTTER_ACTOR (widget));

      if (error)
        {
          g_warning ("Could not load content image: %s", error->message);
          g_error_free (error);
        }

      g_boxed_free (MX_TYPE_BORDER_IMAGE, content_image);
      g_free (icon_name);

      return;
    }

  if (icon_name && !priv->icon_set &&
      (!priv->icon_name || !g_str_equal (icon_name, priv->icon_name)))
    {
      g_free (priv->icon_name);
      priv->icon_name = g_strdup (icon_name);
      changed = TRUE;

      g_object_notify (G_OBJECT (self), "icon-name");
    }
  else if (!icon_name && !priv->icon_set && priv->icon_name)
    {
      /* icon has been unset */
      g_free (priv->icon_name);
      priv->icon_name = NULL;
      priv->icon_set = FALSE;

      changed = TRUE;

      g_object_notify (G_OBJECT (self), "icon-name");
    }

  if ((icon_size > 0) && !priv->size_set && (priv->icon_size != icon_size))
    {
      priv->icon_size = icon_size;
      changed = TRUE;

      g_object_notify (G_OBJECT (self), "icon-size");
    }

  if ((icon_suffix != priv->icon_suffix) &&
      (!icon_suffix || !priv->icon_suffix ||
       !g_str_equal (icon_suffix, priv->icon_suffix)))
    {
      g_free (priv->icon_suffix);
      priv->icon_suffix = icon_suffix;

      changed = TRUE;
    }
  else
    g_free (icon_suffix);

  if (changed)
    mx_icon_update (self);
}

static void
mx_icon_init (MxIcon *self)
{
  self->priv = MX_ICON_GET_PRIVATE (self);

  self->priv->icon_size = 48;

  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_icon_style_changed_cb), NULL);

  /* make sure we are not reactive */
  clutter_actor_set_reactive (CLUTTER_ACTOR (self), FALSE);

  /* Reload the icon when the theme changes */
  g_signal_connect (mx_icon_theme_get_default (), "notify::theme-name",
                    G_CALLBACK (mx_icon_notify_theme_name_cb), self);
}

/**
 * mx_icon_new:
 *
 * Create a newly allocated #MxIcon
 *
 * Returns: A newly allocated #MxIcon
 */
ClutterActor *
mx_icon_new (void)
{
  return g_object_new (MX_TYPE_ICON, NULL);
}


const gchar *
mx_icon_get_icon_name (MxIcon *icon)
{
  g_return_val_if_fail (MX_IS_ICON (icon), NULL);

  return icon->priv->icon_name;
}

void
mx_icon_set_icon_name (MxIcon      *icon,
                       const gchar *icon_name)
{
  MxIconPrivate *priv;

  g_return_if_fail (MX_IS_ICON (icon));

  priv = icon->priv;

  /* Unset the icon name if necessary */
  if (!icon_name)
    {
      if (priv->icon_set)
        {
          priv->icon_set = FALSE;
          mx_stylable_style_changed (MX_STYLABLE (icon), MX_STYLE_CHANGED_NONE);
        }

      return;
    }

  priv->icon_set = TRUE;

  /* Check if there's no change */
  if (priv->icon_name && g_str_equal (priv->icon_name, icon_name))
    return;

  g_free (priv->icon_name);
  priv->icon_name = g_strdup (icon_name);

  mx_icon_update (icon);

  g_object_notify (G_OBJECT (icon), "icon-name");
}

gint
mx_icon_get_icon_size (MxIcon *icon)
{
  g_return_val_if_fail (MX_IS_ICON (icon), -1);

  return icon->priv->icon_size;
}

void
mx_icon_set_icon_size (MxIcon *icon,
                       gint    size)
{
  MxIconPrivate *priv;

  g_return_if_fail (MX_IS_ICON (icon));

  priv = icon->priv;

  if (size < 0)
    {
      if (priv->size_set)
        {
          priv->size_set = FALSE;
          mx_stylable_style_changed (MX_STYLABLE (icon), MX_STYLE_CHANGED_NONE);
        }

      return;
    }
  else if (priv->icon_size != size)
    {
      priv->icon_size = size;
      mx_icon_update (icon);

      g_object_notify (G_OBJECT (icon), "icon-size");
    }

  priv->size_set = TRUE;
}
