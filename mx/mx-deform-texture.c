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

/**
 * SECTION:mx-deform-texture
 * @short_description: Deformable texture abstract-widget
 *
 * An abstract widget that provides the interface for producing mesh
 * deformation effects with a texture.
 */

#include "mx-deform-texture.h"
#include "mx-offscreen.h"
#include "mx-private.h"

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

  ClutterActor       *front;
  ClutterActor       *back;

  gboolean            dirty;
};

enum
{
  PROP_0,

  PROP_TILES_X,
  PROP_TILES_Y,
  PROP_FRONT,
  PROP_BACK,
};

static void
mx_deform_texture_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
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

    case PROP_FRONT:
      g_value_set_object (value, priv->front);
      break;

    case PROP_BACK:
      g_value_set_object (value, priv->back);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mx_deform_texture_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
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

    case PROP_FRONT:
      mx_deform_texture_set_textures (texture,
                                      (ClutterTexture *)
                                      g_value_get_object (value),
                                      (ClutterTexture *)priv->back);
      break;

    case PROP_BACK:
      mx_deform_texture_set_textures (texture,
                                      (ClutterTexture *)priv->front,
                                      (ClutterTexture *)
                                      g_value_get_object (value));
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

  if (priv->front)
    {
      clutter_actor_unparent (priv->front);
      priv->front = NULL;
    }

  if (priv->back)
    {
      clutter_actor_unparent (priv->back);
      priv->back = NULL;
    }

  G_OBJECT_CLASS (mx_deform_texture_parent_class)->dispose (object);
}

static void
mx_deform_texture_finalize (GObject *object)
{
  G_OBJECT_CLASS (mx_deform_texture_parent_class)->finalize (object);
}

static void
mx_deform_texture_paint (ClutterActor *actor)
{
  gint i, j;
  gboolean depth, cull;
  CoglHandle front_material, back_material;

  MxDeformTexture *self = MX_DEFORM_TEXTURE (actor);
  MxDeformTexturePrivate *priv = self->priv;

  if (priv->dirty)
    {
      guint opacity;
      gfloat width, height;

      opacity = clutter_actor_get_paint_opacity (actor);
      clutter_actor_get_size (actor, &width, &height);

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

  /* Get materials and update FBOs if necessary */
  front_material = back_material = NULL;
  if (priv->front)
    {
      if (MX_IS_OFFSCREEN (priv->front) &&
          mx_offscreen_get_auto_update (MX_OFFSCREEN (priv->front)))
        mx_offscreen_update (MX_OFFSCREEN (priv->front));
      front_material =
        clutter_texture_get_cogl_material (CLUTTER_TEXTURE (priv->front));
    }
  if (priv->back)
    {
      if (MX_IS_OFFSCREEN (priv->back) &&
          mx_offscreen_get_auto_update (MX_OFFSCREEN (priv->back)))
        mx_offscreen_update (MX_OFFSCREEN (priv->back));
      back_material =
        clutter_texture_get_cogl_material (CLUTTER_TEXTURE (priv->back));
    }

  depth = cogl_get_depth_test_enabled ();
  if (!depth)
    cogl_set_depth_test_enabled (TRUE);

  cull = cogl_get_backface_culling_enabled ();
  if (back_material && !cull)
    cogl_set_backface_culling_enabled (TRUE);
  else if (!back_material && cull)
    cogl_set_backface_culling_enabled (FALSE);

  if (front_material)
    {
      cogl_set_source (front_material);
      cogl_vertex_buffer_draw_elements (priv->vbo,
                                        COGL_VERTICES_MODE_TRIANGLE_STRIP,
                                        priv->indices,
                                        0,
                                        (priv->tiles_x + 1) *
                                        (priv->tiles_y + 1),
                                        0,
                                        priv->n_indices);
    }

  if (back_material)
    {
      cogl_set_source (back_material);
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
  if (back_material && !cull)
    cogl_set_backface_culling_enabled (FALSE);
  else if (!back_material && cull)
    cogl_set_backface_culling_enabled (TRUE);
}

static void
mx_deform_texture_get_preferred_width (ClutterActor *actor,
                                       gfloat        for_height,
                                       gfloat       *min_width_p,
                                       gfloat       *natural_width_p)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front || priv->back)
    {
      clutter_actor_get_preferred_width (priv->front ?
                                           priv->front :
                                           priv->back,
                                         for_height,
                                         min_width_p,
                                         natural_width_p);
    }
  else
    {
      if (min_width_p)
        *min_width_p = 0;
      if (natural_width_p)
        *natural_width_p = 0;

      return;
    }
}

static void
mx_deform_texture_get_preferred_height (ClutterActor *actor,
                                        gfloat        for_width,
                                        gfloat       *min_height_p,
                                        gfloat       *natural_height_p)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front || priv->back)
    {
      clutter_actor_get_preferred_height (priv->front ?
                                            priv->front :
                                            priv->back,
                                          for_width,
                                          min_height_p,
                                          natural_height_p);
    }
  else
    {
      if (min_height_p)
        *min_height_p = 0;
      if (natural_height_p)
        *natural_height_p = 0;

      return;
    }
}

static void
mx_deform_texture_allocate (ClutterActor           *actor,
                            const ClutterActorBox  *box,
                            ClutterAllocationFlags  flags)
{
  ClutterActorBox child_box;
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  /* The size has changed, so make sure we recalculate values */
  priv->dirty = TRUE;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (mx_deform_texture_parent_class)->
    allocate (actor, box, flags);

  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y2 = box->y2 - box->y1;

  if (priv->front)
    clutter_actor_allocate (priv->front, &child_box, flags);

  if (priv->back)
    clutter_actor_allocate (priv->back, &child_box, flags);
}

static void
mx_deform_texture_map (ClutterActor *actor)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_deform_texture_parent_class)->map (actor);

  if (priv->front)
    clutter_actor_map (priv->front);

  if (priv->back)
    clutter_actor_map (priv->back);
}

static void
mx_deform_texture_unmap (ClutterActor *actor)
{
  MxDeformTexturePrivate *priv = MX_DEFORM_TEXTURE (actor)->priv;

  if (priv->front)
    clutter_actor_unmap (priv->front);

  if (priv->back)
    clutter_actor_unmap (priv->back);

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

  pspec = g_param_spec_object ("front",
                               "Front-face texture",
                               "ClutterTexture to use for the front-face.",
                               CLUTTER_TYPE_TEXTURE,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FRONT, pspec);

  pspec = g_param_spec_object ("back",
                               "Back-face texture",
                               "ClutterTexture to use for the back-face.",
                               CLUTTER_TYPE_TEXTURE,
                               MX_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_BACK, pspec);
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
/**
 * mx_deform_texture_set_textures:
 * @texture: an #MxDeformTexture
 * @front: (allow-none): #ClutterTexture to use for the front-face.
 * @back: (allow-none): #ClutterTexture to use for the back-face.
 *
 * Set textures to use as the sources of a deformation effect. Textures
 * must not be parented.
 */
void
mx_deform_texture_set_textures (MxDeformTexture *texture,
                                ClutterTexture  *front,
                                ClutterTexture  *back)
{
  MxDeformTexturePrivate *priv = texture->priv;

  if (front != (ClutterTexture *)priv->front)
    {
      if (priv->front)
        {
          clutter_actor_unparent (priv->front);
          priv->front = NULL;
        }

      if (front)
        {
          priv->front = (ClutterActor *)front;
          clutter_actor_set_parent (priv->front, CLUTTER_ACTOR (texture));
        }

      g_object_notify (G_OBJECT (texture), "front");
    }

  if (back != (ClutterTexture *)priv->back)
    {
      if (priv->back)
        {
          clutter_actor_unparent (priv->back);
          priv->back = NULL;
        }

      if (back)
        {
          priv->back = (ClutterActor *)back;
          clutter_actor_set_parent (priv->back, CLUTTER_ACTOR (texture));
        }

      g_object_notify (G_OBJECT (texture), "back");
    }

  clutter_actor_queue_relayout (CLUTTER_ACTOR (texture));
}

/**
 * mx_deform_texture_get_textures:
 * @texture: A #MxDeformTexture
 * @front: (out) (transfer none) (allow-none): The front-facing texture
 * @back: (out) (transfer none) (allow-none): The back-facing texture
 *
 * Retrieves the textures used by @texture.
 */
void
mx_deform_texture_get_textures (MxDeformTexture  *texture,
                                ClutterTexture  **front,
                                ClutterTexture  **back)
{
  MxDeformTexturePrivate *priv = texture->priv;

  if (front)
    *front = (ClutterTexture *)priv->front;
  if (back)
    *back = (ClutterTexture *)priv->back;
}

/**
 * mx_deform_texture_get_resolution:
 * @texture: A #MxDeformTexture
 * @tiles_x: (out) (allow-none): The horizontal resolution
 * @tiles_y: (out) (allow-none): The vertical resolution
 *
 * Retrieve the mesh resolution of the texture.
 * See mx_deform_texture_set_resolution().
 */
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

/**
 * mx_deform_texture_set_resolution:
 * @texture: A #MxDeformTexture
 * @tiles_x: The horizontal resolution
 * @tiles_y: The vertical resolution
 *
 * Sets the amount of sub-divisions used on each axis when generating
 * the mesh, where a value of 1 for each axis will produce a single quad.
 */
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

/**
 * mx_deform_texture_invalidate:
 * @texture: A #MxDeformTexture
 *
 * Make @texture re-calculate its vertices and redraw itself.
 */
void
mx_deform_texture_invalidate (MxDeformTexture *texture)
{
  MxDeformTexturePrivate *priv = texture->priv;
  priv->dirty = TRUE;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (texture));
}
