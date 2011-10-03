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
 * Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "test-mx.h"

#ifdef HAVE_CLUTTER_IMCONTEXT
#include <clutter-imcontext/clutter-imtext.h>
#endif

static void
btn_clicked_cb (ClutterActor *button, MxEntry *entry)
{
  mx_entry_set_text (entry, "Here is some text");
}

static void
clear_btn_clicked_cb (ClutterActor *button, MxEntry *entry)
{
  mx_entry_set_text (entry, "");
}

static void
text_changed_cb (MxEntry *entry, GParamSpec *pspec, void *user_data)
{
  printf ("Text: %s\n", mx_entry_get_text (entry));
}

static void
print_notice (ClutterText *text, gchar *message)
{
  printf ("%s\n", message);
}

void
entry_main (ClutterContainer *stage)
{
  ClutterActor *entry, *button, *clear_button;

  entry = mx_entry_new_with_text ("Hello World!");
  clutter_actor_set_position (entry, 20, 20);
  clutter_actor_set_width (entry, 150);

  clutter_container_add_actor (stage, entry);

  clutter_stage_set_key_focus (CLUTTER_STAGE (clutter_actor_get_stage (entry)),
                               mx_entry_get_clutter_text (MX_ENTRY (entry)));

  entry = mx_entry_new ();
  clutter_actor_set_position (entry, 20, 70);

  clutter_container_add_actor (stage, entry);
  mx_entry_set_hint_text (MX_ENTRY (entry), "hint hint...");

#ifdef HAVE_CLUTTER_IMCONTEXT
  clutter_imtext_set_autoshow_im (CLUTTER_IMTEXT (mx_entry_get_clutter_text (MX_ENTRY (entry))), TRUE);
#else
  g_debug ("Input method support is disabled");
#endif
  g_signal_connect (G_OBJECT (entry),
                    "notify::text", G_CALLBACK (text_changed_cb), NULL);

  button = mx_button_new_with_label ("Set");
  clutter_actor_set_position (button, 20, 120);
  g_signal_connect (button, "clicked", G_CALLBACK (btn_clicked_cb), entry);

  clear_button = mx_button_new_with_label ("clear");
  clutter_actor_set_position (clear_button, 70, 120);
  g_signal_connect (clear_button, "clicked",
                    G_CALLBACK (clear_btn_clicked_cb), entry);

  clutter_container_add (stage, button, clear_button, NULL);


  entry = mx_entry_new ();
  clutter_actor_set_position (entry, 20, 170);
  clutter_container_add_actor (stage, entry);

  mx_entry_set_hint_text (MX_ENTRY (entry), "Search...");
  mx_entry_set_primary_icon_from_file (MX_ENTRY (entry),
                                         "edit-find.png");
  mx_entry_set_secondary_icon_from_file (MX_ENTRY (entry),
                                           "edit-clear.png");

  mx_entry_set_icon_highlight_suffix (MX_ENTRY (entry), "-highlight");

  mx_entry_set_secondary_icon_tooltip_text (MX_ENTRY (entry), "one");

  mx_entry_set_primary_icon_tooltip_text (MX_ENTRY (entry), "two");

  g_signal_connect (entry, "primary-icon-clicked",
                    G_CALLBACK (print_notice), "primary icon clicked\n");
  g_signal_connect (entry, "secondary-icon-clicked",
                    G_CALLBACK (print_notice), "secondary icon clicked\n");

  entry = mx_entry_new ();
  clutter_actor_set_position (entry, 20, 220);
  clutter_container_add_actor (stage, entry);

  mx_entry_set_hint_text (MX_ENTRY (entry), "Secret!");
  mx_entry_set_password_char (MX_ENTRY (entry), 0x2022);
}
