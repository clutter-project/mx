/*
 * mx-modal-frame.c
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
 *
 */

/**
 * SECTION:mx-modal-frame
 * @short_description: a modal, single-widget container
 *
 * #MxModalFrame is a single-widget container. When presented, it performs a
 * suitable transition animation and blocks input to the actors beneath it
 * until it is hidden again.
 */

#include "mx-modal-frame.h"
#include "mx-offscreen.h"

G_DEFINE_TYPE (MxModalFrame, mx_modal_frame, MX_TYPE_BIN)

#define MODAL_FRAME_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_MODAL_FRAME, MxModalFramePrivate))

struct _MxModalFramePrivate
{
  guint visible          : 1;
  guint needs_allocation : 1;
  guint do_paint         : 1;

  guint transition_time;

  ClutterActor    *blur;
  ClutterShader   *shader;

  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
};

#ifdef HAVE_COGL_GLES2

#define GLES2_VARS \
  "precision mediump float;\n" \
  "varying vec2 tex_coord;\n" \
  "varying vec4 frag_color;\n"
#define TEX_COORD "tex_coord"
#define COLOR_VAR "frag_color"

#else

#define GLES2_VARS ""
#define TEX_COORD "gl_TexCoord[0]"
#define COLOR_VAR "gl_Color"

#endif

static gchar *blur_shader =
  GLES2_VARS
  "uniform sampler2D tex;\n"
  "uniform float x_step, y_step;\n"

#if 0
  "void\n"
  "main ()\n"
  "  {\n"
  "    float u, v;\n"
  "    int count = 0;\n"
  "    vec4 color = vec4 (0, 0, 0, 0);\n"

  "    for (u = -1; u <= 1; u++)\n"
  "      for (v = -1; v <= 1; v++)\n"
  "        {\n"
  "          color += texture2D(tex, \n"
  "              vec2(" TEX_COORD ".s + u * x_step, \n"
  "                   " TEX_COORD ".t + v * y_step));\n"
  "          count ++;\n"
  "        }\n"

  "    color = color / float (count);\n"
  "    gl_FragColor = color * "COLOR_VAR";\n"
  "  }\n";
#else
  "vec4 get_rgba_rel(sampler2D tex, float dx, float dy)\n"
  "{\n"
  "  return texture2D (tex, " TEX_COORD ".st \n"
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
  "    color = color / 9;\n"
  "    gl_FragColor = color * "COLOR_VAR";\n"
  "  }\n";
#endif

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
mx_modal_frame_texture_size_change_cb (ClutterActor *texture,
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

static void
mx_modal_frame_allocate_cb (ClutterActor           *parent,
                            const ClutterActorBox  *box,
                            ClutterAllocationFlags  flags,
                            MxModalFrame           *self)
{
  ClutterActorBox child_box;

  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (self), &child_box, flags);
}

static void
mx_modal_frame_paint_cb (ClutterActor *parent,
                         ClutterActor *self)
{
  MxModalFrame *modal_frame = MX_MODAL_FRAME (self);
  MxModalFramePrivate *priv = modal_frame->priv;

  if (priv->needs_allocation)
    {
      ClutterActorBox box;

      clutter_actor_get_allocation_box (parent, &box);
      mx_modal_frame_allocate_cb (parent,
                                  &box,
                                  CLUTTER_ALLOCATION_NONE,
                                  modal_frame);
    }

  priv->do_paint = TRUE;
  clutter_actor_paint (self);
}

static void
mx_modal_frame_pick_cb (ClutterActor *parent,
                        ClutterColor *color,
                        ClutterActor *self)
{
  clutter_actor_paint (self);
}

static void
mx_modal_frame_mapped_cb (ClutterActor *parent,
                          GParamSpec   *pspec,
                          ClutterActor *self)
{
  if (CLUTTER_ACTOR_IS_MAPPED (parent))
    clutter_actor_map (self);
  else
    clutter_actor_unmap (self);
}

static void
mx_modal_frame_dispose (GObject *object)
{
  ClutterActor *parent = clutter_actor_get_parent (CLUTTER_ACTOR (object));
  MxModalFrame *self = MX_MODAL_FRAME (object);
  MxModalFramePrivate *priv = self->priv;

  if (parent)
    {
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_modal_frame_mapped_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_modal_frame_allocate_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_modal_frame_paint_cb, self);
      g_signal_handlers_disconnect_by_func (parent,
                                            mx_modal_frame_pick_cb, self);
    }

  if (priv->blur)
    {
      clutter_actor_destroy (priv->blur);
      priv->blur = NULL;
    }

  G_OBJECT_CLASS (mx_modal_frame_parent_class)->dispose (object);
}

static void
mx_modal_frame_allocate (ClutterActor           *actor,
                         const ClutterActorBox  *box,
                         ClutterAllocationFlags  flags)
{
  MxModalFramePrivate *priv = MX_MODAL_FRAME (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_modal_frame_parent_class)->
    allocate (actor, box, flags);

  mx_bin_allocate_child (MX_BIN (actor), box, flags);

  if (priv->blur)
    {
      ClutterActorBox child_box;

      child_box.x1 = 0;
      child_box.y1 = 0;
      child_box.x2 = box->x2 - box->x1;
      child_box.y2 = box->y2 - box->y1;

      clutter_actor_allocate (priv->blur, &child_box, flags);
    }

  priv->needs_allocation = FALSE;
}

static void
mx_modal_frame_paint (ClutterActor *actor)
{
  MxModalFramePrivate *priv = MX_MODAL_FRAME (actor)->priv;

  if (!priv->do_paint)
    return;

  priv->do_paint = FALSE;

  if (priv->blur)
    {
      CoglHandle material = cogl_material_new ();
      CoglHandle texture =
        clutter_texture_get_cogl_texture (CLUTTER_TEXTURE (priv->blur));

      cogl_material_set_color4ub (material, 0xff, 0xff, 0xff, 0xff);
      cogl_material_set_layer (material, 0, texture);
      cogl_set_source (material);

      cogl_rectangle (0, 0,
                      clutter_actor_get_width (actor),
                      clutter_actor_get_height (actor));

      clutter_actor_paint (priv->blur);
    }

  CLUTTER_ACTOR_CLASS (mx_modal_frame_parent_class)->paint (actor);
}

static void
mx_modal_frame_pick (ClutterActor       *actor,
                     const ClutterColor *color)
{
  ClutterGeometry geom;

  /* Paint a rectangle over our allocation to block input to
   * other actors.
   */
  clutter_actor_get_geometry (actor, &geom);
  cogl_set_source_color4ub (color->red,
                            color->green,
                            color->blue,
                            color->alpha);
  cogl_rectangle (0, 0, geom.width, geom.height);

  /* Chain up */
  CLUTTER_ACTOR_CLASS (mx_modal_frame_parent_class)->pick (actor, color);
}

static void
mx_modal_frame_map (ClutterActor *actor)
{
  MxModalFramePrivate *priv = MX_MODAL_FRAME (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_modal_frame_parent_class)->map (actor);

  if (priv->blur)
    clutter_actor_map (priv->blur);
}

static void
mx_modal_frame_unmap (ClutterActor *actor)
{
  MxModalFramePrivate *priv = MX_MODAL_FRAME (actor)->priv;

  if (priv->blur)
    clutter_actor_unmap (priv->blur);

  CLUTTER_ACTOR_CLASS (mx_modal_frame_parent_class)->unmap (actor);
}

static void
mx_modal_frame_class_init (MxModalFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxModalFramePrivate));

  object_class->dispose = mx_modal_frame_dispose;

  actor_class->allocate = mx_modal_frame_allocate;
  actor_class->paint = mx_modal_frame_paint;
  actor_class->pick = mx_modal_frame_pick;
  actor_class->map = mx_modal_frame_map;
  actor_class->unmap = mx_modal_frame_unmap;
}

static void
mx_modal_frame_parent_set_cb (ClutterActor *actor,
                              ClutterActor *old_parent,
                              MxModalFrame *self)
{
  ClutterActor *parent = clutter_actor_get_parent (actor);

  if (old_parent)
    {
      MxModalFramePrivate *priv = self->priv;

      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_modal_frame_mapped_cb, self);

      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_modal_frame_allocate_cb, self);
      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_modal_frame_paint_cb, self);
      g_signal_handlers_disconnect_by_func (old_parent,
                                            mx_modal_frame_pick_cb, self);

      priv->visible = FALSE;
    }

  if (parent)
    {
      g_signal_connect (parent, "notify::mapped",
                        G_CALLBACK (mx_modal_frame_mapped_cb), self);
    }
}

static void
mx_modal_frame_queue_relayout_cb (ClutterActor *actor,
                                  MxModalFrame *self)
{
  self->priv->needs_allocation = TRUE;
}

static void
mx_modal_frame_completed_cb (ClutterTimeline *timeline,
                             ClutterActor    *self)
{
  ClutterTimelineDirection direction;

  MxModalFramePrivate *priv = MX_MODAL_FRAME (self)->priv;
  ClutterActor *parent = clutter_actor_get_parent (self);

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
  clutter_actor_hide (self);

  if (priv->blur)
    {
      clutter_actor_destroy (priv->blur);
      priv->blur = NULL;
    }

  g_signal_handlers_disconnect_by_func (parent,
                                        mx_modal_frame_paint_cb, self);
  g_signal_handlers_disconnect_by_func (parent,
                                        mx_modal_frame_pick_cb, self);
  g_signal_handlers_disconnect_by_func (parent,
                                        mx_modal_frame_allocate_cb, self);

}

static void
mx_modal_frame_new_frame_cb (ClutterTimeline *timeline,
                             gint             msecs,
                             MxModalFrame    *frame)
{
  MxModalFramePrivate *priv = frame->priv;

  if (priv->blur)
    {
      gfloat opacity = clutter_alpha_get_alpha (priv->alpha);
      clutter_actor_set_opacity (CLUTTER_ACTOR (frame),
                                 (guint8)(opacity * 255.f));
    }
}

static void
mx_modal_frame_init (MxModalFrame *self)
{
  MxModalFramePrivate *priv = self->priv = MODAL_FRAME_PRIVATE (self);

  priv->transition_time = 250;
  priv->timeline = clutter_timeline_new (priv->transition_time);
  priv->alpha = clutter_alpha_new_full (priv->timeline,
                                        CLUTTER_EASE_OUT_QUAD);

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (mx_modal_frame_completed_cb), self);
  g_signal_connect (priv->timeline, "new-frame",
                    G_CALLBACK (mx_modal_frame_new_frame_cb),
                    self);

  g_signal_connect (self, "parent-set",
                    G_CALLBACK (mx_modal_frame_parent_set_cb), self);
  g_signal_connect (self, "queue-relayout",
                    G_CALLBACK (mx_modal_frame_queue_relayout_cb), self);

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
}

/**
 * mx_modal_frame_new:
 *
 * Creates a new #MxModalFrame.
 *
 * Returns: A newly allocated #MxModalFrame
 */
ClutterActor *
mx_modal_frame_new (void)
{
  return g_object_new (MX_TYPE_MODAL_FRAME, NULL);
}

/**
 * mx_modal_frame_set_transient_parent:
 * @modal_frame: A #MxModalFrame
 * @actor: A #ClutterActor
 *
 * Sets the parent of the #MxModalFrame. This is the actor over which the
 * modal frame will appear when mx_modal_frame_show() is called.
 */
void
mx_modal_frame_set_transient_parent (MxModalFrame *modal_frame,
                                     ClutterActor *actor)
{
  g_return_if_fail (MX_IS_MODAL_FRAME (modal_frame));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));

  clutter_actor_push_internal (actor);
  clutter_actor_set_parent (CLUTTER_ACTOR (modal_frame), actor);
  clutter_actor_pop_internal (actor);
}

/**
 * mx_modal_frame_show:
 * @modal_frame: A #MxModalFrame
 *
 * Shows the #MxModalFrame. When the frame is visible, it will block input
 * to its parent.
 */
void
mx_modal_frame_show (MxModalFrame *modal_frame)
{
  MxModalFramePrivate *priv;

  g_return_if_fail (MX_IS_MODAL_FRAME (modal_frame));

  priv = modal_frame->priv;

  if (!priv->visible)
    {
      ClutterActor *self = CLUTTER_ACTOR (modal_frame);
      ClutterActor *child = mx_bin_get_child (MX_BIN (self));
      ClutterActor *parent = clutter_actor_get_parent (self);

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

          if (child)
            {
              ClutterAnimation *animation = clutter_actor_get_animation (child);
              if (animation)
                {
                  ClutterTimeline *timeline =
                    clutter_animation_get_timeline (animation);
                  clutter_timeline_set_direction (timeline, direction);
                }
            }

          return;
        }

      /* Create the blurred background */
      if (clutter_feature_available (CLUTTER_FEATURE_SHADERS_GLSL))
        {
          GError *error = NULL;
          ClutterShader *shader;

          shader = clutter_shader_new ();
          clutter_shader_set_fragment_source (shader, blur_shader, -1);

          if (clutter_shader_compile (shader, &error))
            {
              gint width, height;

              priv->blur = mx_offscreen_new ();
              clutter_actor_push_internal (self);
              clutter_actor_set_parent (priv->blur, self);
              clutter_actor_pop_internal (self);

              mx_offscreen_set_child (MX_OFFSCREEN (priv->blur), parent);
              clutter_actor_set_shader (priv->blur, shader);
              g_object_unref (shader);

              g_signal_connect (priv->blur, "size-change",
                G_CALLBACK (mx_modal_frame_texture_size_change_cb), NULL);
              clutter_texture_get_base_size (CLUTTER_TEXTURE (priv->blur),
                                             &width, &height);
              mx_modal_frame_texture_size_change_cb (priv->blur, width, height);
              clutter_actor_set_shader_param_int (priv->blur, "tex", 0);

            }
          else
            {
              g_warning (G_STRLOC ": Error compiling shader: %s",
                         error->message);
              g_error_free (error);
            }
        }

      /* Hook onto signals necessary for drawing */
      priv->needs_allocation = TRUE;
      g_signal_connect (parent, "allocation-changed",
                        G_CALLBACK (mx_modal_frame_allocate_cb),
                        modal_frame);
      g_signal_connect_after (parent, "paint",
                              G_CALLBACK (mx_modal_frame_paint_cb),
                              modal_frame);
      g_signal_connect_after (parent, "pick",
                              G_CALLBACK (mx_modal_frame_pick_cb),
                              modal_frame);

      clutter_actor_set_opacity (self, 0x00);
      clutter_actor_show (self);
      clutter_alpha_set_mode (priv->alpha, CLUTTER_EASE_OUT_QUAD);
      clutter_timeline_start (priv->timeline);

      if (child)
        {
          clutter_actor_set_scale_with_gravity (child,
                                                1.5, 1.5,
                                                CLUTTER_GRAVITY_CENTER);
          clutter_actor_animate (child, CLUTTER_EASE_OUT_QUAD,
                                 priv->transition_time,
                                 "scale-x", 1.f,
                                 "scale-y", 1.f,
                                 "fixed::scale-gravity", CLUTTER_GRAVITY_CENTER,
                                 NULL);
        }
    }
}

/**
 * mx_modal_frame_hide:
 * @modal_frame: A #MxModalFrame
 *
 * Hides the #MxModalFrame.
 */
void
mx_modal_frame_hide (MxModalFrame *modal_frame)
{
  MxModalFramePrivate *priv;

  g_return_if_fail (MX_IS_MODAL_FRAME (modal_frame));

  priv = modal_frame->priv;

  if (priv->visible)
    {
      ClutterActor *self = CLUTTER_ACTOR (modal_frame);
      ClutterActor *child = mx_bin_get_child (MX_BIN (self));
      ClutterActor *parent = clutter_actor_get_parent (self);

      if (!parent)
        return;

      priv->visible = FALSE;

      if (clutter_timeline_is_playing (priv->timeline))
        {
          ClutterTimelineDirection direction =
            clutter_timeline_get_direction (priv->timeline);

          direction = (direction == CLUTTER_TIMELINE_FORWARD) ?
            CLUTTER_TIMELINE_BACKWARD : CLUTTER_TIMELINE_FORWARD;
          clutter_timeline_set_direction (priv->timeline, direction);

          if (child)
            {
              ClutterAnimation *animation = clutter_actor_get_animation (child);
              if (animation)
                {
                  ClutterTimeline *timeline =
                    clutter_animation_get_timeline (animation);
                  clutter_timeline_set_direction (timeline, direction);
                }
            }

          return;
        }

      if (child)
        clutter_actor_animate (child, CLUTTER_EASE_OUT_QUAD,
                               priv->transition_time,
                               "scale-x", 1.5f,
                               "scale-y", 1.5f,
                               "fixed::scale-gravity", CLUTTER_GRAVITY_CENTER,
                               NULL);

      /* The timeline is running in reverse, so use ease-in quad */
      clutter_alpha_set_mode (priv->alpha, CLUTTER_EASE_IN_QUAD);
      clutter_timeline_start (priv->timeline);
    }
}

