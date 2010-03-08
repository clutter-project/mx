
#include <mx/mx.h>

static gchar *fade =
"uniform sampler2D tex;\n"

"void main ()\n"
"  {\n"
"    vec4 color = texture2D (tex, gl_TexCoord[0].st);\n"
"    color.rgb /= 1.5;\n"
"    gl_FragColor = color * gl_Color;\n"
"  }\n";

int
main (int argc, char **argv)
{
  int uniform;
  ClutterShader *cshader;
  CoglHandle program, shader;
  ClutterActor *stage, *offscreen, *button, *texture;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 500, 250);

  button = mx_button_new_with_label ("Button");
  texture = clutter_texture_new_from_file ("redhand.png", NULL);
  offscreen = mx_offscreen_new ();

  clutter_actor_set_size (button, 250, 250);
  clutter_actor_set_size (texture, 250, 250);
  clutter_actor_set_position (texture, 250, 0);

  clutter_container_add_actor (CLUTTER_CONTAINER (offscreen), button);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), offscreen);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), texture);

  /* Set cogl shader */
  shader = cogl_create_shader (COGL_SHADER_TYPE_FRAGMENT);
  cogl_shader_source (shader, fade);
  cogl_shader_compile (shader);

  program = cogl_create_program ();
  cogl_program_attach_shader (program, shader);
  cogl_program_link (program);

  uniform = cogl_program_get_uniform_location (program, "tex");
  cogl_program_use (program);
  cogl_program_uniform_1i (uniform, 0);
  cogl_program_use (COGL_INVALID_HANDLE);

  mx_offscreen_set_cogl_program (MX_OFFSCREEN (offscreen), program);
  cogl_handle_unref (program);

  /* Set Clutter shader */
  cshader = clutter_shader_new ();
  clutter_shader_set_fragment_source (cshader, fade, -1);
  clutter_shader_compile (cshader, NULL);
  clutter_actor_set_shader (texture, cshader);
  clutter_actor_set_shader_param_int (texture, "tex", 0);

  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
