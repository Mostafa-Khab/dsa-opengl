#version 450

in vec3 a_pos;
in vec4 a_col;
in vec2 a_uv;

out vec4 v_col;
out vec2 v_uv;

void main()
{
  gl_Position = vec4(a_pos, 1.0); 
  v_col = a_col;
  v_uv  = a_uv;
}
