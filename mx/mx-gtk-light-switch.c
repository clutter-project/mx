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

/**
 * SECTION:mx-gtk-light-switch
 * @short_description: a toggle switch between two states
 *
 * A visual representation of a toggle switch that can move between two states.
 */

#include <config.h>
#include "mx-gtk-light-switch.h"

#include <glib/gi18n-lib.h>

/* We use the special gcc constructor attribute so we can avoid
 * requiring an init function to get translations to work! This
 * function is also in mx-utils but we also need it here
 * because that is a separate library */
static void __attribute__ ((constructor))
_start (void)
{
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

G_DEFINE_TYPE (MxGtkLightSwitch, mx_gtk_light_switch, GTK_TYPE_DRAWING_AREA)

#define MX_GTK_LIGHT_SWITCH_GET_PRIVATE(o)                                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_GTK_TYPE_LIGHT_SWITCH, MxGtkLightSwitchPrivate))

static gboolean mx_gtk_light_switch_configure (GtkWidget         *lightswitch,
                                                 GdkEventConfigure *event);
static gboolean mx_gtk_light_switch_expose (GtkWidget      *lightswitch,
                                              GdkEventExpose *event);
static gboolean mx_gtk_light_switch_button_release (GtkWidget      *lightswitch,
                                                      GdkEventButton *event);
static gboolean mx_gtk_light_switch_button_press (GtkWidget      *lightswitch,
                                                    GdkEventButton *event);
static gboolean mx_gtk_light_switch_motion_notify (GtkWidget      *lightswitch,
                                                     GdkEventMotion *event);
static void mx_gtk_light_switch_size_request (GtkWidget      *lightswitch,
                                                GtkRequisition *req);

static void mx_gtk_light_switch_style_set (GtkWidget *lightswitch,
                                             GtkStyle *previous_style);

enum {
  SWITCH_FLIPPED,
  LAST_SIGNAL
};

static guint mx_gtk_light_switch_signals[LAST_SIGNAL] = { 0 };

typedef struct _MxGtkLightSwitchPrivate MxGtkLightSwitchPrivate;

struct _MxGtkLightSwitchPrivate {
  gboolean active; /* boolean state of switch */
  gboolean dragging; /* true if dragging switch */
  gint x;     /* the x position of the switch */
  gint drag_start; /* position dragging started at */
  gint drag_threshold;
  gint switch_width;
  gint switch_height;
  gint trough_width;
  gint offset; /* offset of the mouse to slider when dragging */
};

static void
mx_gtk_light_switch_class_init (MxGtkLightSwitchClass *klass)
{
  GObjectClass   *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (klass);
  widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->configure_event = mx_gtk_light_switch_configure;
  widget_class->expose_event = mx_gtk_light_switch_expose;
  widget_class->button_release_event = mx_gtk_light_switch_button_release;
  widget_class->button_press_event = mx_gtk_light_switch_button_press;
  widget_class->motion_notify_event = mx_gtk_light_switch_motion_notify;
  widget_class->size_request = mx_gtk_light_switch_size_request;
  widget_class->style_set = mx_gtk_light_switch_style_set;

  /* MxGtkLightSwitch signals */
  mx_gtk_light_switch_signals[SWITCH_FLIPPED] =
    g_signal_new ("switch-flipped",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MxGtkLightSwitchClass, switch_flipped),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  g_type_class_add_private (klass,
                            sizeof (MxGtkLightSwitchPrivate));
}

static void
mx_gtk_light_switch_init (MxGtkLightSwitch *self)
{
  MxGtkLightSwitchPrivate *priv;

  priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (self);
  priv->active = FALSE;
  priv->x = 0;

  /* add events, do initial draw/update, etc */
  gtk_widget_add_events (GTK_WIDGET (self),
                         GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK);
}

static void
draw (GtkWidget *lightswitch,
      cairo_t   *cr)
{
  MxGtkLightSwitchPrivate *priv;

  gint on_label_x;
  gint off_label_x;
  gint label_width;
  gint label_height;
  GtkStyle     *style;
  PangoLayout  *layout;
  PangoContext *context;
  GtkStateType state_type;

  priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);
  style = lightswitch->style;
  state_type = GTK_WIDGET_STATE (lightswitch);

  on_label_x = (priv->trough_width / 5) * 0.75;
  off_label_x = (priv->trough_width / 8) * 5;

  /* draw the trough */
  gtk_paint_box (style,
                 lightswitch->window,
                 (priv->active) ? GTK_STATE_SELECTED : state_type,
                 GTK_SHADOW_IN,
                 NULL,
                 NULL,
                 "light-switch-trough",
                 0,
                 0,
                 (priv->trough_width),
                 priv->switch_height);

  if (state_type == GTK_STATE_INSENSITIVE)
    {
      context = gdk_pango_context_get ();
      layout = pango_layout_new (context);
      g_object_unref (context);

      pango_layout_set_font_description (layout, style->font_desc);
      pango_layout_set_text (layout, _("Unavailable"), -1);
      pango_layout_get_size (layout, &label_width, &label_height);
      gtk_paint_layout (style, lightswitch->window, state_type, FALSE,
                        NULL, lightswitch, "lighswitch-label",
                        (priv->trough_width - (label_width / PANGO_SCALE)) / 2,
                        (priv->switch_height - (label_height / PANGO_SCALE)) / 2,
                        layout);
      g_object_unref (layout);
      return;
    }

  /* Draw the first label; "On" */
  context = gdk_pango_context_get ();
  layout = pango_layout_new (context);
  g_object_unref (context);
  pango_layout_set_font_description (layout,
                                     style->font_desc);
  /* TRANSLATORS: If this string takes more than four or five characters in
   * your language, please use the MEDIUM VERTICAL BAR unicode character
   * instead */
  pango_layout_set_text (layout, _("On"), -1);
  pango_layout_get_size (layout,
                         &label_width,
                         &label_height);
  gtk_paint_layout (style,
                    lightswitch->window,
                    (priv->active) ? GTK_STATE_SELECTED : GTK_STATE_NORMAL,
                    FALSE,
                    NULL,
                    (GtkWidget*) lightswitch,
                    "lightswitch-label",
                    on_label_x,
                    (priv->switch_height
                     - (label_height / PANGO_SCALE)) / 2,
                    layout);
  /* Draw the second label; "Off" */
  /* TRANSLATORS: If this string takes more than four or five characters in
   * your language, please use the WHITE CIRCLE U+25CB unicode character
   * instead */
  pango_layout_set_text (layout, _("Off"), -1);
  pango_layout_get_size (layout,
                         &label_width,
                         &label_height);
  gtk_paint_layout (style,
                    lightswitch->window,
                    (priv->active) ? GTK_STATE_SELECTED : GTK_STATE_NORMAL,
                    FALSE,
                    NULL,
                    (GtkWidget*) lightswitch,
                    "lightswitch-label",
                    off_label_x,
                    (priv->switch_height
                     - (label_height / PANGO_SCALE)) / 2,
                    layout);

  /* draw the switch itself */
  gtk_paint_box (style,
                 lightswitch->window,
                 GTK_WIDGET_STATE (lightswitch),
                 GTK_SHADOW_OUT,
                 NULL,
                 NULL,
                 "light-switch-handle",
                 priv->x + style->xthickness,
                 style->ythickness,
                 priv->switch_width - style->xthickness * 2,
                 priv->switch_height - style->ythickness * 2);

  g_object_unref (layout);
}

static void
mx_gtk_light_switch_size_request (GtkWidget      *lightswitch,
                                    GtkRequisition *req)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  req->height = priv->switch_height;
  req->width = priv->trough_width;
}

static void
mx_gtk_light_switch_style_set (GtkWidget *lightswitch,
                                 GtkStyle   *previous_style)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);
  PangoLayout *layout;
  gint label_width, label_height;
  gint off_width, off_height;
  gint on_width, on_height;

  layout = gtk_widget_create_pango_layout (GTK_WIDGET (lightswitch), NULL);
  pango_layout_set_text (layout, _("Off"), -1);
  pango_layout_get_pixel_size (layout, &off_width, &off_height);
  pango_layout_set_text (layout, _("On"), -1);
  pango_layout_get_pixel_size (layout, &on_width, &on_height);
  g_object_unref (layout);

  label_width = MAX (off_width, on_width);
  label_height = MAX (off_height, on_height);

  priv->trough_width = MAX (label_width * 5, 80);
  priv->switch_width = (priv->trough_width / 2) * 1.1;
  priv->switch_height = (label_height * 2) * 1.1;
}

static gboolean
mx_gtk_light_switch_configure (GtkWidget         *lightswitch,
                                 GdkEventConfigure *event)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (priv->active)
    priv->x = priv->trough_width - priv->switch_width;
  else
    priv->x = 0;

  return FALSE;
}

static gboolean
mx_gtk_light_switch_expose (GtkWidget      *lightswitch,
                              GdkEventExpose *event)
{
  cairo_t *cr;
  cr = gdk_cairo_create (lightswitch->window);

  cairo_rectangle (cr,
                   event->area.x,
                   event->area.y,
                   event->area.width,
                   event->area.height);

  cairo_clip (cr);

  draw (lightswitch, cr);

  cairo_destroy (cr);

  return FALSE;
}

static gboolean
mx_gtk_light_switch_motion_notify (GtkWidget      *lightswitch,
                                     GdkEventMotion *event)
{
  MxGtkLightSwitchPrivate *priv;

  priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (ABS (event->x - priv->drag_start) < priv->drag_threshold)
    return TRUE;

  if (event->state & GDK_BUTTON1_MASK)
    {
      gint position = event->x - priv->offset;

      if (position > (priv->trough_width - priv->switch_width))
        priv->x = (priv->trough_width - priv->switch_width);
      else if (position < 0)
        priv->x = 0;
      else
        priv->x = position;

      priv->dragging = TRUE;
      gtk_widget_queue_draw ((GtkWidget *) lightswitch);
    }

  return TRUE;
}

/**
 * mx_gtk_light_switch_get_active:
 * @lightswitch: A #MxGtkLightSwitch
 *
 * Get the value of the "active" property
 *
 * Returns: #TRUE if the switch is "on"
 */
gboolean
mx_gtk_light_switch_get_active (MxGtkLightSwitch *lightswitch)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  return priv->active;
}

/**
 * mx_gtk_light_switch_set_active:
 * @lightswitch: A #MxGtkLightSwitch
 * @active: #TRUE to set the switch to its ON state
 *
 * Set the value of the "active" property
 *
 */
void
mx_gtk_light_switch_set_active (MxGtkLightSwitch *lightswitch,
                                  gboolean active)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (priv->active == active)
    {
      return;
    }
  else
    {
      priv->active = active;
      if (active == TRUE)
        {
          priv->x = priv->trough_width - priv->switch_width;
        }
      else
        {
          priv->x = 0;
        }

      gtk_widget_queue_draw ((GtkWidget *) lightswitch);

      g_signal_emit (lightswitch,
                     mx_gtk_light_switch_signals[SWITCH_FLIPPED],
                     0,
                     priv->active);
    }
}

static gboolean
mx_gtk_light_switch_button_press (GtkWidget      *lightswitch,
                                    GdkEventButton *event)
{
  MxGtkLightSwitchPrivate *priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (priv->active)
    priv->offset = event->x - (priv->trough_width - priv->switch_width);
  else
    priv->offset = event->x;

  priv->drag_start = event->x;

  g_object_get (gtk_widget_get_settings (lightswitch),
                "gtk-dnd-drag-threshold", &priv->drag_threshold,
                NULL);

  return FALSE;
}

static gboolean
mx_gtk_light_switch_button_release (GtkWidget *lightswitch,
                                      GdkEventButton *event)
{
  MxGtkLightSwitchPrivate *priv;

  priv = MX_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  /* detect whereabouts we are and "drop" into a state */
  if (priv->dragging)
    {
      priv->dragging = FALSE;
      if (priv->x + (priv->switch_width / 2) > priv->trough_width / 2)
        {
          mx_gtk_light_switch_set_active ((MxGtkLightSwitch *) lightswitch, TRUE);
          priv->x = priv->trough_width - priv->switch_width;
        }
      else
        {
          mx_gtk_light_switch_set_active ((MxGtkLightSwitch *) lightswitch, FALSE);
          priv->x = 0;
        }
      /* we always need to queue a redraw after dragging to put the slider back
       * in the correct place */
      gtk_widget_queue_draw ((GtkWidget *) lightswitch);
    }
  else
    {
      mx_gtk_light_switch_set_active ((MxGtkLightSwitch *) lightswitch, !priv->active);
    }

  return FALSE;
}

/**
 * mx_gtk_light_switch_new:
 *
 * Create a #MxGtkLightSwitch
 *
 * Returns: a newly allocated #MxGtkLightSwitch
 */
GtkWidget*
mx_gtk_light_switch_new (void)
{
  return g_object_new (MX_GTK_TYPE_LIGHT_SWITCH, NULL);
}

