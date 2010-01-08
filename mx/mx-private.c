#include "mx-private.h"

/* Utility function to modify a child allocation box with respect to the
 * x/y-fill child properties. Expects childbox to contain the available
 * allocation space.
 */
void
_mx_allocate_fill (ClutterActor    *child,
                   ClutterActorBox *childbox,
                   MxAlign          x_alignment,
                   MxAlign          y_alignment,
                   gboolean         x_fill,
                   gboolean         y_fill)
{
  gfloat natural_width, natural_height;
  gfloat min_width, min_height;
  gfloat child_width, child_height;
  gfloat available_width, available_height;
  ClutterRequestMode request;
  ClutterActorBox allocation = { 0, };
  gdouble x_align, y_align;

  if (x_alignment == MX_ALIGN_START)
    x_align = 0.0;
  else if (x_alignment == MX_ALIGN_MIDDLE)
    x_align = 0.5;
  else
    x_align = 1.0;

  if (y_alignment == MX_ALIGN_START)
    y_align = 0.0;
  else if (y_alignment == MX_ALIGN_MIDDLE)
    y_align = 0.5;
  else
    y_align = 1.0;

  available_width  = childbox->x2 - childbox->x1;
  available_height = childbox->y2 - childbox->y1;

  if (available_width < 0)
    available_width = 0;

  if (available_height < 0)
    available_height = 0;

  if (x_fill)
    {
      allocation.x1 = childbox->x1;
      allocation.x2 = (int)(allocation.x1 + available_width);
    }

  if (y_fill)
    {
      allocation.y1 = childbox->y1;
      allocation.y2 = (int)(allocation.y1 + available_height);
    }

  /* if we are filling horizontally and vertically then we're done */
  if (x_fill && y_fill)
    {
      *childbox = allocation;
      return;
    }

  request = CLUTTER_REQUEST_HEIGHT_FOR_WIDTH;
  g_object_get (G_OBJECT (child), "request-mode", &request, NULL);

  if (request == CLUTTER_REQUEST_HEIGHT_FOR_WIDTH)
    {
      clutter_actor_get_preferred_width (child, available_height,
                                         &min_width,
                                         &natural_width);

      child_width = CLAMP (natural_width, min_width, available_width);

      clutter_actor_get_preferred_height (child, child_width,
                                          &min_height,
                                          &natural_height);

      child_height = CLAMP (natural_height, min_height, available_height);
    }
  else
    {
      clutter_actor_get_preferred_height (child, available_width,
                                          &min_height,
                                          &natural_height);

      child_height = CLAMP (natural_height, min_height, available_height);

      clutter_actor_get_preferred_width (child, child_height,
                                         &min_width,
                                         &natural_width);

      child_width = CLAMP (natural_width, min_width, available_width);
    }

  if (!x_fill)
    {
      allocation.x1 = childbox->x1 + (int)((available_width - child_width) * x_align);
      allocation.x2 = allocation.x1 + (int) child_width;
    }

  if (!y_fill)
    {
      allocation.y1 = childbox->y1 + (int)((available_height - child_height) * y_align);
      allocation.y2 = allocation.y1 + (int) child_height;
    }

  *childbox = allocation;

}

void
_mx_widget_set_clutter_text_attributes (MxWidget    *widget,
                                        ClutterText *text)
{
  ClutterColor *real_color = NULL;
  gchar *font_name = NULL;
  gint font_size = 0;
  MxFontWeight font_weight;
  PangoAttribute *attr;
  PangoWeight weight;
  PangoAttrList *attrs;

  mx_stylable_get (MX_STYLABLE (widget),
                   "color", &real_color,
                   "font-family", &font_name,
                   "font-size", &font_size,
                   "font-weight", &font_weight,
                   NULL);

  /* ensure there is an attribute list */
  attrs = clutter_text_get_attributes (text);
  if (!attrs)
    {
      attrs = pango_attr_list_new ();
      clutter_text_set_attributes (text, attrs);
    }

  /* font name */
  clutter_text_set_font_name (text, font_name);
  g_free (font_name);

  /* font size */
  attr = pango_attr_size_new_absolute (font_size * PANGO_SCALE);
  pango_attr_list_change (attrs, attr);

  /* font weight */
  switch (font_weight)
    {
  case MX_WEIGHT_BOLD:
    weight = PANGO_WEIGHT_BOLD;
    break;
  case MX_WEIGHT_LIGHTER:
    weight = PANGO_WEIGHT_LIGHT;
    break;
  case MX_WEIGHT_BOLDER:
    weight = PANGO_WEIGHT_HEAVY;
    break;
  default:
    weight = PANGO_WEIGHT_NORMAL;
    break;
    }
  attr = pango_attr_weight_new (weight);
  pango_attr_list_change (attrs, attr);

  /* font color */
  if (real_color)
    {
      clutter_text_set_color (text, real_color);
      clutter_color_free (real_color);
    }
}
