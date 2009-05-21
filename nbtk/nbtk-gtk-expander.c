/* nbtk-gtk-expander.c */

#include "nbtk-gtk-expander.h"

struct _NbtkGtkExpanderPrivate
{
  gboolean is_open : 1;
  gboolean has_indicator : 1;

  gint indicator_size;
  gint indicator_padding; /* padding between indicator and label */
  gint child_padding; /* padding between label and child */

  GtkWidget *label;
  GdkWindow *event_window;
};

G_DEFINE_TYPE (NbtkGtkExpander, nbtk_gtk_expander, GTK_TYPE_BIN)

#define DEFAULT_INDICATOR_SIZE 12

gboolean
nbtk_gtk_expander_expose_event (GtkWidget      *widget,
                                GdkEventExpose *event)
{
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      NbtkGtkExpanderPrivate *priv = NBTK_GTK_EXPANDER (widget)->priv;
      GtkContainer *container = GTK_CONTAINER (widget);
      GtkShadowType shadow;
      GdkRectangle rect;
      GtkExpanderStyle style;

      if (priv->is_open)
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


      if (priv->has_indicator)
        {
          gint indicator_size, label_h, indicator_x, indicator_y;

          gtk_widget_style_get (widget, "expander-size", &indicator_size, NULL);
          indicator_x = rect.x + widget->style->xthickness + indicator_size / 2;

          if (priv->label)
            label_h = priv->label->allocation.height;
          else
            label_h = 0;


          indicator_y = rect.y + (widget->style->ythickness * 2 +
                                  MAX (label_h, indicator_size)) / 2;

          gtk_paint_expander (widget->style,
                              widget->window,
                              widget->state,
                              NULL,
                              widget,
                              NULL,
                              indicator_x,
                              indicator_y,
                              style);
        }

      if (priv->label)
        gtk_container_propagate_expose (container, priv->label, event);

      /*
      if (GTK_WIDGET_HAS_FOCUS (expander))
        gtk_expander_paint_focus (expander, &event->area);
      */

    }

  /* chain up to get child painted */
  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->expose_event (widget,
                                                                   event);

  return FALSE;
}

void
nbtk_gtk_expander_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;
  GtkWidget *child, *label;
  GtkAllocation label_alloc, child_alloc;
  GtkRequisition label_req = { 0, }, child_req;
  gint label_h;

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->size_allocate (widget,
                                                                    allocation);

  child = gtk_bin_get_child ((GtkBin*) widget);
  label = ((NbtkGtkExpander *) widget)->priv->label;

  if (label && GTK_WIDGET_VISIBLE (label))
    {
      gtk_widget_size_request (label, &label_req);

      label_alloc.x = allocation->x + widget->style->xthickness;
      label_alloc.y = allocation->y + widget->style->ythickness;
      label_alloc.width = label_req.width;
      label_alloc.height = label_req.height;

      if (priv->has_indicator)
        label_alloc.x += priv->indicator_size + priv->indicator_padding;


      gtk_widget_size_allocate (label, &label_alloc);

      label_h = MAX (priv->indicator_size, label_alloc.height);
    }
  else
    label_h = priv->indicator_size;

  if (priv->is_open && child && GTK_WIDGET_VISIBLE (child))
    {
      gtk_widget_size_request (child, &child_req);

      child_alloc.x = allocation->x + widget->style->xthickness;
      child_alloc.y = allocation->y + widget->style->ythickness + label_h
        + priv->child_padding;
      child_alloc.width = child_req.width;
      child_alloc.height = child_req.height;

      gtk_widget_size_allocate (child, &child_alloc);
    }


  if (priv->event_window)
    {
      gdk_window_move_resize (priv->event_window, allocation->x, allocation->y,
                              allocation->width, allocation->height);
    }

}

void
nbtk_gtk_expander_size_request (GtkWidget      *widget,
                                GtkRequisition *requisition)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;
  GtkWidget *child, *label;
  GtkRequisition req;


  child = gtk_bin_get_child ((GtkBin*) widget);
  label = ((NbtkGtkExpander *) widget)->priv->label;

  requisition->width = widget->style->xthickness * 2;
  requisition->height = widget->style->ythickness * 2;

  if (label && GTK_WIDGET_VISIBLE (label))
    {
      gtk_widget_size_request (label, &req);

      requisition->width += req.width;
      requisition->height += req.height;
    }

  if (priv->is_open && child && GTK_WIDGET_VISIBLE (child))
    {
      gtk_widget_size_request (child, &req);

      requisition->width = MAX (requisition->width,
                                req.width + widget->style->xthickness * 2);
      requisition->height += req.height + widget->style->ythickness;
    }
}

static void
nbtk_gtk_expander_map (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  if (priv->label)
    {
      gtk_widget_show (priv->label);
      gtk_widget_map (priv->label);
    }

  if (priv->event_window)
    gdk_window_show (priv->event_window);

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->map (widget);
}

static void
nbtk_gtk_expander_unmap (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  if (priv->label)
    gtk_widget_unmap (priv->label);

  if (priv->event_window)
    gdk_window_hide (priv->event_window);

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->unmap (widget);
}

static void
nbtk_gtk_expander_show (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  if (priv->label)
    gtk_widget_show (priv->label);

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->show (widget);
}

static void
nbtk_gtk_expander_hide (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  if (priv->label)
    gtk_widget_hide (priv->label);

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->hide (widget);
}

static gboolean
nbtk_gtk_expander_button_release (GtkWidget      *widget,
                                  GdkEventButton *event)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  priv->is_open = !priv->is_open;

  if (priv->is_open)
    gtk_widget_show (gtk_bin_get_child (GTK_BIN (widget)));
  else
    gtk_widget_hide (gtk_bin_get_child (GTK_BIN (widget)));

  gtk_widget_queue_resize (widget);

  return FALSE;
}

static void
nbtk_gtk_expander_realize (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;
  GdkWindowAttr attributes;

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->realize (widget);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.event_mask = gtk_widget_get_events (widget) |
                            GDK_BUTTON_PRESS_MASK        |
                            GDK_BUTTON_RELEASE_MASK      |
                            GDK_ENTER_NOTIFY_MASK        |
                            GDK_LEAVE_NOTIFY_MASK;

  priv->event_window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                       &attributes, GDK_WA_X | GDK_WA_Y);
  gdk_window_set_user_data (priv->event_window, widget);

  gdk_window_move_resize (priv->event_window,
                          widget->allocation.x,
                          widget->allocation.y,
                          widget->allocation.width,
                          widget->allocation.height);

}

static void
nbtk_gtk_expander_unrealize (GtkWidget *widget)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  if (priv->event_window)
    {
      gdk_window_destroy (priv->event_window);
      priv->event_window = NULL;
    }

  GTK_WIDGET_CLASS (nbtk_gtk_expander_parent_class)->unrealize (widget);
}

static void
nbtk_gtk_expander_style_set (GtkWidget *widget,
                             GtkStyle  *previous)
{
  NbtkGtkExpanderPrivate *priv = ((NbtkGtkExpander*) widget)->priv;

  priv->child_padding = widget->style->ythickness;
  priv->indicator_padding = widget->style->xthickness;

  gtk_widget_style_get (widget, "expander-size", &priv->indicator_size, NULL);
}

static void
nbtk_gtk_expander_class_init (NbtkGtkExpanderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (NbtkGtkExpanderPrivate));

  widget_class->expose_event = nbtk_gtk_expander_expose_event;
  widget_class->style_set = nbtk_gtk_expander_style_set;

  widget_class->size_request = nbtk_gtk_expander_size_request;
  widget_class->size_allocate = nbtk_gtk_expander_size_allocate;

  widget_class->realize = nbtk_gtk_expander_realize;
  widget_class->unrealize = nbtk_gtk_expander_unrealize;
  widget_class->map = nbtk_gtk_expander_map;
  widget_class->unmap = nbtk_gtk_expander_unmap;
  widget_class->show = nbtk_gtk_expander_show;
  widget_class->hide = nbtk_gtk_expander_hide;

  widget_class->button_release_event = nbtk_gtk_expander_button_release;

  pspec = g_param_spec_int ("expander-size",
                            "Expander Size",
                            "Size of the expander indicator",
                            0, G_MAXINT, DEFAULT_INDICATOR_SIZE,
                            G_PARAM_READABLE);

  gtk_widget_class_install_style_property (widget_class, pspec);


}

static void
nbtk_gtk_expander_init (NbtkGtkExpander *self)
{
  NbtkGtkExpanderPrivate *priv;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            NBTK_TYPE_GTK_EXPANDER,
                                            NbtkGtkExpanderPrivate);

  GTK_WIDGET_SET_FLAGS (self, GTK_CAN_FOCUS);

  priv = self->priv;

  priv->indicator_size = DEFAULT_INDICATOR_SIZE;
  priv->has_indicator = TRUE;
}

GtkWidget*
nbtk_gtk_expander_new (void)
{
  return g_object_new (NBTK_TYPE_GTK_EXPANDER, NULL);
}


void
nbtk_gtk_expander_set_label_widget (NbtkGtkExpander *expander,
                                    GtkWidget       *label)
{
  g_return_if_fail (NBTK_IS_GTK_EXPANDER (expander));
  g_return_if_fail (GTK_IS_WIDGET (label));

  if (expander->priv->label)
    gtk_widget_unparent (expander->priv->label);

  expander->priv->label = label;
  gtk_widget_set_parent (label, (GtkWidget*) expander);
}

GtkWidget*
nbtk_gtk_expander_get_label_widget (NbtkGtkExpander *expander)
{
  g_return_val_if_fail (NBTK_IS_GTK_EXPANDER (expander), NULL);

  return expander->priv->label;
}

void
nbtk_gtk_expander_set_has_indicator (NbtkGtkExpander *expander,
                                     gboolean         has_indicator)
{
  g_return_if_fail (NBTK_IS_GTK_EXPANDER (expander));

  expander->priv->has_indicator = has_indicator;

  gtk_widget_queue_resize ((GtkWidget*) expander);
}

gboolean
nbtk_gtk_expander_get_has_indicator (NbtkGtkExpander *expander)
{
  g_return_val_if_fail (NBTK_IS_GTK_EXPANDER (expander), 0);

  return expander->priv->has_indicator;
}
