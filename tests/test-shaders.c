
#include <mx/mx.h>

static gchar *blur =
"uniform sampler2D tex;\n"
"uniform int radius;\n"
"uniform float x_step, y_step;\n"

"void main ()\n"
"  {\n"
"    float u, v;\n"
"    vec4 color = vec4 (0, 0, 0, 1);\n"
"    for (u = -radius; u <= radius; u++)\n"
"      for (v = -radius; v <= radius; v++)\n"
"        color += texture2D (tex, vec2 (gl_TexCoord[0].s + u * x_step,\n"
"                                       gl_TexCoord[0].t + v * y_step));\n"
"    color.rgba /= (radius + 1) * (radius + 1) * 4;\n"
"    gl_FragColor = color * gl_Color;\n"
"  }\n";

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
size_change_cb (ClutterActor *texture,
                gint          width,
                gint          height)
{
  clutter_actor_set_shader_param_float (texture,
                                        "x_step", 1.0f / next_p2 (width));
  clutter_actor_set_shader_param_float (texture,
                                        "y_step", 1.0f / next_p2 (height));
}

static void
value_cb (MxSlider     *slider,
             GParamSpec   *pspec,
             ClutterActor *actor)
{
  gdouble value = mx_slider_get_value (slider);
  if (value < 1.0/3.0)
    clutter_actor_set_shader_param_int (actor, "radius", 1);
  else if (value > 2.0/3.0)
    clutter_actor_set_shader_param_int (actor, "radius", 3);
  else
    clutter_actor_set_shader_param_int (actor, "radius", 2);
}

int
main (int argc, char **argv)
{
  MxWindow *window;
  gint width, height;
  ClutterShader *shader;
  ClutterActor *stage, *offscreen, *button, *texture, *box, *slider;

  clutter_init (&argc, &argv);

  window = mx_window_new ();
  stage = (ClutterActor *)mx_window_get_clutter_stage (window);
  clutter_actor_set_size (stage, 500, 250);

  button = mx_button_new_with_label ("Button");
  texture = clutter_texture_new_from_file ("redhand.png", NULL);
  clutter_texture_set_keep_aspect_ratio (CLUTTER_TEXTURE (texture), TRUE);
  offscreen = mx_offscreen_new ();
  mx_offscreen_set_pick_child (MX_OFFSCREEN (offscreen), TRUE);

  clutter_container_add_actor (CLUTTER_CONTAINER (offscreen), button);

  box = mx_box_layout_new ();
  mx_box_layout_add_actor_with_properties (MX_BOX_LAYOUT (box), offscreen, 0,
                                           "expand", TRUE,
                                           NULL);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (box), texture, 1);
  mx_window_set_child (window, box);

  /* Set Clutter shader */
  shader = clutter_shader_new ();
  clutter_shader_set_fragment_source (shader, blur, -1);
  clutter_shader_compile (shader, NULL);
  clutter_actor_set_shader (texture, shader);
  clutter_actor_set_shader (offscreen, shader);

  /* Set shader parameters */
  clutter_actor_set_shader_param_int (texture, "tex", 0);
  clutter_actor_set_shader_param_int (offscreen, "tex", 0);
  clutter_actor_set_shader_param_int (texture, "radius", 1);
  clutter_actor_set_shader_param_int (offscreen, "radius", 1);

  /* Hook onto texture size change for step parameters */
  g_signal_connect (texture, "size-change",
                    G_CALLBACK (size_change_cb), NULL);
  clutter_texture_get_base_size (CLUTTER_TEXTURE (texture), &width, &height);
  size_change_cb (texture, width, height);
  g_signal_connect (offscreen, "size-change",
                    G_CALLBACK (size_change_cb), NULL);
  clutter_texture_get_base_size (CLUTTER_TEXTURE (offscreen), &width, &height);
  size_change_cb (offscreen, width, height);

  /* Set up a slider to control the shader radius parameter */
  slider = mx_slider_new ();
  clutter_container_add_actor (
    CLUTTER_CONTAINER (mx_window_get_toolbar (window)), slider);
  mx_bin_set_fill (MX_BIN (mx_window_get_toolbar (window)), TRUE, FALSE);
  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (value_cb), texture);
  g_signal_connect (slider, "notify::value",
                    G_CALLBACK (value_cb), offscreen);

  g_signal_connect (window, "destroy",
                    G_CALLBACK (clutter_main_quit), NULL);

  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
