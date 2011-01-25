/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-private.c
 *
 * Copyright 2010 Intel Corporation.
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
#include "mx-private.h"

static GDebugKey debug_keys[] = 
{
    {"layout", MX_DEBUG_LAYOUT},
    {"inspector", MX_DEBUG_INSPECTOR},
    {"focus", MX_DEBUG_FOCUS},
    {"css", MX_DEBUG_CSS}
};


gboolean
_mx_debug (gint check)
{
  static gint debug = -1;

  if (G_UNLIKELY (debug == -1))
    {
      const char *debug_str;

      debug_str = g_getenv ("MX_DEBUG");

      debug = g_parse_debug_string (debug_str, debug_keys,
                                    G_N_ELEMENTS (debug_keys));
    }


  return debug & check;
}

const gchar *
_mx_enum_to_string (GType type,
                    gint  value)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;
  const gchar *val;

  enum_class = g_type_class_ref (type);
  enum_value = g_enum_get_value (enum_class, value);
  if (enum_value)
    val = enum_value->value_nick;
  else
    val = "<invalid enum value>";

  g_type_class_unref (enum_class);

  return val;
}

gboolean
_mx_string_to_enum (GType        type,
                    const gchar *nick,
                    gint        *value)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;
  gboolean ret = FALSE;

  enum_class = g_type_class_ref (type);
  enum_value = g_enum_get_value_by_nick (enum_class, nick);
  if (enum_value) {
      if (value)
        *value = enum_value->value;
      ret = TRUE;
  }

  g_type_class_unref (enum_class);

  return ret;
}
