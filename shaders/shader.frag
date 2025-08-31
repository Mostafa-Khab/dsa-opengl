#version 450

uniform sampler2D tex0;

in vec4 v_col;
in vec2 v_uv;

out vec4 fragColor;

void main()
{
  fragColor = v_col * texture(tex0, v_uv);
}
