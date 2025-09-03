#version 450

uniform mat4 u_mvp;
uniform vec2 u_resolution;

in vec3 a_pos;
in vec4 a_col;
in vec2 a_uv;

out vec4 v_col;
out vec2 v_uv;

void main()
{
  float aspect = u_resolution.x / u_resolution.y;
  vec3 pos = a_pos;
  pos.x /= aspect;

  gl_Position = u_mvp * vec4(pos, 1.0); 
  v_col = a_col;
  v_uv  = a_uv;
}
