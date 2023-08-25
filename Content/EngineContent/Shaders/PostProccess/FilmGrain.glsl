#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;

void main(void)
{
	gl_Position = vec4(a_Position, 1.0);
	UV = a_TexCoord;
}

#type fragment
#version 330 core

in vec2 UV;
out vec4 out_Color;

uniform float u_Seed;
uniform float u_Jitter;
uniform float u_Intensity;
uniform sampler2D u_Texture;

void main(void)
{
  float x = (gl_FragCoord.x + 4) * (gl_FragCoord.y + 4) * (sin(13.0) * 10) + 14; // Seed;
  vec4 grain = u_Intensity * vec4(mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005);

  // Calculate noise and sample texture
  float mdf = 0.1 * u_Intensity; // increase for noise amount 
  float noise = (fract(sin(dot(UV, vec2(12.9898,78.233)*2.0)) * 43758.5453)) * ((sin((u_Seed * 10.0 * u_Jitter) + cos(UV.x * 3331) * tan(UV.y + 5437) * 3323789) + 1.0) / 2.0);
  vec4 tex = texture(u_Texture, UV);
    
  //mdf *= sin(u_Seed) + 1.0; // animate the effect's strength
    
  out_Color = tex - noise * mdf;

  //out_Color = texture(u_Texture, UV) * vec4(grain.xyz, 1.0);
}