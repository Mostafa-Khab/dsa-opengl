#version 450

uniform vec2      u_resolution;
uniform float     u_time;

uniform sampler2D u_tex0;
uniform sampler2D u_tex1;

in vec4 v_col;
in vec2 v_uv;

out vec4 fragColor;

void main()
{
  vec2 uv  = gl_FragCoord.xy / u_resolution.xy;
  float d  = length(fract(uv));
  vec4 col = vec4(d * sin(u_time) + 1, (1.0 - d) * cos(1.25 * u_time) + 1, (1.0 - d) * sin(0.75 * u_time) + 1, 1.0);

  //fragColor = texture(u_tex1, v_uv) * col* v_col; //v_col
  fragColor = mix(texture(u_tex0, v_uv), texture(u_tex1, v_uv), 0.5);
}
