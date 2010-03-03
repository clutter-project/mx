/*
 * mx-deform-texture.c: A texture deformation actor
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

#include "mx-deform-texture.h"

G_DEFINE_ABSTRACT_TYPE (MxDeformTexture, mx_deform_texture, MX_TYPE_WIDGET)

#define DEFORM_TEXTURE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MX_TYPE_DEFORM_TEXTURE, MxDeformTexturePrivate))

struct _MxDeformTexturePrivate
{
  gint                tiles_x;
  gint                tiles_y;

  CoglHandle          vbo;
  gint                n_indices;
  CoglHandle          indices;
  CoglHandle          bf_indices;
  CoglTextureVertex  *vertices;

  CoglHandle          front_face;
  CoglHandle          back_face;

  ClutterActor       *front_actor;
  ClutterActor       *back_actor;
  CoglHandle          front_fbo;
  CoglHandle          back_fbo;

  gboolean            dirty;
};

enum
{
  PROP_0,

  PROP_TILES_X,
  PROP_TILES_Y,
  PROP_FRONT_FACE,
  PROP_BACK_FACE,
  PROP_FRONT_ACTOR,
  PROP_BACK_ACTOR
};

static void
mx_deform_texture_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (object)->priv;

  switch (property_id)
    {
    case PROP_TILES_X:
      g_value_set_int (value, priv->tiles_x);
      break;

    case PROP_TILES_Y:
      g_value_set_int (value, priv->tiles_y);
      break;

    case PROP_FRONT_FACE:
      g_value_set_pointer (value, priv->front_face);
      break;

    case PROP_BACK_FACE:
      g_value_set_pointer (value, priv->back_face);
      break;

    case PROP_FRONT_ACTOR:
      g_value_set_object (value, priv->front_actor);
      break;

    case PROP_BACK_ACTOR:
      g_value_set_object (value, priv->back_actor);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_texture_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  MxDeformTexture *texture = MX_DEFORM_TEXTURE (object);
  MxDeformTexturePrivate *priv = texture->priv;

  switch (property_id)
    {
    case PROP_TILES_X:
      mx_deform_texture_set_resolution (texture,
                                        g_value_get_int (value),
                                        priv->tiles_y);
      break;

    case PROP_TILES_Y:
      mx_deform_texture_set_resolution (texture,
                                        priv->tiles_x,
                                        g_value_get_int (value));
      break;

    case PROP_FRONT_FACE:
      mx_deform_texture_set_materials (texture,
                                       (CoglHandle)g_value_get_pointer (value),
                                       priv->back_face);
      break;

    case PROP_BACK_FACE:
      mx_deform_texture_set_materials (texture,
                                       priv->front_face,
                                       (CoglHandle)g_value_get_pointer (value));
      break;

    case PROP_FRONT_ACTOR:
      mx_deform_texture_set_actors (texture,
                                    (ClutterActor *)g_value_get_object (value),
                                    priv->back_actor);
      break;

    case PROP_BACK_ACTOR:
      mx_deform_texture_set_actors (texture,
                                    priv->front_actor,
                                    (ClutterActor *)g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_texture_free_arrays (MxDeformTexture *self)
{
  MxDeformTexturePrivate *priv = self->priv;

  if (priv->vbo)
    {
      cogl_handle_unref (priv->vbo);
      priv->vbo = NULL;
    }

  if (priv->indices)
    {
      cogl_handle_unref (priv->indices);
      priv->indices = NULL;
    }

  g_free (priv->vertices);
  priv->vertices = NULL;
}

static void
mx_deform_texture_dispose (GObject *object)
{
  MxDeformTexture *self = MX_DEFORM_TEXTURE (object);
  MxDeformTexturePrivate *priv = self->priv;

  mx_deform_texture_free_arrays (self);

  if (priv->front_face)
    {
      cogl_handle_unref (priv->front_face);
      priv->front_face = NULL;
    }

  if (priv->back_face)
    {
      cogl_handle_unref (priv->back_face);
      priv->back_face = NULL;
    }

  if (priv->front_actor)
    {
      clutter_actor_unparent (priv->front_actor);
      priv->front_actor = NULL;
    }

  if (priv->back_actor)
    {
      clutter_actor_unparent (priv->back_actor);
      priv->back_actor = NULL;
    }

  if (priv->front_fbo)
    {
      cogl_handle_unref (priv->front_fbo);
      priv->front_fbo = NULL;
    }

  if (priv->back_fbo)
    {
      cogl_handle_unref (priv->back_fbo);
      priv->back_fbo = NULL;
    }

  G_OBJECT_CLASS (mx_deform_texture_parent_class)->dispose (object);
}

static void
mx_deform_texture_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_deform_texture_parent_class)->finalize (object);
}

#if CLUTTER_CHECK_VERSION(1,2,0)
static void
mx_deform_texture_offscreen_buffer (ClutterActor  *actor,
                                    CoglHandle    *material,
                                    CoglHandle    *fbo,
                                    GObject       *object,
                                    const gchar   *notify)
{
  CoglMatrix matrix;
  CoglHandle texture;
  gfloat width, height;
  CoglColor zero_colour;

  clutter_actor_get_size (actor, &width, &height);

  /* Check our texture is the correct size */
  texture = NULL;
  if (*material)
    {
      const GList *layers = cogl_material_get_layers (*material);
      texture = cogl_material_layer_get_texture ((CoglHandle)layers->data);

      if ((cogl_texture_get_width (texture) != (guint)width) ||
          (cogl_texture_get_height (texture) != (guint)height))
        {
          cogl_handle_unref (*material);
          *material = NULL;
        }
    }

  /* Create the texture if necessary */
  if (!(*material))
    {
      texture = cogl_texture_new_with_size ((guint)width,
                                            (guint)height,
                                            COGL_TEXTURE_NO_SLICING,
                                            COGL_PIXEL_FORMAT_RGBA_8888_PRE);
      *material = cogl_material_new ();

      if (texture)
        {
          cogl_material_set_layer (*material, 0, texture);
          cogl_handle_unref (texture);
          if (object && notify)
            g_object_notify (object, notify);
        }

      /* Recreated the texture, get rid of the fbo */
      if (*fbo)
        {
          cogl_handle_unref (*fbo);
          *fbo = NULL;
        }
    }

  if (!texture)
    {
      g_debug ("Unable to create texture for actor");
      return;
    }

  /* Create fbo if necessary */
  if (!(*fbo))
    {
      gfloat z_camera;
      ClutterActor *stage;
      ClutterPerspective perspective;

      *fbo = cogl_offscreen_new_to_texture (texture);
      if (!(*fbo))
        {
          g_debug ("Unable to create fbo for actor");
          return;
        }

      /* Setup the viewport (code derived from Clutter) */
      /* FIXME: This code will eventually be a public function in Clutter,
       *        so replace this when it is.
       */
      cogl_push_framebuffer (*fbo);

      stage = clutter_actor_get_stage (actor);
      clutter_stage_get_perspective (CLUTTER_STAGE (stage), &perspective);
      clutter_actor_get_size (stage, &width, &height);

      cogl_set_viewport (0, 0, (int)width, (int)height);
      cogl_perspective (perspective.fovy,
                        perspective.aspect,
                        perspective.z_near,
                        perspective.z_far);

      cogl_get_projection_matrix (&matrix);
      z_camera = 0.5 * matrix.xx;

      cogl_matrix_init_identity (&matrix);
      cogl_matrix_translate (&matrix, -0.5f, -0.5f, -z_camera);
      cogl_matrix_scale (&matrix, 1.f / width, -1.f / height, 1.f / width);
      cogl_matrix_translate (&matrix, 0.f, -1.f * height, 0.f);

      cogl_set_modelview_matrix (&matrix);

      cogl_pop_framebuffer ();
    }

  /* Start drawing */
  cogl_push_framebuffer (*fbo);
  cogl_push_matrix ();

  /* Draw actor */
  cogl_color_set_from_4ub (&zero_colour, 0x00, 0x00, 0x00, 0x00);
  cogl_clear (&zero_colour,
              COGL_BUFFER_BIT_COLOR |
              COGL_BUFFER_BIT_STENCIL |
              COGL_BUFFER_BIT_DEPTH);
  clutter_actor_paint (actor);

  /* Restore state */
  cogl_pop_matrix ();
  cogl_pop_framebuffer ();
}
#endif

static void
mx_deform_texture_paint (ClutterActor *actor)
{
  gint i, j;
  gboolean depth, cull;

  MxDeformTexture *self = MX_DEFORM_TEXTURE (actor);
  MxDeformTexturePrivate *priv = self->priv;

  if (priv->dirty)
    {
      guint opacity;
      gfloat width, height;
      ClutterActorBox box;

      opacity = clutter_actor_get_paint_opacity (actor);
      clutter_actor_get_allocation_box (actor, &box);
      width = box.x2 - box.x1;
      height = box.y2 - box.y1;

      for (i = 0; i <= priv->tiles_y; i++)
        {
          for (j = 0; j <= priv->tiles_x; j++)
            {
              CoglTextureVertex *vertex =
                &priv->vertices[(i * (priv->tiles_x + 1)) + j];

              vertex->tx = j/(gfloat)priv->tiles_x;
              vertex->ty = i/(gfloat)priv->tiles_y;
              vertex->x = width * vertex->tx;
              vertex->y = height * vertex->ty;
              vertex->z = 0;
              cogl_color_set_from_4ub (&vertex->color,
                                       0xff, 0xff, 0xff, opacity);

              MX_DEFORM_TEXTURE_GET_CLASS (self)->
                deform (self, vertex, width, height);
            }
        }

      /* We add all three attributes again, although in an ideal case,
       * we'd add only those that had changed. Because we provide the
       * ability to change each, unless we had a 'changed' gboolean * in
       * the function prototype, we have to upload all of it.
       */
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_Vertex",
                              3,
                              COGL_ATTRIBUTE_TYPE_FLOAT,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->x);
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_MultiTexCoord0",
                              2,
                              COGL_ATTRIBUTE_TYPE_FLOAT,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->tx);
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_Color",
                              4,
                              COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->color);
      cogl_vertex_buffer_submit (priv->vbo);

      priv->dirty = FALSE;
    }

#if CLUTTER_CHECK_VERSION(1,2,0)
  /* Update FBOs if necessary */
  if (priv->front_actor)
    mx_deform_texture_offscreen_buffer (priv->front_actor,
                                        &priv->front_face,
                                        &priv->front_fbo,
                                        G_OBJECT (self),
                                        "front-face");
  if (priv->back_actor)
    mx_deform_texture_offscreen_buffer (priv->back_actor,
                                        &priv->back_face,
                                        &priv->back_fbo,
                                        G_OBJECT (self),
                                        "back-face");
#endif

  depth = cogl_get_depth_test_enabled ();
  if (!depth)
    cogl_set_depth_test_enabled (TRUE);

  cull = cogl_get_backface_culling_enabled ();
  if (priv->back_face && !cull)
    cogl_set_backface_culling_enabled (TRUE);
  else if (!priv->back_face && cull)
    cogl_set_backface_culling_enabled (FALSE);

  if (priv->front_face)
    {
      cogl_set_source (priv->front_face);
      cogl_vertex_buffer_draw_elements (priv->vbo,
                                        COGL_VERTICES_MODE_TRIANGLE_STRIP,
                                        priv->indices,
                                        0,
                                        (priv->tiles_x + 1) *
                                        (priv->tiles_y + 1),
                                        0,
                                        priv->n_indices);
    }

  if (priv->back_face)
    {
      cogl_set_source (priv->back_face);
      cogl_vertex_buffer_draw_elements (priv->vbo,
                                        COGL_VERTICES_MODE_TRIANGLE_STRIP,
                                        priv->bf_indices,
                                        0,
                                        (priv->tiles_x + 1) *
                                        (priv->tiles_y + 1),
                                        0,
                                        priv->n_indices);
    }

  if (!depth)
    cogl_set_depth_test_enabled (FALSE);
  if (priv->back_face && !cull)
    cogl_set_backface_culling_enabled (FALSE);
  else if (!priv->back_face && cull)
    cogl_set_backface_culling_enabled (TRUE);
}

static void
mx_deform_texture_get_preferred_width (ClutterActor *actor,
                                       gfloat        for_height,
                                       gfloat       *min_width_p,
                                       gfloat       *natural_width_p)
{
  CoglHandle layer;
  gfloat width, height;

  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front_actor || priv->back_actor)
    {
      clutter_actor_get_preferred_width (priv->front_actor ?
                                           priv->front_actor :
                                           priv->back_actor,
                                         for_height,
                                         min_width_p,
                                         natural_width_p);
    }
  else
    {
      if (priv->front_face)
        layer =
          g_list_nth_data ((GList *)cogl_material_get_layers (priv->front_face),
                           0);
      else if (priv->back_face)
        layer =
          g_list_nth_data ((GList *)cogl_material_get_layers (priv->back_face),
                           0);
      else
        layer = NULL;

      if (layer && (layer = cogl_material_layer_get_texture (layer)))
        {
          width = cogl_texture_get_width (layer);
          height = cogl_texture_get_height (layer);
        }
      else
        {
          if (min_width_p)
            *min_width_p = 0;
          if (natural_width_p)
            *natural_width_p = 0;

          return;
        }

      if (min_width_p)
        *min_width_p = 0;

      if (for_height >= 0)
        width = for_height / height * width;

      if (natural_width_p)
        *natural_width_p = width;
    }
}

static void
mx_deform_texture_get_preferred_height (ClutterActor *actor,
                                        gfloat        for_width,
                                        gfloat       *min_height_p,
                                        gfloat       *natural_height_p)
{
  CoglHandle layer;
  gfloat width, height;
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front_actor || priv->back_actor)
    {
      clutter_actor_get_preferred_height (priv->front_actor ?
                                            priv->front_actor :
                                            priv->back_actor,
                                          for_width,
                                          min_height_p,
                                          natural_height_p);
    }
  else
    {
      if (priv->front_face)
        layer =
          g_list_nth_data ((GList *)cogl_material_get_layers (priv->front_face),
                           0);
      else if (priv->back_face)
        layer =
          g_list_nth_data ((GList *)cogl_material_get_layers (priv->back_face),
                           0);
      else
        layer = NULL;

      if (layer && (layer = cogl_material_layer_get_texture (layer)))
        {
          width = cogl_texture_get_width (layer);
          height = cogl_texture_get_height (layer);
        }
      else
        {
          if (min_height_p)
            *min_height_p = 0;
          if (natural_height_p)
            *natural_height_p = 0;

          return;
        }

      if (min_height_p)
        *min_height_p = 0;

      if (for_width >= 0)
        height = for_width / width * height;

      if (natural_height_p)
        *natural_height_p = height;
    }
}

static void
mx_deform_texture_allocate (ClutterActor           *actor,
                            const ClutterActorBox  *box,
                            ClutterAllocationFlags  flags)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  /* The size has changed, so make sure we recalculate values */
  priv->dirty = TRUE;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (mx_deform_texture_parent_class)->
    allocate (actor, box, flags);

  if (priv->front_actor)
    clutter_actor_allocate_preferred_size (priv->front_actor, flags);

  if (priv->back_actor)
    clutter_actor_allocate_preferred_size (priv->back_actor, flags);
}

static void
mx_deform_texture_map (ClutterActor *actor)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_deform_texture_parent_class)->map (actor);

  if (priv->front_actor)
    clutter_actor_map (priv->front_actor);

  if (priv->back_actor)
    clutter_actor_map (priv->back_actor);
}

static void
mx_deform_texture_unmap (ClutterActor *actor)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front_actor)
    clutter_actor_unmap (priv->front_actor);

  if (priv->back_actor)
    clutter_actor_unmap (priv->back_actor);

  CLUTTER_ACTOR_CLASS (mx_deform_texture_parent_class)->unmap (actor);
}

static void
mx_deform_texture_class_init (MxDeformTextureClass *klass)
{
  GParamSpec *pspec;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MxDeformTexturePrivate));

  object_class->get_property = mx_deform_texture_get_property;
  object_class->set_property = mx_deform_texture_set_property;
  object_class->dispose = mx_deform_texture_dispose;
  object_class->finalize = mx_deform_texture_finalize;

  actor_class->get_preferred_width = mx_deform_texture_get_preferred_width;
  actor_class->get_preferred_height = mx_deform_texture_get_preferred_height;
  actor_class->allocate = mx_deform_texture_allocate;
  actor_class->paint = mx_deform_texture_paint;
  actor_class->map = mx_deform_texture_map;
  actor_class->unmap = mx_deform_texture_unmap;

  pspec = g_param_spec_int ("tiles-x",
                            "Horizontal tiles",
                            "Amount of horizontal tiles to split the "
                            "texture into.",
                            1, G_MAXINT, 32,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_NAME |
                            G_PARAM_STATIC_NICK |
                            G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_TILES_X, pspec);

  pspec = g_param_spec_int ("tiles-y",
                            "Vertical tiles",
                            "Amount of vertical tiles to split the "
                            "texture into.",
                            1, G_MAXINT, 32,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_NAME |
                            G_PARAM_STATIC_NICK |
                            G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_TILES_Y, pspec);

  pspec = g_param_spec_pointer ("front-face",
                                "Front-face",
                                "Front-face Cogl texture handle.",
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_NAME |
                                G_PARAM_STATIC_NICK |
                                G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_FRONT_FACE, pspec);

  pspec = g_param_spec_pointer ("back-face",
                                "Back-face",
                                "Back-face Cogl texture handle.",
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_NAME |
                                G_PARAM_STATIC_NICK |
                                G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_BACK_FACE, pspec);

  pspec = g_param_spec_object ("front-actor",
                               "Front-face actor",
                               "ClutterActor to use for the front-face.",
                               CLUTTER_TYPE_ACTOR,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_NAME |
                               G_PARAM_STATIC_NICK |
                               G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_FRONT_ACTOR, pspec);

  pspec = g_param_spec_object ("back-actor",
                               "Back-face actor",
                               "ClutterActor to use for the back-face.",
                               CLUTTER_TYPE_ACTOR,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_NAME |
                               G_PARAM_STATIC_NICK |
                               G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_BACK_ACTOR, pspec);
}

static void
mx_deform_texture_init_arrays (MxDeformTexture *self)
{
  GLushort *idx, *bf_idx;
  gint x, y, direction;
  GLushort *static_indices, *static_bf_indices;
  MxDeformTexturePrivate *priv = self->priv;

  mx_deform_texture_free_arrays (self);

  priv->n_indices = (2 + 2 * priv->tiles_x) *
                    priv->tiles_y +
                    (priv->tiles_y - 1);
  static_indices = g_new (GLushort, priv->n_indices);
  static_bf_indices = g_new (GLushort, priv->n_indices);

#define MESH_INDEX(X, Y) (Y) * (priv->tiles_x + 1) + (X)

  direction = 1;

  idx = static_indices;
  idx[0] = MESH_INDEX (0, 0);
  idx[1] = MESH_INDEX (0, 1);
  idx += 2;

  bf_idx = static_bf_indices;
  bf_idx[0] = MESH_INDEX (priv->tiles_x, 0);
  bf_idx[1] = MESH_INDEX (priv->tiles_x, 1);
  bf_idx += 2;

  for (y = 0; y < priv->tiles_y; y++)
    {
      for (x = 0; x < priv->tiles_x; x++)
        {
          /* Add 2 triangles for a quad */
          if (direction)
            {
              idx[0] = MESH_INDEX (x + 1, y);
              idx[1] = MESH_INDEX (x + 1, y + 1);
              bf_idx[0] = MESH_INDEX (priv->tiles_x - (x + 1), y);
              bf_idx[1] = MESH_INDEX (priv->tiles_x - (x + 1), y + 1);
            }
          else
            {
              idx[0] = MESH_INDEX (priv->tiles_x - x - 1, y);
              idx[1] = MESH_INDEX (priv->tiles_x - x - 1, y + 1);
              bf_idx[0] = MESH_INDEX (x + 1, y);
              bf_idx[1] = MESH_INDEX (x + 1, y + 1);
            }
          idx += 2;
          bf_idx += 2;
        }

      /* Link rows together to draw in one call */
      if (y == (priv->tiles_y - 1))
        break;

      if (direction)
        {
          idx[0] = MESH_INDEX (priv->tiles_x, y + 1);
          idx[1] = MESH_INDEX (priv->tiles_x, y + 1);
          idx[2] = MESH_INDEX (priv->tiles_x, y + 2);
          bf_idx[0] = MESH_INDEX (0, y + 1);
          bf_idx[1] = MESH_INDEX (0, y + 1);
          bf_idx[2] = MESH_INDEX (0, y + 2);
        }
      else
        {
          idx[0] = MESH_INDEX (0, y + 1);
          idx[1] = MESH_INDEX (0, y + 1);
          idx[2] = MESH_INDEX (0, y + 2);
          bf_idx[0] = MESH_INDEX (priv->tiles_x, y + 1);
          bf_idx[1] = MESH_INDEX (priv->tiles_x, y + 1);
          bf_idx[2] = MESH_INDEX (priv->tiles_x, y + 2);
        }

      idx += 3;
      bf_idx += 3;
      direction = !direction;
    }

  priv->indices =
    cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                    static_indices,
                                    priv->n_indices);
  priv->bf_indices =
    cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                    static_bf_indices,
                                    priv->n_indices);
  g_free (static_indices);
  g_free (static_bf_indices);

  priv->vertices = g_new (CoglTextureVertex,
                          (priv->tiles_x + 1) * (priv->tiles_y + 1));

  priv->vbo = cogl_vertex_buffer_new ((priv->tiles_x + 1) *
                                      (priv->tiles_y + 1));

  priv->dirty = TRUE;
}

static void
mx_deform_texture_init (MxDeformTexture *self)
{
  MxDeformTexturePrivate *priv = self->priv = DEFORM_TEXTURE_PRIVATE (self);

  priv->tiles_x = 32;
  priv->tiles_y = 32;
  mx_deform_texture_init_arrays (self);
}

void
mx_deform_texture_set_materials (MxDeformTexture *texture,
                                 CoglHandle       front_face,
                                 CoglHandle       back_face)
{
  MxDeformTexturePrivate *priv = texture->priv;

#if CLUTTER_CHECK_VERSION(1,2,0)
  /* Remove actor sources */
  if (front_face || back_face)
    {
      ClutterActor *front_actor, *back_actor;

      /* If we're setting the same face that was there before, maintain
       * whatever actor may have been there before too.
       */
      if (front_face != priv->front_face)
        front_actor = NULL;
      else
        front_actor = priv->front_actor;

      if (back_face != priv->back_face)
        back_actor = NULL;
      else
        back_actor = priv->back_actor;

      mx_deform_texture_set_actors (texture, front_actor, back_actor);
    }
#endif

  if (front_face != priv->front_face)
    {
      if (priv->front_face)
        cogl_handle_unref (priv->front_face);
      priv->front_face = front_face ? cogl_handle_ref (front_face) : NULL;
      g_object_notify (G_OBJECT (texture), "front-face");
    }

  if (back_face != priv->back_face)
    {
      if (priv->back_face)
        cogl_handle_unref (priv->back_face);
      priv->back_face = back_face ? cogl_handle_ref (back_face) : NULL;
      g_object_notify (G_OBJECT (texture), "back-face");
    }

  clutter_actor_queue_redraw (CLUTTER_ACTOR (texture));
}

void
mx_deform_texture_get_materials (MxDeformTexture *texture,
                                 CoglHandle      *front_face,
                                 CoglHandle      *back_face)
{
  MxDeformTexturePrivate *priv = texture->priv;

  if (front_face)
    *front_face = priv->front_face;
  if (back_face)
    *back_face = priv->back_face;
}

/**
 * mx_deform_texture_set_actors:
 * @front_actor: (allow-none): Actor to use for the front-face.
 * @back_actor: (allow-none): Actor to use for the back-face.
 *
 * Set actors to use as the sources of a deformation effect. Actors
 * must not be parented.
 */
void
mx_deform_texture_set_actors (MxDeformTexture *texture,
                              ClutterActor    *front_actor,
                              ClutterActor    *back_actor)
{
#if CLUTTER_CHECK_VERSION(1,2,0)
  MxDeformTexturePrivate *priv = texture->priv;

  /* Remove material sources */
  if (front_actor || back_actor)
    mx_deform_texture_set_materials (texture,
                                     (front_actor != priv->front_actor) ?
                                       NULL : priv->front_face,
                                     (back_actor != priv->back_actor) ?
                                       NULL : priv->back_face);

  if (front_actor != priv->front_actor)
    {
      if (priv->front_actor)
        {
          clutter_actor_unparent (priv->front_actor);
          priv->front_actor = NULL;
        }

      if (priv->front_fbo)
        {
          cogl_handle_unref (priv->front_fbo);
          priv->front_fbo = NULL;
        }

      if (front_actor)
        {
          priv->front_actor = front_actor;
          clutter_actor_set_parent (front_actor, CLUTTER_ACTOR (texture));
        }

      g_object_notify (G_OBJECT (texture), "front-actor");
    }

  if (back_actor != priv->back_actor)
    {
      if (priv->back_actor)
        {
          clutter_actor_unparent (priv->back_actor);
          priv->back_actor = NULL;
        }

      if (priv->back_fbo)
        {
          cogl_handle_unref (priv->back_fbo);
          priv->back_fbo = NULL;
        }

      if (back_actor)
        {
          priv->back_actor = back_actor;
          clutter_actor_set_parent (back_actor, CLUTTER_ACTOR (texture));
        }

      g_object_notify (G_OBJECT (texture), "back-actor");
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (texture));
#else
  if (front_actor)
    {
      g_object_ref_sink (front_actor);
      g_object_unref (front_actor);
    }
  if (back_actor)
    {
      g_object_ref_sink (back_actor);
      g_object_unref (back_actor);
    }
  g_warning ("Deforming actors requires Clutter 1.2.0");
#endif
}

void
mx_deform_texture_get_actors (MxDeformTexture  *texture,
                              ClutterActor    **front_actor,
                              ClutterActor    **back_actor)
{
  MxDeformTexturePrivate *priv = texture->priv;

  if (front_actor)
    *front_actor = priv->front_actor;
  if (back_actor)
    *back_actor = priv->back_actor;
}

/**
 * mx_deform_texture_set_actors:
 * @front_file: (allow-none): File-name to use for the front-face.
 * @back_file: (allow-none): File-name to use for the back-face.
 *
 * Set textures to use as the sources of a deformation effect. Textures
 * will be loaded from the files at the given paths.
 */
void
mx_deform_texture_set_from_files (MxDeformTexture *texture,
                                  const gchar     *front_file,
                                  const gchar     *back_file)
{
  CoglHandle front, back;

  GError *error = NULL;
  MxDeformTexturePrivate *priv = texture->priv;

  if (front_file && *front_file)
    {
      front = cogl_texture_new_from_file (front_file,
                                          COGL_TEXTURE_NO_SLICING,
                                          COGL_PIXEL_FORMAT_ANY,
                                          &error);
      if (error)
        {
          g_warning ("Error loading front face file: %s", error->message);
          g_error_free (error);
        }

      if (front)
        {
          CoglHandle material = cogl_material_new ();
          cogl_material_set_layer (material, 0, front);
          cogl_handle_unref (front);
          front = material;
        }
    }
  else
    front = front_file ? NULL : priv->front_face;

  if (back_file && *back_file)
    {
      back = cogl_texture_new_from_file (back_file,
                                         COGL_TEXTURE_NO_SLICING,
                                         COGL_PIXEL_FORMAT_ANY,
                                         &error);
      if (error)
        {
          g_warning ("Error loading back face file: %s", error->message);
          g_error_free (error);
        }

      if (back)
        {
          CoglHandle material = cogl_material_new ();
          cogl_material_set_layer (material, 0, back);
          cogl_handle_unref (back);
          back = material;
        }
    }
  else
    back = back_file ? NULL : priv->back_face;

  mx_deform_texture_set_materials (texture, front, back);

  if (front_file && *front_file)
    cogl_handle_unref (front);
  if (back_file && *back_file)
    cogl_handle_unref (back);
}

void
mx_deform_texture_get_resolution (MxDeformTexture *texture,
                                  gint            *tiles_x,
                                  gint            *tiles_y)
{
  MxDeformTexturePrivate *priv = texture->priv;

  if (tiles_x)
    *tiles_x = priv->tiles_x;
  if (tiles_y)
    *tiles_y = priv->tiles_y;
}

void
mx_deform_texture_set_resolution (MxDeformTexture *texture,
                                  gint             tiles_x,
                                  gint             tiles_y)
{
  MxDeformTexturePrivate *priv = texture->priv;
  gboolean changed = FALSE;

  g_return_if_fail ((tiles_x > 0) && (tiles_y > 0));

  if (priv->tiles_x != tiles_x)
    {
      priv->tiles_x = tiles_x;
      changed = TRUE;
      g_object_notify (G_OBJECT (texture), "tiles-x");
    }

  if (priv->tiles_y != tiles_y)
    {
      priv->tiles_y = tiles_y;
      changed = TRUE;
      g_object_notify (G_OBJECT (texture), "tiles-y");
    }

  if (changed)
    {
      mx_deform_texture_init_arrays (texture);
      mx_deform_texture_invalidate (texture);
    }
}

void
mx_deform_texture_invalidate (MxDeformTexture *texture)
{
  MxDeformTexturePrivate *priv = texture->priv;
  priv->dirty = TRUE;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (texture));
}
