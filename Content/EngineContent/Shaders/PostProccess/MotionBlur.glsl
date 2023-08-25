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
uniform sampler2D u_Texture;
uniform float u_Intensity;

void main(void)
{
	out_Color = vec4(texture(u_Texture, UV).xyz, u_Intensity);
}