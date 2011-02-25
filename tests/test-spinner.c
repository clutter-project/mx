/*
 * Copyright 2011 Intel Corporation.
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */
#include "test-mx.h"

static void
on_toggle (MxButton   *toggle,
           GParamSpec *pspec,
           MxSpinner  *spinner)
{
  if (mx_button_get_toggled (toggle))
    {
      mx_spinner_set_animating (spinner, TRUE);
      mx_button_set_icon_name (MX_BUTTON (toggle), "media-playback-stop");
    }
  else
    {
      mx_spinner_set_animating (spinner, FALSE);
      mx_button_set_icon_name (MX_BUTTON (toggle), "media-playback-start");
    }
}

static void
on_looped (MxSpinner *spinner,
           MxButton  *button)
{
  mx_button_set_toggled (button, FALSE);
}

void
spinner_main (ClutterContainer *stage)
{
  ClutterActor *toggle, *spinner;

  toggle = mx_button_new ();
  mx_button_set_is_toggle (MX_BUTTON (toggle), TRUE);

  spinner = mx_spinner_new ();

  on_toggle (MX_BUTTON (toggle), NULL, MX_SPINNER (spinner));

  g_signal_connect (toggle, "notify::toggled",
                    G_CALLBACK (on_toggle), spinner);
  g_signal_connect (spinner, "looped",
                    G_CALLBACK (on_looped), toggle);

  clutter_container_add_actor (stage, toggle);
  clutter_actor_set_position (toggle, 20, 20);

  clutter_container_add_actor (stage, spinner);
  clutter_actor_set_position (spinner,
                              36 + clutter_actor_get_width (toggle), 20);
}
