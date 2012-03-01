/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-css-editor.c: Wrapper around the javascript CSS editor
 *
 * Copyright 2012 Intel Corporation.
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
 * Written by: Lionel Landwerlin <lionel.g.landwerlin@linux.intel.com>
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mx-css-editor.h"

#ifdef HAVE_GJS
# include <gjs/gjs.h>
#endif /* HAVE_GJS */

static GjsContext *js_context = NULL;

void
mx_css_editor_init (void)
{
#ifdef HAVE_GJS
  const gchar *script =
    "const CssEditor = imports.cssEditor;\n"
    "const _mx_inspector = new CssEditor.LiveInspector();";
  const gchar *search_paths[] = { PACKAGE_DATA_DIR "/mx-2.0/js", NULL };
  int code;
  GError *error = NULL;


  if (js_context != NULL)
    return;

  js_context = g_object_new (GJS_TYPE_CONTEXT,
                             "search-path", search_paths,
                             "js-version", "1.8",
                             "gc-notifications", FALSE,
                             NULL);

  if (!gjs_context_eval (js_context, script, -1,
                         "<main>", &code, &error))
    {
      g_error ("%s\n", error->message);
    }
#endif
}
