
#include <mx/mx.h>

#ifdef COGL_HAS_GLES2

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

static gchar *blur =
  GLES2_VARS
  "uniform sampler2D tex;\n"
  "uniform float x_step, y_step, x_radius, y_radius;\n"

  "void\n"
  "main ()\n"
  "  {\n"
  "    float u, v, samples;\n"
  "    vec4 color = vec4 (0.0, 0.0, 0.0, 0.0);\n"

  "    samples = 0.0;\n"
  "    for (v = floor (-y_radius); v < ceil (y_radius); v++)\n"
  "      for (u = floor (-x_radius); u < ceil (x_radius); u++)\n"
  "        {\n"
  "          float s, t, mult;\n"

  "          mult = 1.0;\n"

  "          if (u < x_radius)\n"
  "            mult *= x_radius - u;\n"
  "          if (u > x_radius)\n"
  "            mult *= u - x_radius;\n"
  "          if (v < y_radius)\n"
  "            mult *= y_radius - v;\n"
  "          if (v > y_radius)\n"
  "            mult *= v - y_radius;\n"

  "          color += texture2D (tex, vec2("TEX_COORD".s + u * x_step,\n"
  "                                        "TEX_COORD".y + v * y_step)) *\n"
  "                   mult;\n"
  "          samples += mult;\n"
  "        }\n"
  "    color.rgba /= samples;\n"
  "    gl_FragColor = color * "COLOR_VAR";\n"
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
  clutter_actor_set_shader_param_float (actor, "x_radius", (value * 3.f) + 1.f);
  clutter_actor_set_shader_param_float (actor, "y_radius", (value * 3.f) + 1.f);
}

int
main (int argc, char **argv)
{
  MxWindow *window;
  gint width, height;
  ClutterShader *shader;
  ClutterActor *stage, *offscreen, *button, *texture, *box, *slider;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

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
  clutter_actor_set_shader_param_float (texture, "x_radius", 1.f);
  clutter_actor_set_shader_param_float (texture, "y_radius", 1.f);
  clutter_actor_set_shader_param_float (offscreen, "x_radius", 1.f);
  clutter_actor_set_shader_param_float (offscreen, "y_radius", 1.f);

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
