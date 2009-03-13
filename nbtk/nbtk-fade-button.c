/* nbtk-fade-button.c */

#include "nbtk-fade-button.h"

G_DEFINE_TYPE (NbtkFadeButton, nbtk_fade_button, NBTK_TYPE_BUTTON)

static gboolean
nbtk_button_fade_transition (NbtkButton *button, ClutterActor *old_bg)
{
  const gchar *pseudo_class;
  NbtkButtonPrivate *priv = button->priv;

  pseudo_class = nbtk_stylable_get_pseudo_class (NBTK_STYLABLE (button));
  if (old_bg && g_strcmp0 ("active", pseudo_class))
    {
      gint duration = 0;
      g_object_get (button, "transition-duration", &duration, NULL);
      clutter_actor_animate (old_bg, CLUTTER_LINEAR,
                             duration,
                             "opacity", 0,
                             NULL);
    }
  else
    {
      /* remove the old image to perform instant transition when pressed */
      return TRUE;
    }
  return FALSE;
}

static void
nbtk_fade_button_class_init (NbtkFadeButtonClass *klass)
{
  NbtkButtonClass *button_class = NBTK_BUTTON_CLASS (klass);

  button_class->transition = nbtk_button_fade_transition;
}

static void
nbtk_fade_button_init (NbtkFadeButton *self)
{
}

NbtkWidget*
nbtk_fade_button_new (void)
{
  return g_object_new (NBTK_TYPE_FADE_BUTTON, NULL);
}

NbtkWidget*
nbtk_fade_button_new_with_label (const gchar *text)
{
  return g_object_new (NBTK_TYPE_FADE_BUTTON,
                       "label", text,
                       NULL);
}
