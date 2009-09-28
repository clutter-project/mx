/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
#include "mx-private.h"

static GDebugKey debug_keys[] = 
{
    {"layout", MX_DEBUG_LAYOUT},
    {"inspector", MX_DEBUG_INSPECTOR},
};


gboolean
_mx_debug (gint check)
{
  static gint debug = -1;

  if (debug == -1)
    {
      const char *debug_str;

      debug_str = g_getenv ("MX_DEBUG");

      debug = g_parse_debug_string (debug_str, debug_keys,
                                    G_N_ELEMENTS (debug_keys));
    }


  return debug & check;
}
