/* nbtk-gtk-expander.c */

#include "nbtk-gtk-expander.h"

G_DEFINE_TYPE (NbtkGtkExpander, nbtk_gtk_expander, GTK_TYPE_EXPANDER)

/* taken from gtkexpander.c */
static void
get_expander_bounds (GtkExpander  *expander,
		     GdkRectangle *rect)
{
  GtkWidget *widget;
  gint border_width;
  gint expander_size;
  gint expander_spacing;
  gboolean interior_focus;
  gint focus_width;
  gint focus_pad;
  gboolean ltr;
  GtkWidget *label_widget = gtk_expander_get_label_widget (expander);

  widget = GTK_WIDGET (expander);

  border_width = GTK_CONTAINER (expander)->border_width;

  gtk_widget_style_get (widget,
			"interior-focus", &interior_focus,
			"focus-line-width", &focus_width,
			"focus-padding", &focus_pad,
			"expander-size", &expander_size,
			"expander-spacing", &expander_spacing,
			NULL);

  ltr = gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL;

  rect->x = widget->allocation.x + border_width + widget->style->xthickness;
  rect->y = widget->allocation.y + border_width + widget->style->ythickness;

  if (ltr)
    rect->x += expander_spacing;
  else
    rect->x += widget->allocation.width - 2 * border_width -
               expander_spacing - expander_size;

  if (label_widget && GTK_WIDGET_VISIBLE (label_widget))
    {
      GtkAllocation label_allocation;

      label_allocation = label_widget->allocation;

      if (expander_size < label_allocation.height)
	rect->y += focus_width + focus_pad + (label_allocation.height - expander_size) / 2;
      else
	rect->y += expander_spacing;
    }
  else
    {
      rect->y += expander_spacing;
    }

  if (!interior_focus)
    {
      if (ltr)
	rect->x += focus_width + focus_pad;
      else
	rect->x -= focus_width + focus_pad;
      rect->y += focus_width + focus_pad;
    }

  rect->width = rect->height = expander_size;
}


gboolean
nbtk_gtk_expander_expose_event (GtkWidget      *widget,
                                GdkEventExpose *event)
{
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      GtkExpander *expander = GTK_EXPANDER (widget);
      GtkContainer *container = GTK_CONTAINER (widget);
      GtkShadowType shadow;
      GdkRectangle rect, bounds;
      GtkExpanderStyle style;
      gint expander_size = 12;

      if (gtk_expander_get_expanded (expander))
        {
          shadow = GTK_SHADOW_IN;
          style = GTK_EXPANDER_EXPANDED;
        }
      else
        {
          shadow = GTK_SHADOW_OUT;
          style = GTK_EXPANDER_COLLAPSED;
        }

      rect.x = widget->allocation.x;
      rect.y = widget->allocation.y;
      rect.height = widget->allocation.height;
      rect.width = widget->allocation.width;

      gtk_paint_box (widget->style,
                     widget->window,
                     widget->state,
                     shadow,
                     &rect,
                     widget,
                     NULL,
                     rect.x,
                     rect.y,
                     rect.width,
                     rect.height);

      get_expander_bounds (expander, &bounds);

     gtk_widget_style_get (widget,
                           "expander-size", &expander_size,
                           NULL);

      gtk_paint_expander (widget->style,
                          widget->window,
                          widget->state,
                          &bounds,
                          widget,
                          NULL,
                          bounds.x + bounds.width / 2,
                          bounds.y + bounds.height / 2,
                          style);



      gtk_container_propagate_expose (container,
                                      gtk_bin_get_child (GTK_BIN (widget)),
                                      event);
      gtk_container_propagate_expose (container,
                                      gtk_expander_get_label_widget (expander),
                                      event);

      /*
      if (GTK_WIDGET_HAS_FOCUS (expander))
        gtk_expander_paint_focus (expander, &event->area);
      */

    }

  return FALSE;
}

static void
nbtk_gtk_expander_class_init (NbtkGtkExpanderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->expose_event = nbtk_gtk_expander_expose_event;
}

static void
nbtk_gtk_expander_init (NbtkGtkExpander *self)
{
}

GtkWidget*
nbtk_gtk_expander_new (void)
{
  return g_object_new (NBTK_TYPE_GTK_EXPANDER, NULL);
}
