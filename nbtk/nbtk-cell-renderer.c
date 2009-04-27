/* nbtk-cell-renderer.c */

#include "nbtk-cell-renderer.h"

G_DEFINE_TYPE (NbtkCellRenderer,
               nbtk_cell_renderer,
               G_TYPE_INITIALLY_UNOWNED)


static ClutterActor*
nbtk_cell_renderer_real_get_actor (NbtkCellRenderer *renderer)
{
  g_warning ("The cell renderer of type '%s' does not implement the "
             "NbtkCellRenderer::get_actor() virtual function.",
             g_type_name (G_OBJECT_TYPE (renderer)));

  return NULL;
}

static void
nbtk_cell_renderer_class_init (NbtkCellRendererClass *klass)
{
  klass->get_actor = nbtk_cell_renderer_real_get_actor;
}

static void
nbtk_cell_renderer_init (NbtkCellRenderer *self)
{
}

ClutterActor *
nbtk_cell_renderer_get_actor (NbtkCellRenderer *renderer)
{
  NbtkCellRendererClass *class;

  g_return_val_if_fail (NBTK_IS_CELL_RENDERER (renderer), NULL);

  class = NBTK_CELL_RENDERER_GET_CLASS (renderer);

  return class->get_actor (renderer);
}
