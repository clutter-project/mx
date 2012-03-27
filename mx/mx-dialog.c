/*
 * mx-dialog.c
 *
 * Copyright 2010 Intel Corporation.
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
 *             Iain Holmes <iain@linux.intel.com>
 *
 */

/**
 * SECTION:mx-dialog
 * @short_description: a modal, single-widget container
 *
 * #MxDialog is a single-widget container. When presented, it performs a
 * suitable transition animation and blocks input to the actors beneath it
 * until it is hidden again.
 *
 * It also allows actions to be added to it, which will be represented as
 * buttons, using #MxButton.
 *
 * Since: 1.2
 */

#include "mx-dialog.h"
#include "mx-button-group.h"
#include "mx-offscreen.h"
#include "mx-private.h"
#include "mx-stylable.h"
#include "mx-utils.h"

static void mx_stylable_iface_init (MxStylableIface *iface);
static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxDialog, mx_dialog, MX_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_STYLABLE,
                                                mx_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE,
                                                mx_focusable_iface_init))

#define DIALOG_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_DIALOG, MxDialogPrivate))

typedef struct {
  MxAction     *action;
  ClutterActor *button;
} MxDialogAction;

struct _MxDialogPrivate
{
  guint visible          : 1;
  guint needs_allocation : 1;
  guint do_paint         : 1;
  guint child_has_focus  : 1;

  guint  transition_time;
  gfloat angle;

  ClutterActor    *blur;
  ClutterShader   *shader;

  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
  gfloat           zoom;

  /* Dialog-specific variables */
  ClutterActor  *background;
  ClutterActor  *button_box;
  MxButtonGroup *button_group;
  guint          spacing;
  GList         *actions;
};

static gchar *blur_shader =
  "uniform sampler2D tex;\n"
  "uniform float x_step, y_step;\n"

#if 0
  "void\n"
  "main ()\n"
  "  {\n"
  "    float u, v;\n"
  "    int count = 0;\n"
  "    vec4 color = vec4 (0.0, 0.0, 0.0, 0.0);\n"

  "    for (u = -1.0; u <= 1.0; u++)\n"
  "      for (v = -1.0; v <= 1.0; v++)\n"
  "        {\n"
  "          color += texture2D(tex, \n"
  "              vec2(cogl_tex_coord_in[0].s + u * x_step, \n"
  "                   cogl_tex_coord_in[0].t + v * y_step));\n"
  "          count ++;\n"
  "        }\n"

  "    color = color / float (count);\n"
  "    cogl_color_out = color * cogl_color_in;\n"
  "  }\n";
#else
  "vec4 get_rgba_rel(sampler2D tex, float dx, float dy)\n"
  "{\n"
  "  return texture2D (tex, cogl_tex_coord_in[0].st \n"
  "                         + vec2(dx, dy));\n"
  "}\n"

  "void\n"
  "main ()\n"
  "  {\n"
  "    vec4 color;\n"
  "    color =  get_rgba_rel (tex, -x_step, -y_step);\n"
  "    color += get_rgba_rel (tex, -x_step,  0.0);\n"
  "    color += get_rgba_rel (tex, -x_step,  y_step);\n"
  "    color += get_rgba_rel (tex,  0.0,    -y_step);\n"
  "    color += get_rgba_rel (tex,  0.0,     0.0);\n"
  "    color += get_rgba_rel (tex,  0.0,     y_step);\n"
  "    color += get_rgba_rel (tex,  x_step, -y_step);\n"
  "    color += get_rgba_rel (tex,  x_step,  0.0);\n"
  "    color += get_rgba_rel (tex,  x_step,  y_step);\n"
  "    color = color / 9.0;\n"
  "    cogl_color_out = color * cogl_color_in;\n"
  "  }\n";
#endif

static void mx_dialog_show (ClutterActor *self);
static void mx_dialog_hide (ClutterActor *self);

static int
next_p2 (gint a)
{
  /* find the next power of two */
  int rval = 1;

  while (rval < a)
    rval <<= 1;

  return rval;
}

static void
mx_dialog_texture_size_change_cb (ClutterActor *texture,
                                  gint          width,
                                  gint          height)
{
  clutter_actor_set_shader_param_float (texture,
                                        "x_step",
                                        (1.0f / next_p2 (width)) * 2);
  clutter_actor_set_shader_param_float (texture,
                                        "y_step",
                                        (1.0f / next_p2 (height)) * 2);
}

static MxFocusable *
mx_dialog_move_focus (MxFocusable      *focusable,
                      MxFocusDirection  direction,
                      MxFocusable      *from)
{
  ClutterActor *child;
  MxFocusHint hint = MX_FOCUS_HINT_PRIOR;
  MxDialog *self = MX_DIALOG (focusable);
  MxDialogPrivate *priv = self->priv;

  child = mx_bin_get_child (MX_BIN (focusable));
  if (child && !MX_IS_FOCUSABLE (child))
    child = NULL;

  focusable = NULL;
  switch (direction)
    {
    case MX_FOCUS_DIRECTION_PREVIOUS:
      hint = MX_FOCUS_HINT_LAST;
    case MX_FOCUS_DIRECTION_UP:
      if (!priv->child_has_focus && child)
        {
          priv->child_has_focus = TRUE;
          focusable = mx_focusable_accept_focus (MX_FOCUSABLE (child), hint);
        }
      break;

    case MX_FOCUS_DIRECTION_NEXT:
      hint = MX_FOCUS_HINT_FIRST;
    case MX_FOCUS_DIRECTION_DOWN:
      if (priv->child_has_focus && priv->actions)
        {
          priv->child_has_focus = FALSE;
          focusable =
            mx_focusable_accept_focus (MX_FOCUSABLE (priv->button_box), hint);
        }

    default:
      break;
    }

  if (focusable || MX_FOCUS_DIRECTION_OUT)
    return focusable;
  else
    return mx_focusable_accept_focus (MX_FOCUSABLE (self), hint);
}

static MxFocusable *
mx_dialog_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  ClutterActor *child;
  MxDialog *self = MX_DIALOG (focusable);
  MxDialogPrivate *priv = self->priv;

  child = mx_bin_get_child (MX_BIN (focusable));
  if (child && !MX_IS_FOCUSABLE (child))
    {
      hint = MX_FOCUS_HINT_PRIOR;
      child = NULL;
    }

  focusable = NULL;
  switch (hint)
    {
    default:
    case MX_FOCUS_HINT_PRIOR:
      if (child && priv->child_has_focus)
        focusable = mx_focusable_accept_focus (MX_FOCUSABLE (child), hint);
      if (focusable)
        break;

    case MX_FOCUS_HINT_LAST:
      priv->child_has_focus = FALSE;
      if (priv->actions)
        focusable = mx_focusable_accept_focus (MX_FOCUSABLE (priv->button_box),
                                               hint);
      if (focusable)
        break;

    case MX_FOCUS_HINT_FIRST:
      priv->child_has_focus = TRUE;
      if (child)
        focusable = mx_focusable_accept_focus (MX_FOCUSABLE (child), hint);
      break;
    }

  /* If we don't have a focusable child, we still return ourselves so
   * that a dialog can't lose focus.
   */
  return focusable ? focusable : MX_FOCUSABLE (self);
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->move_focus = mx_dialog_move_focus;
  iface->accept_focus = mx_dialog_accept_focus;
}

static void
mx_stylable_iface_init (MxStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
    {
      GParamSpec *pspec;

      is_initialized = TRUE;

      pspec = g_param_spec_uint ("x-mx-spacing",
                                 "Spacing",
                                 "The size of the spacing",
                                 0, G_MAXUINT, 12,
                                 MX_PARAM_READWRITE);
      mx_stylable_iface_install_property (iface, MX_TYPE_DIALOG, pspec);
    }
}

static void
mx_dialog_allocate_cb (ClutterActor           *parent,
                       const ClutterActorBox  *box,
                       ClutterAllocationFlags  flags,
                       MxDialog               *self)
{
  ClutterActorBox child_box;

  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_set_width (self, box->x2 - box->x1);
  clutter_actor_set_height (self, box->y2 - box->y1);
}

static void
mx_dialog_paint_cb (ClutterActor *parent,
                    ClutterActor *self)
{
  MxDialog *dialog = MX_DIALOG (self);
  MxDialogPrivate *priv = dialog->priv;

  if (priv->needs_allocation)
    {
      ClutterActorBox box;

      clutter_actor_get_allocation_box (parent, &box);
      mx_dialog_allocate_cb (parent,
                                  &box,
                                  CLUTTER_ALLOCATION_NONE,
                                  dialog);
    }

  priv->do_paint = TRUE;
  clutter_actor_paint (self);
}

static void
mx_dialog_pick_cb (ClutterActor *parent,
                   ClutterColor *color,
                   ClutterActor *self)
{
  clutter_actor_paint (self);
}

static void
mx_dialog_mapped_cb (ClutterActor *parent,
                     GParamSpec   *pspec,
                     ClutterActor *self)
{
  if (CLUTTER_ACTOR_IS_MAPPED (parent))
    clutter_actor_map (self);
  else
    clutter_actor_unmap (self);
}

static void
mx_dialog_dispose (GObject *object)
{
  ClutterActor *parent = clutter_actor_get_parent (CLUTTER_ACTOR (object));
  MxDialog *self = MX_DIALOG (object);
  MxDialogPrivate *priv = self->priv;

  if (parent)
    {
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_dialog_mapped_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_dialog_allocate_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_dialog_paint_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_dialog_pick_cb, self);
    }

  if (priv->blur)
    {
      clutter_actor_destroy (priv->blur);
      priv->blur = NULL;
    }

  if (priv->background)
    {
      clutter_actor_destroy (priv->background);
      priv->background = NULL;
    }

  if (priv->button_box)
    {
      clutter_actor_destroy (priv->button_box);
      priv->button_box = NULL;
    }

  if (priv->shader)
    {
      g_object_unref (priv->shader);
      priv->shader = NULL;
    }

  G_OBJECT_CLASS (mx_dialog_parent_class)->dispose (object);
}

static void
mx_dialog_finalize (GObject *object)
{
  MxDialog *self = (MxDialog *) object;
  MxDialogPrivate *priv = self->priv;

  if (priv->actions)
    {
      GList *a;

      for (a = priv->actions; a; a = a->next)
        g_slice_free (MxDialogAction, a->data);

      g_list_free (priv->actions);
    }

  G_OBJECT_CLASS (mx_dialog_parent_class)->finalize (object);
}

static void
mx_dialog_get_preferred_width (ClutterActor *actor,
                               gfloat        for_height,
                               gfloat       *min_width_p,
                               gfloat       *nat_width_p)
{
  ClutterActor *child;
  MxPadding padding, ipadding;
  gfloat child_width[2], button_width[2];

  MxDialog *self = MX_DIALOG (actor);
  MxDialogPrivate *priv = self->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  mx_widget_get_padding (MX_WIDGET (priv->background), &ipadding);

  /* If we're getting the preferred-width with a height, make
   * sure to take off the padding from the height, and the space
   * required for the button-box.
   */
  if (for_height >= 0)
    {
      gfloat button_height;

      if (priv->actions)
        clutter_actor_get_preferred_height (priv->button_box, -1,
                                            NULL, &button_height);
      else
        button_height = 0;

      for_height = MAX (0, for_height - padding.top - padding.bottom -
                           ipadding.top - ipadding.bottom -
                           button_height - priv->spacing);
    }

  /* Get the minimum/preferred width of the child */
  child = mx_bin_get_child (MX_BIN (actor));
  if (child)
    clutter_actor_get_preferred_width (child, for_height,
                                       &child_width[0], &child_width[1]);
  else
    child_width[0] = child_width[1] = 0;

  /* Get the minimum/preferred width of the button-box */
  if (priv->actions)
    clutter_actor_get_preferred_width (priv->button_box, -1,
                                       &button_width[0], &button_width[1]);
  else
    button_width[0] = button_width[1] = 0;

  /* Our preferred width is the maximum of the button-box width and the
   * child width, plus space for padding.
   */
  if (min_width_p)
    *min_width_p = MAX (child_width[0], button_width[0]) +
                   padding.left + padding.right +
                   ipadding.left + ipadding.right;
  if (nat_width_p)
    *nat_width_p = MAX (child_width[1], button_width[1]) +
                   padding.left + padding.right +
                   ipadding.left + ipadding.right;
}

static void
mx_dialog_get_preferred_height (ClutterActor *actor,
                                gfloat        for_width,
                                gfloat       *min_height_p,
                                gfloat       *nat_height_p)
{
  ClutterActor *child;
  MxPadding padding, ipadding;
  gfloat min_height, nat_height;

  MxDialog *self = MX_DIALOG (actor);
  MxDialogPrivate *priv = self->priv;

  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  mx_widget_get_padding (MX_WIDGET (priv->background), &ipadding);

  /* Adjust for-width for padding */
  if (for_width >= 0)
    for_width = MAX (0, for_width - padding.left - padding.right -
                        ipadding.left - ipadding.right);

  /* Calculate the height of the button-box + padding */
  if (priv->actions)
    clutter_actor_get_preferred_height (priv->button_box, for_width,
                                        &min_height, &nat_height);
  else
    min_height = nat_height = 0;
  min_height += padding.top + padding.bottom + ipadding.top + ipadding.bottom;
  nat_height += padding.top + padding.bottom + ipadding.top + ipadding.bottom;

  /* If the child exists, add its height + spacing to the result */
  child = mx_bin_get_child (MX_BIN (actor));
  if (child)
    {
      clutter_actor_get_preferred_height (child, for_width,
                                          min_height_p, nat_height_p);
      if (min_height_p)
        *min_height_p += min_height + priv->spacing;
      if (nat_height_p)
        *nat_height_p += nat_height + priv->spacing;
    }
  else
    {
      if (min_height_p)
        *min_height_p = min_height;
      if (nat_height_p)
        *nat_height_p = nat_height;
    }
}

static void
mx_dialog_allocate (ClutterActor           *actor,
                    const ClutterActorBox  *box,
                    ClutterAllocationFlags  flags)
{
  MxPadding padding;
  ClutterActor *child;
  gfloat button_width, button_height;
  ClutterActorBox inner_box, child_box;

  MxDialogPrivate *priv = MX_DIALOG (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->
    allocate (actor, box, flags);

  /* Get the available space */
  mx_widget_get_padding (MX_WIDGET (actor), &padding);
  inner_box.x1 = padding.left;
  inner_box.y1 = padding.top;
  inner_box.x2 = box->x2 - box->x1 - padding.right;
  inner_box.y2 = box->y2 - box->y1 - padding.bottom;

  /* Get the padding of the background box */
  mx_widget_get_padding (MX_WIDGET (priv->background), &padding);

  /* Get the size of the button-box */
  if (priv->actions)
    {
      clutter_actor_get_preferred_size (priv->button_box,
                                        NULL, NULL,
                                        &button_width, &button_height);
      button_height += priv->spacing;
    }
  else
    button_width = button_height = 0;

  /* Allocate the child */
  child = mx_bin_get_child (MX_BIN (actor));

  /* Calculate the available space for the child */
  child_box = inner_box;
  child_box.x1 += padding.left;
  child_box.y1 += padding.top;
  child_box.x2 -= padding.right;
  child_box.y2 -= padding.bottom + button_height;

  /* Allocate the child and work out the allocation box for the
   * button box.
   */
  if (child)
    {
      gboolean x_fill, y_fill, width;
      MxAlign x_align, y_align;

      mx_bin_get_fill (MX_BIN (actor), &x_fill, &y_fill);
      mx_bin_get_alignment (MX_BIN (actor), &x_align, &y_align);
      mx_allocate_align_fill (child, &child_box,
                              x_align, y_align,
                              x_fill, y_fill);

      width = child_box.x2 - child_box.x1;

      /* Adjust the allocation if the button-box is wider than
       * the dialog contents.
       */
      if (button_width > width)
        {
          child_box.x1 += (gint)((button_width - width)/2.f);
          child_box.x2 += (gint)((button_width - width)/2.f);
        }

      inner_box = child_box;
      inner_box.y2 += button_height;
      if (button_width > width)
        {
          inner_box.x1 = inner_box.x2 - button_width;
          child_box.x1 = inner_box.x1;
          child_box.x2 = child_box.x1 + width;
        }

      clutter_actor_allocate (child, &child_box, flags);

      child_box.y1 = child_box.y2 + priv->spacing;
      if (button_width > width)
        child_box.x2 = child_box.x1 + button_width;
      else
        child_box.x1 = child_box.x2 - button_width;
      child_box.y2 = child_box.y1 + button_height - priv->spacing;
    }
  else
    {
      child_box.x1 += (child_box.x2 - child_box.x1 - button_width) / 2.f;
      child_box.y1 += (child_box.y2 - child_box.y1 - button_height) / 2.f;
      child_box.x2 = child_box.x1 + button_width;
      child_box.y2 = child_box.y2 + button_height;

      inner_box = child_box;
    }

  /* Allocate the button-box if necessary */
  if (priv->actions)
    clutter_actor_allocate (priv->button_box, &child_box, flags);

  /* Allocate the background */
  inner_box.x1 -= padding.left;
  inner_box.y1 -= padding.top;
  inner_box.x2 += padding.right;
  inner_box.y2 += padding.bottom;

  clutter_actor_allocate (priv->background, &inner_box, flags);

  /* Allocate the blurred background actor */
  if (priv->blur)
    {
      child_box.x1 = 0;
      child_box.y1 = 0;
      child_box.x2 = box->x2 - box->x1;
      child_box.y2 = box->y2 - box->y1;

      clutter_actor_allocate (priv->blur, &child_box, flags);
    }

  priv->needs_allocation = FALSE;
}

static void
mx_dialog_paint (ClutterActor *actor)
{
  ClutterActor *child;
  gfloat width, height;
  MxDialogPrivate *priv = MX_DIALOG (actor)->priv;

  if (!priv->do_paint)
    return;

  priv->do_paint = FALSE;

  clutter_actor_get_size (actor, &width, &height);

  if (priv->blur)
    {
      CoglHandle material = cogl_material_new ();
      CoglHandle texture =
        clutter_texture_get_cogl_texture (CLUTTER_TEXTURE (priv->blur));

      cogl_material_set_color4ub (material, 0xff, 0xff, 0xff, 0xff);
      cogl_material_set_layer (material, 0, texture);
      cogl_set_source (material);

      cogl_rectangle (0, 0, width, height);

      clutter_actor_paint (priv->blur);
    }

  mx_widget_paint_background (MX_WIDGET (actor));

  cogl_translate (width/2, height/2, 0);
  cogl_scale (priv->zoom, priv->zoom, 1.f);
  cogl_rotate (priv->angle, 0, 0, 1);
  cogl_translate (-width/2, -height/2, 0);

  clutter_actor_paint (priv->background);

  child = mx_bin_get_child (MX_BIN (actor));
  if (child)
    clutter_actor_paint (child);

  if (priv->actions)
    clutter_actor_paint (priv->button_box);
}

static void
mx_dialog_pick (ClutterActor       *actor,
                const ClutterColor *color)
{
  gfloat width, height;
  ClutterGeometry geom;
  MxDialogPrivate *priv = MX_DIALOG (actor)->priv;

  /* Paint a rectangle over our allocation to block input to
   * other actors.
   */
  clutter_actor_get_geometry (actor, &geom);
  cogl_set_source_color4ub (color->red,
                            color->green,
                            color->blue,
                            color->alpha);
  cogl_rectangle (0, 0, geom.width, geom.height);

  clutter_actor_get_size (actor, &width, &height);
  cogl_translate (width/2, height/2, 0);
  cogl_scale (priv->zoom, priv->zoom, 1.f);
  cogl_rotate (priv->angle, 0, 0, 1);
  cogl_translate (-width/2, -height/2, 0);

  /* Chain up */
  CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->pick (actor, color);

  if (priv->actions)
    clutter_actor_paint (priv->button_box);
}

static void
mx_dialog_angle_cb (GObject    *window,
                    GParamSpec *pspec,
                    MxDialog   *dialog)
{
  g_object_get (window, "window-rotation-angle", &dialog->priv->angle, NULL);
  clutter_actor_queue_redraw (CLUTTER_ACTOR (dialog));
}

static void
mx_dialog_map (ClutterActor *actor)
{
  MxWindow *window;
  ClutterActor *stage;
  MxDialog *self = MX_DIALOG (actor);
  MxDialogPrivate *priv = self->priv;

  CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->map (actor);

  if (priv->blur)
    clutter_actor_map (priv->blur);

  clutter_actor_map (priv->background);
  clutter_actor_map (priv->button_box);

  stage = clutter_actor_get_parent (actor);
  if (CLUTTER_IS_STAGE (stage))
    {
      window = mx_window_get_for_stage (CLUTTER_STAGE (stage));
      if (window)
        {
          g_signal_connect (window, "notify::window-rotation-angle",
                            G_CALLBACK (mx_dialog_angle_cb), actor);
          mx_dialog_angle_cb (G_OBJECT (window), NULL, self);
        }
    }
}

static void
mx_dialog_unmap (ClutterActor *actor)
{
  MxWindow *window;
  ClutterActor *stage;
  MxDialogPrivate *priv = MX_DIALOG (actor)->priv;

  stage = clutter_actor_get_parent (actor);
  if (CLUTTER_IS_STAGE (stage))
    {
      window = mx_window_get_for_stage (CLUTTER_STAGE (stage));
      if (window)
        g_signal_handlers_disconnect_by_func (window, mx_dialog_angle_cb,
                                              actor);
    }

  if (priv->button_box)
    clutter_actor_unmap (priv->button_box);

  if (priv->background)
    clutter_actor_unmap (priv->background);

  if (priv->blur)
    clutter_actor_unmap (priv->blur);

  CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->unmap (actor);
}

static void
mx_dialog_apply_style (MxWidget *widget,
                       MxStyle  *style)
{
  MxDialogPrivate *priv = MX_DIALOG (widget)->priv;

  if (priv->background != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->background), style);

  if (priv->button_box != NULL)
    mx_stylable_set_style (MX_STYLABLE (priv->button_box), style);
}

static void
mx_dialog_class_init (MxDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxWidgetClass *widget_class = MX_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDialogPrivate));

  object_class->dispose = mx_dialog_dispose;
  object_class->finalize = mx_dialog_finalize;

  actor_class->get_preferred_width = mx_dialog_get_preferred_width;
  actor_class->get_preferred_height = mx_dialog_get_preferred_height;
  actor_class->allocate = mx_dialog_allocate;
  actor_class->paint = mx_dialog_paint;
  actor_class->pick = mx_dialog_pick;
  actor_class->map = mx_dialog_map;
  actor_class->unmap = mx_dialog_unmap;
  actor_class->show = mx_dialog_show;
  actor_class->hide = mx_dialog_hide;

  widget_class->apply_style = mx_dialog_apply_style;
}

static void
mx_dialog_parent_set_cb (ClutterActor *actor,
                         ClutterActor *old_parent,
                         MxDialog     *self)
{
  ClutterActor *parent = clutter_actor_get_parent (actor);

  if (old_parent)
    {
      MxDialogPrivate *priv = self->priv;

      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_dialog_mapped_cb, self);

      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_dialog_allocate_cb, self);
      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_dialog_paint_cb, self);
      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_dialog_pick_cb, self);

      priv->visible = FALSE;
    }

  if (parent)
    {
      g_signal_connect (parent, "notify::mapped",
                        G_CALLBACK (mx_dialog_mapped_cb), self);
    }
}

static void
mx_dialog_queue_relayout_cb (ClutterActor *actor,
                             MxDialog     *self)
{
  self->priv->needs_allocation = TRUE;
}

static void
mx_dialog_completed_cb (ClutterTimeline *timeline,
                        ClutterActor    *self)
{
  ClutterTimelineDirection direction;

  MxDialogPrivate *priv = MX_DIALOG (self)->priv;
  ClutterActor *parent = clutter_actor_get_parent (self);

  priv->zoom = 1.f;

  /* Reverse the direction and rewind the timeline. This means that when
   * a timeline finishes, its progress stays at 1.0, or 0.0 and it is
   * ready to start again.
   */
  direction = clutter_timeline_get_direction (timeline);
  clutter_timeline_set_direction (timeline,
                                  (direction == CLUTTER_TIMELINE_FORWARD) ?
                                  CLUTTER_TIMELINE_BACKWARD :
                                  CLUTTER_TIMELINE_FORWARD);

  if (direction == CLUTTER_TIMELINE_FORWARD)
    return;

  /* Finish hiding */
  CLUTTER_ACTOR_SET_FLAGS (self, CLUTTER_ACTOR_VISIBLE);
  CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->hide (self);

  if (priv->blur)
    {
      clutter_actor_destroy (priv->blur);
      priv->blur = NULL;
    }

  g_signal_handlers_disconnect_by_func (parent,
                                        mx_dialog_paint_cb, self);
  g_signal_handlers_disconnect_by_func (parent,
                                        mx_dialog_pick_cb, self);
  g_signal_handlers_disconnect_by_func (parent,
                                        mx_dialog_allocate_cb, self);
}

static void
mx_dialog_new_frame_cb (ClutterTimeline *timeline,
                        gint             msecs,
                        ClutterActor    *self)
{
  MxDialog *frame = MX_DIALOG (self);
  MxDialogPrivate *priv = frame->priv;
  ClutterActor *parent = clutter_actor_get_parent (self);
  gfloat opacity = clutter_alpha_get_alpha (priv->alpha);

  priv->zoom = 1.0f + (1.f - opacity) / 2.f;
  clutter_actor_set_opacity (self, (guint8)(opacity * 255.f));

  /* Queue a redraw on the parent, as having our hidden flag set will
   * short-circuit the redraw queued on ourselves via set_opacity.
   */
  if (parent)
    clutter_actor_queue_redraw (parent);
}

static void
mx_dialog_style_changed_cb (MxDialog *self)
{
  guint spacing;
  MxDialogPrivate *priv = self->priv;

  mx_stylable_get (MX_STYLABLE (self),
                   "x-mx-spacing", &spacing,
                   NULL);

  if (priv->spacing != spacing)
    {
      priv->spacing = spacing;
      clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
    }
}

static void
mx_dialog_init (MxDialog *self)
{
  ClutterActor *actor = CLUTTER_ACTOR (self);
  MxDialogPrivate *priv = self->priv = DIALOG_PRIVATE (self);

  priv->transition_time = 250;
  priv->timeline = clutter_timeline_new (priv->transition_time);
  priv->alpha = clutter_alpha_new_full (priv->timeline,
                                        CLUTTER_EASE_OUT_QUAD);

  priv->background = mx_frame_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->background),
                               "MxDialogBackground");

  priv->button_box = mx_box_layout_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->button_box),
                               "MxDialogButtonBox");

  priv->button_group = mx_button_group_new ();
  priv->child_has_focus = TRUE;

  clutter_actor_push_internal (actor);
  clutter_actor_set_parent (priv->background, actor);
  clutter_actor_set_parent (priv->button_box, actor);
  clutter_actor_pop_internal (actor);

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (mx_dialog_completed_cb), self);
  g_signal_connect (priv->timeline, "new-frame",
                    G_CALLBACK (mx_dialog_new_frame_cb), self);

  g_signal_connect (self, "parent-set",
                    G_CALLBACK (mx_dialog_parent_set_cb), self);
  g_signal_connect (self, "queue-relayout",
                    G_CALLBACK (mx_dialog_queue_relayout_cb), self);
  g_signal_connect (self, "style-changed",
                    G_CALLBACK (mx_dialog_style_changed_cb), self);

  g_object_set (G_OBJECT (self), "show-on-set-parent", FALSE, NULL);
  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);

  /* Compile the shader when creating the instance so it's ready when we need
   * it */
  if (!CLUTTER_CHECK_VERSION (1, 10, 0) &&
      clutter_feature_available (CLUTTER_FEATURE_SHADERS_GLSL) &&
      clutter_feature_available (CLUTTER_FEATURE_OFFSCREEN))
    {
      GError *error = NULL;
      ClutterShader *shader;

      shader = clutter_shader_new ();
      clutter_shader_set_fragment_source (shader, blur_shader, -1);

      if (clutter_shader_compile (shader, &error))
        {
          priv->shader = shader;
        }
      else
        {
          g_warning (G_STRLOC ": Error compiling shader: %s", error->message);
          g_error_free (error);
          g_object_unref (shader);
        }
    }
}

/**
 * mx_dialog_new:
 *
 * Creates a new #MxDialog.
 *
 * Returns: A newly allocated #MxDialog
 *
 * Since: 1.2
 */
ClutterActor *
mx_dialog_new (void)
{
  return g_object_new (MX_TYPE_DIALOG, NULL);
}

/**
 * mx_dialog_set_transient_parent:
 * @dialog: A #MxDialog
 * @actor: A #ClutterActor
 *
 * Sets the parent of the #MxDialog. This is the actor over which the
 * modal frame will appear when clutter_actor_show() is called.
 *
 * Since: 1.2
 */
void
mx_dialog_set_transient_parent (MxDialog *dialog,
                                ClutterActor *actor)
{
  g_return_if_fail (MX_IS_DIALOG (dialog));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));

  clutter_actor_push_internal (actor);
  clutter_actor_set_parent (CLUTTER_ACTOR (dialog), actor);
  clutter_actor_pop_internal (actor);
}

static void
mx_dialog_steal_focus (MxDialog *self)
{
  ClutterActor *stage = clutter_actor_get_stage (CLUTTER_ACTOR (self));
  if (stage)
    {
      MxFocusManager *manager =
        mx_focus_manager_get_for_stage (CLUTTER_STAGE (stage));
      mx_focus_manager_push_focus (manager, MX_FOCUSABLE (self));
    }
}

static void
mx_dialog_show (ClutterActor *self)
{
  MxDialog *dialog = MX_DIALOG (self);
  MxDialogPrivate *priv = dialog->priv;

  if (!priv->visible)
    {
      ClutterActor *parent = clutter_actor_get_parent (self);
      ClutterActorBox box;

      if (!parent)
        return;

      priv->visible = TRUE;

      if (clutter_timeline_is_playing (priv->timeline))
        {
          ClutterTimelineDirection direction =
            clutter_timeline_get_direction (priv->timeline);

          direction = (direction == CLUTTER_TIMELINE_FORWARD) ?
            CLUTTER_TIMELINE_BACKWARD : CLUTTER_TIMELINE_FORWARD;
          clutter_timeline_set_direction (priv->timeline, direction);

          CLUTTER_ACTOR_SET_FLAGS (self, CLUTTER_ACTOR_VISIBLE);
          mx_dialog_steal_focus (dialog);

          return;
        }

      /* Create the blurred background */
      if (priv->shader)
        {
          gint width, height;

          priv->blur = mx_offscreen_new ();
          clutter_actor_push_internal (self);
          clutter_actor_set_parent (priv->blur, self);
          clutter_actor_pop_internal (self);

          mx_offscreen_set_child (MX_OFFSCREEN (priv->blur), parent);
          clutter_actor_set_shader (priv->blur, priv->shader);

          g_signal_connect (priv->blur, "size-change",
                            G_CALLBACK (mx_dialog_texture_size_change_cb),
                            NULL);
          clutter_texture_get_base_size (CLUTTER_TEXTURE (priv->blur),
                                         &width, &height);
          mx_dialog_texture_size_change_cb (priv->blur, width, height);
          clutter_actor_set_shader_param_int (priv->blur, "tex", 0);

        }

      /* Hook onto signals necessary for drawing */
      priv->needs_allocation = TRUE;
      g_signal_connect (parent, "allocation-changed",
                        G_CALLBACK (mx_dialog_allocate_cb),
                        dialog);
      g_signal_connect_after (parent, "paint",
                              G_CALLBACK (mx_dialog_paint_cb),
                              dialog);
      g_signal_connect_after (parent, "pick",
                              G_CALLBACK (mx_dialog_pick_cb),
                              dialog);

      clutter_actor_set_opacity (self, 0x00);
      CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->show (self);
      clutter_alpha_set_mode (priv->alpha, CLUTTER_EASE_OUT_QUAD);
      clutter_timeline_start (priv->timeline);

      mx_dialog_steal_focus (dialog);


      /* ensure the initial size is correct */
      clutter_actor_get_allocation_box (parent, &box);
      mx_dialog_allocate_cb (parent, &box, CLUTTER_ALLOCATION_NONE, dialog);
    }
}

static void
mx_dialog_hide (ClutterActor *self)
{
  MxDialog *dialog = MX_DIALOG (self);
  MxDialogPrivate *priv = dialog->priv;

  if (priv->visible)
    {
      ClutterActor *stage;
      ClutterActor *parent = clutter_actor_get_parent (self);

      priv->visible = FALSE;

      if (!parent)
        {
          CLUTTER_ACTOR_CLASS (mx_dialog_parent_class)->hide (self);
          return;
        }

      CLUTTER_ACTOR_UNSET_FLAGS (self, CLUTTER_ACTOR_VISIBLE);

      stage = clutter_actor_get_stage (self);
      if (stage)
        {
          MxFocusManager *manager =
            mx_focus_manager_get_for_stage (CLUTTER_STAGE (stage));
          mx_focus_manager_move_focus (manager, MX_FOCUS_DIRECTION_OUT);
        }

      if (clutter_timeline_is_playing (priv->timeline))
        {
          ClutterTimelineDirection direction =
            clutter_timeline_get_direction (priv->timeline);

          direction = (direction == CLUTTER_TIMELINE_FORWARD) ?
            CLUTTER_TIMELINE_BACKWARD : CLUTTER_TIMELINE_FORWARD;
          clutter_timeline_set_direction (priv->timeline, direction);

          return;
        }

      /* The timeline is running in reverse, so use ease-in quad */
      clutter_alpha_set_mode (priv->alpha, CLUTTER_EASE_IN_QUAD);
      clutter_timeline_start (priv->timeline);
    }
}

/**
 * mx_dialog_add_action:
 * @dialog: A #MxDialog
 * @action: A #MxAction
 *
 * Adds an #MxButton that represents @action to the button area of @dialog
 *
 * Since: 1.2
 */
void
mx_dialog_add_action (MxDialog *dialog,
                      MxAction *action)
{
  MxDialogPrivate *priv;
  ClutterActor *button;
  MxDialogAction *da;

  g_return_if_fail (MX_IS_DIALOG (dialog));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = dialog->priv;

  button = mx_button_new ();
  mx_button_set_action (MX_BUTTON (button), action);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->button_box), button);
  mx_button_group_add (priv->button_group, (MxButton *) button);

  /* So we can maintain the two way relationship between action and button */
  da = g_slice_new (MxDialogAction);
  da->action = action;
  da->button = button;
  priv->actions = g_list_append (priv->actions, da);
}

/**
 * mx_dialog_remove_action:
 * @dialog: A #MxDialog
 * @action: A #MxAction
 *
 * Removes the button associated with @action from the button area of @dialog
 *
 * Since: 1.2
 */
void
mx_dialog_remove_action (MxDialog *dialog,
                         MxAction *action)
{
  MxDialogPrivate *priv;
  MxDialogAction *da;
  GList *a;

  g_return_if_fail (MX_IS_DIALOG (dialog));
  g_return_if_fail (MX_IS_ACTION (action));

  priv = dialog->priv;

  da = NULL;
  for (a = priv->actions; a; a = a->next)
    {
      MxDialogAction *data = a->data;

      if (data->action == action)
        {
          priv->actions = g_list_delete_link (priv->actions, a);
          da = data;
          break;
        }
    }

  if (da == NULL)
    {
      g_warning ("Action '%s' was not found in dialog",
                 mx_action_get_name (action));
      return;
    }

  mx_button_group_remove (priv->button_group, MX_BUTTON (da->button));
  clutter_container_remove_actor (CLUTTER_CONTAINER (priv->button_box),
                                  da->button);
  g_slice_free (MxDialogAction, da);
}

/**
 * mx_dialog_get_actions:
 * @dialog: A #MxDialog
 *
 * Retrieves a list of actions added to @dialog.
 *
 * Returns: (transfer container) (element-type Mx.Action): A newly allocated
 *   #GList of #MxAction objects. The actions in the list are owned by the
 *   dialog.
 *
 * Since: 1.2
 */
GList *
mx_dialog_get_actions (MxDialog *dialog)
{
  GList *a, *list;
  MxDialogPrivate *priv;

  g_return_val_if_fail (MX_IS_DIALOG (dialog), NULL);

  priv = dialog->priv;
  list = NULL;

  for (a = priv->actions; a; a = a->next)
    {
      MxDialogAction *da = a->data;
      list = g_list_prepend (list, da->action);
    }

  return g_list_reverse (list);
}
