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

void main(void)
{
	vec2 pos = (UV * 2.0) - vec2(1.0);
	if (distance(vec2(0.0), pos) > 1) discard;

	float ang = atan(pos.y, pos.x); // <-pi,+pi>

	out_Color = vec4(1.0, 0.0, 1.0, 1.0);
}