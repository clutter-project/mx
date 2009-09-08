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
#include "nbtk-gtk-frame.h"
#include <math.h>

static GdkColor nbtk_gtk_frame_default_border_color = { 0, 0xdddd, 0xe2e2, 0xe5e5 };

static void nbtk_gtk_frame_buildable_init (GtkBuildableIface *iface);
static void nbtk_gtk_frame_buildable_add_child (GtkBuildable *buildable,
                                                GtkBuilder   *builder,
                                                GObject      *child,
                                                const gchar  *type);

G_DEFINE_TYPE_WITH_CODE (NbtkGtkFrame, nbtk_gtk_frame, GTK_TYPE_FRAME,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, nbtk_gtk_frame_buildable_init))

static void
nbtk_gtk_frame_dispose (GObject *object)
{
  G_OBJECT_CLASS (nbtk_gtk_frame_parent_class)->dispose (object);
}

static void
nbtk_gtk_frame_finalize (GObject *object)
{
  G_OBJECT_CLASS (nbtk_gtk_frame_parent_class)->finalize (object);
}

static void
nbtk_gtk_frame_update_style (NbtkGtkFrame *frame)
{
  GdkColor *border_color;
  char *font = NULL;

  gtk_widget_style_get (GTK_WIDGET (frame),
                        "border-color", &border_color,
                        "title-font", &font,
                        NULL);

  if (border_color)
    {
      frame->border_color = *border_color;
      gdk_color_free (border_color);
    }
  else
    {
      frame->border_color = nbtk_gtk_frame_default_border_color;
    }

  if (font)
    {
      if (GTK_FRAME (frame)->label_widget)
        {
          PangoFontDescription *desc;
          desc = pango_font_description_from_string (font);
          gtk_widget_modify_font (GTK_FRAME (frame)->label_widget, desc);
          pango_font_description_free (desc);
        }
      g_free (font);
    }
}

static void
label_changed_cb (NbtkGtkFrame *frame)
{
  char *font = NULL;
  GtkFrame *gtk_frame = GTK_FRAME (frame);

  if (!gtk_frame->label_widget)
    return;

  /* ensure font is correct */
  gtk_widget_style_get (GTK_WIDGET (frame),
                        "title-font", &font,
                        NULL);
  if (font)
    {
      PangoFontDescription *desc;
      desc = pango_font_description_from_string (font);
      gtk_widget_modify_font (gtk_frame->label_widget, desc);
      pango_font_description_free (desc);
      g_free (font);
    }

  gtk_misc_set_alignment (GTK_MISC (gtk_frame->label_widget), 0.0, 1.0);
}
static void
rounded_rectangle (cairo_t * cr,
                   double x, double y, double w, double h,
                   guint radius)
{
  if (radius > w / 2)
    radius = w / 2;
  if (radius > h / 2)
    radius = h / 2;

  cairo_move_to (cr, x + radius, y);
  cairo_arc (cr, x + w - radius, y + radius, radius, M_PI * 1.5, M_PI * 2);
  cairo_arc (cr, x + w - radius, y + h - radius, radius, 0, M_PI * 0.5);
  cairo_arc (cr, x + radius, y + h - radius, radius, M_PI * 0.5, M_PI);
  cairo_arc (cr, x + radius, y + radius, radius, M_PI, M_PI * 1.5);
}

static void
nbtk_gtk_frame_paint (GtkWidget *widget, GdkRectangle *area)
{
  NbtkGtkFrame *frame = NBTK_GTK_FRAME (widget);
  cairo_t *cairo;
  GtkStyle *style;
  guint width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (NBTK_GTK_IS_FRAME (widget));
  g_return_if_fail (area != NULL);

  style = gtk_widget_get_style (widget);
  cairo = gdk_cairo_create (widget->window);
  width = gtk_container_get_border_width (GTK_CONTAINER (widget));

  /* clip to area */
  gdk_cairo_rectangle (cairo, area);
  cairo_clip (cairo);

  /* initialise the background */
  gdk_cairo_set_source_color (cairo, &style->bg[GTK_WIDGET_STATE (widget)]);
  cairo_rectangle (cairo, widget->allocation.x, widget->allocation.y,
                   widget->allocation.width, widget->allocation.height);
  cairo_fill (cairo);

  /* draw border */
  if (width != 0)
    {
      gdk_cairo_set_source_color (cairo, &frame->border_color);
      cairo_set_line_width (cairo, width);

      rounded_rectangle (cairo,
                         widget->allocation.x + (width / 2),
                         widget->allocation.y + (width / 2),
                         widget->allocation.width - (width),
                         widget->allocation.height - (width),
                         width);

      cairo_stroke (cairo);
    }

  cairo_destroy (cairo);
}

static gboolean
nbtk_gtk_frame_expose (GtkWidget *widget,
                       GdkEventExpose *event)
{
  GtkWidgetClass *grand_parent;

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      nbtk_gtk_frame_paint (widget, &event->area);

      grand_parent = GTK_WIDGET_CLASS (g_type_class_peek_parent (nbtk_gtk_frame_parent_class));
      grand_parent->expose_event (widget, event);
    }
  return FALSE;
}

static void
nbtk_gtk_frame_size_request (GtkWidget *widget,
                             GtkRequisition *requisition)
{
  GtkFrame *frame = GTK_FRAME (widget);
  GtkBin *bin = GTK_BIN (widget);
  GtkRequisition child_req;
  GtkRequisition title_req;

  child_req.width = child_req.height = 0;
  if (bin->child)
    gtk_widget_size_request (bin->child, &child_req);

  title_req.width = title_req.height = 0;
  if (frame->label_widget)
    {
      gtk_widget_size_request (frame->label_widget, &title_req);
    }

  requisition->width = MAX (child_req.width, title_req.width) +
                       2 * (GTK_CONTAINER (widget)->border_width +
                            GTK_WIDGET (widget)->style->xthickness);
  requisition->height = title_req.height + child_req.height +
                        2 * (GTK_CONTAINER (widget)->border_width +
                             GTK_WIDGET (widget)->style->ythickness);
}



static void
nbtk_gtk_frame_size_allocate (GtkWidget *widget,
                              GtkAllocation *allocation)
{
  GtkBin *bin = GTK_BIN (widget);
  GtkFrame *frame = GTK_FRAME (widget);
  GtkAllocation child_allocation, title_allocation;
  int xmargin, ymargin;

  widget->allocation = *allocation;
  xmargin = GTK_CONTAINER (widget)->border_width +
            widget->style->xthickness;
  ymargin = GTK_CONTAINER (widget)->border_width +
            widget->style->ythickness;

  title_allocation.height = title_allocation.width = 0;
  if (frame->label_widget)
    {
      GtkRequisition title_req;
      gtk_widget_get_child_requisition (frame->label_widget, &title_req);


      title_allocation.x = allocation->x + xmargin;
      title_allocation.y = allocation->y + ymargin;
      title_allocation.width = MIN (title_req.width,
                                    allocation->width - 2 * xmargin);
      title_allocation.height = title_req.height;
      gtk_widget_size_allocate (frame->label_widget, &title_allocation);

    }

  child_allocation.x = allocation->x + xmargin;
  child_allocation.y = allocation->y + ymargin + title_allocation.height;
  child_allocation.width = allocation->width - 2 * xmargin;
  child_allocation.height = allocation->height - 2 * ymargin - title_allocation.height;

  if (GTK_WIDGET_MAPPED (widget) &&
      (child_allocation.x != frame->child_allocation.x ||
       child_allocation.y != frame->child_allocation.y ||
       child_allocation.width != frame->child_allocation.width ||
       child_allocation.height != frame->child_allocation.height))
    {
      gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);
    }

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      gtk_widget_size_allocate (bin->child, &child_allocation);
    }

  frame->child_allocation = child_allocation;
}

static void nbtk_gtk_frame_style_set (GtkWidget *widget,
                                      GtkStyle *previous)
{
  NbtkGtkFrame *frame = NBTK_GTK_FRAME (widget);

  nbtk_gtk_frame_update_style (frame);

  GTK_WIDGET_CLASS (nbtk_gtk_frame_parent_class)->style_set (widget, previous);
}

static void
nbtk_gtk_frame_class_init (NbtkGtkFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GParamSpec *pspec;

  object_class->dispose = nbtk_gtk_frame_dispose;
  object_class->finalize = nbtk_gtk_frame_finalize;

  widget_class->expose_event = nbtk_gtk_frame_expose;
  widget_class->size_request = nbtk_gtk_frame_size_request;
  widget_class->size_allocate = nbtk_gtk_frame_size_allocate;
  widget_class->style_set = nbtk_gtk_frame_style_set;

  pspec = g_param_spec_boxed ("border-color",
                              "Border color",
                              "Color of the outside border",
                              GDK_TYPE_COLOR,
                              G_PARAM_READABLE);
  gtk_widget_class_install_style_property (widget_class, pspec);
  pspec = g_param_spec_string ("title-font",
                               "Title font",
                               "Pango font description string for title text",
                               "12",
                               G_PARAM_READWRITE);
  gtk_widget_class_install_style_property (widget_class, pspec);
}

static void
nbtk_gtk_frame_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder *builder,
                                    GObject *child,
                                    const gchar *type)
{
  if (!type)
    gtk_container_add (GTK_CONTAINER (buildable), GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (NBTK_GTK_FRAME (buildable), type);
}

static void
nbtk_gtk_frame_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = nbtk_gtk_frame_buildable_add_child;
}

static void
nbtk_gtk_frame_init (NbtkGtkFrame *self)
{
  g_signal_connect (self, "notify::label-widget",
                    G_CALLBACK (label_changed_cb), NULL);
}

/**
 * nbtk_gtk_frame_new:
 *
 * Create a new specially styled frame.
 *
 * Returns: a newly allocated #NbtkGtkFrame
 */
GtkWidget*
nbtk_gtk_frame_new (void)
{
  return g_object_new (NBTK_GTK_TYPE_FRAME,
                       "border-width", 4,
                       NULL);
}

