/*
 * mx-button-group.c: A group handler for buttons
 *
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
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */

#include "mx-button-group.h"

G_DEFINE_TYPE (MxButtonGroup, mx_button_group, G_TYPE_INITIALLY_UNOWNED)

#define BUTTON_GROUP_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_BUTTON_GROUP, MxButtonGroupPrivate))

struct _MxButtonGroupPrivate
{
};


static void
mx_button_group_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_button_group_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_button_group_dispose (GObject *object)
{
  G_OBJECT_CLASS (mx_button_group_parent_class)->dispose (object);
}

static void
mx_button_group_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_button_group_parent_class)->finalize (object);
}

static void
mx_button_group_class_init (MxButtonGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxButtonGroupPrivate));

  object_class->get_property = mx_button_group_get_property;
  object_class->set_property = mx_button_group_set_property;
  object_class->dispose = mx_button_group_dispose;
  object_class->finalize = mx_button_group_finalize;
}

static void
mx_button_group_init (MxButtonGroup *self)
{
  self->priv = BUTTON_GROUP_PRIVATE (self);
}

MxButtonGroup *
mx_button_group_new (void)
{
  return g_object_new (MX_TYPE_BUTTON_GROUP, NULL);
}
