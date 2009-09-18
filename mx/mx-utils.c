/*
 * mx-utils.c: General utility functions used in Moblin
 *
 * Copyright 2009 Intel Corporation
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
 * Authors: Emmanuele Bassi <ebassi@linux.intel.com>
 *          Rob Bradford <rob@linux.intel.com>
 *          Neil Roberts <neil@linux.intel.com>
 */

/**
 * SECTION:mx-utils
 * @short_description: General utility functions useful for GUIs
 *
 * Utilities useful for creating user interfaces.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "mx-utils.h"

/* We use the special gcc constructor attribute so we can avoid
 * requiring an init function to get translations to work! This
 * function is also in mx-gtk-light-switch but we also need it here
 * because that is a separate library */
static void __attribute__ ((constructor))
_start (void)
{
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

/* This code is based on a similar function in Tweet */

/**
 * mx_utils_format_time:
 * @time_: a time value
 *
 * Generates a string describing the time given in @time_ using
 * colloquial language suitable for display to the user. Examples of
 * what might be returned are "A few minutes ago" or "Yesterday".
 *
 * Returns: a string. Free with g_free().
 */
gchar *
mx_utils_format_time (GTimeVal *time_)
{
  GTimeVal now;
  struct tm tm_mtime;
  gchar *retval = NULL;
  GDate d1, d2;
  gint secs_diff, mins_diff, hours_diff, days_diff, months_diff, years_diff;

  g_return_val_if_fail (time_->tv_usec >= 0 &&
                        time_->tv_usec < G_USEC_PER_SEC, NULL);

  g_get_current_time (&now);

#ifdef HAVE_LOCALTIME_R
  localtime_r ((time_t *) &(time_->tv_sec), &tm_mtime);
#else
  {
    struct tm *ptm = localtime ((time_t *) &(time_->tv_sec));

    if (!ptm)
      {
        g_warning ("ptm != NULL failed");
        return NULL;
      }
    else
      memcpy ((void *) &tm_mtime, (void *) ptm, sizeof (struct tm));
  }
#endif /* HAVE_LOCALTIME_R */

  secs_diff = now.tv_sec - time_->tv_sec;
  if (secs_diff < 60)
    return g_strdup (_("Less than a minute ago"));

  mins_diff = secs_diff / 60;
  if (mins_diff < 60)
    return g_strdup (_("A few minutes ago"));

  hours_diff = mins_diff / 60;
  if (hours_diff < 3)
    return g_strdup (_("A couple of hours ago"));

  g_date_set_time_t (&d1, now.tv_sec);
  g_date_set_time_t (&d2, time_->tv_sec);
  days_diff = g_date_get_julian (&d1) - g_date_get_julian (&d2);

  if (days_diff == 0)
    return g_strdup (_("Earlier today"));

  if (days_diff == 1)
    return g_strdup (_("Yesterday"));

  if (days_diff < 7)
    {
      const gchar *format = NULL;
      gchar *locale_format = NULL;
      gchar buf[256];

      format = _("On %A"); /* day of the week */
      locale_format = g_locale_from_utf8 (format, -1, NULL, NULL, NULL);

      if (strftime (buf, sizeof (buf), locale_format, &tm_mtime) != 0)
        retval = g_locale_to_utf8 (buf, -1, NULL, NULL, NULL);
      else
        retval = g_strdup (_("Unknown"));

      g_free (locale_format);
      return retval;
    }

  if (days_diff < 14)
    return g_strdup (_("Last week"));

  if (days_diff < 21)
    return g_strdup (_("A couple of weeks ago"));

  months_diff = g_date_get_month (&d1) - g_date_get_month (&d2);
  years_diff = g_date_get_year (&d1) - g_date_get_year (&d2);

  if (years_diff == 0 && months_diff == 0)
    return g_strdup (_("This month"));

  if ((years_diff == 0 && months_diff == 1) ||
      (years_diff == 1 && months_diff == -11)) /* Now Jan., last used in Dec. */
    return g_strdup (_("Last month"));

  if (years_diff == 0)
    return g_strdup (_("This year"));

  if (years_diff == 1)
    return g_strdup (_("Last year"));

  return g_strdup (_("Ages ago"));
}
