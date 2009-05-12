#include <config.h>
#include "nbtk-gtk-light-switch.h"

#include <glib/gi18n-lib.h>

G_DEFINE_TYPE (NbtkGtkLightSwitch, nbtk_gtk_light_switch, GTK_TYPE_DRAWING_AREA)

#define NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE(o)                                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NBTK_GTK_TYPE_LIGHT_SWITCH, NbtkGtkLightSwitchPrivate))

static gboolean nbtk_gtk_light_switch_expose (GtkWidget      *lightswitch,
                                              GdkEventExpose *event);
static gboolean nbtk_gtk_light_switch_button_press (GtkWidget      *lightswitch,
                                                    GdkEventButton *event);
static gboolean nbtk_gtk_light_switch_button_release (GtkWidget      *lightswitch,
                                                      GdkEventButton *event);
static gboolean nbtk_gtk_light_switch_motion_notify (GtkWidget      *lightswitch,
                                                     GdkEventMotion *event);
static void nbtk_gtk_light_switch_size_request (GtkWidget      *lightswitch,
                                                GtkRequisition *req);

static void nbtk_gtk_light_switch_style_set (GtkWidget *lightswitch,
                                             GtkStyle *previous_style);

enum {
  SWITCH_FLIPPED,
  LAST_SIGNAL
};

static guint nbtk_gtk_light_switch_signals[LAST_SIGNAL] = { 0 };

typedef struct _NbtkGtkLightSwitchPrivate NbtkGtkLightSwitchPrivate;

struct _NbtkGtkLightSwitchPrivate {
  gboolean active; /* boolean state of switch */
  gboolean dragging; /* true if dragging switch */
  gboolean single; /* Single click detection */
  gint x;     /* the x position of the switch */
  gint switch_width;
  gint switch_height;
  gint trough_width;
};

static void
nbtk_gtk_light_switch_class_init (NbtkGtkLightSwitchClass *klass)
{
  GObjectClass   *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (klass);
  widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->expose_event = nbtk_gtk_light_switch_expose;
  widget_class->button_press_event = nbtk_gtk_light_switch_button_press;
  widget_class->button_release_event = nbtk_gtk_light_switch_button_release;
  widget_class->motion_notify_event = nbtk_gtk_light_switch_motion_notify;
  widget_class->size_request = nbtk_gtk_light_switch_size_request;
  widget_class->style_set = nbtk_gtk_light_switch_style_set;

  /* NbtkGtkLightSwitch signals */
  nbtk_gtk_light_switch_signals[SWITCH_FLIPPED] =
    g_signal_new ("switch-flipped",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (NbtkGtkLightSwitchClass, switch_flipped),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  g_type_class_add_private (klass,
                            sizeof (NbtkGtkLightSwitchPrivate));
}

static void
nbtk_gtk_light_switch_init (NbtkGtkLightSwitch *self)
{
  NbtkGtkLightSwitchPrivate *priv;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (self);
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
  NbtkGtkLightSwitchPrivate *priv;

  gint on_label_x;
  gint off_label_x;
  gint label_width;
  gint label_height;
  GtkStyle     *style;
  PangoLayout  *layout;
  PangoContext *context;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);
  style = lightswitch->style;

  on_label_x = (priv->trough_width / 5) * 0.75;
  off_label_x = (priv->trough_width / 8) * 5;

  /* draw the trough */
  gtk_paint_box (style,
                 lightswitch->window,
                 GTK_STATE_SELECTED,
                 GTK_SHADOW_IN,
                 NULL,
                 NULL,
                 NULL,
                 0,
                 0,
                 (priv->trough_width / 2),
                 priv->switch_height);
  gtk_paint_box (style,
                 lightswitch->window,
                 GTK_STATE_NORMAL,
                 GTK_SHADOW_IN,
                 NULL,
                 NULL,
                 NULL,
                 (priv->trough_width / 2),
                 0,
                 (priv->trough_width / 2),
                 priv->switch_height);

  /* Draw the first label; "On" */
  context = gdk_pango_context_get ();
  layout = pango_layout_new (context);
  g_object_unref (context);
  pango_layout_set_font_description (layout,
                                     style->font_desc);
  pango_layout_set_text (layout, _ ("On"), -1);
  pango_layout_get_size (layout,
                         &label_width,
                         &label_height);
  gdk_draw_layout (lightswitch->window,
                   style->fg_gc[GTK_STATE_SELECTED],
                   on_label_x,
                   (priv->switch_height
                    - (label_height / PANGO_SCALE)) / 2,
                   layout);
  /* Draw the second label; "Off" */
  pango_layout_set_text (layout, _ ("Off"), -1);
  pango_layout_get_size (layout,
                         &label_width,
                         &label_height);
  gdk_draw_layout (lightswitch->window,
                   style->fg_gc[GTK_STATE_NORMAL],
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
                 NULL,
                 priv->x,
                 0,
                 priv->switch_width,
                 priv->switch_height);

  g_object_unref (layout);
}

static void
nbtk_gtk_light_switch_size_request (GtkWidget      *lightswitch,
                                    GtkRequisition *req)
{
  NbtkGtkLightSwitchPrivate *priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  req->height = priv->switch_height;
  req->width = priv->trough_width;
}

static void
nbtk_gtk_light_switch_style_set (GtkWidget *lightswitch,
                                 GtkStyle   *previous_style)
{
  NbtkGtkLightSwitchPrivate *priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);
  PangoLayout *layout;
  gint label_width, label_height;
  gint off_width, off_height;
  gint on_width, on_height;

  layout = gtk_widget_create_pango_layout (GTK_WIDGET (lightswitch), NULL);
  pango_layout_set_text (layout, _ ("Off"), -1);
  pango_layout_get_pixel_size (layout, &off_width, &off_height);
  pango_layout_set_text (layout, _ ("On"), -1);
  pango_layout_get_pixel_size (layout, &on_width, &on_height);
  g_object_unref (layout);

  label_width = MAX (off_width, on_width);
  label_height = MAX (off_height, on_height);

  priv->trough_width = label_width * 5;
  priv->switch_width = (priv->trough_width / 2) * 1.1;
  priv->switch_height = (label_height * 2) * 1.1;
}

static gboolean
nbtk_gtk_light_switch_expose (GtkWidget      *lightswitch,
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
nbtk_gtk_light_switch_button_press (GtkWidget      *lightswitch,
                                    GdkEventButton *event)
{
  NbtkGtkLightSwitchPrivate *priv;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (event->x > priv->x && event->x < (priv->x + priv->switch_width))
    {
      /* we are in the lightswitch */
      priv->dragging = TRUE;
    }
  else
    {
      priv->single = TRUE;
    }

  return FALSE;
}

static void
emit_state_changed_signal (NbtkGtkLightSwitch *lightswitch,
                           int x)
{
  NbtkGtkLightSwitchPrivate *priv;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (priv->dragging == FALSE)
    {
      if (x > priv->trough_width / 2)
        {
          priv->x = priv->trough_width - priv->switch_width;
          priv->active = TRUE;
          g_signal_emit (lightswitch,
                         nbtk_gtk_light_switch_signals[SWITCH_FLIPPED],
                         0,
                         priv->active);
        }
      else
        {
          priv->x = 0;
          priv->active = FALSE;
          g_signal_emit (lightswitch,
                         nbtk_gtk_light_switch_signals[SWITCH_FLIPPED],
                         0,
                         priv->active);
        }
    }
  else
    {
      priv->x = x;
    }

  gtk_widget_queue_draw (GTK_WIDGET (lightswitch));
}

static gboolean
nbtk_gtk_light_switch_motion_notify (GtkWidget      *lightswitch,
                                     GdkEventMotion *event)
{
  NbtkGtkLightSwitchPrivate *priv;
  gint new_x;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  if (priv->dragging)
    {
      if (event->x > (priv->trough_width - priv->switch_width))
        new_x = (priv->trough_width - priv->switch_width);
      else if (event->x < 0)
        new_x = 0;
      else
        new_x = event->x;
      emit_state_changed_signal (NBTK_GTK_LIGHT_SWITCH (lightswitch),
                                 new_x);
    }

  return TRUE;
}

gboolean
nbtk_gtk_light_switch_get_active (NbtkGtkLightSwitch *lightswitch)
{
  NbtkGtkLightSwitchPrivate *priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  return priv->active;
}

void
nbtk_gtk_light_switch_set_active (NbtkGtkLightSwitch *lightswitch,
                                  gboolean active)
{
  NbtkGtkLightSwitchPrivate *priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

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
    }
}

static gboolean
nbtk_gtk_light_switch_button_release (GtkWidget *lightswitch,
                                      GdkEventButton *event)
{
  NbtkGtkLightSwitchPrivate *priv;

  priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);

  /* detect whereabouts we are and "drop" into a state */
  if (priv->dragging)
    {
      priv = NBTK_GTK_LIGHT_SWITCH_GET_PRIVATE (lightswitch);
      priv->dragging = FALSE;
      emit_state_changed_signal (NBTK_GTK_LIGHT_SWITCH (lightswitch),
                                 event->x);
    }
  else if (priv->single)
    {
      priv->single = FALSE;
      emit_state_changed_signal (NBTK_GTK_LIGHT_SWITCH (lightswitch),
                                 event->x);
    }

  /* If we aren't dragging we might want to just toggle */

  return FALSE;
}

GtkWidget*
nbtk_gtk_light_switch_new (void)
{
  return g_object_new (NBTK_GTK_TYPE_LIGHT_SWITCH, NULL);
}

